/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONFIGMGR_DBBE_UTIL_GRAPH_HXX_

#include "graph.hxx"
#include <vector>
#include <algorithm>

namespace configmgr 
{
    namespace dbbe
    {
        graph::graph(Db& aDatabase)
        {  
            using namespace std;
            
            Dbc *cursorp= NULL;
            OSL_VERIFY(!aDatabase.cursor(NULL, &cursorp, 0));
            OSL_ASSERT(cursorp);
            Dbt key, data;
            
            int ret= 1;
            bool done= false;
            while (!done)
            {
                ret= cursorp->get(&key, &data, DB_NEXT);

                switch(ret)
                {
                    case DB_NOTFOUND:
                        done= true;
                        break;
                        
                    case 0:
                    {
                        OSL_ASSERT(key.get_data());
                        OSL_ASSERT(data.get_data());
                        
                        Record* pRecord= Record::getFromDbt(data);
                        pRecord->unMarshal();
                        int swap= 0;
                        OSL_ASSERT(!aDatabase.get_byteswapped(&swap));
                        if (swap)
                            pRecord->bytesex();
                        
                        //stupid question:
                        //  do I need to specify Dbt's to use malloc? I don't want my pointers
                        //  to be corrupted
                        NodeHash[static_cast<sal_Char*>(key.get_data())]= *(pRecord);
                        nodes.insert(static_cast<sal_Char*>(key.get_data()));
                        vector<sal_Char*> sublayers= pRecord->listSubLayers();
                        for (vector<sal_Char*>::iterator it= sublayers.begin();
                             it != sublayers.end();
                             it++)
                        {
                            children.insert(*it);
                            nodes.insert(*it);
                        }        
                    }
                    break;
                    
                    default:
                        abort();
                        break;
                }
            }
                vector<sal_Char*> orphKeys; 

                set_difference(nodes.begin(), nodes.end(),
                               children.begin(), children.end(),
                               orphKeys.begin());
                               
                for (vector<sal_Char*>::iterator v = orphKeys.begin(); 
                     v != orphKeys.end(); 
                     v++) 
                {
                    orphans.insert(*v);
                }
        }
        

        void graph::descend(void callback(sal_Char *, const Record&, void*), 
                            nodeHash::iterator it,
                            void *state)
        {
            using namespace std;

            callback(const_cast<sal_Char*>(it->first), it->second, state);
            if (it->second.numSubLayers)
            {
                vector<sal_Char*> sublayers= it->second.listSubLayers();
                for (vector<sal_Char*>::iterator sub= sublayers.begin();
                     sub != sublayers.end();
                     sub++)
                {
                    nodeHash::iterator child= NodeHash.find(*sub);
                    descend(callback, child, state);
                }
            }
        }
        
        void graph::traverse(void callback(sal_Char *, const Record&, void*),
                             void* state)
        {
            using namespace std;

            for (keySet::iterator orphan= orphans.begin();
                 orphan != orphans.end();
                 orphan++)
            {
                traverse(callback, orphan, state);
            }
        }
        
        void graph::traverse(void callback(sal_Char *, const Record&, void*),
                             keySet::iterator it,
                             void* state)
        {
            nodeHash::iterator it2= NodeHash.find(*it);
            descend(callback, it2, state);
        }

        void graph::graphviz(std::ostream &out)
        {
            using namespace std;

            out << "digraph G " << endl << "{" << endl;
            for (nodeHash::iterator it= NodeHash.begin();
                 it != NodeHash.end();
                 it++)
            {
                out << "\t\"" << it->first << "\";" << endl;
                vector<sal_Char*> children= it->second.listSubLayers();
                for (vector<sal_Char*>::iterator child= children.begin();
                     child != children.end();
                     child++)
                {
                    out << "\t\"" << it->first << "\" -> \"" << *child << "\";" << endl;
                }
            }
            out << "}" << endl;
        }
        
    }; //dbbe
}; //configmgr

#endif //CONFIGMGR_DBBE_UTIL_GRAPH_HXX_
