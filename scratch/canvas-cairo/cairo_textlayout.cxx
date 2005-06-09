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
#include "cairo_textlayout.hxx"

#ifndef BOOST_SCOPED_ARRAY_HPP_INCLUDED
#include <boost/scoped_array.hpp>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP_
#include <com/sun/star/rendering/TextDirection.hpp>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "cairo_impltools.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        void setupLayoutMode( OutputDevice& rOutDev,
                              sal_Int8		nTextDirection )	
        {
            // TODO(P3): avoid if already correctly set
            ULONG nLayoutMode;
            switch( nTextDirection )
            {
                default:
                    nLayoutMode = 0;
                    break;
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_LTR;
                    break;
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_BIDI_STRONG;
                    break;
                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_RTL;
                    break;
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG;
                    break;
            }

            // set calculated layout mode. Origin is always the left edge,
            // as required at the API spec
            rOutDev.SetLayoutMode( nLayoutMode | TEXT_LAYOUT_TEXTORIGIN_LEFT );
        }
    }

    TextLayout::TextLayout( const rendering::StringContext& aText,
                            sal_Int8                        nDirection,
                            sal_Int64                       nRandomSeed,
                            const CanvasFont::ImplRef&      rFont,
                            const OutDevProviderSharedPtr&	rRefDevice ) :
        TextLayout_Base( m_aMutex ),
        maText( aText ),
        maLogicalAdvancements(),
        mpFont( rFont ),
        mpRefDevice( rRefDevice ),
        mnTextDirection( nDirection )
    {
    }

    TextLayout::~TextLayout()
    {
    }

    void SAL_CALL TextLayout::disposing()
    {
        tools::LocalGuard aGuard;

        mpFont.reset();
        mpRefDevice.reset();
    }

    // XTextLayout
    uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > > SAL_CALL TextLayout::queryTextShapes(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryInkMeasures(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryMeasures(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< double > SAL_CALL TextLayout::queryLogicalAdvancements(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return maLogicalAdvancements;
    }

    void SAL_CALL TextLayout::applyLogicalAdvancements( const uno::Sequence< double >& aAdvancements ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        CHECK_AND_THROW( aAdvancements.getLength() == maText.Length,
                         "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );

        maLogicalAdvancements = aAdvancements;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        VirtualDevice aVDev( mpRefDevice->getOutDev() );
        aVDev.SetFont( mpFont->getVCLFont() );

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( aVDev.GetFontMetric() );

        setupLayoutMode( aVDev, mnTextDirection );

        const sal_Int32 nAboveBaseline( -aMetric.GetIntLeading() - aMetric.GetAscent() );
        const sal_Int32 nBelowBaseline( aMetric.GetDescent() );

        if( maLogicalAdvancements.getLength() )
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              maLogicalAdvancements[ maLogicalAdvancements.getLength()-1 ],
                                              nBelowBaseline );
        }
        else
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              aVDev.GetTextWidth(
                                                  maText.Text,
                                                  ::canvas::tools::numeric_cast<USHORT>(maText.StartPosition),
                                                  ::canvas::tools::numeric_cast<USHORT>(maText.Length) ),
                                              nBelowBaseline );
        }
    }

    double SAL_CALL TextLayout::justify( double nSize ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return 0.0;
    }

    double SAL_CALL TextLayout::combinedJustify( const uno::Sequence< uno::Reference< rendering::XTextLayout > >& aNextLayouts, double nSize ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return 0.0;
    }

    rendering::TextHit SAL_CALL TextLayout::getTextHit( const geometry::RealPoint2D& aHitPoint ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return rendering::TextHit();
    }

    rendering::Caret SAL_CALL TextLayout::getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return rendering::Caret();
    }

    sal_Int32 SAL_CALL TextLayout::getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return 0;
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    double SAL_CALL TextLayout::getBaselineOffset(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return 0.0;
    }

    sal_Int8 SAL_CALL TextLayout::getMainTextDirection(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL TextLayout::getFont(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return mpFont.getRef();
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return maText;
    }

    bool TextLayout::draw( OutputDevice&                 rOutDev,
                           const Point&                  rOutpos,
                           const rendering::ViewState&   viewState,
                           const rendering::RenderState& renderState ) const
    {
        tools::LocalGuard aGuard;

        setupLayoutMode( rOutDev, mnTextDirection );

        if( maLogicalAdvancements.getLength() )
        {
            // TODO(P2): cache that
            ::boost::scoped_array< long > aOffsets(new long[maLogicalAdvancements.getLength()]);
            setupTextOffsets( aOffsets.get(), maLogicalAdvancements, viewState, renderState );
            
            // TODO(F3): ensure correct length and termination for DX
            // array (last entry _must_ contain the overall width)
            
            rOutDev.DrawTextArray( rOutpos,
                                   maText.Text,
                                   aOffsets.get(),
                                   ::canvas::tools::numeric_cast<USHORT>(maText.StartPosition),
                                   ::canvas::tools::numeric_cast<USHORT>(maText.Length) );
        }
        else
        {
            rOutDev.DrawText( rOutpos,
                              maText.Text,
                              ::canvas::tools::numeric_cast<USHORT>(maText.StartPosition),
                              ::canvas::tools::numeric_cast<USHORT>(maText.Length) );
        }

        return true;
    }

    namespace
    {
        class OffsetTransformer
        {
        public:
            OffsetTransformer( const ::basegfx::B2DHomMatrix& rMat ) :
                maMatrix( rMat )
            {
            }

            long operator()( const double& rOffset )
            {
                // This is an optimization of the normal rMat*[x,0]
                // transformation of the advancement vector (in x
                // direction), followed by a length calculation of the
                // resulting vector: advancement' =
                // ||rMat*[x,0]||. Since advancements are vectors, we
                // can ignore translational components, thus if [x,0],
                // it follows that rMat*[x,0]=[x',0] holds. Thus, we
                // just have to calc the transformation of the x
                // component.

                // TODO(F2): Handle non-horizontal advancements!
                return ::basegfx::fround( hypot(maMatrix.get(0,0)*rOffset,
												maMatrix.get(1,0)*rOffset) );
            }

        private:
            ::basegfx::B2DHomMatrix maMatrix;
        };
    }

    void TextLayout::setupTextOffsets( long*							outputOffsets,
                                       const uno::Sequence< double >& 	inputOffsets,
                                       const rendering::ViewState& 		viewState,
                                       const rendering::RenderState& 	renderState		) const
    {
        ENSURE_AND_THROW( outputOffsets!=NULL,
                          "TextLayout::setupTextOffsets offsets NULL" );

        ::basegfx::B2DHomMatrix aMatrix;

        ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                     viewState,
                                                     renderState);

        // fill integer offsets
        ::std::transform( const_cast< uno::Sequence< double >& >(inputOffsets).getConstArray(),
                          const_cast< uno::Sequence< double >& >(inputOffsets).getConstArray()+inputOffsets.getLength(),
                          outputOffsets,
                          OffsetTransformer( aMatrix ) );
    }


#define SERVICE_NAME "com.sun.star.rendering.TextLayout"

    ::rtl::OUString SAL_CALL TextLayout::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( TEXTLAYOUT_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL TextLayout::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL TextLayout::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}
