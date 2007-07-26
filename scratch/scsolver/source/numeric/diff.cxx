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

#include "numeric/diff.hxx"
#include "numeric/funcobj.hxx"
#include "tool/global.hxx"
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace std;

namespace scsolver { namespace numeric {

const double NumericalDiffer::OMEGA = 2.0;

NumericalDiffer::NumericalDiffer() : 
	m_nPrec( 2 ), 
	m_nVarIndex( 0 ), 
	m_bSecondOrder( false ),
    m_pFuncObj(NULL)
{
}

NumericalDiffer::~NumericalDiffer() throw()
{
}

void NumericalDiffer::setPrecision( unsigned long n )
{
	m_nPrec = n;
}

void NumericalDiffer::setSecondOrder( bool b )
{
	m_bSecondOrder = b;
	setDirty();
}

void NumericalDiffer::setVariables( const vector<double>& cnX )
{
	m_cnX = cnX;
	setDirty();
}

void NumericalDiffer::setVarIndex( unsigned long n )
{
	if ( m_nVarIndex != n )
	{
		m_nVarIndex = n;
		setDirty();
	}
}

void NumericalDiffer::setFuncObject(BaseFuncObj* p)
{
    m_pFuncObj = p;
    setDirty();
}

void NumericalDiffer::initialize()
{
	const double fInitH = 0.0512;
	m_cnH.clear();
	m_cnH.push_back( fInitH );
	m_cnH.push_back( fInitH / 3.0 * 2.0 );
}

void NumericalDiffer::setDirty()
{
	m_cnT.clear();
}

void NumericalDiffer::appendNewH()
{
	m_cnH.push_back( m_cnH.at( m_cnH.size() - 2 ) / 2.0 );
}

void NumericalDiffer::setT( unsigned long m, unsigned long i, double fVal )
{
	size_t nTSize = m_cnT.size();
	if ( nTSize < m + 1 )
		for ( unsigned long nIdx = 0; nIdx < m + 1 - nTSize; ++nIdx )
		{
			vector<double> cn;
			m_cnT.push_back( cn );
		}

	vector<double>& cnRow = m_cnT.at( m );
	size_t nRowSize = cnRow.size();
	if ( nRowSize < i + 1 )
		for ( unsigned long nIdx = 0; nIdx < i + 1 - nRowSize; ++nIdx )
			cnRow.push_back( 0.0 );

	cnRow.at( i ) = fVal;
}

double NumericalDiffer::getT( unsigned long m, unsigned long i )
{
	if ( m_cnT.empty() || m_cnT.size() - 1 < m )
		throw std::out_of_range( "" );

	vector<double> cnRow = m_cnT.at( m );
	if ( cnRow.empty() || cnRow.size() - 1 < i )
		throw std::out_of_range( "" );

	return cnRow.at( i );
}

double NumericalDiffer::T0( unsigned long i )
{
	double fXOrig = m_cnX.at(m_nVarIndex);
	double fVal = 0.0;
	double fH = m_cnH.at(i);
    m_pFuncObj->setVars(m_cnX);

	if (m_bSecondOrder)
	{
        m_pFuncObj->setVar(m_nVarIndex, fXOrig + fH);
        fVal = m_pFuncObj->eval();
        m_pFuncObj->setVar(m_nVarIndex, fXOrig);
		fVal -= 2.0*m_pFuncObj->eval();
        m_pFuncObj->setVar(m_nVarIndex, fXOrig - fH);
		fVal += m_pFuncObj->eval();
		fVal /= fH*fH;
	}
	else
	{
        m_pFuncObj->setVar(m_nVarIndex, fXOrig + fH);
		fVal = m_pFuncObj->eval();
        m_pFuncObj->setVar(m_nVarIndex, fXOrig - fH);
		fVal -= m_pFuncObj->eval();
		fVal /= 2.0*fH;
	}

	setT(0, i, fVal);
	return fVal;
}

double NumericalDiffer::Tm()
{
	unsigned long m = m_cnH.size() - 1;
	return Tm( m );
}

double NumericalDiffer::Tm( unsigned long m, unsigned long i )
{
	if ( m_cnH.empty() )
		throw exception();

	try
	{
		return getT( m, i );
	}
	catch( const std::out_of_range& )
	{
	}

	if ( m == 0 )
		return T0( i );

	double fT1 = Tm( m-1, i+1 );
	double fT2 = Tm( m-1, i );
	double fVal = fT1 + ( fT1 - fT2 ) / ( pow( m_cnH.at( i )/m_cnH.at( i+m ), NumericalDiffer::OMEGA ) - 1 );
	setT( m, i, fVal );

	return fVal;
}

double NumericalDiffer::run()
{
	if ( m_pFuncObj == NULL )
		throw FuncObjectNotSet();

	cout << m_pFuncObj->getFuncString() << endl;
	initialize();
	double fVal = Tm();
	double fOldVal = fVal;

	double fTor = 1.0;	
	for ( unsigned long n = 0; n < m_nPrec; ++n )
		fTor /= 10.0;
	while ( true )
	{
		appendNewH();
		fVal = Tm();
		double fDiff = fabs( fVal - fOldVal );
		if ( fDiff <= fTor )
			return fVal;
		fOldVal = fVal;
	}
	
	throw std::exception();

	return 0.0;
}

}}

