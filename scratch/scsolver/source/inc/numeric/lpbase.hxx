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


#ifndef _LPBASE_HXX_
#define _LPBASE_HXX_

#include <numeric/matrix.hxx>
#include <memory>

namespace scsolver { namespace numeric { namespace opres { namespace lp {

class Model;
class BaseAlgorithmImpl;


class BaseAlgorithm
{
public:
	BaseAlgorithm();
	virtual ~BaseAlgorithm() throw() = 0;

	virtual void solve() = 0;

	Model* getModel() const;
	void setModel( Model* );

	/**
     * This method returns a pointer to a canonical version of stored model.  A
     * canonical model is an original model with all its constant equivalent
     * variables taken out.  Note that when this method is used to obtain a
     * model, then the solution reached needs to be set via
     * setCanonicalSolution() call, or else the behavior is undefined.
	 * 
     * @return Model* pointer to canonical model
	 */
	Model* getCanonicalModel() const;
	
	Matrix getSolution() const;
	void setSolution( const Matrix& );

	/**
     * This method must be called to set a final solution if a canonical model
     * is solved instead of an original model.  When this is called, it
     * reconstructs a solution for the original model by bringing back those
     * constant equivalent variables which have been removed when
     * getCanonicalModel() method was called.
     * 
     * @param mx solution matrix
	 */
	void setCanonicalSolution( const Matrix& mx );

private:
	::std::auto_ptr<BaseAlgorithmImpl> m_pImpl;
};

}}}}

#endif
