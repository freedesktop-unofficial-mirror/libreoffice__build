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


#ifndef _MATRIX_HXX_
#define _MATRIX_HXX_

#include <boost/numeric/ublas/matrix.hpp>
#include <vector>
#include <memory>

namespace scsolver { namespace numeric {

/**
 * This exception is thrown when an element being accessed is
 * outside the current matrix boundary.
 */
class BadIndex : public std::exception 
{
public:
    virtual const char* what() const throw();
};

class MatrixSizeMismatch : public std::exception 
{
public:
    virtual const char* what() const throw();
};

class MatrixNotDiagonal : public std::exception
{
public:
    virtual const char* what() const throw();
};

class OperationOnEmptyMatrix : public std::exception
{
public:
    virtual const char* what() const throw();
};

class SingularMatrix : public std::exception
{
public:
    virtual const char* what() const throw();
};

class NonSquareMatrix : public std::exception
{
public:
    virtual const char* what() const throw();
};

class Matrix
{
public:
    Matrix();
    Matrix(size_t row, size_t col, bool identity_matrix = false);
    Matrix( const Matrix& );
    Matrix( const Matrix* );
    Matrix( ::boost::numeric::ublas::matrix<double> );
    ~Matrix() throw();
    
    void setResizable(bool resizable);
    void swap( Matrix& ) throw();
    void clear();
    void copy( const Matrix& );
    Matrix clone() const;

    const double getValue(size_t row, size_t col) const;
    double& getValue(size_t row, size_t col);
    void setValue(size_t row, size_t col, double val);
    ::boost::numeric::ublas::matrix< ::std::string > getDisplayElements(int prec, size_t colspace, bool formula) const;    
    void print( size_t = 2, size_t = 1 ) const;
    
    Matrix getColumn(size_t col);
    Matrix getRow(size_t row);

    void deleteColumn( size_t );
    void deleteColumns( const std::vector<size_t>& );
    void deleteRow( size_t );
    void deleteRows( const std::vector<size_t>& );
    
    const Matrix adj() const;
    double cofactor( size_t, size_t ) const;

    /**
     * This method calculates the determinant of a square matrix. 
     * It throws a NonSquareMatrix exception of the matrix being 
     * operated upon is not square. 
     * 
     * @return double determinant.
     */
    double det() const;
    const Matrix inverse() const;
    const Matrix trans() const;
    double minors( size_t, size_t ) const;
    void resize( size_t, size_t );

    /**
     * @return size_t current row size
     */
    size_t rows() const;

    /**
     * @return size_t current column size
     */
    size_t cols() const;
    
    bool empty() const;
    bool isRowEmpty( size_t ) const;
    bool isColumnEmpty( size_t ) const;
    bool isSameSize( const Matrix& ) const;
    bool isSquare() const;
    
    // Overloaded Operators
    
    Matrix& operator=( const Matrix& );
    const Matrix operator+( const Matrix& ) const;
    const Matrix operator-( const Matrix& ) const;
    const Matrix operator*( double ) const;
    const Matrix operator*( const Matrix& ) const;
    const Matrix operator/( double ) const;
    Matrix& operator+=( const Matrix& );
    Matrix& operator+=( double );
    Matrix& operator-=( const Matrix& );
    Matrix& operator*=( double );
    Matrix& operator/=( double );

    const double operator()(size_t row, size_t col) const;
    double& operator()(size_t row, size_t col);
    
    bool operator==( const Matrix& ) const;
    bool operator!=( const Matrix& ) const;
    
private:

    void maybeExpand(size_t row, size_t col);
    void throwIfEmpty() const;

    bool m_bResizable;
    ::boost::numeric::ublas::matrix<double, ::boost::numeric::ublas::row_major, std::vector<double> > m_aArray;
};

const Matrix operator+( const Matrix&, double );
const Matrix operator+( double, const Matrix& );

}}

#endif //_MATRIX_HXX_
