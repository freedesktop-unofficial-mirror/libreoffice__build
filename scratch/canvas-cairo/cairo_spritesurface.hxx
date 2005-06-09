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

#ifndef _VCLCANVAS_SPRITESURFACE_HXX
#define _VCLCANVAS_SPRITESURFACE_HXX

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#include "cairo_sprite.hxx"

class Point;
class Size;
class Rectangle;

namespace vclcanvas
{
	/* Definition of SpriteSurface interface */
    
    class SpriteSurface
    {
    public:
        virtual ~SpriteSurface() {}

        // call this when XSprite::show() is called
        virtual void showSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::hide() is called
        virtual void hideSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::move() is called
        virtual void moveSprite( const Sprite::ImplRef&	sprite, 
                                 const Point& 			rOldPos,
                                 const Point& 			rNewPos,
                                 const Size& 			rSpriteSize ) = 0;

        // call this when some part of your sprite has changed. That
        // includes show/hide�, i.e. for show, both showSprite and
        // updateSprite must be called.
        virtual void updateSprite( const Sprite::ImplRef& 	sprite, 
                                   const Point& 			rPos,
                                   const Rectangle& 		rUpdateArea ) = 0;
        
    };
}

#endif /* _VCLCANVAS_SPRITESURFACE_HXX */
