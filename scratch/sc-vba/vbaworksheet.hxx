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
#ifndef SC_VBA_WORKSHEET_HXX
#define SC_VBA_WORKSHEET_HXX

#include <cppuhelper/implbase2.hxx>
#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <org/openoffice/excel/XWorksheet.hpp>
#include <org/openoffice/excel/XComments.hpp>
#include <org/openoffice/excel/XRange.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <org/openoffice/excel/XOutline.hpp>
#include <org/openoffice/excel/XChartObjects.hpp>

#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XWorksheet >  WorksheetImpl_BASE;

class ScVbaWorksheet : public WorksheetImpl_BASE
{
	css::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
	css::uno::Reference< css::frame::XModel > mxModel;
	css::uno::Reference< oo::excel::XChartObjects > mxCharts;
		
	css::uno::Reference< oo::excel::XWorksheet > getSheetAtOffset(SCTAB offset) throw (css::uno::RuntimeException);
	css::uno::Reference< oo::excel::XRange > getSheetRange() throw (css::uno::RuntimeException);

	css::uno::Any getControl( const rtl::OUString& sName );
	css::uno::Any getControlShape( const rtl::OUString& sName );
protected:

	virtual css::uno::Reference< css::frame::XModel > getModel()
	{ return mxModel; }
	virtual css::uno::Reference< css::sheet::XSpreadsheet > getSheet()
	{ return mxSheet; }

	ScVbaWorksheet( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext );
    ScDocument* getScDocument() throw (css::uno::RuntimeException);
public:
	ScVbaWorksheet( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,
		const css::uno::Reference< css::uno::XComponentContext >& xContext,
		const css::uno::Reference< css::sheet::XSpreadsheet >& xSheet, 
		const css::uno::Reference< css::frame::XModel >& xModel )throw (css::uno::RuntimeException)  ;
	ScVbaWorksheet( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException );

	virtual ~ScVbaWorksheet() {}


    // Attributes
	virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setName( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
	virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (css::uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getStandardWidth() throw (css::uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getStandardHeight() throw (css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL getProtectionMode() throw (css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL getProtectContents() throw (css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL getProtectDrawingObjects() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getUsedRange() throw (css::uno::RuntimeException) ;
	virtual css::uno::Any SAL_CALL ChartObjects( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XOutline > SAL_CALL Outline( ) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getNext() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getPrevious() throw (css::uno::RuntimeException);

    // Methods
	virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
	virtual void SAL_CALL Select() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Move( const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException) ;
 	virtual void SAL_CALL Copy( const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Paste( const css::uno::Any& Destination, const css::uno::Any& Link ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Protect( const css::uno::Any& Password, const css::uno::Any& DrawingObjects, const css::uno::Any& Contents, const css::uno::Any& Scenarios, const css::uno::Any& UserInterfaceOnly ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Unprotect( const css::uno::Any& Password ) throw (css::uno::RuntimeException);

	virtual void SAL_CALL Calculate(  ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL CheckSpelling( const css::uno::Any& CustomDictionary,const css::uno::Any& IgnoreUppercase,const css::uno::Any& AlwaysSuggest, const css::uno::Any& SpellingLang ) throw (css::uno::RuntimeException);
	// Hacks (?)
	virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol ) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Rows(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Columns(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);

	virtual css::uno::Any SAL_CALL Evaluate( const ::rtl::OUString& Name ) throw (css::uno::RuntimeException); 
	virtual css::uno::Any SAL_CALL PivotTables( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Comments( const css::uno::Any& Index ) throw (css::uno::RuntimeException);

	virtual css::uno::Any SAL_CALL OLEObjects( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL setEnableCalculation( ::sal_Bool EnableCalculation ) throw ( css::script::BasicErrorException, css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL getEnableCalculation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException); 
	// XInvocation
	virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
	virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    // CodeName
    virtual rtl::OUString SAL_CALL getCodeName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCodeName( const rtl::OUString& sCodeName ) throw (css::uno::RuntimeException);
	// XHelperInterface
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();	
};

#endif /* SC_VBA_WORKSHEET_HXX */

