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

#ifndef _CAIROCANVAS_SPRITEHELPER_HXX
#define _CAIROCANVAS_SPRITEHELPER_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <canvas/base/canvascustomspritehelper.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include "cairo_spritecanvas.hxx"


namespace cairocanvas
{
	/* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

    	This class implements all sprite-related functionality, like
    	that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        /** Create sprite helper
         */
        SpriteHelper();

        /** Late-init the sprite helper

        	@param rSpriteSize
            Size of the sprite

            @param rSpriteCanvas
            Sprite canvas this sprite is part of. Object stores
            ref-counted reference to it, thus, don't forget to pass on
            disposing()!

            @param rDevice
            DX device to use

            @param rSpriteSurface
            The surface of the sprite (not the DX texture, but the
            persistent target of content rendering)

            @param bShowSpriteBounds
            When true, little debug bound rects for sprites are shown
         */
        void init( const ::com::sun::star::geometry::RealSize2D&	rSpriteSize,
                   const SpriteCanvasRef&							rSpriteCanvas,
		   ::cairo::Surface* pBufferSurface);

        void disposing();

        /** Repaint sprite content to associated sprite canvas

        	@param rPos
            Output position (sprite's own position is disregarded)

            @param io_bSurfacesDirty
            When true, the referenced sprite surfaces (backBuffer and
            backBufferMask) have been modified since last call.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        void redraw( ::cairo::Cairo*            pCairo,
                     const ::basegfx::B2DPoint& rPos,
                     bool& 						bSurfacesDirty,
                     bool                       bBufferedUpdate ) const;
        
    private:
        virtual void clearSurface();
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( 
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const;


        SpriteCanvasRef				mpSpriteCanvas;
        mutable bool                mbTextureDirty;  // when true, texture needs update

	::cairo::Surface* mpBufferSurface;
    };
}

#endif /* _CAIROCANVAS_SPRITEHELPER_HXX */
