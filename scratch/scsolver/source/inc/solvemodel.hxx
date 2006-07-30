/*************************************************************************
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Kohei Yoshida.
 *    1039 Kingsway Dr., Apex, NC 27502, USA
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

 
#ifndef _SOLVEMODEL_HXX_
#define _SOLVEMODEL_HXX_

#include <memory>

namespace scsolver {

namespace numeric {
	class Matrix;
}

class SolverImpl;
class SolveModelImpl;

/**
 * This class oversees the actual process of solving a model,
 * i.e. constructing a model from a sheet, passing it to the
 * backend model container, executing the algorithm to get a
 * solution, and (if successful) put the solution back into the
 * sheet.  This class should be the only class exposed to class
 * SolverImpl wrt solving action.
 */
class SolveModel
{
public:
	SolveModel( SolverImpl* );
	~SolveModel() throw();
	
	void solve();
	bool isSolved() const;
	
private:
	std::auto_ptr<SolveModelImpl> m_pImpl;	
};


}


#endif
