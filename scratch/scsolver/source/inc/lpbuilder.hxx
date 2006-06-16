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


#ifndef _DATA_HXX_
#define _DATA_HXX_

#include "type.hxx"
#include <memory>
#include <vector>
#include <com/sun/star/table/CellAddress.hpp>

using namespace ::com::sun::star;


namespace scsolver {

namespace numeric { namespace opres { namespace lp {
	class Model;
}}}

class LpModelBuilderImpl;


/** This is a container class that holds the 3D addresses of cells that 
	respectively contain the left and right hand side of the constraint 
	formula and its equality. */
class ConstraintAddress
{
public:
	ConstraintAddress();
	ConstraintAddress( const ConstraintAddress& );
	~ConstraintAddress() throw();

	bool equals( const ConstraintAddress& ) const;
	bool operator==( const ConstraintAddress & ) const;

	table::CellAddress getLeftCellAddr() const;
	void setLeftCellAddr( const table::CellAddress& addr );

	table::CellAddress getRightCellAddr() const;
	void setRightCellAddr( const table::CellAddress& addr );
	double getRightCellValue() const;
	void setRightCellValue( double value );
	bool isRightCellNumeric() const;

	numeric::opres::Equality getEquality() const;
	void setEquality( numeric::opres::Equality eq );

private:

	table::CellAddress Left;
	table::CellAddress Right;
	numeric::opres::Equality Equal;

	bool m_bIsRHSNumber;
	double m_fRHSValue;
};

/** This class is responsible for transforming a user-defined model on 
	a spreadsheet into a standard linear model.  The model thereby created
	is to be passed onto one of the actual algorithm classes for solving it. */
class LpModelBuilder
{
public:
	LpModelBuilder();
	~LpModelBuilder();

	numeric::opres::lp::Model getModel();

	// Optimization objective
	numeric::opres::Goal getGoal() const;
	void setGoal( numeric::opres::Goal );

	// Objective Formula
	const table::CellAddress getObjectiveFormulaAddress() const;
	void setObjectiveFormulaAddress( const table::CellAddress& );
	
	// Decision Variables
	void setDecisionVarAddress( const table::CellAddress& );
	std::vector< table::CellAddress > getAllDecisionVarAddresses() const;
	void clearDecisionVarAddresses();
	
	// Cost Vector
	double getCostVector( const table::CellAddress& );
	void setCostVector( const table::CellAddress&, double );

	// Constraints
	void clearConstraintAddresses();
	void setConstraintAddress( const ConstraintAddress& );
	std::vector< ConstraintAddress > getAllConstraintAddresses() const;
	void setConstraintMatrixSize( size_t, size_t );
	void setConstraintCoefficient( const table::CellAddress&, const ConstraintAddress&, double, double );
	
	// Temporary formula strings
	const rtl::OUString getTempCellFormula( const table::CellAddress& ) const;
	void setTempCellFormula( const table::CellAddress&, const rtl::OUString& );
	
private:

	std::auto_ptr<LpModelBuilderImpl> m_pImpl;
};



}

#endif
