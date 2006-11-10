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

#ifndef _SCSOLVER_OPTIONDLG_HXX_
#define _SCSOLVER_OPTIONDLG_HXX_

#include "basedlg.hxx"
#include "option.hxx"

#include <memory>

namespace scsolver {

class ActionObject;
class CloseBtnListener;

struct OptionDialogImpl;

class OptionDialog : public BaseDialog
{
public:
	OptionDialog( SolverImpl* p );
	virtual ~OptionDialog() throw();

	/**
	 * This method is called when a range selection is finished.  If
	 * the dialog does not contain a range selection widget pair,
	 * just leave this method empty.
	 *
	 * @return false for signaling to the calling function that
	 *  the selection is invalid and should be discarded, or true if
	 *  the selection is satisfactory.
	 */
	virtual bool doneRangeSelection() const;

	/**
	 * Must return a unique name that identifies its dialog type.
	 * Should be deprecated and its use in a nested-if or switch
	 * statement is discouraged because it is not very elegant and
	 * non-safe.
	 */
	virtual const rtl::OUString getDialogName() const;

	virtual void setVisible( bool b );

	OptModelType getModelType() const;
	void setModelType( OptModelType type );


private:
	void initialize();
	void registerListeners();
	void unregisterListeners() throw();

	::std::auto_ptr<OptionDialogImpl> m_pImpl;
};

}

#endif
