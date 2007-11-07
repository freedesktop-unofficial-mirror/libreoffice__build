/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaname.cxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: npower $ $Date: 2007/07/23 11:47:24 $
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
#include "helperdecl.hxx"

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>

#include "vbaname.hxx"
#include "vbarange.hxx"
#include "vbaglobals.hxx"
#include <vector>
#include <rangenam.hxx>
#include <vcl/msgbox.hxx>
#include "tabvwsh.hxx"
#include "viewdata.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaName::ScVbaName(const css::uno::Reference< oo::vba::XHelperInterface >& xParent, 
			const css::uno::Reference< css::uno::XComponentContext >& xContext,
			const css::uno::Reference< css::sheet::XNamedRange >& xName,
			const css::uno::Reference< css::sheet::XNamedRanges >& xNames,
			const css::uno::Reference< css::frame::XModel >& xModel ):
			NameImpl_BASE(  xParent , xContext ), 
			mxModel( xModel ),
			mxNamedRange( xName ),
			mxNames( xNames )
{
}

ScVbaName::~ScVbaName()
{
}

css::uno::Reference< oo::excel::XWorksheet >
ScVbaName::getWorkSheet() throw (css::uno::RuntimeException)
{
	return ScVbaGlobals::getGlobalsImpl( mxContext )->getActiveSheet();
}

::rtl::OUString
ScVbaName::getName() throw (css::uno::RuntimeException)
{
	String sName;
	sName += UniString( getWorkSheet()->getName());
	sName += String::CreateFromAscii("!");
	sName += UniString ( mxNamedRange->getName() );
	return ::rtl::OUString( sName );
}

void
ScVbaName::setName( const ::rtl::OUString & rName ) throw (css::uno::RuntimeException)
{
	mxNamedRange->setName( rName );
}

::rtl::OUString
ScVbaName::getNameLocal() throw (css::uno::RuntimeException)
{
	return getName();
}

void
ScVbaName::setNameLocal( const ::rtl::OUString & rName ) throw (css::uno::RuntimeException)
{
	setName( rName );
}

sal_Bool
ScVbaName::getVisible() throw (css::uno::RuntimeException)
{
	return true;
}

void
ScVbaName::setVisible( sal_Bool /*bVisible*/ ) throw (css::uno::RuntimeException)
{
}

::rtl::OUString
ScVbaName::getValue() throw (css::uno::RuntimeException)
{
	::rtl::OUString sValue = mxNamedRange->getContent();
	::rtl::OUString sSheetName = getWorkSheet()->getName();
    ::rtl::OUString sSegmentation = ::rtl::OUString::createFromAscii( ";" );
    ::rtl::OUString sNewSegmentation = ::rtl::OUString::createFromAscii( "," );
    ::rtl::OUString sResult;
    sal_Int32 nFrom = 0;
    sal_Int32 nTo = 0;
    nTo = sValue.indexOf( sSegmentation, nFrom );
    while ( nTo != -1 )
    {
        ::rtl::OUString sTmpValue = sValue.copy( nFrom, nTo - nFrom );
        if ( sTmpValue.toChar() == '$' )
        {
            ::rtl::OUString sTmp = sTmpValue.copy( 1 );
            sTmp = sTmp.replaceAt(0, (sSheetName + ::rtl::OUString::createFromAscii(".")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("!"));
            sResult += sTmp; 
            sResult += sNewSegmentation;
        }
        nFrom = nTo + 1;
        nTo = sValue.indexOf( sSegmentation, nFrom );
    }
    ::rtl::OUString sTmpValue = sValue.copy( nFrom );
	if ( sTmpValue.toChar() == '$' )
	{
		::rtl::OUString sTmp = sTmpValue.copy(1);
        sTmp = sTmp.replaceAt(0, (sSheetName + ::rtl::OUString::createFromAscii(".")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("!"));
		sResult += sTmp;
	}
	if (sResult.indexOf('=') != 0)
	{
		sResult = ::rtl::OUString::createFromAscii("=") + sResult;
	}
	return sResult;
}

void 
ScVbaName::setValue( const ::rtl::OUString & rValue ) throw (css::uno::RuntimeException)
{
	::rtl::OUString sSheetName = getWorkSheet()->getName();
	::rtl::OUString sValue = rValue;
    ::rtl::OUString sSegmentation = ::rtl::OUString::createFromAscii( "," );
    ::rtl::OUString sNewSegmentation = ::rtl::OUString::createFromAscii( ";" );
    ::rtl::OUString sResult;
    sal_Int32 nFrom = 0;
    sal_Int32 nTo = 0;
	if (sValue.indexOf('=') == 0)
	{
		::rtl::OUString sTmp = sValue.copy(1);
        sValue = sTmp;
	}
    nTo = sValue.indexOf( sSegmentation, nFrom );
    while ( nTo != -1 )
    {
        ::rtl::OUString sTmpValue = sValue.copy( nFrom, nTo - nFrom );
        sTmpValue = sTmpValue.replaceAt(0, (sSheetName + ::rtl::OUString::createFromAscii("!")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("."));
        if (sTmpValue.copy(0, sSheetName.getLength()).equals(sSheetName))
        {
            sTmpValue = ::rtl::OUString::createFromAscii("$") + sTmpValue;
        }
        sTmpValue += sNewSegmentation;
        sResult += sTmpValue;
        nFrom = nTo + 1;
        nTo = sValue.indexOf( sSegmentation, nFrom );
    }
    ::rtl::OUString sTmpValue = sValue.copy( nFrom );
	sTmpValue = sTmpValue.replaceAt(0, (sSheetName + ::rtl::OUString::createFromAscii("!")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("."));
	if (sTmpValue.copy(0, sSheetName.getLength()).equals(sSheetName))
	{
		sTmpValue = ::rtl::OUString::createFromAscii("$") + sTmpValue;
	}
    sResult += sTmpValue;
	mxNamedRange->setContent(sResult);
}

::rtl::OUString 
ScVbaName::getRefersTo() throw (css::uno::RuntimeException)
{
	return getValue();
}

void 
ScVbaName::setRefersTo( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
	setValue( rRefersTo );
}

::rtl::OUString 
ScVbaName::getRefersToLocal() throw (css::uno::RuntimeException)
{
	return getRefersTo();
}

void 
ScVbaName::setRefersToLocal( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
	setRefersTo( rRefersTo );
}

::rtl::OUString
ScVbaName::getRefersToR1C1() throw (css::uno::RuntimeException)
{
	return getRefersTo();
}

void
ScVbaName::setRefersToR1C1( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
	setRefersTo( rRefersTo );
}

::rtl::OUString 
ScVbaName::getRefersToR1C1Local() throw (css::uno::RuntimeException)
{
	return getRefersTo();
}

void
ScVbaName::setRefersToR1C1Local( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
	setRefersTo( rRefersTo );
}

css::uno::Reference< oo::excel::XRange >
ScVbaName::getRefersToRange() throw (css::uno::RuntimeException)
{
    uno::Reference< oo::excel::XRange > xRange = ScVbaRange::getRangeObjectForName( mxContext, mxNamedRange->getName(), getDocShell( mxModel ), ScAddress::CONV_XL_R1C1 );
	return xRange;
}

void
ScVbaName::setRefersToRange( const css::uno::Reference< oo::excel::XRange > /*rRange*/ ) throw (css::uno::RuntimeException)
{
}

void
ScVbaName::Delete() throw (css::uno::RuntimeException)
{
	mxNames->removeByName( mxNamedRange->getName() );	
}

rtl::OUString&
ScVbaName::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaName") );
	return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaName::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Name" ) );
	}
	return aServiceNames;
}

