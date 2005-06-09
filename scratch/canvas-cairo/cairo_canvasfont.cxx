/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvas/debug.hxx>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

#include "cairo_outdevprovider.hxx"
#include "cairo_canvasfont.hxx"
#include "cairo_textlayout.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest& 					rFontRequest,
                            const uno::Sequence< beans::PropertyValue >& 	rExtraFontProperties, 
                            const geometry::Matrix2D& 						rFontMatrix,
                            const OutDevProviderSharedPtr&					rDevice ) :
        CanvasFont_Base( m_aMutex ),
        maFont( Font( rFontRequest.FontDescription.FamilyName,
                      rFontRequest.FontDescription.StyleName,
                      Size( 0, ::basegfx::fround(rFontRequest.CellSize) ) ) ),
        maFontRequest( rFontRequest ),
        mpRefDevice( rDevice )
    {
        CHECK_AND_THROW( mpRefDevice.get(),
                         "CanvasFont::CanvasFont(): Invalid ref device" );

        maFont->SetAlign( ALIGN_BASELINE );
        maFont->SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==com::sun::star::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        maFont->SetVertical( (rFontRequest.FontDescription.IsVertical==com::sun::star::util::TriState_YES) ? TRUE : FALSE );

        // TODO(F2): improve panose->vclenum conversion
        maFont->SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        maFont->SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );

        // adjust to stretched font
        if( !::rtl::math::approxEqual( rFontMatrix.m00, rFontMatrix.m11) )
        {
            const OutputDevice& rOutDev( mpRefDevice->getOutDev() );
            const Size aSize = rOutDev.GetFontMetric( *maFont ).GetSize();

            const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
            double fStretch = (rFontMatrix.m00 + rFontMatrix.m01);            

            if( !::basegfx::fTools::equalZero( fDividend) )
                fStretch /= fDividend;

            const long nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

            maFont->SetWidth( nNewWidth );
        }
    }

    CanvasFont::~CanvasFont()
    {
    }

    void SAL_CALL CanvasFont::disposing()
    {
        tools::LocalGuard aGuard;

        mpRefDevice.reset();
    }

    uno::Reference< rendering::XTextLayout > SAL_CALL  CanvasFont::createTextLayout( const rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpRefDevice.get() )
            return uno::Reference< rendering::XTextLayout >(); // we're disposed

        return new TextLayout( aText, nDirection, nRandomSeed, ImplRef( this ), mpRefDevice );
    }

    rendering::FontRequest SAL_CALL  CanvasFont::getFontRequest(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL  CanvasFont::getFontMetrics(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return rendering::FontMetrics();
    }

    uno::Sequence< double > SAL_CALL  CanvasFont::getAvailableSizes(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< double >();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL  CanvasFont::getExtraFontProperties(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< beans::PropertyValue >();
    }

#define SERVICE_NAME "com.sun.star.rendering.CanvasFont"

    ::rtl::OUString SAL_CALL CanvasFont::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVASFONT_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasFont::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    ::Font CanvasFont::getVCLFont() const
    {
        return *maFont;
    }
}
