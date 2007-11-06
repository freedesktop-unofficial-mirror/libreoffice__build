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
#include <memory>
#include <string>

using namespace ::scsolver::numeric;

class TestFunc1 : public SingleVarFuncObj
{
public:
    TestFunc1()
    {
    }

    virtual ~TestFunc1() throw()
    {
    }

    virtual void setVar(double var)
    {
        mVar = var;
    }

    virtual double getVar() const
    {
        return mVar;
    }

    virtual double eval() const
    {
        return (mVar - 2.0)*(mVar*2.0 + 5.0) + 10.0;
    }

    /**
     * Return a display-friendly function string (e.g. x^3 + 2*x^2 + 4).
     */
    virtual const string getFuncString() const
    {
        return string("(x - 2) * (2x + 5) + 10");
    }
private:
    double mVar;
};

void run()
{
    auto_ptr<SingleVarFuncObj> pFuncObj(new TestFunc1);
    QuadFitLineSearch qfSearch(pFuncObj.get());
    qfSearch.setGoal(QuadFitLineSearch::MINIMIZE);
    qfSearch.solve();
}

int main()
{
    run();
}
