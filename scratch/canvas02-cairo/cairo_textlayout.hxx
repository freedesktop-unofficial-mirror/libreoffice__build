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

#ifndef _CAIROCANVAS_TEXTLAYOUT_HXX
#define _CAIROCANVAS_TEXTLAYOUT_HXX

#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XTextLayout.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <boost/utility.hpp>

#include "cairo_cairo.hxx"
#include "cairo_canvasfont.hxx"
#include "cairo_usagecounter.hxx"


/* Definition of TextLayout class */

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XTextLayout,
                         					  ::com::sun::star::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public ::comphelper::OBaseMutex, 
                       public TextLayout_Base,
                       private UsageCounter< TextLayout >,
                       private ::boost::noncopyable
    {
    public:
        TextLayout( const ::com::sun::star::rendering::StringContext& aText, 
                    sal_Int8                                                  nDirection, 
                    sal_Int64                                                 nRandomSeed,
                    const CanvasFont::ImplRef&                                rFont );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XTextLayout
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > > SAL_CALL queryTextShapes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryInkMeasures(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryMeasures(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL queryLogicalAdvancements(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL applyLogicalAdvancements( const ::com::sun::star::uno::Sequence< double >& aAdvancements ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::geometry::RealRectangle2D SAL_CALL queryTextBounds(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL justify( double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL combinedJustify( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XTextLayout > >& aNextLayouts, double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::TextHit SAL_CALL getTextHit( const ::com::sun::star::geometry::RealPoint2D& aHitPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::Caret SAL_CALL getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL getBaselineOffset(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getMainTextDirection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::StringContext SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException);
        
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        bool draw( ::cairo::Cairo* pCairo );

    protected:
        ~TextLayout(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        ::com::sun::star::rendering::StringContext maText;
        ::com::sun::star::uno::Sequence< double >  maLogicalAdvancements;
        CanvasFont::ImplRef                        mpFont;
        sal_Int8                                   mnTextDirection;

	void useFont( ::cairo::Cairo* pCairo );
    };

}

#endif /* _CAIROCANVAS_TEXTLAYOUT_HXX */
