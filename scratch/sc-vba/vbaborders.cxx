#include "vbaborders.hxx"

#include <cppuhelper/implbase3.hxx>
#include <org/openoffice/vba/Excel/XlBordersIndex.hpp>
#include <org/openoffice/vba/Excel/XlBorderWeight.hpp>
#include <org/openoffice/vba/Excel/XlLineStyle.hpp>
#include <org/openoffice/vba/Excel/Constants.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/TableBorder.hpp>

#include "vbapalette.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;
using namespace ::org::openoffice::vba::Excel;


typedef ::cppu::WeakImplHelper1<container::XIndexAccess > RangeBorders_Base;
typedef ::cppu::WeakImplHelper1<vba::XBorder > ScVbaBorder_Base;

// #TODO sort these indexes to match the order in which Excel iterates over the
// borders, the enumeration will match the order in this list
static const sal_Int16 supportedIndexTable[] = {  XlBordersIndex::xlEdgeLeft, XlBordersIndex::xlEdgeTop, XlBordersIndex::xlEdgeBottom, XlBordersIndex::xlEdgeRight, XlBordersIndex::xlDiagonalDown, XlBordersIndex::xlDiagonalUp, XlBordersIndex::xlInsideVertical, XlBordersIndex::xlInsideHorizontal };

const static rtl::OUString sTableBorder( RTL_CONSTASCII_USTRINGPARAM("TableBorder") );

//  Equiv widths in in 1/100 mm
const static sal_Int32 OOLineThin = 35;
const static sal_Int32 OOLineMedium = 88;
const static sal_Int32 OOLineThick = 141;
const static sal_Int32 OOLineHairline = 2;

class ScVbaBorder : public ScVbaBorder_Base
{
private:
	uno::Reference< beans::XPropertySet > m_xProps;
	uno::Reference< uno::XComponentContext > m_xContext;
	sal_Int32 m_LineType;	
	ScVbaPalette m_Palette;
	bool setBorderLine( table::BorderLine& rBorderLine )
	{
		table::TableBorder aTableBorder;
		m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;

		switch ( m_LineType )
		{
			case XlBordersIndex::xlEdgeLeft:
				aTableBorder.IsLeftLineValid = sal_True;
				aTableBorder.LeftLine= rBorderLine;
				break;
			case XlBordersIndex::xlEdgeTop:
				aTableBorder.IsTopLineValid = sal_True;
				aTableBorder.TopLine = rBorderLine;
				break;
			
			case XlBordersIndex::xlEdgeBottom:
				aTableBorder.IsBottomLineValid = sal_True;
				aTableBorder.BottomLine = rBorderLine;
				break;
			case XlBordersIndex::xlEdgeRight:
				aTableBorder.IsRightLineValid = sal_True;
				aTableBorder.RightLine = rBorderLine;
				break;
			case XlBordersIndex::xlDiagonalDown:
			case XlBordersIndex::xlDiagonalUp:
			case XlBordersIndex::xlInsideVertical:
			case XlBordersIndex::xlInsideHorizontal:
				// #TODO have to ignore at the momement, would be
				// nice to investigate what we can do here
				break; 
			default:
					return false;
		}
		m_xProps->setPropertyValue( sTableBorder, uno::makeAny(aTableBorder) );
		return true;
	}	

	bool getBorderLine( table::BorderLine& rBorderLine )
	{
		table::TableBorder aTableBorder;
		m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;
		switch ( m_LineType )
		{
			case XlBordersIndex::xlEdgeLeft:
				if ( aTableBorder.IsLeftLineValid )
					rBorderLine = aTableBorder.LeftLine;
				break;
			case XlBordersIndex::xlEdgeTop:
				if ( aTableBorder.IsTopLineValid )
					rBorderLine = aTableBorder.TopLine;
				break;
			
			case XlBordersIndex::xlEdgeBottom:
				if ( aTableBorder.IsBottomLineValid )
					rBorderLine = aTableBorder.BottomLine;
				break;
			case XlBordersIndex::xlEdgeRight:
				if ( aTableBorder.IsRightLineValid )
					rBorderLine = aTableBorder.RightLine;
				break;
			case XlBordersIndex::xlDiagonalDown:
			case XlBordersIndex::xlDiagonalUp:
			case XlBordersIndex::xlInsideVertical:
			case XlBordersIndex::xlInsideHorizontal:
				// #TODO have to ignore at the momement, would be
				// nice to investigate what we can do here
				break; 
			default:
					return false;
		}
		return true;
	}	
	ScVbaBorder(); // no impl
public:
	ScVbaBorder( const uno::Reference< beans::XPropertySet > & xProps, const uno::Reference< uno::XComponentContext >& xContext, sal_Int32 lineType, ScVbaPalette& rPalette) : m_xProps( xProps ), m_xContext( xContext ), m_LineType( lineType ), m_Palette( rPalette ) {}  	

