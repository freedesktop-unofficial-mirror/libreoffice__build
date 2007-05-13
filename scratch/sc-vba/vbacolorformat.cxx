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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/msforms/XLineFormat.hpp>
#include "vbacolorformat.hxx"

using namespace org::openoffice;
using namespace com::sun::star;

const sal_Int32
MsoColorIndizes::getColorIndex( sal_Int32 nIndex )
{
    const static sal_Int32 COLORINDIZES[56] =
    {   HAPICOLOR_BLACK, HAPICOLOR_WITHE, HAPICOLOR_RED, HAPICOLOR_BRIGHTGREEN, HAPICOLOR_BLUE, HAPICOLOR_YELLOW, HAPICOLOR_PINK,
        HAPICOLOR_TURQUOISE, HAPICOLOR_DARKRED, HAPICOLOR_GREEN, HAPICOLOR_DARKBLUE, HAPICOLOR_DARKYELLOW, HAPICOLOR_VIOLET,
        HAPICOLOR_TEAL, HAPICOLOR_GRAY_25_PERCENT, HAPICOLOR_GRAY_50_PERCENT, HAPICOLOR_PERIWINCKLE, HAPICOLOR_PLUM,
        HAPICOLOR_IVORY, HAPICOLOR_LIGHTTURQUOISE, HAPICOLOR_DARKPRUPLE, HAPICOLOR_CORAL, HAPICOLOR_OCEANBLUE, HAPICOLOR_ICEBLUE,
        HAPICOLOR_GREEN, HAPICOLOR_PINK, HAPICOLOR_YELLOW, HAPICOLOR_TURQUOISE, HAPICOLOR_VIOLET, HAPICOLOR_DARKRED, HAPICOLOR_TEAL,
        HAPICOLOR_BLUE, HAPICOLOR_SKYBLUE, HAPICOLOR_LIGHTTURQUOISE, HAPICOLOR_LIGHTGREEN, HAPICOLOR_LIGHTYELLOW, HAPICOLOR_PALEBLUE,
        HAPICOLOR_ROSE, HAPICOLOR_LAVENDER, HAPICOLOR_TAN, HAPICOLOR_LIGHTBLUE, HAPICOLOR_AQUA, HAPICOLOR_LIME, HAPICOLOR_GOLD,
        HAPICOLOR_LIGHTORANGE, HAPICOLOR_ORANGE, HAPICOLOR_BLUEGRAY, HAPICOLOR_GRAY_40_PERCENT, HAPICOLOR_DARKTEAL,
        HAPICOLOR_SEAGREEN, HAPICOLOR_NONAME, HAPICOLOR_OLIVEGREEN, HAPICOLOR_BROWN, HAPICOLOR_PLUM, HAPICOLOR_INDIGO,
        HAPICOLOR_GRAY_80_PERCENT
    };
    return COLORINDIZES[nIndex];
}
ScVbaColorFormat::ScVbaColorFormat( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< vba::XHelperInterface > xInternalParent, const uno::Reference< drawing::XShape > xShape, const sal_Int16 nColorFormatType ) : ScVbaColorFormat_BASE( xParent, xContext ), m_xInternalParent( xInternalParent ), m_xShape( xShape ), m_nColorFormatType( nColorFormatType )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nFillFormatBackColor = 0;
    try
    {
        uno::Reference< oo::msforms::XFillFormat > xFillFormat( xInternalParent, uno::UNO_QUERY_THROW );
        m_pFillFormat = ( ScVbaFillFormat* )( xFillFormat.get() );
    }catch ( uno::RuntimeException  e )
    {
        m_pFillFormat = NULL;
    }
}

void
ScVbaColorFormat::setColorFormat( sal_Int16 _ntype )
{
    m_nColorFormatType = _ntype;
}

// Attribute
sal_Int32 SAL_CALL 
ScVbaColorFormat::getRGB() throw (uno::RuntimeException)
{
    sal_Int32 nRGB = 0;
    switch( m_nColorFormatType )
    {
    case ColorFormatType::LINEFORMAT_FORECOLOR:
        m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("LineColor") ) >>= nRGB;
        break;
    case ColorFormatType::LINEFORMAT_BACKCOLOR:
        //TODO BackColor not supported
        // m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("Color"), uno::makeAny( nRGB ) );
        break;
    case ColorFormatType::FILLFORMAT_FORECOLOR:
        m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("FillColor") ) >>= nRGB;
        break;
    case ColorFormatType::FILLFORMAT_BACKCOLOR:
        nRGB = m_nFillFormatBackColor;
        break;
    default:
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Second parameter of ColorFormat is wrong."), uno::Reference< uno::XInterface >() );
    }
    nRGB = OORGBToXLRGB( nRGB );
    return nRGB;
}

void SAL_CALL 
ScVbaColorFormat::setRGB( sal_Int32 _rgb ) throw (uno::RuntimeException)
{
    sal_Int32 nRGB = XLRGBToOORGB( _rgb );
    switch( m_nColorFormatType )
    {
    case ColorFormatType::LINEFORMAT_FORECOLOR:
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineColor" ), uno::makeAny( _rgb ) );
        break;
    case ColorFormatType::LINEFORMAT_BACKCOLOR:
        // TODO BackColor not supported
        break;
    case ColorFormatType::FILLFORMAT_FORECOLOR:
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "FillColor" ), uno::makeAny( _rgb ) );
        if( m_pFillFormat )
        {
            m_pFillFormat->setForeColorAndInternalStyle(nRGB);
        }
        break;
    case ColorFormatType::FILLFORMAT_BACKCOLOR:
        m_nFillFormatBackColor = nRGB;
        if( m_pFillFormat )
        {
            m_pFillFormat->setForeColorAndInternalStyle(nRGB);
        }
        break;
    default:    
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Second parameter of ColorFormat is wrong."), uno::Reference< uno::XInterface >() );
    }
}

sal_Int32 SAL_CALL 
ScVbaColorFormat::getSchemeColor() throw (uno::RuntimeException)
{
    sal_Int32 nColor = getRGB();
    sal_Int32 i;
    for( i = 0; i < 56; i++ )
    {
        if( nColor == MsoColorIndizes::getColorIndex(i) )
            break; 
    }
    if( i == 56 )
    {
        i = -2;
    }
    return ( i + 2 );
}

void SAL_CALL 
ScVbaColorFormat::setSchemeColor( sal_Int32 _schemecolor ) throw (uno::RuntimeException)
{
    sal_Int32 nColor = MsoColorIndizes::getColorIndex( _schemecolor );
    setRGB( OORGBToXLRGB( nColor ) );
}


rtl::OUString&
ScVbaColorFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaColorFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaColorFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msforms.ColorFormat" ) );
    }
    return aServiceNames;
}

