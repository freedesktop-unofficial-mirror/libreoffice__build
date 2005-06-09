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

#ifndef _VCLCANVAS_REDRAWMANAGER_HXX
#define _VCLCANVAS_REDRAWMANAGER_HXX

#include <list>
#include <vector>

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_RANGE_B2DCONNECTEDRANGES_HXX
#include <basegfx/range/b2dconnectedranges.hxx>
#endif

#include <canvas/vclwrapper.hxx>
#include <canvas/elapsedtime.hxx>
#include <canvas/redrawmanagement.hxx>

#include "cairo_sprite.hxx"
#include "cairo_backbuffer.hxx"
#include "cairo_impltools.hxx"


/* Definition of RedrawManager class */

namespace vclcanvas
{

    /** This class manages smooth SpriteCanvas updates

		@attention This class does not perform Solar mutex locking,
		this must be handled by the client code.
     */
    class RedrawManager
    {
    public:
        /** Constructed with references to back buffer and output
            device, the RedrawManager keeps track of sprite movements,
            and repaints only those areas on updateScreen, that
            actually needs an update.

            @param rOutDev
            Reference to output device, typically a window on
            screen. Does not take ownership.

            @param rBackBuffer
            Reference to a back buffer. Back buffer content must be
            provided from elsewhere, everytime back buffer content
            changes, backgroundDirty() must be called.
         */
        RedrawManager( OutputDevice&				rOutDev,
                       const BackBufferSharedPtr& 	rBackBuffer );

        void updateScreen( bool bUpdateAll );
        void backgroundDirty();

        // SpriteSurface interface, is delegated to e.g. from SpriteCanvas
        void showSprite( const Sprite::ImplRef& sprite );
        void hideSprite( const Sprite::ImplRef& sprite );
        void moveSprite( const Sprite::ImplRef& sprite, 
                         const Point& 			rOldPos,
                         const Point& 			rNewPos,
                         const Size& 			rSpriteSize );
        void updateSprite( const Sprite::ImplRef& 	sprite, 
                           const Point& 			rPos,
                           const Rectangle& 		rUpdateArea );

        typedef ::basegfx::B2DConnectedRanges< Sprite::ImplRef >			SpriteConnectedRanges;

    private:
        typedef ::canvas::internal::SpriteChangeRecord< Sprite::ImplRef > 	SpriteChangeRecord;
        typedef ::std::vector< SpriteChangeRecord > 						ChangeRecords;

        // default: disabled copy/assignment
        RedrawManager(const RedrawManager&);
        RedrawManager& operator=( const RedrawManager& );

        /// Update given area of the screen
        void updateArea( const SpriteConnectedRanges::ConnectedComponents& rComponents );

        /// Update sprite in given area of the outdev
        void updateSpriteForArea( const Rectangle& 								rUpdateArea, 
                                  OutputDevice&									rOutputDevice,
                                  const SpriteConnectedRanges::ComponentType&	rComponent );
            
        void renderInfoText( const ::rtl::OUString& rStr,
                             const Point&		   	rPos );
        void renderFrameCounter();
        void renderSpriteCount();
        void renderMemUsage();

        ::std::list< Sprite::ImplRef >					maSprites; // list of active
			                                            	       // sprite
    	    		                                        	   // objects. this
		            		                                       // list is only
    		                		                               // used for full
        		                    		                       // repaints,
            		                        		               // otherwise, we
                		                            		       // rely on the
		            	        	                               // active sprites
        		        	        	                           // itself to notify
                		    	        	                       // us.
        ChangeRecords									maChangeRecords; // vector
                                                    			         // of
		                                                    	         // sprites
			                                                             // changes
    	    		                                                     // since
        	        		                                             // last
            	            		                                     // updateScreen()
                	                		                             // call
        OutputDevice&									mrOutDev;
        BackBufferSharedPtr 							mpBackBuffer;
        ::canvas::vcltools::VCLObject<VirtualDevice>	maVDev; 		// for the repaint accumulator
        ::canvas::tools::ElapsedTime					maLastUpdate;	// for the frame counter
        bool											mbBackgroundDirty;
    };
}

#endif /* _VCLCANVAS_REDRAWMANAGER_HXX */
