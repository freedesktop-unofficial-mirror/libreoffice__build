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

#ifndef _NUMERIC_ALGORITHM_HXX_
#define _NUMERIC_ALGORITHM_HXX_

#include <memory>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace scsolver { namespace numeric {

class BaseFuncObj;

class FuncObjectNotSet : public std::exception {};

/** 
 * Algorithm derived from Chapter 4.2 (p.93) of "A First Course In Numerical 
 * Analysis 2nd ed. by Anthony Ralston and Philip Rabinowitz".
 */
class NumericalDiffer
{
    static const double OMEGA;

public:
    NumericalDiffer();
    ~NumericalDiffer() throw();

    void setPrecision(unsigned long n);
    void setSecondOrder(bool b);
    void setVariables(const ::std::vector<double>& cnX);
    void setVarIndex(unsigned long n);
    void setFuncObject(BaseFuncObj* p);

    double run();

private:
    void initialize();
    void setDirty();
    void appendNewH();
    void setT(unsigned long m, unsigned long i, double fVal);
    double getT(unsigned long m, unsigned long i);
    double T0(unsigned long i);
    double Tm();
    double Tm(unsigned long m, unsigned long i = 0);

private:
    unsigned long m_nPrec;
    unsigned long m_nVarIndex;
    bool m_bSecondOrder;
    BaseFuncObj* m_pFuncObj;

    ::std::vector<double> m_cnX;
    ::std::vector<double> m_cnH;
    ::std::vector< ::std::vector<double> > m_cnT;
};

}}

#endif
