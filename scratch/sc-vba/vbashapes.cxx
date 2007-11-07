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
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <org/openoffice/msforms/XShapeRange.hpp>
#include <org/openoffice/office/MsoAutoShapeType.hpp>

#include "vbashapes.hxx"
#include "vbashape.hxx"
#include "vbashaperange.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

class VbShapeEnumHelper : public EnumerationHelper_BASE
{
        uno::Reference<msforms::XShapes > m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
	VbShapeEnumHelper( const uno::Reference< msforms::XShapes >& xParent,  const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xParent( xParent ), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                ScVbaShapes* pShapes = dynamic_cast< ScVbaShapes* >(m_xParent.get());
                if ( pShapes && hasMoreElements() )
                    return pShapes->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

void ScVbaShapes::initBaseCollection()
{
	if ( m_xNameAccess.is() ) // already has NameAccess
		return;
	// no NameAccess then use ShapeCollectionHelper
	XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec mShapes;
	sal_Int32 nLen = m_xIndexAccess->getCount();
	mShapes.reserve( nLen );
	for ( sal_Int32 index=0; index<nLen; ++index )
		mShapes.push_back( uno::Reference< drawing::XShape >( m_xIndexAccess->getByIndex( index ) , uno::UNO_QUERY ) );
	uno::Reference< container::XIndexAccess > xShapes( new XNamedObjectCollectionHelper< drawing::XShape >( mShapes ) );
	m_xIndexAccess.set( xShapes, uno::UNO_QUERY );
	m_xNameAccess.set( xShapes, uno::UNO_QUERY );
}

ScVbaShapes::ScVbaShapes( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess > xShapes ): ScVbaShapes_BASE( xParent, xContext, xShapes ), m_nNewShapeCount(0)
{
    m_xShapes.set( xShapes, uno::UNO_QUERY_THROW );
    m_xDrawPage.set( xShapes, uno::UNO_QUERY_THROW ); 
    initBaseCollection();
}

uno::Reference< container::XEnumeration >
ScVbaShapes::createEnumeration() throw (uno::RuntimeException)
{
    return new VbShapeEnumHelper( this,  m_xIndexAccess );
}

uno::Any
ScVbaShapes::createCollectionObject( const css::uno::Any& aSource )
{
    if( aSource.hasValue() )
    {
        uno::Reference< drawing::XShape > xShape( aSource, uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, ScVbaShape::getType( xShape ) ) ) );
    }
    return uno::Any();
}

uno::Type
ScVbaShapes::getElementType() throw (uno::RuntimeException)
{
    return org::openoffice::msforms::XShape::static_type(0);
}
rtl::OUString& 
ScVbaShapes::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaShapes") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
ScVbaShapes::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msform.Shapes" ) );
	}
	return aServiceNames;
}

css::uno::Reference< css::container::XIndexAccess > 
ScVbaShapes::getShapesByArrayIndices( const uno::Any& Index  ) throw (uno::RuntimeException)
{
	if ( Index.getValueTypeClass() != uno::TypeClass_SEQUENCE )
		throw uno::RuntimeException();
	
	uno::Reference< script::XTypeConverter > xConverter = getTypeConverter(mxContext);
	uno::Any aConverted;
	aConverted = xConverter->convertTo( Index, getCppuType((uno::Sequence< uno::Any >*)0) );

	uno::Sequence< uno::Any > sIndices;
	aConverted >>= sIndices;
	XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec mShapes;
	sal_Int32 nElems = sIndices.getLength();
	for( sal_Int32 index = 0; index < nElems; ++index )
	{
		uno::Reference< drawing::XShape > xShape;
		if ( sIndices[ index ].getValueTypeClass() == uno::TypeClass_STRING )
		{
			rtl::OUString sName;
			sIndices[ index ] >>= sName;
			xShape.set( m_xNameAccess->getByName( sName ), uno::UNO_QUERY );
		}
		else
		{
			sal_Int32 nIndex = 0;
			sIndices[ index ] >>= nIndex;
			// adjust for 1 based mso indexing
			xShape.set( m_xIndexAccess->getByIndex( nIndex - 1 ), uno::UNO_QUERY );		
			
		}
		// populate map with drawing::XShapes
		if ( xShape.is() )
			mShapes.push_back( xShape );
	}  
	uno::Reference< container::XIndexAccess > xIndexAccess( new XNamedObjectCollectionHelper< drawing::XShape >( mShapes ) );
	return xIndexAccess;
}

uno::Any SAL_CALL 
ScVbaShapes::Item( const uno::Any& Index, const uno::Any& Index2 ) throw (uno::RuntimeException)
{
	// I don't think we need to support Array of indices for shapes	
/*
	if ( Index.getValueTypeClass() == uno::TypeClass_SEQUENCE )
	{
		uno::Reference< container::XIndexAccess > xIndexAccess( getShapesByArrayIndices( Index ) );
		// return new collection instance
		uno::Reference< vba::XCollection > xShapesCollection(  new ScVbaShapes( this->getParent(), mxContext, xIndexAccess ) );
		return uno::makeAny( xShapesCollection );
	}
*/
	return 	ScVbaShapes_BASE::Item( Index, Index2 );
}

uno::Reference< msforms::XShapeRange > SAL_CALL 
ScVbaShapes::Range( const uno::Any& shapes ) throw (css::uno::RuntimeException)
{
	// shapes, can be an index or an array of indices
	uno::Reference< container::XIndexAccess > xShapes;
	if ( shapes.getValueTypeClass() == uno::TypeClass_SEQUENCE )
		xShapes = getShapesByArrayIndices( shapes );
	else
	{
		// wrap single index into a sequence
		uno::Sequence< uno::Any > sIndices(1);
		sIndices[ 0 ] = shapes;
		uno::Any aIndex;
		aIndex <<= sIndices;
		xShapes = getShapesByArrayIndices( aIndex );
	}
	return new ScVbaShapeRange(  getParent(), mxContext, xShapes, m_xDrawPage );
}

