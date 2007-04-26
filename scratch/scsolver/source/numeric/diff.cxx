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

//---------------------------------------------------------------------------
// Implementation

/** 
    Algorithm derived from Chapter 4.2 (p.93) of
 
    "A First Course In Numerical Analysis 2nd ed. by Anthony
    Ralston and Philip Rabinowitz"
 */
class DifferentiateImpl
{
	static double OMEGA;

public:
	DifferentiateImpl();
	~DifferentiateImpl() throw();

	void setPrecision( unsigned long );
	void setSecondOrder( bool );
	void setVariables( const vector<double>& );
	void setVarIndex( unsigned long );

    void setFuncObject(BaseFuncObj* p)
    {
    	m_pFuncObj = p;
    	setDirty();
    }

	double run();

private:
	unsigned long m_nPrec;
	unsigned long m_nVarIndex;
	bool m_bSecondOrder;
    BaseFuncObj* m_pFuncObj;

	vector<double> m_cnX;
	vector<double> m_cnH;
	vector< vector<double> > m_cnT;

	void initialize();
	void setDirty();
	void appendNewH();
	void setT( unsigned long, unsigned long, double );
	double getT( unsigned long, unsigned long );
	double T0( unsigned long );
	double Tm();
	double Tm( unsigned long, unsigned long = 0 );
};

double DifferentiateImpl::OMEGA = 2.0;

DifferentiateImpl::DifferentiateImpl() : 
	m_nPrec( 2 ), 
	m_nVarIndex( 0 ), 
	m_bSecondOrder( false ),
    m_pFuncObj(NULL)
{
}

DifferentiateImpl::~DifferentiateImpl() throw()
{
}

void DifferentiateImpl::setPrecision( unsigned long n )
{
	m_nPrec = n;
}

void DifferentiateImpl::setSecondOrder( bool b )
{
	m_bSecondOrder = b;
	setDirty();
}

void DifferentiateImpl::setVariables( const vector<double>& cnX )
{
	m_cnX = cnX;
	setDirty();
}

void DifferentiateImpl::setVarIndex( unsigned long n )
{
	if ( m_nVarIndex != n )
	{
		m_nVarIndex = n;
		setDirty();
	}
}

void DifferentiateImpl::initialize()
{
	const double fInitH = 0.0512;
	m_cnH.clear();
	m_cnH.push_back( fInitH );
	m_cnH.push_back( fInitH / 3.0 * 2.0 );
}

void DifferentiateImpl::setDirty()
{
	m_cnT.clear();
}

void DifferentiateImpl::appendNewH()
{
	m_cnH.push_back( m_cnH.at( m_cnH.size() - 2 ) / 2.0 );
}

void DifferentiateImpl::setT( unsigned long m, unsigned long i, double fVal )
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

double DifferentiateImpl::getT( unsigned long m, unsigned long i )
{
	if ( m_cnT.empty() || m_cnT.size() - 1 < m )
		throw std::out_of_range( "" );

	vector<double> cnRow = m_cnT.at( m );
	if ( cnRow.empty() || cnRow.size() - 1 < i )
		throw std::out_of_range( "" );

	return cnRow.at( i );
}

double DifferentiateImpl::T0( unsigned long i )
{
	vector<double> cnX( m_cnX );

	double fXOrig = cnX.at( m_nVarIndex );
	double fVal = 0.0;
	double fH = m_cnH.at( i );

	if ( m_bSecondOrder )
	{
		cnX.at( m_nVarIndex ) = fXOrig + fH;
        m_pFuncObj->setVars(cnX);
        fVal = m_pFuncObj->eval();
		cnX.at( m_nVarIndex ) = fXOrig;
        m_pFuncObj->setVars(cnX);
		fVal -= 2.0*m_pFuncObj->eval();
		cnX.at( m_nVarIndex ) = fXOrig - fH;
        m_pFuncObj->setVars(cnX);
		fVal += m_pFuncObj->eval();
		fVal /= fH*fH;
	}
	else
	{
		cnX.at( m_nVarIndex ) = fXOrig + fH;
        m_pFuncObj->setVars(cnX);
		fVal = m_pFuncObj->eval();
		cnX.at( m_nVarIndex ) = fXOrig - fH;
        m_pFuncObj->setVars(cnX);
		fVal -= m_pFuncObj->eval();
		fVal /= 2.0*fH;
	}
	setT( 0, i, fVal );

	return fVal;
}

double DifferentiateImpl::Tm()
{
	unsigned long m = m_cnH.size() - 1;
	return Tm( m );
}

double DifferentiateImpl::Tm( unsigned long m, unsigned long i )
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
	double fVal = fT1 + ( fT1 - fT2 ) / ( pow( m_cnH.at( i )/m_cnH.at( i+m ), DifferentiateImpl::OMEGA ) - 1 );
	setT( m, i, fVal );

	return fVal;
}

double DifferentiateImpl::run()
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

//---------------------------------------------------------------------------
// Public Interface

Differentiate::Differentiate() : m_pImpl( new DifferentiateImpl )
{
}

Differentiate::~Differentiate() throw()
{
}

void Differentiate::setPrecision( unsigned long n )
{
	m_pImpl->setPrecision( n );
}

void Differentiate::setSecondOrder( bool b )
{
	m_pImpl->setSecondOrder( b );
}

void Differentiate::setVariables( const vector<double>& cnX )
{
	m_pImpl->setVariables( cnX );
}

/** Set an variable index with respect of which to differentiate.  For example,
	an index of 0 will differentiate with respect to the first variable, and an
	index of 1 will with respect to the second, and so on.

	@param nIndex variable index
 */
void Differentiate::setVarIndex( unsigned long nIndex )
{
	m_pImpl->setVarIndex( nIndex );
}

void Differentiate::setFuncObject(BaseFuncObj* p)
{
	m_pImpl->setFuncObject( p );
}

double Differentiate::run()
{
	return m_pImpl->run();
}

}}

