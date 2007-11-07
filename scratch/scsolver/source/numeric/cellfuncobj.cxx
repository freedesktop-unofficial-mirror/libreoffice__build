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

#include "numeric/cellfuncobj.hxx"
#include "xcalc.hxx"
#include "unoglobal.hxx"
#include "com/sun/star/table/CellAddress.hpp"
#include <vector>
#include <sstream>
#include <iostream>
#include <stdio.h>

using com::sun::star::table::CellAddress;
using ::std::vector;
using ::std::string;
using ::std::ostringstream;

namespace scsolver { namespace numeric {

struct CellFuncObjImpl
{
	CalcInterface* pCalc;
	CellAddress TargetCell;
	vector<CellAddress> DecVarCells;
};

//-----------------------------------------------------------------

/*
CellFuncObj::CellFuncObj()
{
	// disabled
}
*/

CellFuncObj::CellFuncObj( CalcInterface* pCalc ) :
	m_pImpl( new CellFuncObjImpl )
{
	m_pImpl->pCalc = pCalc;
}

CellFuncObj::~CellFuncObj() throw()
{
}

const vector<double>& CellFuncObj::getVars() const
{
    // Does absolutely nothing....
    vector<double> fake;
    return fake;
}

void CellFuncObj::setVars(const vector<double>& vars)
{
	vector<CellAddress>::const_iterator itr, 
		itrBeg = m_pImpl->DecVarCells.begin(), 
		itrEnd = m_pImpl->DecVarCells.end();

    vector<double>::const_iterator itrVar,
        itrVarBeg = vars.begin(), itrVarEnd = vars.end();

    for (itr = itrBeg, itrVar = itrVarBeg; itr != itrEnd && itrVar != itrVarEnd; ++itr, ++itrVar)
        m_pImpl->pCalc->setCellValue(*itr, *itrVar);
}

void CellFuncObj::setVar(size_t index, double var)
{
    if ( index >= m_pImpl->DecVarCells.size() )
        return;

    m_pImpl->pCalc->setCellValue( m_pImpl->DecVarCells.at(index), var);
}

double CellFuncObj::eval()
{
	return m_pImpl->pCalc->getCellValue( m_pImpl->TargetCell );
}

const std::string CellFuncObj::getFuncString() const
{
	ostringstream os;
	int sheet = m_pImpl->TargetCell.Sheet;
	int column = m_pImpl->TargetCell.Column;
	int row = m_pImpl->TargetCell.Row;
	os << "(" << sheet << ", " << column << ", " << row << ")";

	return os.str();
}

void CellFuncObj::setTargetCell( const CellAddress& addr )
{
	m_pImpl->TargetCell = addr;
}

void CellFuncObj::appendDecVarCell( const CellAddress& addr )
{
	m_pImpl->DecVarCells.push_back(addr);
}

}}

