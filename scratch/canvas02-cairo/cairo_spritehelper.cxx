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
#include <canvas/verbosetrace.hxx>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritehelper.hxx"

#include <memory>


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    SpriteHelper::SpriteHelper() :
        mpSpriteCanvas(),
        mbTextureDirty( true ),
	mpBufferSurface( NULL )
    {
    }
    
    void SpriteHelper::init( const geometry::RealSize2D&	rSpriteSize,
                             const SpriteCanvasRef&			rSpriteCanvas,
			     Surface* pBufferSurface )
    {
        ENSURE_AND_THROW( rSpriteCanvas.get(),
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mbTextureDirty     = true;

        // also init base class
        CanvasCustomSpriteHelper::init( rSpriteSize,
                                        rSpriteCanvas.get() );

	if( mpBufferSurface )
	    cairo_surface_destroy( mpBufferSurface );
	mpBufferSurface = pBufferSurface;
	cairo_surface_reference( mpBufferSurface );
    }

    void SpriteHelper::disposing()
    {
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();

	if( mpBufferSurface ) {
	    cairo_surface_destroy( mpBufferSurface );
	    mpBufferSurface = NULL;
	}
    }

    void SpriteHelper::redraw( Cairo*                     pCairo,
                               const ::basegfx::B2DPoint& rPos,
                               bool&                      io_bSurfacesDirty,
                               bool                       bBufferedUpdate ) const
    {
	#ifdef CAIRO_CANVAS_PERF_TRACE
	struct timespec aTimer;
	mxDevice->startPerfTrace( &aTimer );
        #endif

        const double fAlpha( getAlpha() );
	const ::basegfx::B2DHomMatrix aTransform( getTransformation() );

	if( isActive() && !::basegfx::fTools::equalZero( fAlpha ) ) {
	    OSL_TRACE ("CanvasCustomSprite::redraw called\n");
	    if( pCairo ) {

		basegfx::B2DVector aSize = getSizePixel();
		cairo_save( pCairo );

		double fX, fY;

		fX = rPos.getX();
		fY = rPos.getY();

 		if( !aTransform.isIdentity() ) {
 		    cairo_matrix_t aMatrix, aInverseMatrix;
 		    cairo_matrix_init( &aMatrix,
 				       aTransform.get( 0, 0 ), aTransform.get( 1, 0 ), aTransform.get( 0, 1 ),
 				       aTransform.get( 1, 1 ), aTransform.get( 0, 2 ), aTransform.get( 1, 2 ) );

		    aMatrix.x0 = round( aMatrix.x0 );
		    aMatrix.y0 = round( aMatrix.y0 );

		    cairo_matrix_init( &aInverseMatrix, aMatrix.xx, aMatrix.yx, aMatrix.xy, aMatrix.yy, aMatrix.x0, aMatrix.y0 );
		    cairo_matrix_invert( &aInverseMatrix );
		    cairo_matrix_transform_distance( &aInverseMatrix, &fX, &fY );


 		    cairo_set_matrix( pCairo, &aMatrix );
 		}

// 		fX = floor( fX );
// 		fY = floor( fY );

		cairo_matrix_t aOrigMatrix;
		cairo_get_matrix( pCairo, &aOrigMatrix );
		cairo_translate( pCairo, round(fX), round(fY) );

                if( getClip().is() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly(
                        ::canvas::tools::polyPolygonFromXPolyPolygon2D( 
                            getClip() ));

		    cairo_reset_clip( pCairo );
		    drawPolyPolygonImplementation( aClipPoly, Clip, pCairo );
		}

		OSL_TRACE ("aSize %d x %d\n", aSize.getX(), aSize.getY() );
		cairo_rectangle( pCairo, 0, 0, ceil( aSize.getX() ), ceil( aSize.getY() ) );
		cairo_clip( pCairo );
		cairo_set_matrix( pCairo, &aOrigMatrix );

		if( isContentFullyOpaque() )
		    cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
		cairo_set_source_surface( pCairo, mpBufferSurface, fX, fY );
                if( ::rtl::math::approxEqual( fAlpha, 1.0 ) )
		    cairo_paint( pCairo );
		else
		    cairo_paint_with_alpha( pCairo, fAlpha );

		cairo_restore( pCairo );
	    }
	}

	#ifdef CAIRO_CANVAS_PERF_TRACE
	mxDevice->stopPerfTrace( &aTimer, "sprite redraw" );
        #endif
    }

    void SpriteHelper::clearSurface()
    {
	Cairo* pCairo = cairo_create( mpBufferSurface );
	const ::basegfx::B2DVector& rSize( getSizePixel() );

	cairo_set_source_rgb( pCairo, 1, 1, 1 );
	cairo_rectangle( pCairo, 0, 0, rSize.getX(), rSize.getY() );
	cairo_fill( pCairo );

	cairo_destroy( pCairo );
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::canvas::tools::polyPolygonFromXPolyPolygon2D( xPoly );
    }
}
