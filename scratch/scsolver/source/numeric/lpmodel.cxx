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


#include "numeric/lpmodel.hxx"
#include "numeric/lpbase.hxx"
#include "numeric/matrix.hxx"
#include "tool/global.hxx"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <boost/numeric/ublas/matrix.hpp>

using ::std::vector;
using ::std::string;
using ::std::map;
using ::std::cout;
using ::std::endl;
using ::std::ostringstream;
using ::scsolver::numeric::Matrix;

namespace scsolver { namespace numeric { namespace lp {

class NonBoundingException : public ::std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "Non bounding exception";
	}
};

struct AttrBound
{
	bool Enabled;
	double Value;
};

struct VarBounds
{
	AttrBound Upper;
	AttrBound Lower;
};

typedef map<size_t,VarBounds> VarBoundMap;

class ModelImpl
{
public:
	ModelImpl();
	ModelImpl( const ModelImpl& );
	~ModelImpl() throw();
	void swap( ModelImpl& ) throw();
	void print() const;

	size_t getDecisionVarSize() const { return m_mxCost.cols(); }
	size_t getConstraintCount() const { return m_mxConstraint.rows(); }

	double getCost( size_t rowid ) const { return m_mxCost( 0, rowid ); }
	Matrix getCostVector() const { return m_mxCost; }
	void setCostVectorElement( size_t, double );
	void setCostVector( const std::vector<double>& );
	void deleteCostVectorElements( const std::vector<size_t>& );

	AttrBound getVarBoundAttribute( size_t, Bound ) const;
	double    getVarBound( size_t, Bound ) const;
	void      setVarBound( size_t, Bound, double );
	bool  isVarBounded( size_t, Bound ) const;
	void deleteVarRanges( const std::vector<size_t>& );

	void deleteVariables( const std::vector<size_t>& );

	Goal getGoal() const { return m_eGoal; }
	void setGoal( Goal e ) { m_eGoal = e; }
	
	double getObjectiveFuncConstant() const { return m_fObjFuncConstant; }
	void setObjectiveFuncConstant( double f ) { m_fObjFuncConstant = f; }

	unsigned long getPrecision() const { return m_nPrecision; }
	void       setPrecision( unsigned long n ) { m_nPrecision = n; }

	bool getVarPositive() const { return m_bVarPositive; }
	void setVarPositive( bool b ) { m_bVarPositive = b; }

	bool getVarInteger() const
	{
		return m_bVarInteger;
	}

	void setVarInteger( bool b )
	{
		m_bVarInteger =  b;
	}

	bool getVerbose() const { return m_bVerbose; }
	void setVerbose( bool b ) { m_bVerbose = b; }

	double getConstraint( size_t, size_t ) const;
	Matrix getConstraintMatrix() const { return m_mxConstraint; }
	Matrix getRhsVector() const { return m_mxRHS; }
	double getRhsValue( size_t ) const;
	void setRhsValue( size_t, double );
	vector<Equality> getEqualityVector() const { return m_eEqualities; }
	Equality getEqualityByRowId( size_t i ) const { return m_eEqualities.at( i ); }
	void addConstraint( const std::vector<double>&, Equality, double );
	void setStandardConstraintMatrix( const Matrix&, const Matrix& );
	void deleteConstraintMatrixColumns( const std::vector<size_t>& );
	
private:
	Matrix m_mxCost;	// row vector
	Matrix m_mxConstraint;
	Matrix m_mxRHS;		// column vector

	vector<VarBounds> m_cnVarRanges;
	vector<Equality> m_eEqualities;

	Goal m_eGoal;
	unsigned long m_nPrecision;
	bool m_bVarPositive;
	bool m_bVarInteger;

