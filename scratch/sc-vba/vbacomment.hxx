/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
#ifndef SC_VBA_COMMENT_HXX
#define SC_VBA_COMMENT_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/excel/XComment.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1< oo::excel::XComment > ScVbaComment_BASE;

class ScVbaComment : public ScVbaComment_BASE
{
	css::uno::Reference< css::table::XCellRange > mxRange;
	css::uno::Reference< css::uno::XComponentContext > m_xContext;

private:
	css::uno::Reference< css::sheet::XSheetAnnotation > SAL_CALL getAnnotation() throw (css::uno::RuntimeException);
	css::uno::Reference< css::sheet::XSheetAnnotations > SAL_CALL getAnnotations() throw (css::uno::RuntimeException);
	sal_Int32 SAL_CALL getAnnotationIndex() throw (css::uno::RuntimeException);
	css::uno::Reference< oo::excel::XComment > SAL_CALL getCommentByIndex( sal_Int32 Index ) throw (css::uno::RuntimeException);
public:
    ScVbaComment( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::lang::IllegalArgumentException );

    virtual ~ScVbaComment() {}

    // Attributes
    virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getAuthor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAuthor( const rtl::OUString& _author ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getParent() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XComment > SAL_CALL Next() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XComment > SAL_CALL Previous() throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL Text( const css::uno::Any& Text, const css::uno::Any& Start, const css::uno::Any& Overwrite ) throw (css::uno::RuntimeException);
};

#endif /* SC_VBA_COMMENT_HXX */

