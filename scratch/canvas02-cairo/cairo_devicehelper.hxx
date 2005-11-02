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

#ifndef _CAIROCANVAS_DEVICEHELPER_HXX
#define _CAIROCANVAS_DEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <boost/utility.hpp>

#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>

#include "cairo_cairo.hxx"

/* Definition of DeviceHelper class */

namespace cairocanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class DeviceHelper : private ::boost::noncopyable
    {
    public:
        DeviceHelper();

        void init( Window& 				rOutputWindow,
		   SpriteCanvas&				rSpriteCanvas,
                   const ::basegfx::B2ISize&	rSize,
                   bool                         bFullscreen );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::com::sun::star::geometry::RealSize2D getPhysicalResolution();
        ::com::sun::star::geometry::RealSize2D getPhysicalSize();
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 								 rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 								 		 rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleBitmap( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 	rDevice,
            const ::com::sun::star::geometry::IntegerSize2D& 										size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileBitmap( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 	rDevice,
            const ::com::sun::star::geometry::IntegerSize2D& 										size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleAlphaBitmap( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 	rDevice,
            const ::com::sun::star::geometry::IntegerSize2D& 										size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileAlphaBitmap( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& 	rDevice,
            const ::com::sun::star::geometry::IntegerSize2D& 										size );
        sal_Bool hasFullScreenMode(  );
        sal_Bool enterFullScreenMode( sal_Bool bEnter );

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void 		destroyBuffers(  );
        ::sal_Bool  showBuffer( ::sal_Bool bUpdateAll );
        ::sal_Bool  switchBuffer( ::sal_Bool bUpdateAll );

        const void* getDeviceHandle() const;
        const void* getSurfaceHandle() const;

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );
	void setSize( const ::basegfx::B2ISize&	rSize );

	::cairo::Surface* getBufferSurface();
	::cairo::Surface* getWindowSurface();
	::cairo::Surface* getSurface( const ::basegfx::B2ISize& rSize, ::cairo::Content aContent = ::cairo::CAIRO_CONTENT_COLOR_ALPHA );
	::cairo::Surface* getSurface( ::cairo::Content aContent = ::cairo::CAIRO_CONTENT_COLOR_ALPHA );
	::cairo::Surface* getSurface( BitmapSystemData& rData, const Size& rSize );
	const ::basegfx::B2ISize& getSizePixel();
	void flush();

    private:
        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*			mpSpriteCanvas;
        ::basegfx::B2ISize      maSize;
        bool                    mbFullScreen;

        // TODO(Q3): Lifetime issue. Cannot control pointer validity
        // over object lifetime, since we're a UNO component. Now that
        // we've changed the ::Window canvas reference to a weak ref,
        // might be okay to hold a uno::Reference to the VCL window
        // here.
        Window* mpOutputWindow;

        const SystemEnvData* mpSysData;
        ::cairo::Surface* mpWindowSurface;
        ::cairo::Surface* mpBufferSurface;
	::cairo::Cairo*   mpBufferCairo;
    };
}

#endif /* _CAIROCANVAS_WINDOWGRAPHICDEVICE_HXX */
