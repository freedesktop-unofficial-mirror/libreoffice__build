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

#include <canvas/debug.hxx>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#include <com/sun/star/rendering/Endianness.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <comphelper/sequence.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>

#include "cairo_spritecanvas.hxx"
#include "cairo_canvasfont.hxx"
#include "cairo_textlayout.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_canvasbitmap.hxx"

#include <algorithm>

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasHelper::CanvasHelper() :
        mpDevice( NULL ),
        maSize(),
        mbHaveAlpha()
    {
    }

    void CanvasHelper::disposing()
    {
        mpDevice = NULL;
	mpCairo = NULL;
    }

    void CanvasHelper::init( const ::basegfx::B2ISize& rSize,
                             SpriteCanvas&             rDevice,
			     Cairo*                    pCairo,
                             bool                      bHasAlpha )
    {
        mpDevice = &rDevice;
        maSize = rSize;
        mbHaveAlpha = bHasAlpha;
	mpCairo = pCairo;
    }

    void CanvasHelper::useStates( const rendering::ViewState& viewState,
				  const rendering::RenderState& renderState,
				  bool setColor )
    {
	Matrix aViewMatrix;
	Matrix aRenderMatrix;
	Matrix aCombinedMatrix;

	cairo_matrix_init( &aViewMatrix,
				    viewState.AffineTransform.m00, viewState.AffineTransform.m10, viewState.AffineTransform.m01,
				    viewState.AffineTransform.m11, viewState.AffineTransform.m02, viewState.AffineTransform.m12);
	cairo_matrix_init( &aRenderMatrix,
				    renderState.AffineTransform.m00, renderState.AffineTransform.m10, renderState.AffineTransform.m01,
				    renderState.AffineTransform.m11, renderState.AffineTransform.m02, renderState.AffineTransform.m12);
	cairo_matrix_multiply( &aCombinedMatrix, &aRenderMatrix, &aViewMatrix);

        if( viewState.Clip.is() ) {
	    OSL_TRACE ("view clip\n");

	    aViewMatrix.x0 = round( aViewMatrix.x0 );
	    aViewMatrix.y0 = round( aViewMatrix.y0 );
	    cairo_set_matrix( mpCairo, &aViewMatrix );
	    drawPolyPolygonPath( viewState.Clip, Clip );
	}

	aCombinedMatrix.x0 = round( aCombinedMatrix.x0 );
	aCombinedMatrix.y0 = round( aCombinedMatrix.y0 );
	cairo_set_matrix( mpCairo, &aCombinedMatrix );

        if( renderState.Clip.is() ) {
	    OSL_TRACE ("render clip BEGIN\n");

	    drawPolyPolygonPath( renderState.Clip, Clip );
	    OSL_TRACE ("render clip END\n");
	}

	if( setColor ) {
	    if( renderState.DeviceColor.getLength() > 3 )
		cairo_set_source_rgba( mpCairo,
				       renderState.DeviceColor [0],
				       renderState.DeviceColor [1],
				       renderState.DeviceColor [2],
				       renderState.DeviceColor [3] );
	    else if (renderState.DeviceColor.getLength() == 3)
		cairo_set_source_rgb( mpCairo,
				      renderState.DeviceColor [0],
				      renderState.DeviceColor [1],
				      renderState.DeviceColor [2] );
	}
    }

    void CanvasHelper::drawPoint( const rendering::XCanvas* 	pCanvas, 
                                  const geometry::RealPoint2D& 	aPoint, 
                                  const rendering::ViewState& 	viewState, 
                                  const rendering::RenderState&	renderState )
    {
    }

    void CanvasHelper::drawLine( const rendering::XCanvas* 		pCanvas, 
                                 const geometry::RealPoint2D& 	aStartPoint, 
                                 const geometry::RealPoint2D& 	aEndPoint, 
                                 const rendering::ViewState& 	viewState, 
                                 const rendering::RenderState& 	renderState )
    {
	if( mpCairo ) {
	    cairo_save( mpCairo );

	    cairo_set_line_width( mpCairo, 1 );

	    useStates( viewState, renderState, true );

	    cairo_move_to( mpCairo, aStartPoint.X, aStartPoint.Y );
	    cairo_line_to( mpCairo, aEndPoint.X, aEndPoint.Y );
	    cairo_stroke( mpCairo );

	    cairo_restore( mpCairo );
	}
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas* 			pCanvas, 
                                   const geometry::RealBezierSegment2D&	aBezierSegment, 
                                   const geometry::RealPoint2D& 		aEndPoint,
                                   const rendering::ViewState& 			viewState, 
                                   const rendering::RenderState& 		renderState )
    {
    }

