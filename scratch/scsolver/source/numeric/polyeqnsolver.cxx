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

#include "numeric/polyeqnsolver.hxx"
#include "numeric/matrix.hxx"
#include <stdio.h>

namespace scsolver { namespace numeric {

const char* NotEnoughDataPoints::what() const throw()
{
    return "not enough data points to solve a polynomial equation (minimum of 2 required)";
}

DataPoint::DataPoint(double x, double y) :
    X(x), Y(y)
{
}

PolyEqnSolver::PolyEqnSolver()
{
}

PolyEqnSolver::~PolyEqnSolver() throw()
{
}

void PolyEqnSolver::addDataPoint(double x, double y)
{
    DataPoint pt(x, y);
    m_DataPoints.push_back(pt);
}

const Matrix PolyEqnSolver::solve()
{
    size_t nDPSize = m_DataPoints.size();
    if (nDPSize < 2)
    {
        // We need at least 2 data points to form a polynomial equation.
        throw NotEnoughDataPoints();
    }

    Matrix mxRight(nDPSize, nDPSize), mxLeft(nDPSize, 1);
    list<DataPoint>::const_iterator itr = m_DataPoints.begin(), itrEnd = m_DataPoints.end();
    for (size_t nRow = 0; itr != itrEnd; ++itr, ++nRow)
    {
        double varTerm = 1.0;
        for (size_t nCol = 0; nCol < nDPSize; ++nCol)
        {
            mxRight(nRow, nCol) = varTerm;
            varTerm *= itr->X;
        }
        mxLeft(nRow, 0) = itr->Y;
    }

    return mxRight.inverse() * mxLeft;
}

void PolyEqnSolver::clear()
{
    m_DataPoints.clear();
}

size_t PolyEqnSolver::size() const
{
    return m_DataPoints.size();
}

//--------------------------------------------------------------------

void getQuadraticPeak(double& x, double& y, const Matrix& coef)
{
    // coef = (c, b, a)
    double a = coef(2, 0);
    double b = coef(1, 0);
    double c = coef(0, 0);
    x = b / (-2.0*a);
    y = x * x * a + x * b + c;
}

}}
