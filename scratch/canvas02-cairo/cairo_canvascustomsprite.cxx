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

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritecanvas.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D& 	rSpriteSize,
                                            const SpriteCanvasRef&                          rRefDevice ) :
        mpSpriteCanvas( rRefDevice )
    {
        ENSURE_AND_THROW( rRefDevice.get(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

	::basegfx::B2ISize aSize( ::canvas::tools::roundUp( rSpriteSize.Width ),
				  ::canvas::tools::roundUp( rSpriteSize.Height ) );

	mpBufferSurface = mpSpriteCanvas->getSurface( aSize );
	mpCairo = cairo_create( mpBufferSurface );

        maCanvasHelper.init( aSize,
                             *rRefDevice.get(),
			     mpCairo,
                             true );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice,
			     mpBufferSurface );
    }
    
    void SAL_CALL CanvasCustomSprite::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();

	if( mpCairo ) {
	    cairo_destroy( mpCairo );
	    mpCairo = NULL;
	}

	if( mpBufferSurface ) {
	    cairo_surface_destroy( mpBufferSurface );
	    mpBufferSurface = NULL;
	}

        // forward to parent
        CanvasCustomSpriteBaseT::disposing();
    }

    void CanvasCustomSprite::redraw( Cairo* pCairo,
				     bool bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        redraw( pCairo, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( Cairo* pCairo,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
				     bool bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( pCairo,
                               rOrigOutputPos, 
                               mbSurfaceDirty, 
                               bBufferedUpdate );

        mbSurfaceDirty = false;
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasCustomSprite"
#define SERVICE_NAME "com.sun.star.rendering.CanvasCustomSprite"

    ::rtl::OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
        
        return aRet;
    }
}
