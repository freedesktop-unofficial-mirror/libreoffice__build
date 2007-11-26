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

#ifndef _SCSOLVER_NUMERIC_POLYEQNSOLVER_HXX_
#define _SCSOLVER_NUMERIC_POLYEQNSOLVER_HXX

#include <list>
#include <exception>

namespace scsolver { namespace numeric {

class Matrix;

class NotEnoughDataPoints : public ::std::exception 
{
public:
    virtual const char* what() const throw();
};

struct DataPoint
{
    double X;
    double Y;

    DataPoint(double x, double y);
};

/**
 * This class is used to solve a polynomial equation of an arbitrary degree
 * based on a set of data points.  It returns an array of coefficients from 
 * the lowest order to highest, and the order of the equation equals the 
 * number of data points minus 1 e.g. if two data points are given, the 
 * equation to be solved will be linear, if three data points are given, the 
 * equation will be quadratic, and so on.
 */
class PolyEqnSolver
{
public:
    PolyEqnSolver();
    ~PolyEqnSolver() throw();

    void addDataPoint(double x, double y);
    const Matrix solve();
    void clear();
    size_t size() const;

private:
    ::std::list<DataPoint> m_DataPoints;
};

//--------------------------------------------------------------------

/**
 * Calculate the peak position of a quadratic equation.  The quadratic 
 * equation must be specified in terms of a set of three coefficients: A, B 
 * and C as in f(x) = Ax^2 + Bx + C, and they must be given as a column matrix 
 * of (C, B, A) (note the reverse order). 
 *  
 * @param x the x position of the quadratic peak
 * @param y the y value at the quadratic peak
 * @param coef = (C, B, A) as in f(x) = Ax^2 + Bx + C.  Note that the order of
 *             the coefficients are from the x^0 term toward x^2.
 */
void getQuadraticPeak(double& x, double& y, const Matrix& coef);

}}

#endif
