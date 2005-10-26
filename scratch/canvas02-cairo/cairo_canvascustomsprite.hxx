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

#ifndef _CAIROCANVAS_CANVASCUSTOMSPRITE_HXX
#define _CAIROCANVAS_CANVASCUSTOMSPRITE_HXX

#include <cppuhelper/compbase4.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/canvascustomspritebase.hxx>

#include "sprite.hxx"
#include "cairo_cairo.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_spritehelper.hxx"
#include "cairo_spritecanvas.hxx"
#include "cairo_usagecounter.hxx"


namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::rendering::XCustomSprite,
									 		  ::com::sun::star::rendering::XBitmapCanvas,
											  ::com::sun::star::rendering::XIntegerBitmap,
                         			 		  ::com::sun::star::lang::XServiceInfo >	CanvasCustomSpriteBase_Base;
	/** Mixin Sprite

    	Have to mixin the Sprite interface before deriving from
    	::canvas::CanvasCustomSpriteBase, as this template should
    	already implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelperN
        template args, and furthermore, don't want to derive
        ::canvas::CanvasCustomSpriteBase directly from
        ::canvas::Sprite (because derivees of
        ::canvas::CanvasCustomSpriteBase have to explicitely forward
        the XInterface methods (e.g. via DECLARE_UNO3_AGG_DEFAULTS)
        anyway). Basically, ::canvas::CanvasCustomSpriteBase should
        remain a base class that provides implementation, not to
        enforce any specific interface on its derivees.
     */
	class CanvasCustomSpriteSpriteBase_Base : public ::canvas::BaseMutexHelper< CanvasCustomSpriteBase_Base >,
    									   	  public Sprite
    {
    };

	typedef ::canvas::CanvasCustomSpriteBase< CanvasCustomSpriteSpriteBase_Base,
                                              SpriteHelper, 
                                              CanvasHelper, 
                                              ::osl::MutexGuard,
                                              ::cppu::OWeakObject > 					CanvasCustomSpriteBaseT;

	/* Definition of CanvasCustomSprite class */

    class CanvasCustomSprite : public CanvasCustomSpriteBaseT,
                               private UsageCounter< CanvasCustomSprite >
    {
    public:
        /** Create a custom sprite

			@param rSpriteSize
            Size of the sprite in pixel

            @param rRefDevice
            Associated output device

            @param rSpriteCanvas
            Target canvas

            @param rDevice
            Target DX device
         */
        CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D& 	rSpriteSize,
                            const SpriteCanvasRef&                          rRefDevice );

        virtual void SAL_CALL disposing();

		// Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname           Base doing refcount          Base implementing the XComponent interface
        //                                          |                    |                         |
        //                                          V                    V                         V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( CanvasCustomSprite, CanvasCustomSpriteBase_Base, ::cppu::WeakComponentImplHelperBase );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // Sprite
        virtual void redraw( ::cairo::Cairo* pCairo,
			     bool bBufferedUpdate ) const;
	virtual void redraw( ::cairo::Cairo* pCairo,
			     const ::basegfx::B2DPoint& rOrigOutputPos,
			     bool bBufferedUpdate ) const;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting) 
        */
        SpriteCanvasRef mpSpriteCanvas;
	::cairo::Surface*       mpBufferSurface;
	::cairo::Cairo*         mpCairo;
    };
}

#endif /* _CAIROCANVAS_CANVASCUSTOMSPRITE_HXX */
