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

#include "numeric/quadfitlinesearch.hxx"
#include "numeric/funcobj.hxx"
#include "numeric/polyeqnsolver.hxx"
#include "numeric/matrix.hxx"
#include "numeric/diff.hxx"

#include <stdio.h>
#include <memory>

using namespace std;

namespace scsolver { namespace numeric {

struct SearchData
{
    double P1;
    double P2;
    double P3;

    QuadFitLineSearch::GoalType Goal;
    SingleVarFuncObj* pFunc;

    explicit SearchData(SingleVarFuncObj* p) :
        pFunc(p)
    {
    }
};

bool iterate(SearchData& data)
{
    // Given three points, 
    return false;
}

void evalStepLength(SingleVarFuncObj* pFunc, double step)
{
    double e = 0.2;
    auto_ptr<BaseFuncObj> pBaseFunc(pFunc->toBaseFuncObj());

    NumericalDiffer diff;
    diff.setFuncObject(pBaseFunc.get());

    // TODO: to be continued ...
}

/**
 * Find an initial set of points (P1, P2 and P3 in this order from left to 
 * right) such that P1 > P2 and P2 < P3.
 *  
 * @param data
 * 
 * @return bool true if successful, or false otherwise.
 */
bool findInitialPoints(SearchData& data)
{
    fprintf(stdout, "numeric::lclFindInitialPoints: \n");fflush(stdout);

    // First, find an acceptable step length.
    double step = 0.1;
    evalStepLength(data.pFunc, step);

    data.P1 = 0.0;

    data.pFunc->setVar(1.0);
    double f = data.pFunc->eval();
    exit(0);
    return false;
}

// --------------------------------------------------------------------------

QuadFitLineSearch::QuadFitLineSearch(SingleVarFuncObj* pFuncObj) :
    mpFuncObj(pFuncObj),
    m_eGoal(QuadFitLineSearch::MINIMIZE)
{
}

QuadFitLineSearch::~QuadFitLineSearch() throw()
{
}

void QuadFitLineSearch::setGoal(QuadFitLineSearch::GoalType goal)
{
    m_eGoal = goal;
}

bool QuadFitLineSearch::solve()
{
    if ( !mpFuncObj )
        return false;

    printf("%s\n", mpFuncObj->getFuncString().c_str());

    SearchData data(mpFuncObj);
    data.Goal = m_eGoal;

    // 1.  Find three points such that the 2nd point be the lowest.
    // 2.  Find the quadratic function that passes through all three points.
    // 3.  Find the minimizer point of that quadratic function.
    // 4.  Replace one of the three points with the minimizer point based on some conditions.
    // 5.  Terminate when P1 - P3 < e.
    findInitialPoints(data);
    
    // Solve the quadratic function.
    PolyEqnSolver eqnSolver;
    mpFuncObj->setVar(data.P1);
    eqnSolver.addDataPoint(data.P1, mpFuncObj->eval());
    mpFuncObj->setVar(data.P2);
    eqnSolver.addDataPoint(data.P2, mpFuncObj->eval());
    mpFuncObj->setVar(data.P3);
    eqnSolver.addDataPoint(data.P3, mpFuncObj->eval());
    Matrix sol = eqnSolver.solve();
    sol.print();
    double x, y;

    // Get the peak of that quad function.
    getQuadraticPeak(x, y, sol);
    fprintf(stdout, "QuadFitLineSearch::solve: peak at (%g, %g)\n", x, y);
    fflush(stdout);

    return true;
}

}}
