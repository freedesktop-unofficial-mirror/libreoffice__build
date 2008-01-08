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
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include "vbashaperange.hxx"
#include "vbashape.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

class VbShapeRangeEnumHelper : public EnumerationHelper_BASE
{
        uno::Reference< vba::XCollection > m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
	VbShapeRangeEnumHelper( const uno::Reference< vba::XCollection >& xParent,  const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xParent( xParent ), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                ScVbaShapeRange* pCollectionImpl = dynamic_cast< ScVbaShapeRange* >(m_xParent.get());
                if ( pCollectionImpl && hasMoreElements() )
                    return pCollectionImpl->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

ScVbaShapeRange::ScVbaShapeRange( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xShapes, const uno::Reference< drawing::XDrawPage >& xDrawPage ) : ScVbaShapeRange_BASE( xParent, xContext, xShapes ), m_xDrawPage( xDrawPage ), m_nShapeGroupCount(0)
{
}

// Methods
void SAL_CALL 
ScVbaShapeRange::Select(  ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< view::XSelectionSupplier > xSelectSupp( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( getShapes() ) );
}

uno::Reference< msforms::XShape > SAL_CALL 
ScVbaShapeRange::Group() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShapeGrouper > xShapeGrouper( m_xDrawPage, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShapeGroup > xShapeGroup( xShapeGrouper->group( getShapes() ), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape( xShapeGroup, uno::UNO_QUERY_THROW );
    return uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, getShapes(), office::MsoShapeType::msoGroup ) );
}

uno::Reference< drawing::XShapes > 
ScVbaShapeRange::getShapes() throw (uno::RuntimeException)
{
	if ( !m_xShapes.is() )
	{
		uno::Reference< lang::XMultiServiceFactory > xMSF( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
		m_xShapes.set( xMSF->createInstance( rtl::OUString::createFromAscii( "com.sun.star.drawing.ShapeCollection" ) ), uno::UNO_QUERY_THROW );
		sal_Int32 nLen = m_xIndexAccess->getCount();
		for ( sal_Int32 index = 0; index < nLen; ++index )
			m_xShapes->add( uno::Reference< drawing::XShape >( m_xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW ) );

	}
	return m_xShapes;
}


void SAL_CALL 
ScVbaShapeRange::IncrementRotation( double Increment ) throw (uno::RuntimeException)
{
	sal_Int32 nLen = getCount();
	for ( sal_Int32 index = 1; index <= nLen; ++index )
	{
		uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
		xShape->IncrementRotation( Increment );
	}
}

void SAL_CALL 
ScVbaShapeRange::IncrementLeft( double Increment ) throw (uno::RuntimeException)
{
	sal_Int32 nLen = getCount();
	for ( sal_Int32 index = 1; index <= nLen; ++index )
	{
		uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
		xShape->IncrementLeft( Increment );
	}
}

void SAL_CALL 
ScVbaShapeRange::IncrementTop( double Increment ) throw (uno::RuntimeException)
{
	sal_Int32 nLen = getCount();
	for ( sal_Int32 index = 1; index <= nLen; ++index )
	{
		uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
		xShape->IncrementTop( Increment );
	}
}

uno::Type SAL_CALL
ScVbaShapeRange::getElementType() throw (uno::RuntimeException)
{
    return msforms::XShape::static_type(0);
}

uno::Reference< container::XEnumeration > SAL_CALL 
ScVbaShapeRange::createEnumeration() throw (uno::RuntimeException)
{
	return new VbShapeRangeEnumHelper( this, m_xIndexAccess );
}

uno::Any
ScVbaShapeRange:: createCollectionObject( const css::uno::Any& aSource )
{
	uno::Reference< drawing::XShape > xShape( aSource, uno::UNO_QUERY_THROW );
	// #TODO  #FIXME Shape parent should always be the sheet the shapes belong 
	// to 
	uno::Reference< msforms::XShape > xVbShape( new ScVbaShape( uno::Reference< vba::XHelperInterface >(), mxContext, xShape, getShapes(), ScVbaShape::getType( xShape ) ) ); 
        return uno::makeAny( xVbShape );
}

rtl::OUString& 
ScVbaShapeRange::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaShapeRange") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
ScVbaShapeRange::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msform.ShapeRange" ) );
	}
	return aServiceNames;
}