	// XBorder
	uno::Any SAL_CALL getColor() throw (uno::RuntimeException)
	{
		table::BorderLine aBorderLine;
		if ( getBorderLine( aBorderLine ) )
			return uno::makeAny( OORGBToXLRGB( aBorderLine.Color ) ); 
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
	}
	void SAL_CALL setColor( const uno::Any& _color ) throw (uno::RuntimeException)
	{
		sal_Int32 nColor;
		_color >>= nColor;
		table::BorderLine aBorderLine;
		if ( getBorderLine( aBorderLine ) )
		{
			aBorderLine.Color = XLRGBToOORGB( nColor );
			setBorderLine( aBorderLine );	
		}
		else
			throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
	}

	uno::Any SAL_CALL getColorIndex() throw (uno::RuntimeException)
	{
		sal_Int32 nColor;
		XLRGBToOORGB( getColor() ) >>= nColor;
		uno::Reference< container::XIndexAccess > xIndex = m_Palette.getPalette();
		sal_Int32 nElems = xIndex->getCount();
		sal_Int32 nIndex = -1;
		for ( sal_Int32 count=0; count<nElems; ++count )
		{
			sal_Int32 nPaletteColor;
			xIndex->getByIndex( count ) >>= nPaletteColor;
			if ( nPaletteColor == nColor )
			{					
				nIndex = count + 1;
				break;
			}
		}
		return uno::makeAny(nIndex);
	}

	void SAL_CALL setColorIndex( const uno::Any& _colorindex ) throw (uno::RuntimeException)
	{
		sal_Int32 nColor;
		_colorindex >>= nColor;
		if ( nColor == vba::Excel::Constants::xlColorIndexAutomatic )
                nColor = 1;
		setColor( OORGBToXLRGB( m_Palette.getPalette()->getByIndex( --nColor )  ) );
	}
	uno::Any SAL_CALL getWeight() throw (uno::RuntimeException)
	{
		table::BorderLine aBorderLine;
		if ( getBorderLine( aBorderLine ) )
		{
			switch ( aBorderLine.OuterLineWidth )
			{
				case 0:	// Thin = default OO thickness
				case OOLineThin:	
					return uno::makeAny( XlBorderWeight::xlThin );
				case OOLineMedium:	
					return uno::makeAny( XlBorderWeight::xlMedium );
				case OOLineThick:	
					return uno::makeAny( XlBorderWeight::xlThick );
				case OOLineHairline:	
					return uno::makeAny( XlBorderWeight::xlHairline );
				default: 
					break;
			}
		}
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Method failed" ) ), uno::Reference< uno::XInterface >() );
	}
	void SAL_CALL setWeight( const uno::Any& _weight ) throw (uno::RuntimeException)
	{
		sal_Int32 nWeight;
		_weight >>= nWeight;
		table::BorderLine aBorderLine;
		if ( getBorderLine( aBorderLine ) )
		{
			switch ( nWeight )
			{
				case XlBorderWeight::xlThin:	
					aBorderLine.OuterLineWidth = OOLineThin;
					break;
				case XlBorderWeight::xlMedium:
					aBorderLine.OuterLineWidth = OOLineMedium;
					break;
				case XlBorderWeight::xlThick:
					aBorderLine.OuterLineWidth = OOLineThick;
					break;
				case XlBorderWeight::xlHairline:
					aBorderLine.OuterLineWidth = OOLineHairline;
					break;
				default: 
					throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Bad param" ) ), uno::Reference< uno::XInterface >() );
					break;
			}
			setBorderLine( aBorderLine );
		}
		else
					throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Method failed" ) ), uno::Reference< uno::XInterface >() );
	}