void SAL_CALL 
ScVbaShapes::SelectAll() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< view::XSelectionSupplier > xSelectSupp( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    try
    {
        xSelectSupp->select( uno::makeAny( m_xShapes ) );
    }
    // viewuno.cxx ScTabViewObj::select will throw IllegalArgumentException
    // if one of the shapes is no 'markable' e.g. a button 
    // the method still works
    catch( lang::IllegalArgumentException& )
    {
    }
}

uno::Reference< drawing::XShape > 
ScVbaShapes::createShape( rtl::OUString service ) throw (css::uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xMSF( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape( xMSF->createInstance( service ), uno::UNO_QUERY_THROW );
    return xShape;
}

uno::Any 
ScVbaShapes::AddRectangle( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, uno::Any aRange ) throw (css::uno::RuntimeException)
{
    rtl::OUString sCreateShapeName( rtl::OUString::createFromAscii( "com.sun.star.drawing.RectangleShape" ) );
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( startX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( startY );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );

    uno::Reference< drawing::XShape > xShape( createShape( sCreateShapeName ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    rtl::OUString sName = createName( rtl::OUString::createFromAscii( "Rectangle" ) );
    setDefaultShapeProperties( xShape );
    setShape_NameProperty( xShape, sName );

    awt::Point aMovePositionIfRange(0, 0);
    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition( position );

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize( size );

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, ScVbaShape::getType( xShape ) );
    pScVbaShape->setRange( aRange ); 
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

uno::Any 
ScVbaShapes::AddEllipse( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, uno::Any aRange ) throw (css::uno::RuntimeException)
{
    rtl::OUString sCreateShapeName( rtl::OUString::createFromAscii( "com.sun.star.drawing.EllipseShape" ) );
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( startX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( startY );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );

    uno::Reference< drawing::XShape > xShape( createShape( sCreateShapeName ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    awt::Point aMovePositionIfRange( 0, 0 );
    //TODO helperapi using a writer document
    /*
    XDocument xDocument = (XDocument)getParent();
    if (AnyConverter.isVoid(_aRange))
    {
        _aRange = xDocument.Range(new Integer(0), new Integer(1));
        // Top&Left in Word is Top&Left of the paper and not the writeable area.
        aMovePositionIfRange = calculateTopLeftMargin((HelperInterfaceAdaptor)xDocument);
    }

    setShape_AnchorTypeAndRangeProperty(xShape, _aRange);
    */
    rtl::OUString name = createName( rtl::OUString::createFromAscii( "Oval" ));
    setDefaultShapeProperties(xShape);
    setShape_NameProperty(xShape, name);

    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition(position);

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, ScVbaShape::getType( xShape ) );
    pScVbaShape->setRange( aRange ); 
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

//helpeapi calc
uno::Any SAL_CALL
ScVbaShapes::AddLine( sal_Int32 StartX, sal_Int32 StartY, sal_Int32 endX, sal_Int32 endY ) throw (uno::RuntimeException)
{
    sal_Int32 nLineWidth = endX - StartX;
    sal_Int32 nLineHeight = endY - StartY;

    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( StartX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( StartY );
    
    uno::Reference< drawing::XShape > xShape( createShape( rtl::OUString::createFromAscii("com.sun.star.drawing.LineShape") ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    awt::Point aMovePositionIfRange( 0, 0 );
    
    rtl::OUString name = createName( rtl::OUString::createFromAscii( "Line" ) );
    setDefaultShapeProperties(xShape);
    setShape_NameProperty(xShape, name);

    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition(position);

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, ScVbaShape::getType( xShape ) );
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

uno::Any SAL_CALL
ScVbaShapes::AddShape( sal_Int32 _nType, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (uno::RuntimeException)
{
    uno::Any _aAnchor;
    if (_nType == office::MsoAutoShapeType::msoShapeRectangle)
    {
        return AddRectangle(_nLeft, _nTop, _nWidth, _nHeight, _aAnchor);
    }
    else if (_nType == office::MsoAutoShapeType::msoShapeOval)
    {
        return AddEllipse(_nLeft, _nTop, _nWidth, _nHeight, _aAnchor);
    }
    return uno::Any();
}

void
ScVbaShapes::setDefaultShapeProperties( uno::Reference< drawing::XShape > xShape ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "FillStyle" ), uno::makeAny( rtl::OUString::createFromAscii( "SOLID" ) ) );
    xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "FillColor"), uno::makeAny( sal_Int32(0xFFFFFF) )  );
    xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "TextWordWrap"), uno::makeAny( text::WrapTextMode_THROUGHT )  );
    //not find in OOo2.3
    //xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "Opaque"), uno::makeAny( sal_True )  );
}

void
ScVbaShapes::setShape_NameProperty( uno::Reference< css::drawing::XShape > xShape, rtl::OUString sName )
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    try
    {
        xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "Name" ), uno::makeAny( sName ) );
    }
    catch( script::BasicErrorException e )
    {
    }
}

rtl::OUString
ScVbaShapes::createName( rtl::OUString sName )
{
    sal_Int32 nActNumber = 1 + m_nNewShapeCount;
    m_nNewShapeCount++; 
    sName += rtl::OUString::valueOf( nActNumber );
    return sName;
}

awt::Point
calculateTopLeftMargin( uno::Reference< vba::XHelperInterface > xDocument )
{
    awt::Point aPoint( 0, 0 );
    uno::Reference< frame::XModel > xModel( xDocument, uno::UNO_QUERY_THROW );
    return awt::Point();
}