#define CANVASBITMAP_IMPLEMENTATION_NAME "CairoCanvas::CanvasBitmap"
#define PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME "Canvas::ParametricPolyPolygon"

    static Cairo* cairoFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, bool &bHasAlpha )
    {
	CanvasBitmap* pBitmapImpl = dynamic_cast< CanvasBitmap* >( xBitmap.get() );

	if( pBitmapImpl ) {
	    // TODO(Q1): Maybe use dynamic_cast here
	    bHasAlpha = pBitmapImpl->hasAlpha();

	    return pBitmapImpl->getCairo();
	}
 
	return NULL;
    }

    static ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
    {
	uno::Reference< lang::XUnoTunnel > xTunnel( xBitmap, uno::UNO_QUERY );
	if( xTunnel.is() )
            {
		sal_Int64 nPtr = xTunnel->getSomething( vcl::unotools::getTunnelIdentifier( vcl::unotools::Id_BitmapEx ) );
		if( nPtr )
		    return BitmapEx( *(BitmapEx*)nPtr );
	    }

	// TODO(F1): extract pixel from XBitmap interface
	ENSURE_AND_THROW( false, 
			  "bitmapExFromXBitmap(): could not extract BitmapEx" );
 
	return ::BitmapEx();
    }

    static bool readAlpha( BitmapReadAccess* pAlphaReadAcc, long nY, const long nWidth, unsigned char* data, long nOff )
    {
	bool bIsAlpha = false;
	long nX;
	int nAlpha;
	Scanline pReadScan;

	nOff += 3;

	switch( pAlphaReadAcc->GetScanlineFormat() ) {
	case BMP_FORMAT_8BIT_TC_MASK:
	    pReadScan = pAlphaReadAcc->GetScanline( nY );
	    for( nX = 0; nX < nWidth; nX++ ) {
		nAlpha = data[ nOff ] = 255 - ( *pReadScan++ );
		if( nAlpha != 255 )
		    bIsAlpha = true;
		nOff += 4;
	    }
	    break;
	case BMP_FORMAT_8BIT_PAL:
	    pReadScan = pAlphaReadAcc->GetScanline( nY );
	    for( nX = 0; nX < nWidth; nX++ ) {
		nAlpha = data[ nOff ] = 255 - ( pAlphaReadAcc->GetPaletteColor( *pReadScan++ ).GetBlue() );
		if( nAlpha != 255 )
		    bIsAlpha = true;
		nOff += 4;
	    }
	    break;
	default:
	    OSL_TRACE( "fallback to GetColor for alpha - slow, format: %d\n", pAlphaReadAcc->GetScanlineFormat() );
	    for( nX = 0; nX < nWidth; nX++ ) {
		nAlpha = data[ nOff ] = 255 - pAlphaReadAcc->GetColor( nY, nX ).GetBlue();
		if( nAlpha != 255 )
		    bIsAlpha = true;
		nOff += 4;
	    }
	}

	return bIsAlpha;
    }

    static Surface* surfaceFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, const DeviceRef& rDevice, unsigned char*& data, bool& bHasAlpha )
    {
	Surface* pSurface = NULL;
	Cairo* pCairo = cairoFromXBitmap( xBitmap, bHasAlpha );
	if( pCairo ) {
	    pSurface = cairo_get_target( pCairo );
	    cairo_surface_reference( pSurface );
	    data = NULL;
	} else {
	    BitmapEx aBmpEx = bitmapExFromXBitmap(xBitmap);
	    Bitmap aBitmap = aBmpEx.GetBitmap();
	    AlphaMask aAlpha = aBmpEx.GetAlpha();

	    // there's no pixmap for alpha bitmap. we might still
	    // use rgb pixmap and only access alpha pixels the
	    // slow way. now we just speedup rgb bitmaps
	    if( !aBmpEx.IsTransparent() && !aBmpEx.IsAlpha() ) {
		pSurface = rDevice->getSurface( aBitmap );
		data = NULL;
		bHasAlpha = false;
	    }

	    if( !pSurface ) {

		BitmapReadAccess*	pBitmapReadAcc = aBitmap.AcquireReadAccess();
		BitmapReadAccess*	pAlphaReadAcc = NULL;
		const long		nWidth = pBitmapReadAcc->Width();
		const long		nHeight = pBitmapReadAcc->Height();
		long nX, nY;
		bool bIsAlpha = false;

		if( aBmpEx.IsTransparent() || aBmpEx.IsAlpha() )
		    pAlphaReadAcc = aAlpha.AcquireReadAccess();

		data = (unsigned char*) malloc( nWidth*nHeight*4 );

		long nOff = 0;
		Color aColor;
		unsigned int nAlpha = 255;

		for( nY = 0; nY < nHeight; nY++ ) {
		    Scanline pReadScan;

		    switch( pBitmapReadAcc->GetScanlineFormat() ) {
		    case BMP_FORMAT_8BIT_PAL:
			pReadScan = pBitmapReadAcc->GetScanline( nY );
			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = 255;
			    aColor = pBitmapReadAcc->GetPaletteColor( *pReadScan++ );

			    data[ nOff++ ] = ( nAlpha*( aColor.GetBlue() ) )/255;
			    data[ nOff++ ] = ( nAlpha*( aColor.GetGreen() ) )/255;
			    data[ nOff++ ] = ( nAlpha*( aColor.GetRed() ) )/255;

			    nOff++;
			}
			break;
		    case BMP_FORMAT_24BIT_TC_BGR:
			pReadScan = pBitmapReadAcc->GetScanline( nY );
			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = 255;
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;

			    nOff++;
			}
			break;
		    case BMP_FORMAT_24BIT_TC_RGB:
			pReadScan = pBitmapReadAcc->GetScanline( nY );
			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = 255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 2 ] ) )/255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 1 ] ) )/255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 0 ] ) )/255;

			    nOff++;
			    pReadScan += 3;
			}
			break;
		    case BMP_FORMAT_32BIT_TC_BGRA:
			pReadScan = pBitmapReadAcc->GetScanline( nY );
			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = pReadScan[ 3 ];
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;
			    data[ nOff++ ] = ( nAlpha*( *pReadScan++ ) )/255;

			    nOff++;
			    pReadScan++;
			}
			break;
		    case BMP_FORMAT_32BIT_TC_RGBA:
			pReadScan = pBitmapReadAcc->GetScanline( nY );
			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = 255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 2 ] ) )/255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 1 ] ) )/255;
			    data[ nOff++ ] = ( nAlpha*( pReadScan[ 0 ] ) )/255;

			    nOff++;
			    pReadScan += 4;
			}
			break;
		    default:
			OSL_TRACE( "fallback to GetColor - slow, format: %d\n", pBitmapReadAcc->GetScanlineFormat() );

			if( pAlphaReadAcc )
			    if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
				bIsAlpha = true;

			for( nX = 0; nX < nWidth; nX++ ) {
			    aColor = pBitmapReadAcc->GetColor( nY, nX );

				// cairo need premultiplied color values
				// TODO(rodo) handle endianess
			    if( pAlphaReadAcc )
				nAlpha = data[ nOff + 3 ];
			    else
				nAlpha = data[ nOff + 3 ] = 255;

			    data[ nOff++ ] = ( nAlpha*aColor.GetBlue() )/255;
			    data[ nOff++ ] = ( nAlpha*aColor.GetGreen() )/255;
			    data[ nOff++ ] = ( nAlpha*aColor.GetRed() )/255;

			    nOff ++;
			}
		    }
		}
		    
		aBitmap.ReleaseAccess( pBitmapReadAcc );
		if( pAlphaReadAcc )
		    aAlpha.ReleaseAccess( pAlphaReadAcc );

		Surface* pImageSurface;
		if( bIsAlpha )
		    pImageSurface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, nWidth, nHeight, nWidth*4 );
		else
		    pImageSurface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_RGB24, nWidth, nHeight, nWidth*4 );

		// 		    WindowGraphicDevice::ImplRef xDevice = rDevice;
		// 		    pSurface = xDevice->getSimilarSurface( Size( nWidth, nHeight ), bIsAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
		// 		    Cairo* pTargetCairo = cairo_create( pSurface );
		// 		    cairo_set_source_surface( pTargetCairo, pImageSurface, 0, 0 );

		// 		    //if( !bIsAlpha )
		// 		    //cairo_set_operator( pTargetCairo, CAIRO_OPERATOR_SOURCE );

		// 		    cairo_paint( pTargetCairo );
		// 		    cairo_destroy( pTargetCairo );
		// 		    cairo_surface_destroy( pImageSurface );
		pSurface = pImageSurface;

		bHasAlpha = bIsAlpha;

		OSL_TRACE("image: %d x %d alpha: %d alphaRead %p", nWidth, nHeight, bIsAlpha, pAlphaReadAcc);
	    }
	}

	return pSurface;
    }

    static void addColorStop( Pattern* pPattern, double nOffset, const uno::Sequence< double >& rColor )
    {
	if( rColor.getLength() == 3 )
	    cairo_pattern_add_color_stop_rgb( pPattern, nOffset, rColor[0], rColor[1], rColor[2] );
	else if( rColor.getLength() == 4 )
	    cairo_pattern_add_color_stop_rgba( pPattern, nOffset, rColor[0], rColor[1], rColor[2], rColor[3] );
    }

    static Pattern* patternFromParametricPolyPolygon( ::canvas::ParametricPolyPolygon& rPolygon, Matrix& rMatrix )
    {
	Pattern* pPattern = NULL;
	const ::canvas::ParametricPolyPolygon::Values aValues = rPolygon.getValues();
	double x0, x1, y0, y1, cx, cy, r0, r1;

// undef macros from vclenum.hxx which conflicts with GradientType enum values
#undef GRADIENT_LINEAR
#undef GRADIENT_AXIAL
#undef GRADIENT_ELLIPTICAL

	switch( aValues.meType ) {
	case ::canvas::ParametricPolyPolygon::GRADIENT_LINEAR:
	    x0 = 0;
	    y0 = 0;
	    x1 = 1;
	    y1 = 0;
	    cairo_matrix_transform_point( &rMatrix, &x0, &y0 );
	    cairo_matrix_transform_point( &rMatrix, &x1, &y1 );
	    pPattern = cairo_pattern_create_linear( x0, y0, x1, y1 );
	    addColorStop( pPattern, 0, aValues.maColor1 );
	    addColorStop( pPattern, 1, aValues.maColor2 );
	    break;

	// FIXME: NYI
	case ::canvas::ParametricPolyPolygon::GRADIENT_RECTANGULAR:
	case ::canvas::ParametricPolyPolygon::GRADIENT_AXIAL:
	    x0 = 0;
	    y0 = 0;
	    x1 = 1;
	    y1 = 0;
	    cairo_matrix_transform_point( &rMatrix, &x0, &y0 );
	    cairo_matrix_transform_point( &rMatrix, &x1, &y1 );
	    pPattern = cairo_pattern_create_linear( x0, y0, x1, y1 );
	    addColorStop( pPattern, 0, aValues.maColor1 );
	    addColorStop( pPattern, 0.5, aValues.maColor2 );
	    addColorStop( pPattern, 1, aValues.maColor1 );
	    break;

	case ::canvas::ParametricPolyPolygon::GRADIENT_ELLIPTICAL:
	    cx = 0.5;
	    cy = 0.5;
	    r0 = 0;
	    r1 = 0.5;
	    Matrix* pMatrix = &rMatrix;
	    Matrix aScaledMatrix, aScaleMatrix;

	    cairo_matrix_transform_point( &rMatrix, &cx, &cy );
	    cairo_matrix_transform_distance( &rMatrix, &r0, &r1 );
	    pPattern = cairo_pattern_create_radial( cx, cy, r0, cx, cy, r1 );
	    addColorStop( pPattern, 0, aValues.maColor1 );
	    addColorStop( pPattern, 1, aValues.maColor2 );

	    if( ! ::rtl::math::approxEqual( aValues.mnAspectRatio, 1 ) ) {
		cairo_matrix_init_scale( &aScaleMatrix, 1, aValues.mnAspectRatio );
		cairo_pattern_set_matrix( pPattern, &aScaleMatrix );
	    }
	    break;
	}

	return pPattern;
    }

    void doOperation( Operation aOperation,
		      Cairo* pCairo,
		      sal_uInt32 nPolygonIndex,
		      const uno::Sequence< rendering::Texture >* pTextures,
		      SpriteCanvas* pDevice )
    {
	switch( aOperation ) {
	case Fill:
	    if( pTextures ) {
		::com::sun::star::rendering::Texture aTexture ( (*pTextures)[nPolygonIndex] );
		if( aTexture.Bitmap.is() ) {
		    unsigned char* data;
		    bool bHasAlpha;
		    Surface* pSurface = surfaceFromXBitmap( (*pTextures)[nPolygonIndex].Bitmap, pDevice, data, bHasAlpha );

		    if( pSurface ) {
			cairo_pattern_t* pPattern;

			cairo_save( pCairo );

			::com::sun::star::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
			Matrix aScaleMatrix, aTextureMatrix, aScaledTextureMatrix, aOrigMatrix, aNewMatrix;

			cairo_matrix_init( &aTextureMatrix,
					   aTransform.m00, aTransform.m10, aTransform.m01,
					   aTransform.m11, aTransform.m02, aTransform.m12);

			geometry::IntegerSize2D aSize = aTexture.Bitmap->getSize();

			cairo_matrix_init_scale( &aScaleMatrix, 1.0/aSize.Width, 1.0/aSize.Height );
			cairo_matrix_multiply( &aScaledTextureMatrix, &aTextureMatrix, &aScaleMatrix );
			cairo_matrix_invert( &aScaledTextureMatrix );

#if 0
			OSL_TRACE("slow workaround");
			// workaround for X/glitz and/or cairo bug
			// we create big enough temporary surface, copy texture bitmap there and use it for the pattern
			// it only happens on enlargening matrices with REPEAT mode enabled
			Surface* pTmpSurface = mxDevice->getSimilarSurface();
			Cairo* pTmpCairo = cairo_create( pTmpSurface );
			cairo_set_source_surface( pTmpCairo, pSurface, 0, 0 );
			cairo_paint( pTmpCairo );
			pPattern = cairo_pattern_create_for_surface( pTmpSurface );
#else

			// we don't care about repeat mode yet, so the workaround is disabled for now
			pPattern = cairo_pattern_create_for_surface( pSurface );
#endif
 			if( aTexture.RepeatModeX == rendering::TexturingMode::REPEAT &&
			    aTexture.RepeatModeY == rendering::TexturingMode::REPEAT )
			    cairo_pattern_set_extend( pPattern, CAIRO_EXTEND_REPEAT );
			aScaledTextureMatrix.x0 = round( aScaledTextureMatrix.x0 );
			aScaledTextureMatrix.y0 = round( aScaledTextureMatrix.y0 );
			cairo_pattern_set_matrix( pPattern, &aScaledTextureMatrix );

			cairo_set_source( pCairo, pPattern );
			if( !bHasAlpha )
			    cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
			cairo_fill( pCairo );

			cairo_restore( pCairo );

			cairo_pattern_destroy( pPattern );
			cairo_surface_destroy( pSurface );

#if 0
			cairo_destroy( pTmpCairo );
			cairo_surface_destroy( pTmpSurface );
#endif
		    }

		    if( data )
			free( data );
		} else if( aTexture.Gradient.is() ) {
		    uno::Reference< lang::XServiceInfo > xRef( aTexture.Gradient, uno::UNO_QUERY );

		    OSL_TRACE( "gradient fill\n" );
		    if( xRef.is() && 
			xRef->getImplementationName().equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME ) ) ) ) {
				// TODO(Q1): Maybe use dynamic_cast here
				
				// TODO(E1): Return value
				// TODO(F1): FillRule
			OSL_TRACE( "known implementation\n" );

			::canvas::ParametricPolyPolygon* pPolyImpl = static_cast< ::canvas::ParametricPolyPolygon* >( aTexture.Gradient.get() );
			::com::sun::star::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
			Matrix aTextureMatrix;

			cairo_matrix_init( &aTextureMatrix,
					   aTransform.m00, aTransform.m10, aTransform.m01,
					   aTransform.m11, aTransform.m02, aTransform.m12);
			Pattern* pPattern = patternFromParametricPolyPolygon( *pPolyImpl, aTextureMatrix );

			if( pPattern ) {
			    OSL_TRACE( "filling with pattern\n" );

			    cairo_save( pCairo );

			    cairo_set_source( pCairo, pPattern );
			    cairo_fill( pCairo );
			    cairo_restore( pCairo );

			    cairo_pattern_destroy( pPattern );
			}
		    }
		}
	    } else
		cairo_fill( pCairo );
	    OSL_TRACE("fill\n");
	break;
	case Stroke:
	    cairo_stroke( pCairo );
	    OSL_TRACE("stroke\n");
	break;
	case Clip:
	    cairo_clip( pCairo );
	    OSL_TRACE("clip\n");
	break;
	}
    }

    static void clipNULL( Cairo *pCairo )
    {
	OSL_TRACE("clipNULL\n");
	Matrix aOrigMatrix, aIdentityMatrix;

	/* we set identity matrix here to overcome bug in cairo 0.9.2
	   where XCreatePixmap is called with zero width and height.

	   it also reaches faster path in cairo clipping code.
	*/
	cairo_matrix_init_identity( &aIdentityMatrix );
	cairo_get_matrix( pCairo, &aOrigMatrix );
	cairo_set_matrix( pCairo, &aIdentityMatrix );

	cairo_reset_clip( pCairo );
	cairo_rectangle( pCairo, 0, 0, 1, 1 );
	cairo_clip( pCairo );
	cairo_rectangle( pCairo, 2, 0, 1, 1 );
	cairo_clip( pCairo );

	/* restore the original matrix */
	cairo_set_matrix( pCairo, &aOrigMatrix );
    }

    void drawPolyPolygonImplementation( ::basegfx::B2DPolyPolygon aPolyPolygon,
					Operation aOperation,
					Cairo* pCairo,
					const uno::Sequence< rendering::Texture >* pTextures,
					SpriteCanvas* pDevice )
    {
	bool bOpToDo = false;
	Matrix aOrigMatrix, aIdentityMatrix;
	double nX, nY, nBX, nBY, nPX, nPY;

	cairo_get_matrix( pCairo, &aOrigMatrix );
	cairo_matrix_init_identity( &aIdentityMatrix );
	cairo_set_matrix( pCairo, &aIdentityMatrix );

	for( sal_uInt32 nPolygonIndex = 0; nPolygonIndex < aPolyPolygon.count(); nPolygonIndex++ ) {
	    ::basegfx::B2DPolygon aPolygon = aPolyPolygon.getB2DPolygon( nPolygonIndex );

	    if( aPolygon.count() > 1) {
		bool bIsBezier = aPolygon.areControlPointsUsed();
		::basegfx::B2DPoint aA, aB, aP;

		aP = aPolygon.getB2DPoint( 0 );
		nX = aP.getX();
		nY = aP.getY();
		cairo_matrix_transform_point( &aOrigMatrix, &nX, &nY );
		if( bIsBezier )
		    cairo_move_to( pCairo, nX, nY );
		else
		    cairo_move_to( pCairo, round(nX), round(nY) );
		OSL_TRACE( "move to %f,%f\n", round(nX), round(nY) );

		if( bIsBezier ) {
		    aA = aPolygon.getControlPointA( 0 );
		    aB = aPolygon.getControlPointB( 0 );
		}

		for( sal_uInt32 j = 1; j < aPolygon.count(); j++ ) {
		    aP = aPolygon.getB2DPoint( j );

		    nX = aP.getX();
		    nY = aP.getY();
		    cairo_matrix_transform_point( &aOrigMatrix, &nX, &nY );

		    if( bIsBezier ) {
			nBX = aB.getX();
			nBY = aB.getY();
			nPX = aP.getX();
			nPY = aP.getY();
			cairo_matrix_transform_point( &aOrigMatrix, &nBX, &nBY );
			cairo_matrix_transform_point( &aOrigMatrix, &nPX, &nPY );
			cairo_curve_to( pCairo, nX, nY, nBX, nBY, nPX, nPY );
			
			aA = aPolygon.getControlPointA( j );
			aB = aPolygon.getControlPointB( j );
		    } else {
			cairo_line_to( pCairo, round(nX), round(nY) );
			OSL_TRACE( "line to %f,%f\n", round(nX), round(nY) );
		    }
		    bOpToDo = true;
		}

		if( aPolygon.isClosed() )
		    cairo_close_path( pCairo );

		if( aOperation == Fill && pTextures )
		    doOperation( aOperation, pCairo, nPolygonIndex, pTextures );
	    } else {
		OSL_TRACE( "empty polygon for op: %d\n\n", aOperation );
		if( aOperation == Clip ) {
		    clipNULL( pCairo );

		    return;
		}
	    }
	}
	if( bOpToDo && ( aOperation != Fill || !pTextures ) )
	    doOperation( aOperation, pCairo );

	cairo_set_matrix( pCairo, &aOrigMatrix );

// fixme, spec says even clipping polypolygon with zero polygons means NULL clip, but it breaks animations with sprites
// 	if( aPolyPolygon.count() == 0 && aOperation == Clip )
// 	    clipNULL( pCairo );
    }

    void CanvasHelper::drawPolyPolygonPath( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
					    Operation aOperation,
					    const uno::Sequence< rendering::Texture >* pTextures,
					    Cairo* pCairo ) const
    {
	::basegfx::B2DPolyPolygon aPoly = ::canvas::tools::polyPolygonFromXPolyPolygon2D( xPolyPolygon );

	if( !pCairo )
	    pCairo = mpCairo;

	drawPolyPolygonImplementation( aPoly, aOperation, pCairo, pTextures );
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, 
                                                                                 const rendering::ViewState& 						viewState, 
                                                                                 const rendering::RenderState& 						renderState )
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

	if( mpCairo ) {
	    cairo_save( mpCairo );

	    cairo_set_line_width( mpCairo, 1 );

	    useStates( viewState, renderState, true );
	    drawPolyPolygonPath( xPolyPolygon, Stroke );
	    
	    cairo_restore( mpCairo );
	} else
	    OSL_TRACE ("CanvasHelper called after it was disposed");

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "drawPolyPolygon" );
        #endif

	return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >& 	xPolyPolygon, 
                                                                                   const rendering::ViewState& 							viewState, 
                                                                                   const rendering::RenderState& 						renderState, 
                                                                                   const rendering::StrokeAttributes& 					strokeAttributes )
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

	if( mpCairo ) {
	    cairo_save( mpCairo );

	    useStates( viewState, renderState, true );

	    cairo_set_line_width( mpCairo, strokeAttributes.StrokeWidth );
	    cairo_set_miter_limit( mpCairo, strokeAttributes.MiterLimit );

	    // FIXME: cairo doesn't handle end cap so far (rodo)
	    switch( strokeAttributes.StartCapType ) {
	    case rendering::PathCapType::BUTT:
		cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_BUTT );
		break;
	    case rendering::PathCapType::ROUND:
		cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_ROUND );
		break;
	    case rendering::PathCapType::SQUARE:
		cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_SQUARE );
		break;
	    }

	    switch( strokeAttributes.JoinType ) {
		// cairo doesn't have join type NONE so we use MITTER as it's pretty close
	    case rendering::PathJoinType::NONE:
	    case rendering::PathJoinType::MITER:
		cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_MITER );
		break;
	    case rendering::PathJoinType::ROUND:
		cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_ROUND );
		break;
	    case rendering::PathJoinType::BEVEL:
		cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_BEVEL );
		break;
	    }

	    if( strokeAttributes.DashArray.getLength() > 0 ) {
		double* pDashArray = new double[ strokeAttributes.DashArray.getLength() ];
		for( sal_Int32 i=0; i<strokeAttributes.DashArray.getLength(); i++ )
		    pDashArray[i]=strokeAttributes.DashArray[i];
		cairo_set_dash( mpCairo, pDashArray, strokeAttributes.DashArray.getLength(), 0 );
		delete[] pDashArray;
	    }

	    // TODO(rodo) use LineArray of strokeAttributes

	    drawPolyPolygonPath( xPolyPolygon, Stroke );

	    cairo_restore( mpCairo );
	} else
	    OSL_TRACE ("CanvasHelper called after it was disposed");

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "strokePolyPolygon" );
        #endif

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >& 	xPolyPolygon, 
                                                                                           const rendering::ViewState& 							viewState, 
                                                                                           const rendering::RenderState& 						renderState, 
                                                                                           const uno::Sequence< rendering::Texture >& 			textures, 
                                                                                           const rendering::StrokeAttributes& 					strokeAttributes )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&	xPolyPolygon, 
                                                                                                const rendering::ViewState& 						viewState, 
                                                                                                const rendering::RenderState& 						renderState, 
                                                                                                const uno::Sequence< rendering::Texture >& 			textures, 
                                                                                                const uno::Reference< geometry::XMapping2D >& 		xMapping, 
                                                                                                const rendering::StrokeAttributes& 					strokeAttributes )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas* 							pCanvas, 
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >& 	xPolyPolygon, 
                                                                                   const rendering::ViewState& 							viewState, 
                                                                                   const rendering::RenderState& 						renderState, 
                                                                                   const rendering::StrokeAttributes& 					strokeAttributes )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, 
                                                                                 const rendering::ViewState& 						viewState, 
                                                                                 const rendering::RenderState& 						renderState )
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

	if( mpCairo ) {
	    cairo_save( mpCairo );

	    useStates( viewState, renderState, true );
	    drawPolyPolygonPath( xPolyPolygon, Fill );
	    
	    cairo_restore( mpCairo );
	} else
	    OSL_TRACE ("CanvasHelper called after it was disposed");

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "fillPolyPolygon" );
        #endif

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState& 						viewState,
                                                                                         const rendering::RenderState& 						renderState,
                                                                                         const uno::Sequence< rendering::Texture >& 		textures )
    {
	if( mpCairo ) {
	    cairo_save( mpCairo );

	    useStates( viewState, renderState, true );
	    drawPolyPolygonPath( xPolyPolygon, Fill, &textures );

	    cairo_restore( mpCairo );
	}

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas* 							pCanvas, 
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >& 	xPolyPolygon, 
                                                                                              const rendering::ViewState& 							viewState, 
                                                                                              const rendering::RenderState& 						renderState, 
                                                                                              const uno::Sequence< rendering::Texture >& 			textures, 
                                                                                              const uno::Reference< geometry::XMapping2D >& 		xMapping )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCanvasFont > CanvasHelper::createFont( const rendering::XCanvas* 					pCanvas, 
                                                                       const rendering::FontRequest& 				fontRequest, 
                                                                       const uno::Sequence< beans::PropertyValue >& extraFontProperties, 
                                                                       const geometry::Matrix2D& 					fontMatrix )
    {
        return uno::Reference< rendering::XCanvasFont >( 
            new CanvasFont(fontRequest, extraFontProperties, fontMatrix ) );
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas* 						pCanvas, 
                                                                            const rendering::FontInfo& 						aFilter, 
                                                                            const uno::Sequence< beans::PropertyValue >& 	aFontProperties )
    {
        // TODO
        return uno::Sequence< rendering::FontInfo >();
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas* 						pCanvas, 
                                                                          const rendering::StringContext& 					text, 
                                                                          const uno::Reference< rendering::XCanvasFont >& 	xFont, 
                                                                          const rendering::ViewState& 						viewState, 
                                                                          const rendering::RenderState& 					renderState, 
                                                                          sal_Int8				 							textDirection )
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

	if ( mpCairo ) {
	    cairo_save( mpCairo );

	    useStates( viewState, renderState, true );

	    // fixme (rodo) - use xFont
	    cairo_select_font_face( mpCairo, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	    /* move to 0, 0 as cairo_show_text advances current point and current point is not restored by cairo_restore.
	       before we were depending on unmodified current point which I believed was preserved by save/restore */
	    cairo_move_to( mpCairo, 0, 0 );
	    cairo_show_text( mpCairo, ::rtl::OUStringToOString( text.Text, RTL_TEXTENCODING_UTF8 ) );

	    cairo_restore( mpCairo );
	} else
	    OSL_TRACE ("CanvasHelper called after it was disposed");

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "drawText" );
        #endif

	return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawTextLayout( const rendering::XCanvas* 						pCanvas, 
                                                                                const uno::Reference< rendering::XTextLayout >& xLayoutetText, 
                                                                                const rendering::ViewState& 					viewState, 
                                                                                const rendering::RenderState& 					renderState )
    {
        ENSURE_AND_THROW( xLayoutetText.is(), 
                          "CanvasHelper::drawTextLayout: layout is NULL");

	if( mpCairo ) {
	    TextLayout* pTextLayout = 
		dynamic_cast< TextLayout* >( xLayoutetText.get() );
        
	    ENSURE_AND_THROW( pTextLayout,
			      "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );

	    useStates( viewState, renderState, true );

	    pTextLayout->draw( mpCairo );
	}

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::implDrawBitmapSurface( Surface* pSurface,
										       const rendering::ViewState& viewState,
										       const rendering::RenderState& renderState,
										       bool bModulateColors,
										       bool bHasAlpha )
    {
	if( mpCairo ) {
	    cairo_save( mpCairo );

 	    cairo_rectangle( mpCairo, 0, 0, maSize.getX(), maSize.getY() );
 	    cairo_clip( mpCairo );

	    useStates( viewState, renderState, true );

//   	    if( !bHasAlpha )
//   		cairo_set_operator( mpCairo, CAIRO_OPERATOR_SOURCE );

	    cairo_set_source_surface( mpCairo, pSurface, 0, 0 );
	    cairo_paint( mpCairo );
	    cairo_restore( mpCairo );
	} else
	    OSL_TRACE ("CanvasHelper called after it was disposed");

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas* 					pCanvas, 
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap, 
                                                                            const rendering::ViewState& 				viewState, 
                                                                            const rendering::RenderState& 				renderState )
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

	uno::Reference< rendering::XCachedPrimitive > rv;
	unsigned char* data;
	bool bHasAlpha;
	Surface* pSurface = surfaceFromXBitmap( xBitmap, mpDevice, data, bHasAlpha );

	if( pSurface ) {
	    rv = implDrawBitmapSurface( pSurface, viewState, renderState, false, bHasAlpha );

	    cairo_surface_destroy( pSurface );

	    if( data )
		free( data );
	} else
	    rv = uno::Reference< rendering::XCachedPrimitive >(NULL);

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "drawBitmap" );
        #endif

	return rv;
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas* 						pCanvas, 
                                                                                     const uno::Reference< rendering::XBitmap >& 	xBitmap, 
                                                                                     const rendering::ViewState& 					viewState, 
                                                                                     const rendering::RenderState& 					renderState )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XGraphicDevice > CanvasHelper::getDevice()
    {
        return uno::Reference< rendering::XGraphicDevice >(mpDevice);
    }

    void CanvasHelper::copyRect( const rendering::XCanvas* 							pCanvas, 
                                 const uno::Reference< rendering::XBitmapCanvas >&	sourceCanvas, 
                                 const geometry::RealRectangle2D& 					sourceRect, 
                                 const rendering::ViewState& 						sourceViewState, 
                                 const rendering::RenderState& 						sourceRenderState, 
                                 const geometry::RealRectangle2D& 					destRect, 
                                 const rendering::ViewState& 						destViewState, 
                                 const rendering::RenderState& 						destRenderState )
    {
        // TODO(F2): copyRect NYI
    }

    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpDevice )
            geometry::IntegerSize2D(1, 1); // we're disposed

        return ::basegfx::unotools::integerSize2DFromB2ISize( maSize );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D&	newSize, 
                                                                        sal_Bool 					beFast )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     bitmapLayout, 
                                                     const geometry::IntegerRectangle2D& rect )
    {
        // TODO
        return uno::Sequence< sal_Int8 >();
    }

    void CanvasHelper::setData( const uno::Sequence< sal_Int8 >& 		data, 
                                const rendering::IntegerBitmapLayout&   bitmapLayout, 
                                const geometry::IntegerRectangle2D& 	rect )
    {
    }

    void CanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       color, 
                                 const rendering::IntegerBitmapLayout&  bitmapLayout, 
                                 const geometry::IntegerPoint2D&        pos )
    {
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout&   bitmapLayout, 
                                                      const geometry::IntegerPoint2D&   pos )
    {
        return uno::Sequence< sal_Int8 >();
    }

    uno::Reference< rendering::XBitmapPalette > CanvasHelper::getPalette()
    {
        // TODO(F1): Palette bitmaps NYI
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        // TODO(F1): finish memory layout initialization
        rendering::IntegerBitmapLayout aLayout;

        const geometry::IntegerSize2D& rBmpSize( getSize() );

        aLayout.ScanLines = rBmpSize.Width;
        aLayout.ScanLineBytes = rBmpSize.Height * 4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        aLayout.ColorSpace.set( mpDevice ); 
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks.realloc(4);
        aLayout.ComponentMasks[0] = 0x00FF0000;
        aLayout.ComponentMasks[1] = 0x0000FF00;
        aLayout.ComponentMasks[2] = 0x000000FF;
        aLayout.ComponentMasks[3] = 0xFF000000;
        aLayout.Palette.clear();
        aLayout.Endianness = rendering::Endianness::LITTLE;
        aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
        aLayout.IsMsbFirst = sal_False;

        return aLayout;
    }

    void CanvasHelper::flush() const
    {
    }

    bool CanvasHelper::hasAlpha() const
    {
        return mbHaveAlpha;
    }

}