	bool m_bVerbose;
	double m_fObjFuncConstant;
};


ModelImpl::ModelImpl() : 
	m_mxCost( 0, 0 ), m_mxConstraint( 0, 0 ), m_mxRHS( 0, 0 ),
	m_nPrecision( 2 ), m_bVarPositive( true ), m_bVerbose( false ),
	m_fObjFuncConstant( 0.0 )
{
}

ModelImpl::ModelImpl( const ModelImpl& other ) :
	m_mxCost( other.m_mxCost ), 
	m_mxConstraint( other.m_mxConstraint ), 
	m_mxRHS( other.m_mxRHS ),
	m_cnVarRanges( other.m_cnVarRanges ),
	m_eEqualities( other.m_eEqualities ), 
	m_eGoal( other.m_eGoal ), 
	m_nPrecision( other.m_nPrecision ), 
	m_bVarPositive( other.m_bVarPositive ), 
	m_bVerbose( other.m_bVerbose )
{
}

ModelImpl::~ModelImpl() throw()
{
}

void ModelImpl::swap( ModelImpl& other ) throw()
{
	m_mxCost.swap( other.m_mxCost );
	m_mxConstraint.swap( other.m_mxConstraint );
	m_mxRHS.swap( other.m_mxRHS );
	std::swap( m_cnVarRanges, other.m_cnVarRanges );
	std::swap( m_eEqualities, other.m_eEqualities );
	std::swap( m_eGoal, other.m_eGoal );
	std::swap( m_nPrecision, other.m_nPrecision );
	std::swap( m_bVarPositive, other.m_bVarPositive );
	std::swap( m_bVarInteger, other.m_bVarInteger );
	std::swap( m_bVerbose, other.m_bVerbose );
}

void ModelImpl::setCostVectorElement( size_t nId, double fVal )
{
	// Cost vector must be a row vector because the X vector is a column.
	m_mxCost( 0, nId ) = fVal;
}

void ModelImpl::setCostVector( const std::vector<double>& cn )
{
	std::vector<double>::const_iterator it, 
		itBeg = cn.begin(), itEnd = cn.end();
	for ( it = itBeg; it != itEnd; ++it )
	{
		size_t nDist = std::distance( itBeg, it );
		setCostVectorElement( nDist, *it );
	}
}

void ModelImpl::deleteCostVectorElements( const std::vector<size_t>& cn )
{
	m_mxCost.deleteColumns( cn );
}

AttrBound ModelImpl::getVarBoundAttribute( size_t i, Bound e ) const
{
	AttrBound ab;
	if ( i >= m_cnVarRanges.size() )
	{
		// requested variable index is out of bound.
		ab.Enabled = false;
		ab.Value = 0.0;
	}
	else
	{
		VarBounds vb = m_cnVarRanges[i];
		switch ( e )
		{
		case BOUND_LOWER:
			ab = vb.Lower;
			break;
		case BOUND_UPPER:
			ab = vb.Upper;
			break;
		default:
			OSL_ASSERT( !"unknown boundary" );
		}
	}

	return ab;
}

double ModelImpl::getVarBound( size_t i, Bound e ) const
{
	AttrBound ab = getVarBoundAttribute( i, e );
	if ( ab.Enabled )
		return ab.Value;
	else
		throw NonBoundingException();
}

void ModelImpl::setVarBound( size_t nVarId, Bound e, double fBound )
{
	size_t nSize = m_cnVarRanges.size();
	cout << "size is " << nSize << " and request is " << nVarId << endl;
	if ( nVarId >= nSize )
	{
		// Fill the container as necessary
		for ( size_t i = 0; i < nVarId + 1 - nSize; ++i )
		{
			cout << "inserting new VarBounds at " << i << endl;
			VarBounds vb;
			vb.Lower.Enabled = false;
			vb.Lower.Value = 0.0;
			vb.Upper.Enabled = false;
			vb.Upper.Value = 0.0;
			m_cnVarRanges.push_back( vb );
		}
	}

	switch ( e )
	{
	case BOUND_LOWER:

		m_cnVarRanges[nVarId].Lower.Enabled = true;
		m_cnVarRanges[nVarId].Lower.Value = fBound;
		break;
	case BOUND_UPPER:
		m_cnVarRanges[nVarId].Upper.Enabled = true;
		m_cnVarRanges[nVarId].Upper.Value = fBound;
		break;
	default:
		OSL_ASSERT( !"unknown boundary" );
	}
}

bool ModelImpl::isVarBounded( size_t i, Bound e ) const
{
	AttrBound ab = getVarBoundAttribute( i, e );
	return ab.Enabled;
}

void ModelImpl::deleteVarRanges( const std::vector<size_t>& cnVarIds )
{
	vector<size_t> Ids( cnVarIds );
	::std::sort( Ids.begin(), Ids.end() );
    vector<VarBounds>::iterator itr, 
		itrBeg = m_cnVarRanges.begin(), itrEnd = m_cnVarRanges.end();

	vector<VarBounds> cnVarRanges( m_cnVarRanges.size() );
	for ( itr = itrBeg; itr != itrEnd; ++itr )
	{
		size_t nCurId = distance( itrBeg, itr );
		if ( find( cnVarIds.begin(), cnVarIds.end(), nCurId ) == cnVarIds.end() )
			// ID not in delete list
			cnVarRanges.push_back( *itr );
	}
	m_cnVarRanges.swap( cnVarRanges );
}

void ModelImpl::deleteVariables( const std::vector<size_t>& cnVarIds )
{
	Debug( "deleteVariables" );
	deleteCostVectorElements( cnVarIds );
	deleteConstraintMatrixColumns( cnVarIds );
	deleteVarRanges( cnVarIds );
}

double ModelImpl::getConstraint( size_t nRow, size_t nCol ) const
{
	return m_mxConstraint( nRow, nCol );
}

double ModelImpl::getRhsValue( size_t nId ) const
{
	return m_mxRHS( nId, 0 );
}

void ModelImpl::setRhsValue( size_t nId, double fVal )
{
	m_mxRHS( nId, 0 ) = fVal;
}

void ModelImpl::addConstraint( const std::vector<double>& aConst, Equality eEqual, double fRHS )
{
	size_t nRowId = m_mxConstraint.rows();
	vector<double>::const_iterator pos;
	for ( pos = aConst.begin(); pos != aConst.end(); ++pos )
	{
		size_t nColId = distance( aConst.begin(), pos );
		m_mxConstraint( nRowId, nColId ) = *pos;
	}
	m_eEqualities.push_back( eEqual );
	
	// RHS vector must be a column.
	m_mxRHS( m_mxRHS.rows(), 0 ) = fRHS;
}

void ModelImpl::setStandardConstraintMatrix( const Matrix& A, const Matrix& B )
{
	if ( A.rows() != B.rows() )
		throw MatrixSizeMismatch();
	
	for ( size_t i = 0; i < A.rows(); ++i )
	{
		vector<double> aConst;
		for ( size_t j = 0; j < A.cols(); ++j )
			aConst.push_back( A( i, j ) );
		addConstraint( aConst, EQUAL, B( i, 0 ) );
	}
}

void ModelImpl::deleteConstraintMatrixColumns( const std::vector<size_t>& cn )
{
	m_mxConstraint.deleteColumns( cn );
}

void ModelImpl::print() const
{
	using namespace ::boost::numeric::ublas;
	size_t nColSpace = 2;
	string sX = "x_";
	cout << endl << repeatString( "-", 70 ) << endl;
	string sGoal;
	if ( m_eGoal == GOAL_MAXIMIZE )
		sGoal = "max";
	else if ( m_eGoal == GOAL_MINIMIZE )
		sGoal = "min";
	else
		//OSL_ASSERT( !"wrong goal" );
		sGoal = "unknown";

	ostringstream osLine;
	osLine << "Objective: ";
	bool bFirst = true;
	for ( size_t j = 0; j < m_mxCost.cols(); ++j )
	{
		double fVal = m_mxCost( 0, j );
		if ( fVal != 0.0 )
		{
			if ( bFirst )
			{
				bFirst = false;
				osLine << fVal << sX << j;
			}
			else
			{
				if ( fVal > 0.0 )
					osLine << " + " << fVal << sX << j;
				else
					osLine << " - " << abs(fVal) << sX << j;
			}
		}
	}
	osLine << "  (" << sGoal << ")";
	cout << osLine.str() << endl << repeatString( "-", 70 ) << endl;
	cout << "Subject to Constraints:" << endl << endl;
	matrix< string > mElements = m_mxConstraint.getDisplayElements( 0, nColSpace, true );
	matrix< string > mRHS = m_mxRHS.getDisplayElements( 0, nColSpace, false );
	
	// Print constraints
	for ( size_t i = 0; i < m_mxConstraint.rows(); ++i )
	{
		osLine.str( "" );
		for ( size_t j = 0; j < m_mxConstraint.cols(); ++j )
		{
			string s = mElements( i, j );
			double f = m_mxConstraint( i, j );
			ostringstream osVar;
			osVar << s << sX << j;
			if ( f == 0.0 )
				osLine << repeatString( " ", osVar.str().length() );
			else
				osLine << osVar.str();
		}
		
		osLine << repeatString( " ", nColSpace );
		
		switch ( m_eEqualities.at( i ) )
		{
			case GREATER_THAN_EQUAL:
				osLine << ">=";
				break;
			case EQUAL:
				osLine << " =";
				break;
			case LESS_THAN_EQUAL:
				osLine << "<=";
				break;
			default:
				OSL_ASSERT( !"wrong case" );
		}
		
		osLine << repeatString( " ", nColSpace ) << m_mxRHS( i, 0 );
		cout << osLine.str() << endl;
	}
	cout << repeatString( "-", 70 ) << endl;
	
	if ( m_bVarPositive )
	{
		cout << "All decision variables are assumed positive" << endl;
		cout << repeatString( "-", 70 ) << endl;
	}
	
	// Display variable boundaries.
	Matrix mxCost = getCostVector(); // row vector
	osLine.str( "" );
	for ( size_t i = 0; i < mxCost.cols(); ++i )
	{
		osLine << "x_" << i << ": ";
		bool bLower = isVarBounded( i, BOUND_LOWER );
		bool bUpper = isVarBounded( i, BOUND_UPPER );
		if ( bLower || bUpper )
		{
			if ( bLower )
			{
				double fBound = getVarBound( i, BOUND_LOWER );
				osLine << fBound;
			}
			else
				osLine << "[none]";

			osLine << " - ";
			
			if ( bUpper )
			{
				double fBound = getVarBound( i, BOUND_UPPER );
				osLine << fBound;
			}
			else
				osLine << "[none]";
		}
		else
			osLine << "[unbounded]";
		osLine << endl;
	}
	cout << osLine.str();
	cout << repeatString( "-", 70 ) << endl;
}


//---------------------------------------------------------------------------
// Model

Model::Model() : m_pImpl( new ModelImpl() )
{
}

Model::Model( const Model& other ) : m_pImpl( new ModelImpl( *other.m_pImpl.get() ) )
{
}

Model::~Model()
{
}

void Model::print() const
{
	m_pImpl->print();
}

size_t Model::getDecisionVarSize() const
{
	return m_pImpl->getDecisionVarSize();
}

size_t Model::getConstraintCount() const
{
	return m_pImpl->getConstraintCount();
}

double Model::getCost( size_t rowid ) const
{
	return m_pImpl->getCost( rowid );
}

/** get a matrix representing a cost vector.  Note that
 * a cost vector is a row matrix i.e. it only has one row.
 * 
 * @return row matrix representing a cost vector
 */
Matrix Model::getCostVector() const
{
	return m_pImpl->getCostVector();
}

void Model::setCostVectorElement( size_t i, double fVal )
{
	m_pImpl->setCostVectorElement( i, fVal );
}

void Model::setCostVector( const std::vector<double>& cn )
{
	m_pImpl->setCostVector( cn );
}


/** Beware that the caller is responsible for making sure that the specified 
	boundary exists by calling isVarBounded(...) beforehand.
 */
double Model::getVarBound( size_t i, Bound e ) const
{
	return m_pImpl->getVarBound( i, e );
}

void Model::setVarBound( size_t i, Bound e, double fValue )
{
	m_pImpl->setVarBound( i, e, fValue );
}

bool Model::isVarBounded( size_t i, Bound e ) const
{
	return m_pImpl->isVarBounded( i, e );
}

void Model::deleteVariables( const std::vector<size_t>& cnVarIds )
{
	m_pImpl->deleteVariables( cnVarIds );
}

Goal Model::getGoal() const
{
	return m_pImpl->getGoal();
}

void Model::setGoal( Goal e )
{
	m_pImpl->setGoal( e );
}

double Model::getObjectiveFuncConstant() const
{
	return m_pImpl->getObjectiveFuncConstant();
}

void Model::setObjectiveFuncConstant( double f )
{
	m_pImpl->setObjectiveFuncConstant( f );
}

unsigned long Model::getPrecision() const
{
	return m_pImpl->getPrecision();
}

void Model::setPrecision( unsigned long i )
{
	m_pImpl->setPrecision( i );
}

bool Model::getVarPositive() const
{
	return m_pImpl->getVarPositive();
}

void Model::setVarPositive( bool b )
{
	m_pImpl->setVarPositive( b );
}

bool Model::getVarInteger() const
{
	return m_pImpl->getVarInteger();
}

void Model::setVarInteger( bool b )
{
	m_pImpl->setVarInteger( b );
}

bool Model::getVerbose() const
{
	return m_pImpl->getVerbose();
}

void Model::setVerbose( bool b )
{
	m_pImpl->setVerbose( b );
}

double Model::getConstraint( size_t nRow, size_t nCol ) const
{
	return m_pImpl->getConstraint( nRow, nCol );
}

Matrix Model::getConstraintMatrix() const
{
	return m_pImpl->getConstraintMatrix();
}

Matrix Model::getRhsVector() const
{
	return m_pImpl->getRhsVector();
}

double Model::getRhsValue( size_t nId ) const
{
	return m_pImpl->getRhsValue( nId );
}

void Model::setRhsValue( size_t nId, double fVal )
{
	m_pImpl->setRhsValue( nId, fVal );
}

std::vector<Equality> Model::getEqualityVector() const
{
	return m_pImpl->getEqualityVector();
}

Equality Model::getEquality( size_t i ) const
{
	return m_pImpl->getEqualityByRowId( i );
}

void Model::addConstraint( const std::vector< double >& v, Equality e, double fRhs )
{
	m_pImpl->addConstraint( v, e, fRhs );
}

void Model::setStandardConstraintMatrix( const Matrix& mxConst, const Matrix& mxRhs )
{
	m_pImpl->setStandardConstraintMatrix( mxConst, mxRhs );
}


}}}