	uno::Any SAL_CALL getLineStyle() throw (uno::RuntimeException)
	{
		// always return xlContinuous;
		return uno::makeAny( XlLineStyle::xlContinuous );
	}
	void SAL_CALL setLineStyle( const uno::Any& /*_linestyle*/ ) throw (uno::RuntimeException) 
	{
		// Urk no choice but to silently ignore we don't support this attribute
		// #TODO would be nice to support the excel line styles
	}
};

class RangeBorders : public RangeBorders_Base
{
private:
	uno::Reference< table::XCellRange > m_xRange;
	uno::Reference< uno::XComponentContext > m_xContext;
	ScVbaPalette m_Palette;
	sal_Int32 getTableIndex( sal_Int32 nConst )
	{
		// hokay return position of the index in the table
		sal_Int32 nIndexes = getCount();
		sal_Int32 realIndex = 0;
		const sal_Int16* pTableEntry = supportedIndexTable;
		for ( ; realIndex < nIndexes; ++realIndex, ++pTableEntry )
		{
			if ( *pTableEntry == nConst )
				return realIndex;
		}
		return getCount(); // error condition
	}
public:
	RangeBorders(  const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, ScVbaPalette& rPalette ) : m_xRange( xRange ), m_xContext( xContext ), m_Palette( rPalette )
	{
	}
	// XIndexAccess 
	virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
	{
		return sizeof( supportedIndexTable ) / sizeof( supportedIndexTable[0] );
	}
	virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException) 
	{
			
		sal_Int32 nIndex = getTableIndex( Index );
		if ( nIndex >= 0 && nIndex < getCount() )
		{
			uno::Reference< beans::XPropertySet > xProps( m_xRange, uno::UNO_QUERY_THROW );
			return uno::makeAny( uno::Reference< vba::XBorder >( new ScVbaBorder( xProps, m_xContext, supportedIndexTable[ nIndex ], m_Palette )) );
		}
		throw lang::IndexOutOfBoundsException();
	}
	virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
	{
		return  vba::XBorder::static_type(0);
	}
	virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
	{
		return sal_True;
	}
};

uno::Reference< container::XIndexAccess > 
rangeToBorderIndexAccess( const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, ScVbaPalette& rPalette )
{
	return new RangeBorders( xRange, xContext, rPalette );
}

class RangeBorderEnumWrapper : public EnumerationHelper_BASE
{
	uno::Reference<container::XIndexAccess > m_xIndexAccess;
	sal_Int32 nIndex;
public:
	RangeBorderEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
	virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
	{
		return ( nIndex < m_xIndexAccess->getCount() );
	}

	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
	{
		if ( nIndex < m_xIndexAccess->getCount() )
			return m_xIndexAccess->getByIndex( nIndex++ );
		throw container::NoSuchElementException();
	}
};

ScVbaBorders::ScVbaBorders( const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< table::XCellRange >& xRange, ScVbaPalette& rPalette  ):  ScVbaBorders_BASE( xContext, rangeToBorderIndexAccess( xRange ,xContext, rPalette ) ) 
{
}

uno::Reference< container::XEnumeration >
ScVbaBorders::createEnumeration() throw (uno::RuntimeException)
{
	return new RangeBorderEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaBorders::createCollectionObject( const css::uno::Any& aSource )
{
	return aSource; // its already a Border object
}

uno::Type 
ScVbaBorders::getElementType() throw (uno::RuntimeException)
{
	return vba::XBorders::static_type(0);
}

uno::Any
ScVbaBorders::getItemByIntIndex( const sal_Int32 nIndex )  throw (uno::RuntimeException)
{
	return createCollectionObject( m_xIndexAccess->getByIndex( nIndex ) );
}


uno::Any SAL_CALL ScVbaBorders::getColor() throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
void SAL_CALL ScVbaBorders::setColor( const uno::Any& _color ) throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ScVbaBorders::getColorIndex() throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
void SAL_CALL ScVbaBorders::setColorIndex( const uno::Any& _colorindex ) throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ScVbaBorders::getLineStyle() throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
void SAL_CALL ScVbaBorders::setLineStyle( const uno::Any& _linestyle ) throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ScVbaBorders::getWeight() throw (uno::RuntimeException)
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
void SAL_CALL ScVbaBorders::setWeight( const uno::Any& _weight ) throw (uno::RuntimeException) 
{
	throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No Implementation available" ) ), uno::Reference< uno::XInterface >() );
}
