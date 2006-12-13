#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/sheet/XCellSeries.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/TableSortField.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/awt/XDevice.hpp>

//#include <com/sun/star/sheet/CellDeleteMode.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>

#include <org/openoffice/vba/xlPasteSpecialOperation.hpp>
#include <org/openoffice/vba/xlPasteType.hpp>
#include <org/openoffice/vba/Excel/Constants.hpp>
#include <org/openoffice/vba/Excel/XlFindLookIn.hpp>
#include <org/openoffice/vba/Excel/XlLookAt.hpp>
#include <org/openoffice/vba/Excel/XlSearchOrder.hpp>
#include <org/openoffice/vba/Excel/XlSortOrder.hpp>
#include <org/openoffice/vba/Excel/XlYesNoGuess.hpp>
#include <org/openoffice/vba/Excel/XlSortOrientation.hpp>
#include <org/openoffice/vba/Excel/XlSortMethod.hpp>
#include <org/openoffice/vba/Excel/XlDirection.hpp>
#include <org/openoffice/vba/Excel/XlSortDataOption.hpp>
#include <org/openoffice/vba/Excel/XlDeleteShiftDirection.hpp>
#include <org/openoffice/vba/Excel/XlReferenceStyle.hpp>
#include <org/openoffice/vba/Excel/XlBordersIndex.hpp>
#include <org/openoffice/vba/Excel/XlPageBreak.hpp>
#include <org/openoffice/vba/Excel/XlTextParsingType.hpp>
#include <org/openoffice/vba/Excel/XlTextQualifier.hpp>

#include <scitems.hxx>
#include <svx/srchitem.hxx>
#include <cellsuno.hxx>
#include <dbcolect.hxx>
#include "docfunc.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/itemwrapper.hxx>
#include <sc.hrc>
#include <unonames.hxx>

#include "vbarange.hxx"
#include "vbafont.hxx"
#include "vbacomment.hxx"
#include "vbainterior.hxx"
#include "vbacharacters.hxx"
#include "vbaborders.hxx"
#include "vbaworksheet.hxx"

#include "tabvwsh.hxx"
#include "rangelst.hxx"
#include "convuno.hxx"
#include "compiler.hxx"

#include <comphelper/anytostring.hxx>

#include <global.hxx>

#include "vbaglobals.hxx"
#include <vector>
#include <vbacollectionimpl.hxx>
// begin test includes
#include <com/sun/star/sheet/FunctionArgument.hpp>
// end test includes

using namespace ::org::openoffice;
using namespace ::org::openoffice::vba::Excel;
using namespace ::com::sun::star;

//    * 1 point = 1/72 inch = 20 twips
//    * 1 inch = 72 points = 1440 twips
//    * 1 cm = 567 twips
double lcl_hmmToPoints( double nVal ) { return ( (double)((nVal /1000 ) * 567 ) / 20 ); }
double lcl_pointsToHmm( double nVal ) { return (double)( ( nVal * 20 ) / 567 ) * 1000; }

static const sal_Int16 supportedIndexTable[] = {  XlBordersIndex::xlEdgeLeft, XlBordersIndex::xlEdgeTop, XlBordersIndex::xlEdgeBottom, XlBordersIndex::xlEdgeRight, XlBordersIndex::xlDiagonalDown, XlBordersIndex::xlDiagonalUp, XlBordersIndex::xlInsideVertical, XlBordersIndex::xlInsideHorizontal };

USHORT lcl_pointsToTwips( double nVal ) 
{ 
	nVal = nVal * static_cast<double>(20);
	short nTwips = static_cast<short>(nVal);
	return nTwips;
}
double lcl_TwipsToPoints( USHORT nVal ) 
{ 
	double nPoints = nVal;
	return nPoints / 20; 
}

double lcl_Round2DecPlaces( double nVal )
{
	nVal  = (nVal * (double)100);
	long tmp = static_cast<long>(nVal);
	if ( ( ( nVal - tmp ) >= 0.5 ) )
		++tmp;
	nVal = tmp;
	nVal = nVal/100;
	return nVal;
}

uno::Any lcl_makeRange( uno::Reference< uno::XComponentContext >& xContext, const uno::Any aAny )
{
	uno::Reference< table::XCellRange > xCellRange( aAny, uno::UNO_QUERY_THROW );
	return uno::makeAny( uno::Reference< vba::XRange >( new ScVbaRange( xContext, xCellRange ) ) );
}

SfxItemSet*  ScVbaRange::getCurrentDataSet( ) throw ( uno::RuntimeException )
{
	ScCellRangeObj* pUnoCellRange = dynamic_cast< ScCellRangeObj* >( mxRange.get() );
	SfxItemSet* pDataSet = 	pUnoCellRange->GetCurrentDataSet( true );
	
	if ( !pDataSet )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can't access Itemset for range" ) ), uno::Reference< uno::XInterface >() );
	return pDataSet;	
}

class SingleRangeEnumeration : public EnumerationHelper_BASE
{
	uno::Reference< table::XCellRange > m_xRange;
	uno::Reference< uno::XComponentContext > m_xContext;
	bool bHasMore;
public:

	SingleRangeEnumeration( const uno::Reference< css::uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException ) : m_xRange( xRange ), m_xContext( xContext ), bHasMore( true ) { }
	virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException) { return bHasMore; }
	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{
		if ( !bHasMore )
			throw container::NoSuchElementException();
		bHasMore = false;
		return lcl_makeRange( m_xContext, uno::makeAny( m_xRange ) );
	}
};

// very simple class to pass to ScVbaCollectionBaseImpl containing
// just one item
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > SingleRange_BASE;

class SingleRangeIndexAccess : public SingleRange_BASE
{
private:
	uno::Reference< table::XCellRange > m_xRange;
	uno::Reference< uno::XComponentContext > m_xContext;
	SingleRangeIndexAccess(); // not defined
public:
	SingleRangeIndexAccess( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ):m_xRange( xRange ), m_xContext( xContext ) {}
	// XIndexAccess
	virtual ::sal_Int32 SAL_CALL getCount() throw (::uno::RuntimeException) { return 1; }
	virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
	{ 
		if ( Index != 0 )
			throw lang::IndexOutOfBoundsException();
		return uno::makeAny( m_xRange ); 
	}
        // XElementAccess
        virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException){ return table::XCellRange::static_type(0); }
 
        virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException) { return sal_True; }
	// XEnumerationAccess
	virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException) { return new SingleRangeEnumeration( m_xContext, m_xRange ); }

};



class RangesEnumerationImpl : public EnumerationHelperImpl
{
public:

	RangesEnumerationImpl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ) {}
	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		return lcl_makeRange( m_xContext, m_xEnumeration->nextElement() );
	}
};


class ScVbaRangeAreas : public ScVbaCollectionBaseImpl
{

public:
	ScVbaRangeAreas( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess ) : ScVbaCollectionBaseImpl( xContext, xIndexAccess ) {}

	// XEnumerationAccess
	virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException);

	// XElementAccess
	virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException){ return vba::XRange::static_type(0); }

	virtual uno::Any createCollectionObject( const uno::Any& aSource );
};


uno::Reference< container::XEnumeration > SAL_CALL 
ScVbaRangeAreas::createEnumeration() throw (uno::RuntimeException)
{
	uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
	return new RangesEnumerationImpl( m_xContext, xEnumAccess->createEnumeration() );

}

uno::Any 
ScVbaRangeAreas::createCollectionObject( const uno::Any& aSource )
{
	return lcl_makeRange( m_xContext, aSource );
}




ScDocShell* getDocShellFromRange( const uno::Reference< table::XCellRange >& xRange )
{
	// need the ScCellRangeObj to get docshell
	ScCellRangeObj* pUno = static_cast<  ScCellRangeObj* >( xRange.get() );
			
	if ( !pUno )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying uno range object" ) ), uno::Reference< uno::XInterface >()  );
	return pUno->GetDocShell();
}

ScDocument* getDocumentFromRange( const uno::Reference< table::XCellRange >& xRange )
{
	ScDocShell* pDocShell = getDocShellFromRange( xRange );
	if ( !pDocShell )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying docshell from uno range object" ) ), uno::Reference< uno::XInterface >() );
	ScDocument* pDoc = pDocShell->GetDocument();
	return pDoc;
}

class NumFormatHelper
{
	uno::Reference< util::XNumberFormatsSupplier > mxSupplier;
	uno::Reference< beans::XPropertySet > mxRangeProps;
	uno::Reference< util::XNumberFormats > mxFormats;
public:
	NumFormatHelper( const uno::Reference< table::XCellRange >& xRange )
	{
		mxSupplier.set( getCurrentDocument(), uno::UNO_QUERY_THROW );
		mxRangeProps.set( xRange, uno::UNO_QUERY_THROW);
		mxFormats = mxSupplier->getNumberFormats();
	}
	uno::Reference< beans::XPropertySet > getNumberProps()
	{	
		long nIndexKey;
		uno::Any aValue = mxRangeProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat")));
		aValue >>= nIndexKey;

		if ( mxFormats.is() )
			return  mxFormats->getByKey( nIndexKey );
		return	uno::Reference< beans::XPropertySet > ();
	}

	bool isBooleanType()
	{
	
		if ( getNumberFormat() & util::NumberFormat::LOGICAL )
			return true;
		return false;
	}

	rtl::OUString getNumberFormatString()
	{
		ScCellRangeObj* pUnoCellRange = dynamic_cast<  ScCellRangeObj* >( mxRangeProps.get() );
		if ( pUnoCellRange )
		{
			SfxItemSet* pDataSet = 	pUnoCellRange->GetCurrentDataSet( true );
			SfxItemState eState = pDataSet->GetItemState( ATTR_VALUE_FORMAT, TRUE, NULL);
			// one of the cells in the range is not like the other ;-)
			// so return a zero length format to indicate that
			if ( eState == SFX_ITEM_DONTCARE )
				return rtl::OUString();
		}
		
	
		uno::Reference< beans::XPropertySet > xNumberProps( getNumberProps(), uno::UNO_QUERY_THROW );
		::rtl::OUString aFormatString;
		uno::Any aString = xNumberProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FormatString")));
		aString >>= aFormatString;
		return aFormatString;
	}

	sal_Int16 getNumberFormat()
	{
		uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps(); 	
		sal_Int16 nType = ::comphelper::getINT16(
        	xNumberProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Type" ) ) );
		return nType;
	}

	bool setNumberFormat( const  rtl::OUString& rFormat )
	{
		lang::Locale aLocale;
		uno::Reference< beans::XPropertySet > xNumProps = getNumberProps(); 	
		xNumProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aLocale;
		sal_Int32 nNewIndex = mxFormats->queryKey(rFormat, aLocale, false );
		if ( nNewIndex == -1 ) // format not defined
		{
			nNewIndex = mxFormats->addNew( rFormat, aLocale );
		}
		mxRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat") ), uno::makeAny( nNewIndex ) );				
		return true;
	}

	bool setNumberFormat( sal_Int16 nType )
	{
		uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps(); 	
		lang::Locale aLocale;
		xNumberProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aLocale;
		uno::Reference<util::XNumberFormatTypes> xTypes( mxFormats, uno::UNO_QUERY );
		if ( xTypes.is() )
		{
			sal_Int32 nNewIndex = xTypes->getStandardFormat( nType, aLocale );
       		mxRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat") ), uno::makeAny( nNewIndex ) );				
			return true;
		}
		return false;
	}

};

struct CellPos
{
	CellPos():m_nRow(-1), m_nCol(-1) {};
	CellPos( sal_Int32 nRow, sal_Int32 nCol ):m_nRow(nRow), m_nCol(nCol) {};
sal_Int32 m_nRow;
sal_Int32 m_nCol;
};

typedef ::cppu::WeakImplHelper1< container::XEnumeration > CellsEnumeration_BASE;
typedef vector< CellPos > vCellPos;

class CellsEnumeration : public CellsEnumeration_BASE
{
	uno::Reference< uno::XComponentContext > m_xContext;
	uno::Reference< table::XCellRange > m_xRange;
	vCellPos m_CellPositions;	
	vCellPos::const_iterator m_it; 
public:
	CellsEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ): m_xContext( xContext ), m_xRange( xRange )
	{
		uno::Reference< table::XColumnRowRange > xColumnRowRange(m_xRange, uno::UNO_QUERY_THROW );
		sal_Int32 nRowCount =  xColumnRowRange->getRows()->getCount();
		sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
		for ( sal_Int32 i=0; i<nRowCount; ++i )
			for ( sal_Int32 j=0; j<nColCount; ++j )
				m_CellPositions.push_back( CellPos( i,j ) );
		m_it = m_CellPositions.begin();
	}
	virtual ::sal_Bool SAL_CALL hasMoreElements() throw (::uno::RuntimeException){ return m_it != m_CellPositions.end(); }

	virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
	{
		if ( !hasMoreElements() )
			throw container::NoSuchElementException();
		CellPos aPos = *(m_it)++;
		uno::Reference< table::XCellRange > xCellRange( m_xRange->getCellByPosition(  aPos.m_nCol, aPos.m_nRow ), uno::UNO_QUERY_THROW );
		return uno::makeAny( uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, xCellRange ) ) );
	}
};


const sal_Int32 RANGE_PROPERTY_ID_DFLT=1;
// name is not defineable in IDL so no chance of a false detection of the
// another property/method of the same name
const static ::rtl::OUString RANGE_PROPERTY_DFLT( RTL_CONSTASCII_USTRINGPARAM( "_$DefaultProp" ) );
const static ::rtl::OUString ISVISIBLE(  RTL_CONSTASCII_USTRINGPARAM( "IsVisible"));
const static ::rtl::OUString WIDTH(  RTL_CONSTASCII_USTRINGPARAM( "Width"));
const static ::rtl::OUString HEIGHT(  RTL_CONSTASCII_USTRINGPARAM( "Height"));
const static rtl::OUString EQUALS( RTL_CONSTASCII_USTRINGPARAM("=") );

class CellValueSetter : public ValueSetter
{
protected:
	uno::Any maValue;
	uno::TypeClass mTypeClass;
public:
	CellValueSetter( const uno::Any& aValue );
	virtual bool processValue( const uno::Any& aValue,  const uno::Reference< table::XCell >& xCell );
	virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell );
		
};

CellValueSetter::CellValueSetter( const uno::Any& aValue ): maValue( aValue ), mTypeClass( aValue.getValueTypeClass() ) {}

void
CellValueSetter::visitNode( sal_Int32 /*i*/, sal_Int32 /*j*/, const uno::Reference< table::XCell >& xCell )
{
	processValue( maValue, xCell );
}

bool
CellValueSetter::processValue( const uno::Any& aValue, const uno::Reference< table::XCell >& xCell )
{

	bool isExtracted = false;
	switch ( aValue.getValueTypeClass() )
	{
		case  uno::TypeClass_BOOLEAN:
		{
			sal_Bool bState;
			if ( aValue >>= bState 	 )
			{
				uno::Reference< table::XCellRange > xRange( xCell, uno::UNO_QUERY_THROW );
				if ( bState )
					xCell->setValue( (double) 1 );
				else
					xCell->setValue( (double) 0 );
				NumFormatHelper cellNumFormat( xRange );
				cellNumFormat.setNumberFormat( util::NumberFormat::LOGICAL );
			}
			break;
		}
		case uno::TypeClass_STRING:
		{
			rtl::OUString aString;
			if ( aValue >>= aString )
			{
				uno::Reference< text::XTextRange > xTextRange( xCell, uno::UNO_QUERY_THROW );
				xTextRange->setString( aString );
			}
			else
				isExtracted = false;	
			break;
		}
		default:
		{
			double nDouble;
			if ( aValue >>= nDouble )
				xCell->setValue( nDouble );
			else
				isExtracted = false;	
			break;
		}
	}
	return isExtracted;
		
}


class CellValueGetter : public ValueGetter
{
protected:
	uno::Any maValue;
	uno::TypeClass mTypeClass;
public:
	CellValueGetter() {}
	virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell );
	virtual void processValue( sal_Int32 x, sal_Int32 y, const uno::Any& aValue );
	const uno::Any& getValue() const { return maValue; }
		
};

void
CellValueGetter::processValue(  sal_Int32 /*x*/, sal_Int32 /*y*/, const uno::Any& aValue )
{
	maValue = aValue;
}
void CellValueGetter::visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
{
	uno::Any aValue;
	table::CellContentType eType = xCell->getType();
	if( eType == table::CellContentType_VALUE || eType == table::CellContentType_FORMULA )
	{
		if ( eType == table::CellContentType_FORMULA )
		{
			rtl::OUString sFormula = xCell->getFormula();
			if ( sFormula.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("=TRUE()") ) ) )
				aValue <<= sal_True;
			else if ( sFormula.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("=FALSE()") ) ) )
				aValue <<= sal_False;
			else 	
				aValue <<= xCell->getValue();
		}
		else
		{
			uno::Reference< table::XCellRange > xRange( xCell, uno::UNO_QUERY_THROW );
			NumFormatHelper cellFormat( xRange );
			if ( cellFormat.isBooleanType() )
				aValue = uno::makeAny( ( xCell->getValue() != 0.0 ) );
			else
				aValue <<= xCell->getValue();
		}
	}
	if( eType == table::CellContentType_TEXT )
	{
		uno::Reference< text::XTextRange > xTextRange(xCell, ::uno::UNO_QUERY_THROW);
		aValue <<= xTextRange->getString();
	}
	processValue( x,y,aValue );
}

class CellFormulaValueSetter : public CellValueSetter
{
private:
	ScDocument*  m_pDoc;
	ScAddress::Convention m_eConv;
public:
	CellFormulaValueSetter( const uno::Any& aValue, ScDocument* pDoc, ScAddress::Convention eConv  ):CellValueSetter( aValue ),  m_pDoc( pDoc ), m_eConv( eConv ){}
protected:
	bool processValue( const uno::Any& aValue, const uno::Reference< table::XCell >& xCell )
	{
		rtl::OUString sFormula;
		if ( aValue >>= sFormula )
		{
			// get current convention
			ScAddress::Convention eConv = m_pDoc->GetAddressConvention();
			if ( eConv != m_eConv )	
			{
				ScCellRangesBase* pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xCell.get() );
				if ( pUnoRangesBase )
				{
					ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();	
					ScCompiler aCompiler( m_pDoc, aCellRanges.First()->aStart );
					// compile the string in the format passed in
					aCompiler.CompileString( sFormula, m_eConv );
					// set desired convention to that of the document
					aCompiler.SetRefConvention( eConv );
					String sConverted;
					aCompiler.CreateStringFromTokenArray(sConverted);
					sFormula = EQUALS + sConverted;
				}
			}

			xCell->setFormula( sFormula );
			return true;
		}
		return false;
	}
		
};

class CellFormulaValueGetter : public CellValueGetter
{
private:
	ScDocument*  m_pDoc;
	ScAddress::Convention m_eConv;
public:
	CellFormulaValueGetter(ScDocument* pDoc, ScAddress::Convention eConv ) : CellValueGetter( ), m_pDoc( pDoc ), m_eConv( eConv ) {}
	virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
	{
		uno::Any aValue;
		aValue <<= xCell->getFormula();	
		// get current convention
		ScAddress::Convention eConv = m_pDoc->GetAddressConvention();
		rtl::OUString sVal;
		aValue >>= sVal;
		ScCellRangesBase* pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xCell.get() );
		if ( pUnoRangesBase )
		{
			ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();	
			ScCompiler aCompiler( m_pDoc, aCellRanges.First()->aStart );
			aCompiler.CompileString( sVal,  ScAddress::CONV_OOO );
			// set desired convention
			aCompiler.SetRefConvention( m_eConv );
			String sConverted;
			aCompiler.CreateStringFromTokenArray(sConverted);
			rtl::OUString sOUString = sConverted;
			aValue <<= sOUString;
		}
		processValue( x,y,aValue );
	}
		
};


class Dim2ArrayValueGetter : public ArrayVisitor
{
protected:
	uno::Any maValue;
	ValueGetter& mValueGetter;
	virtual void processValue( sal_Int32 x, sal_Int32 y, const uno::Any& aValue )
	{
		uno::Sequence< uno::Sequence< uno::Any > >& aMatrix = *( uno::Sequence< uno::Sequence< uno::Any > >* )( maValue.getValue() );
		aMatrix[x][y] = aValue;
	}

public:
	Dim2ArrayValueGetter(sal_Int32 nRowCount, sal_Int32 nColCount, ValueGetter& rValueGetter ): mValueGetter(rValueGetter) 
	{
		uno::Sequence< uno::Sequence< uno::Any > > aMatrix;
		aMatrix.realloc( nRowCount );	
		for ( sal_Int32 index = 0; index < nRowCount; ++index )
			aMatrix[index].realloc( nColCount );
		maValue <<= aMatrix;
	}
	void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )

	{
		mValueGetter.visitNode( x, y, xCell );
		processValue( x, y, mValueGetter.getValue() );
	}
	const uno::Any& getValue() const { return maValue; }

};

const static rtl::OUString sNA = rtl::OUString::createFromAscii("#N/A"); 

class Dim1ArrayValueSetter : public ArrayVisitor
{
	uno::Sequence< uno::Any > aMatrix;
	sal_Int32 nColCount;
	ValueSetter& mCellValueSetter;
public:
	Dim1ArrayValueSetter( const uno::Any& aValue, ValueSetter& rCellValueSetter ):mCellValueSetter( rCellValueSetter )
	{
		aValue >>= aMatrix;
		nColCount = aMatrix.getLength();
	}
	virtual void visitNode( sal_Int32 /*x*/, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
	{
		if ( y < nColCount )
			mCellValueSetter.processValue( aMatrix[ y ], xCell );
		else
			mCellValueSetter.processValue( uno::makeAny( sNA ), xCell );
	}
};



class Dim2ArrayValueSetter : public ArrayVisitor
{
	uno::Sequence< uno::Sequence< uno::Any > > aMatrix;
	ValueSetter& mCellValueSetter;
	sal_Int32 nRowCount;
	sal_Int32 nColCount;
public:
	Dim2ArrayValueSetter( const uno::Any& aValue, ValueSetter& rCellValueSetter ) : mCellValueSetter( rCellValueSetter )
	{
		aValue >>= aMatrix;
		nRowCount = aMatrix.getLength();
		nColCount = aMatrix[0].getLength();  
	}

	virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
	{
		if ( x < nRowCount && y < nColCount )
			mCellValueSetter.processValue( aMatrix[ x ][ y ], xCell );
		else
			mCellValueSetter.processValue( uno::makeAny( sNA ), xCell );
			
	}
};

class RangeProcessor
{
public:
	virtual void process( const uno::Reference< vba::XRange >& xRange ) = 0;
};

class RangeValueProcessor : public RangeProcessor
{
	const uno::Any& m_aVal;
public:
	RangeValueProcessor( const uno::Any& rVal ):m_aVal( rVal ) {}
	virtual void process( const uno::Reference< vba::XRange >& xRange )
	{
		xRange->setValue( m_aVal );
	}
};

class RangeFormulaProcessor : public RangeProcessor
{
	const uno::Any& m_aVal;
public:
	RangeFormulaProcessor( const uno::Any& rVal ):m_aVal( rVal ) {}
	virtual void process( const uno::Reference< vba::XRange >& xRange ) 
	{
		xRange->setFormula( m_aVal );
	}
};

class RangeCountProcessor : public RangeProcessor
{
	double nCount;
public:
	RangeCountProcessor():nCount(0){}
	virtual void process( const uno::Reference< vba::XRange >& xRange )
	{
		nCount = nCount + xRange->getCount();
	}
	double value() { return nCount; }
};
class AreasVisitor
{
private:
	uno::Reference< vba::XCollection > m_Areas;
public:
	AreasVisitor( const uno::Reference< vba::XCollection >& rAreas ):m_Areas( rAreas ){} 
	
	void visit( RangeProcessor& processor )
	{
		if ( m_Areas.is() )
		{
			sal_Int32 nItems = m_Areas->getCount();
			for ( sal_Int32 index=1; index <= nItems; ++index )
			{
				uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
				processor.process( xRange ); 
			}
		}	
	}
};

class RangeHelper
{
	uno::Reference< table::XCellRange > m_xCellRange;
	
public:
	RangeHelper( const uno::Reference< table::XCellRange >& xCellRange ) throw (uno::RuntimeException) : m_xCellRange( xCellRange ) 
	{
		if ( !m_xCellRange.is() )
			throw uno::RuntimeException();
	}
	RangeHelper( const uno::Any aCellRange ) throw (uno::RuntimeException)
	{
		m_xCellRange.set( aCellRange, uno::UNO_QUERY_THROW );
	}
	uno::Reference< sheet::XSheetCellRange > getSheetCellRange() throw (uno::RuntimeException)
	{
		return uno::Reference< sheet::XSheetCellRange >(m_xCellRange, uno::UNO_QUERY_THROW);
	}
	uno::Reference< sheet::XSpreadsheet >  getSpreadSheet() throw (uno::RuntimeException)
	{
		return getSheetCellRange()->getSpreadsheet();
	}	

	uno::Reference< table::XCellRange > getCellRangeFromSheet() throw (uno::RuntimeException)
	{
		return uno::Reference< table::XCellRange >(getSpreadSheet(), uno::UNO_QUERY_THROW );
	}

	uno::Reference< sheet::XCellRangeAddressable >  getCellRangeAddressable() throw (uno::RuntimeException)
	{
		return uno::Reference< sheet::XCellRangeAddressable >(m_xCellRange, ::uno::UNO_QUERY_THROW);

	}

	uno::Reference< sheet::XSheetCellCursor > getSheetCellCursor() throw ( uno::RuntimeException )
	{
		return 	uno::Reference< sheet::XSheetCellCursor >( getSpreadSheet()->createCursorByRange( getSheetCellRange() ), uno::UNO_QUERY_THROW );
	}	

	static uno::Reference< vba::XRange > createRangeFromRange( const uno::Reference<uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange, const uno::Reference< sheet::XCellRangeAddressable >& xCellRangeAddressable, sal_Int32 nStartColOffset = 0, sal_Int32 nStartRowOffset = 0,
 sal_Int32 nEndColOffset = 0, sal_Int32 nEndRowOffset = 0 )
	{
		return uno::Reference< vba::XRange >( new ScVbaRange( xContext, 
			xRange->getCellRangeByPosition(
				xCellRangeAddressable->getRangeAddress().StartColumn + nStartColOffset,
				xCellRangeAddressable->getRangeAddress().StartRow + nStartRowOffset,
				xCellRangeAddressable->getRangeAddress().EndColumn + nEndColOffset,
				xCellRangeAddressable->getRangeAddress().EndRow + nEndRowOffset ) ) );
	}
	
};

static table::CellRangeAddress getCellRangeAddress( const uno::Any& aParam,
const uno::Reference< sheet::XSpreadsheet >& xDoc )
{
	uno::Reference< table::XCellRange > xRangeParam;
	switch ( aParam.getValueTypeClass() )
	{
		case uno::TypeClass_STRING:
		{
			rtl::OUString rString;
			aParam >>= rString;
			xRangeParam = ScVbaRange::getCellRangeForName( rString, xDoc );
			break;
		}
		case uno::TypeClass_INTERFACE:
		{
			uno::Reference< vba::XRange > xRange;
			aParam >>= xRange;
			if ( xRange.is() )
				xRange->getCellRange() >>= xRangeParam;
			break;
		}
		default:
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Can't extact CellRangeAddress from type" ) ), uno::Reference< uno::XInterface >() );
	}
	uno::Reference< sheet::XCellRangeAddressable > xAddressable( xRangeParam, uno::UNO_QUERY_THROW );
	return xAddressable->getRangeAddress();
}

bool
getCellRangesForAddress( USHORT& rResFlags, const rtl::OUString& sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, ScAddress::Convention& eConv )
{
	
	ScDocument* pDoc = NULL;
	if ( pDocSh )
	{
		pDoc = pDocSh->GetDocument();
		String aString(sAddress);
		USHORT nMask = SCA_VALID;
		//USHORT nParse = rCellRanges.Parse( sAddress, pDoc, nMask, ScAddress::CONV_XL_A1 );
		rResFlags = rCellRanges.Parse( sAddress, pDoc, nMask, eConv, 0 );
		if ( rResFlags & SCA_VALID )
		{
			return true;
		}
	} 
	return false;
} 

ScVbaRange*
getRangeForName( const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sName, ScDocShell* pDocSh, table::CellRangeAddress& pAddr ) throw ( uno::RuntimeException )
{
	rtl::OUString sAddress = sName;
	ScAddress::Convention eConv = ScAddress::CONV_XL_A1; 
	// see if there is a match with a named range
	uno::Reference< beans::XPropertySet > xProps( getCurrentDocument(), uno::UNO_QUERY_THROW );
	uno::Reference< container::XNameAccess > xNameAccess( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NamedRanges") ) ), uno::UNO_QUERY_THROW );

	if ( xNameAccess->hasByName( sName ) )
	{
		uno::Reference< sheet::XNamedRange > xNamed( xNameAccess->getByName( sName ), uno::UNO_QUERY_THROW );
		sAddress = xNamed->getContent();
		// As the address comes from OOO, the addressing
		// style is may not be XL_A1
		eConv = pDocSh->GetDocument()->GetAddressConvention();
	}	
	ScRange refRange;	
	ScUnoConversion::FillScRange( refRange, pAddr );
	ScRangeList aCellRanges;
	USHORT nFlags = 0;
	if ( !getCellRangesForAddress( nFlags, sAddress, pDocSh, aCellRanges, eConv ) )
		throw uno::RuntimeException();

	bool bTabFromReferrer = !( nFlags & SCA_TAB_3D );

	for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
	{
		pRange->aStart.SetCol( refRange.aStart.Col() + pRange->aStart.Col() );
		pRange->aStart.SetRow( refRange.aStart.Row() + pRange->aStart.Row() );
                pRange->aStart.SetTab( bTabFromReferrer ? refRange.aStart.Tab()  : pRange->aStart.Tab() );
		pRange->aEnd.SetCol( refRange.aStart.Col() + pRange->aEnd.Col() );
		pRange->aEnd.SetRow( refRange.aStart.Row() + pRange->aEnd.Row() );
                pRange->aEnd.SetTab( bTabFromReferrer ? refRange.aEnd.Tab()  : pRange->aEnd.Tab() );
	}

	// Single range
	if ( aCellRanges.First() == aCellRanges.Last() )
	{
		uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pDocSh, *aCellRanges.First() ) );
		return new ScVbaRange( xContext, xRange );
	}
	uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDocSh, aCellRanges ) );
 	
	return new ScVbaRange( xContext, xRanges );
	
}

uno::Reference< vba::XCollection >
lcl_setupBorders( const uno::Reference<uno::XComponentContext>& xContext,  uno::Reference< table::XCellRange >& xRange  ) throw( uno::RuntimeException )
{
	ScDocument* pDoc = getDocumentFromRange(xRange);
	if ( !pDoc )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );
	ScVbaPalette aPalette( pDoc->GetDocumentShell() );
 	uno::Reference< vba::XCollection > borders( new ScVbaBorders( xContext, xRange, aPalette ) );
	return borders;
}

void
ScVbaRange::setDfltPropHandler()
{

	msDftPropName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Value" ) );
	registerProperty( RANGE_PROPERTY_DFLT, RANGE_PROPERTY_ID_DFLT,
beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::BOUND, &msDftPropName, ::getCppuType( &msDftPropName ) );
}

ScVbaRange::ScVbaRange( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange, sal_Bool bIsRows, sal_Bool bIsColumns ) throw( lang::IllegalArgumentException )
:OPropertyContainer(GetBroadcastHelper())
,mxRange( xRange ),
                m_xContext(xContext),
                mbIsRows( bIsRows ),
                mbIsColumns( bIsColumns )
{
	if  ( !xContext.is() )
		throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "context is not set " ) ), uno::Reference< uno::XInterface >() , 1 );
	if  ( !xRange.is() )
		throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "range is not set " ) ), uno::Reference< uno::XInterface >() , 1 );

	uno::Reference< container::XIndexAccess > xIndex( new SingleRangeIndexAccess( m_xContext, xRange ) );
	m_Areas = new ScVbaRangeAreas( m_xContext, xIndex );
	
	m_Borders = lcl_setupBorders( m_xContext, mxRange );
	setDfltPropHandler();

}

ScVbaRange::ScVbaRange( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges,  sal_Bool bIsRows, sal_Bool bIsColumns  ) throw ( lang::IllegalArgumentException )
:OPropertyContainer(GetBroadcastHelper()),  m_xContext(xContext), mxRanges( xRanges ),mbIsRows( bIsRows ), mbIsColumns( bIsColumns )

{
	uno::Reference< container::XIndexAccess >  xIndex( mxRanges, uno::UNO_QUERY_THROW );
	m_Areas	 = new ScVbaRangeAreas( m_xContext, xIndex );

	// Some methods functions seem to operate on the first range defined
	// but I don't want to enable the line below right now, I'd prefer the 
	// Selection stuff to be non functional and enable it method by method
	//m_xRange.set( mxRanges->getByIndex( 0 ); 
	setDfltPropHandler();
	uno::Reference< table::XCellRange > xRange( mxRanges->getByIndex(0), uno::UNO_QUERY_THROW );
	m_Borders = lcl_setupBorders( m_xContext, xRange );
}

ScVbaRange::~ScVbaRange()
{
}

uno::Reference< script::XTypeConverter >
ScVbaRange::getTypeConverter() throw (uno::RuntimeException)
{
	static uno::Reference< script::XTypeConverter > xTypeConv( m_xContext->getServiceManager()->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter") ), m_xContext ), uno::UNO_QUERY_THROW );
	return xTypeConv;
}

void
ScVbaRange::visitArray( ArrayVisitor& visitor )
{
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
	sal_Int32 nRowCount = xColumnRowRange->getRows()->getCount();
	sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
	for ( sal_Int32 i=0; i<nRowCount; ++i )
	{
		for ( sal_Int32 j=0; j<nColCount; ++j )
		{
			uno::Reference< table::XCell > xCell( mxRange->getCellByPosition( j, i ), uno::UNO_QUERY_THROW );

			visitor.visitNode( i, j, xCell );
		}
	}
}



uno::Any 
ScVbaRange::getValue( ValueGetter& valueGetter) throw (uno::RuntimeException)
{
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
	// single cell range
	if ( isSingleCellRange() )
	{
		visitArray( valueGetter );
		return valueGetter.getValue();
	}
	sal_Int32 nRowCount = xColumnRowRange->getRows()->getCount();
	sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
	// multi cell range ( return array )
	Dim2ArrayValueGetter arrayGetter( nRowCount, nColCount, valueGetter );
	visitArray( arrayGetter );
	return uno::makeAny( script::ArrayWrapper( sal_False, arrayGetter.getValue() ) );
}

uno::Any SAL_CALL
ScVbaRange::getValue() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getValue();
	}

	CellValueGetter valueGetter;
	return getValue( valueGetter );

}


void 
ScVbaRange::setValue(  const uno::Any  &aValue,  ValueSetter& valueSetter ) throw (uno::RuntimeException)
{
	uno::TypeClass aClass = aValue.getValueTypeClass();
	if ( aClass == uno::TypeClass_SEQUENCE )
	{
		uno::Reference< script::XTypeConverter > xConverter = getTypeConverter();
		uno::Any aConverted;
		try
		{
			// test for single dimension, could do 
			// with a better test than this	
			if ( aValue.getValueTypeName().indexOf('[') ==  aValue.getValueTypeName().lastIndexOf('[') )
			{
				aConverted = xConverter->convertTo( aValue, getCppuType((uno::Sequence< uno::Any >*)0) );
				Dim1ArrayValueSetter setter( aConverted, valueSetter );
				visitArray( setter );
			}
			else
			{
				aConverted = xConverter->convertTo( aValue, getCppuType((uno::Sequence< uno::Sequence< uno::Any > >*)0) );
				Dim2ArrayValueSetter setter( aConverted, valueSetter );
				visitArray( setter );
			}
		}
		catch ( uno::Exception& e )
		{
			OSL_TRACE("Bahhh, caught exception %s", 
				rtl::OUStringToOString( e.Message,
					RTL_TEXTENCODING_UTF8 ).getStr() );
		}
	}
	else
	{
		visitArray( valueSetter );
	}
}

void SAL_CALL
ScVbaRange::setValue( const uno::Any  &aValue ) throw (uno::RuntimeException)
{
	// If this is a multiple selection apply setValue over all areas
	if ( m_Areas->getCount() > 1 )
	{
		AreasVisitor aVisitor( m_Areas );
		RangeValueProcessor valueProcessor( aValue );	
		aVisitor.visit( valueProcessor );
		return;
	}	
	CellValueSetter valueSetter( aValue );
	setValue( aValue, valueSetter );
}

void
ScVbaRange::Clear() throw (uno::RuntimeException)
{
	sal_Int32 nFlags = sheet::CellFlags::VALUE | sheet::CellFlags::STRING | sheet::CellFlags::HARDATTR | sheet::CellFlags::FORMATTED | sheet::CellFlags::EDITATTR | sheet::CellFlags::FORMULA;
	ClearContents( nFlags );
}

//helper ClearContent
void
ScVbaRange::ClearContents( sal_Int32 nFlags ) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() ); 
			if ( pRange )
				pRange->ClearContents( nFlags );	
		}
		return;
	}


	uno::Reference< sheet::XSheetOperation > xSheetOperation(mxRange, uno::UNO_QUERY_THROW);
	xSheetOperation->clearContents( nFlags );
}
void
ScVbaRange::ClearComments() throw (uno::RuntimeException)
{
	ClearContents( sheet::CellFlags::ANNOTATION );
}

void
ScVbaRange::ClearContents() throw (uno::RuntimeException)
{
	sal_Int32 nClearFlags = ( sheet::CellFlags::VALUE |
		sheet::CellFlags::STRING |  sheet::CellFlags::DATETIME | 
		sheet::CellFlags::FORMULA );
	ClearContents( nClearFlags );
}

void
ScVbaRange::ClearFormats() throw (uno::RuntimeException)
{
	//FIXME: need to check if we need to combine sheet::CellFlags::FORMATTED
	sal_Int32 nClearFlags = sheet::CellFlags::HARDATTR | sheet::CellFlags::FORMATTED | sheet::CellFlags::EDITATTR;
	ClearContents( nClearFlags );
}

void
ScVbaRange::setFormulaValue( const uno::Any& rFormula, ScAddress::Convention eConv ) throw (uno::RuntimeException)
{
	// If this is a multiple selection apply setFormula over all areas
	if ( m_Areas->getCount() > 1 )
	{
		AreasVisitor aVisitor( m_Areas );
		RangeFormulaProcessor valueProcessor( rFormula );	
		aVisitor.visit( valueProcessor );
		return;
	}	
	CellFormulaValueSetter formulaValueSetter( rFormula, getDocumentFromRange( mxRange ), eConv );
	setValue( rFormula, formulaValueSetter );
}

uno::Any 
ScVbaRange::getFormulaValue( ScAddress::Convention eConv) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getFormula();
	}
	CellFormulaValueGetter valueGetter( getDocumentFromRange( mxRange ), eConv );
	return getValue( valueGetter );
		
}

void
ScVbaRange::setFormula(const uno::Any &rFormula ) throw (uno::RuntimeException)
{
	setFormulaValue( rFormula, ScAddress::CONV_XL_A1 );;
}

uno::Any
ScVbaRange::getFormulaR1C1() throw (::com::sun::star::uno::RuntimeException)
{
	return getFormulaValue( ScAddress::CONV_XL_R1C1 );
}

void
ScVbaRange::setFormulaR1C1(const uno::Any& rFormula ) throw (uno::RuntimeException)
{
	setFormulaValue( rFormula, ScAddress::CONV_XL_R1C1 );
}

uno::Any
ScVbaRange::getFormula() throw (::com::sun::star::uno::RuntimeException)
{
	return getFormulaValue( ScAddress::CONV_XL_A1 );
}

double 
ScVbaRange::getCount() throw (uno::RuntimeException)
{
	// If this is a multiple selection apply setValue over all areas
	if ( m_Areas->getCount() > 1 )
	{
		AreasVisitor aVisitor( m_Areas );
		RangeCountProcessor valueProcessor;
		aVisitor.visit( valueProcessor );
		return valueProcessor.value();
	}	
	double rowCount, colCount;
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
	rowCount = xColumnRowRange->getRows()->getCount();
	colCount = xColumnRowRange->getColumns()->getCount();
	if( IsRows() )
		return rowCount;	
	if( IsColumns() )
		return colCount;
	return rowCount * colCount;
}

sal_Int32 
ScVbaRange::getRow() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getRow();
	}
	uno::Reference< sheet::XCellAddressable > xCellAddressable(mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
	return xCellAddressable->getCellAddress().Row + 1; // Zero value indexing 
}	
		
sal_Int32 
ScVbaRange::getColumn() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getColumn();
	}
	uno::Reference< sheet::XCellAddressable > xCellAddressable(mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
	return xCellAddressable->getCellAddress().Column + 1; // Zero value indexing
}

uno::Any
ScVbaRange::HasFormula() throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		uno::Any aResult = aNULL();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			// if the HasFormula for any area is different to another
			// return null
			if ( index > 1 )
				if ( aResult != xRange->HasFormula() )
					return aNULL();
			aResult = xRange->HasFormula();	
			if ( aNULL() == aResult ) 
				return aNULL();
		}
		return aResult;
	}

	ScCellRangesBase* pThisRanges = dynamic_cast< ScCellRangesBase * > ( mxRange.get() );
	if ( pThisRanges )
	{
		uno::Reference<sheet::XSheetCellRanges>  xRanges( pThisRanges->queryFormulaCells( ( sheet::FormulaResult::ERROR | sheet::FormulaResult::VALUE |  sheet::FormulaResult::STRING ) ), uno::UNO_QUERY_THROW );
		ScCellRangesBase* pFormulaRanges = dynamic_cast< ScCellRangesBase * > ( xRanges.get() );
		// check if there are no formula cell, return false
		if ( pFormulaRanges->GetRangeList().Count() == 0 ) 
			return uno::makeAny(sal_False);
		
		// chech if there are holes (where some cells are not formulas)
		// or returned range is not equal to this range	 
		if ( ( pFormulaRanges->GetRangeList().Count() > 1 ) 
		|| ( pFormulaRanges->GetRangeList().GetObject(0)->aStart != pThisRanges->GetRangeList().GetObject(0)->aStart ) 
		|| ( pFormulaRanges->GetRangeList().GetObject(0)->aEnd != pThisRanges->GetRangeList().GetObject(0)->aEnd ) )
			return aNULL(); // should return aNULL;
	}
	return uno::makeAny( sal_True );
}
void
ScVbaRange::fillSeries( sheet::FillDirection nFillDirection, sheet::FillMode nFillMode, sheet::FillDateMode nFillDateMode, double fStep, double fEndValue ) throw( uno::RuntimeException )
{
	if ( m_Areas->getCount() > 1 )
	{
		// Multi-Area Range
		uno::Reference< vba::XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
		for ( sal_Int32 index = 1; index <= xCollection->getCount(); ++index )
		{
			uno::Reference< vba::XRange > xRange( xCollection->Item( uno::makeAny( index ) ), uno::UNO_QUERY_THROW );
			ScVbaRange* pThisRange = dynamic_cast< ScVbaRange* >( xRange.get() );
			pThisRange->fillSeries( nFillDirection, nFillMode, nFillDateMode, fStep, fEndValue );
				
		}
		return;	
	}
	
	uno::Reference< sheet::XCellSeries > xCellSeries(mxRange, uno::UNO_QUERY_THROW );
	xCellSeries->fillSeries( nFillDirection, nFillMode, nFillDateMode, fStep, fEndValue );
}

void 
ScVbaRange::FillLeft() throw (uno::RuntimeException)
{
	fillSeries(sheet::FillDirection_TO_LEFT, 
		sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void 
ScVbaRange::FillRight() throw (uno::RuntimeException)
{
	fillSeries(sheet::FillDirection_TO_RIGHT, 
		sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void 
ScVbaRange::FillUp() throw (uno::RuntimeException)
{
	fillSeries(sheet::FillDirection_TO_TOP, 
		sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void 
ScVbaRange::FillDown() throw (uno::RuntimeException)
{
	fillSeries(sheet::FillDirection_TO_BOTTOM, 
		sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

::rtl::OUString
ScVbaRange::getText() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getText();
	}
	uno::Reference< text::XTextRange > xTextRange(mxRange->getCellByPosition(0,0), uno::UNO_QUERY_THROW );
	return xTextRange->getString();
}

uno::Reference< vba::XRange >
ScVbaRange::Offset( const ::uno::Any &nRowOff, const uno::Any &nColOff ) throw (uno::RuntimeException)
{
	SCROW nRowOffset = 0;
	SCCOL nColOffset = 0;
	sal_Bool bIsRowOffset = ( nRowOff >>= nRowOffset );
	sal_Bool bIsColumnOffset = ( nColOff >>= nColOffset );
	ScCellRangesBase* pUnoRangesBase = getCellRangesBase();

	ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();
	

	for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
	{
		if ( bIsColumnOffset )
		{
			pRange->aStart.SetCol( pRange->aStart.Col() + nColOffset );
			pRange->aEnd.SetCol( pRange->aEnd.Col() + nColOffset );
		}
		if ( bIsRowOffset )
		{
			pRange->aStart.SetRow( pRange->aStart.Row() + nRowOffset );
			pRange->aEnd.SetRow( pRange->aEnd.Row() + nRowOffset );
		}
	}

	if ( aCellRanges.Count() > 1 ) // Multi-Area
	{
		uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );
		
		return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, xRanges ) );
	}
	// normal range
	uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.First() ) );
	return new ScVbaRange( m_xContext, xRange  );
}

uno::Reference< vba::XRange >
ScVbaRange::CurrentRegion() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->CurrentRegion();
	}
	
	RangeHelper helper( mxRange );
	uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = 
		helper.getSheetCellCursor();
	xSheetCellCursor->collapseToCurrentRegion();
	uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
	return RangeHelper::createRangeFromRange( m_xContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );	
}

uno::Reference< vba::XRange >
ScVbaRange::CurrentArray() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->CurrentArray();
	}
	RangeHelper helper( mxRange );
	uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = 
		helper.getSheetCellCursor();
	xSheetCellCursor->collapseToCurrentArray();
	uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
	return RangeHelper::createRangeFromRange( m_xContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );	
}

uno::Any
ScVbaRange::getFormulaArray() throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->getFormulaArray();
	}
	
	uno::Reference< sheet::XCellRangeFormula> xCellRangeFormula( mxRange, uno::UNO_QUERY_THROW );
	uno::Reference< script::XTypeConverter > xConverter = getTypeConverter();
	uno::Any aMatrix;
	aMatrix = xConverter->convertTo( uno::makeAny( xCellRangeFormula->getFormulaArray() ) , getCppuType((uno::Sequence< uno::Sequence< uno::Any > >*)0)  ) ;
	return aMatrix;
}

void 
ScVbaRange::setFormulaArray(const uno::Any& rFormula) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->setFormulaArray( rFormula );
	}
	// #TODO need to distinguish between getFormula and getFormulaArray e.g. (R1C1)
	// but for the moment its just easier to treat them the same for setting

	setFormula( rFormula );
}

::rtl::OUString
ScVbaRange::Characters(const uno::Any& Start, const uno::Any& Length) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->Characters( Start, Length );
	}

	long nIndex, nCount;
	::rtl::OUString rString;
	uno::Reference< text::XTextRange > xTextRange(mxRange, ::uno::UNO_QUERY_THROW );
	rString = xTextRange->getString();
	if( !( Start >>= nIndex ) && !( Length >>= nCount ) )
		return rString;
	if(!( Start >>= nIndex ) )
		nIndex = 1;
	if(!( Length >>= nCount ) )
		nIndex = rString.getLength();
	return rString.copy( --nIndex, nCount ); // Zero value indexing
}

::rtl::OUString
ScVbaRange::Address(  const uno::Any& RowAbsolute, const uno::Any& ColumnAbsolute, const uno::Any& ReferenceStyle, const uno::Any& External, const uno::Any& RelativeTo ) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		// Multi-Area Range
		rtl::OUString sAddress;
		uno::Reference< vba::XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
                uno::Any aExternalCopy = External;
		for ( sal_Int32 index = 1; index <= xCollection->getCount(); ++index )
		{
			uno::Reference< vba::XRange > xRange( xCollection->Item( uno::makeAny( index ) ), uno::UNO_QUERY_THROW );
			if ( index > 1 )
			{
				sAddress += rtl::OUString( ',' );
                                // force external to be false
                                // only first address should have the
                                // document and sheet specifications
                                aExternalCopy = uno::makeAny(sal_False);
			}
			sAddress += xRange->Address( RowAbsolute, ColumnAbsolute, ReferenceStyle, aExternalCopy, RelativeTo );
		}
		return sAddress;	
		
	}
	ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
	if ( ReferenceStyle.hasValue() )
	{
		sal_Int32 refStyle = vba::Excel::XlReferenceStyle::xlA1;
		ReferenceStyle >>= refStyle;
		if ( refStyle == vba::Excel::XlReferenceStyle::xlR1C1 )
			dDetails = ScAddress::Details( ScAddress::CONV_XL_R1C1, 0, 0 );
	}
	USHORT nFlags = SCA_VALID;
	ScDocument* pDoc =  getDocumentFromRange( mxRange );
	RangeHelper thisRange( mxRange );	
	table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
	ScRange aRange( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCROW >( thisAddress.StartRow ), static_cast< SCTAB >( thisAddress.Sheet ), static_cast< SCCOL >( thisAddress.EndColumn ), static_cast< SCROW >( thisAddress.EndRow ), static_cast< SCTAB >( thisAddress.Sheet ) );
	String sRange;
	USHORT ROW_ABSOLUTE = ( SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE );
	USHORT COL_ABSOLUTE = ( SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE );
	// default
	nFlags |= ( SCA_TAB_ABSOLUTE | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE );
	if ( RowAbsolute.hasValue() )
	{
		sal_Bool bVal = sal_True;
		RowAbsolute >>= bVal;
		if ( !bVal )
			nFlags &= ~ROW_ABSOLUTE;
	}
	if ( ColumnAbsolute.hasValue() )
	{
		sal_Bool bVal = sal_True;
		ColumnAbsolute >>= bVal;
		if ( !bVal )
			nFlags &= ~COL_ABSOLUTE;
	}
	sal_Bool bLocal = sal_False;
	if ( External.hasValue() )
	{
		External >>= bLocal;
		if (  bLocal )
			nFlags |= SCA_TAB_3D | SCA_FORCE_DOC;
	}
	if ( RelativeTo.hasValue() )
	{
		// #TODO should I throw an error if R1C1 is not set?
		
		table::CellRangeAddress refAddress = getCellRangeAddress( RelativeTo, thisRange.getSpreadSheet() );
		dDetails = ScAddress::Details( ScAddress::CONV_XL_R1C1, static_cast< SCROW >( refAddress.StartRow ), static_cast< SCCOL >( refAddress.StartColumn ) );
	}
	aRange.Format( sRange,  nFlags, pDoc, dDetails ); 
	return sRange;
}

uno::Reference < vba::XFont >
ScVbaRange::Font() throw (uno::RuntimeException)
{
	uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY );
	ScDocument* pDoc = getDocumentFromRange(mxRange);
	if ( !pDoc )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

	ScVbaPalette aPalette( pDoc->GetDocumentShell() );	
	return uno::Reference< vba::XFont >( new ScVbaFont( aPalette, xProps, getCurrentDataSet() ) );
}
                                                                                                                             
uno::Reference< vba::XRange >
ScVbaRange::Cells( const uno::Any &nRowIndex, const uno::Any &nColumnIndex ) throw(uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->Cells( nRowIndex, nColumnIndex );
	}

	sal_Int32 nRow = 0, nColumn = 0;
	sal_Bool bIsIndex = nRowIndex >>= nRow, bIsColumnIndex = nColumnIndex >>= nColumn;
                                                                                                                       
	RangeHelper thisRange( mxRange );
	table::CellRangeAddress thisRangeAddress =  thisRange.getCellRangeAddressable()->getRangeAddress();
	uno::Reference< table::XCellRange > xSheetRange = thisRange.getCellRangeFromSheet();
	if( !bIsIndex && !bIsColumnIndex ) // .Cells
		return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, mxRange ) );

	sal_Int32 nIndex = --nRow;
	if( bIsIndex && !bIsColumnIndex ) // .Cells(n)
	{
		uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, ::uno::UNO_QUERY_THROW);
		sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();

		if ( !nIndex || nIndex < 0 )
			nRow = 0;
		else
			nRow = nIndex / nColCount;
		nColumn = nIndex % nColCount;
	}
	else
		--nColumn;
	nRow = nRow + thisRangeAddress.StartRow;
	nColumn =  nColumn + thisRangeAddress.StartColumn;	

	return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, xSheetRange->getCellRangeByPosition( nColumn, nRow,                                        nColumn, nRow ) ) );
}

void
ScVbaRange::Select() throw (uno::RuntimeException)
{
	uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
	uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
	if ( mxRanges.is() )
		xSelection->select( uno::makeAny( mxRanges ) );
	else
		xSelection->select( uno::makeAny( mxRange ) );
}

bool cellInRange( const table::CellRangeAddress& rAddr, const sal_Int32& nCol, const sal_Int32& nRow )
{
	if ( nCol >= rAddr.StartColumn && nCol <= rAddr.EndColumn &&
		nRow >= rAddr.StartRow && nRow <= rAddr.EndRow )
		return true;
	return false;
}

void setCursor(  const SCCOL& nCol, const SCROW& nRow, bool bInSel = true )
{
	ScTabViewShell* pShell = getCurrentBestViewShell();
	if ( pShell )
	{
		if ( bInSel )
			pShell->SetCursor( nCol, nRow );
		else
			pShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_NONE, FALSE, FALSE, TRUE, FALSE );
	}
}

void
ScVbaRange::Activate() throw (uno::RuntimeException)
{
	RangeHelper thisRange( mxRange );
	uno::Reference< sheet::XCellRangeAddressable > xThisRangeAddress = thisRange.getCellRangeAddressable();
	table::CellRangeAddress thisRangeAddress = xThisRangeAddress->getRangeAddress();
	
	// get current selection
	uno::Reference< sheet::XCellRangeAddressable > xRange( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);

	uno::Reference< sheet::XSheetCellRanges > xRanges( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);

	if ( xRanges.is() )
	{
		uno::Sequence< table::CellRangeAddress > nAddrs = xRanges->getRangeAddresses();
		for ( sal_Int32 index = 0; index < nAddrs.getLength(); ++index )
		{
			if ( cellInRange( nAddrs[index], thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
			{
				setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ) );
				return;
			}
			
		}
	}	

	if ( xRange.is() && cellInRange( xRange->getRangeAddress(), thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
		setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ) );
	else
	{
		// if this range is multi cell select the range other
		// wise just position the cell at this single range position
		if ( isSingleCellRange() ) 
			// This top-leftmost cell of this Range is not in the current
			// selection so just select this range
			setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), false  );
		else
			Select();
	}

}

uno::Reference< vba::XRange >
ScVbaRange::Rows(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->Rows( aIndex );
	}
	
	sal_Int32 nValue;
	rtl::OUString sAddress;
	if( aIndex.hasValue() )
	{
		uno::Reference< sheet::XCellRangeAddressable > xAddressable( mxRange, uno::UNO_QUERY );
		table::CellRangeAddress aAddress = xAddressable->getRangeAddress();
		if( aIndex >>= nValue )
		{
			aAddress.StartRow = --nValue;
			aAddress.EndRow = nValue;
		}
	
		else if ( aIndex >>= sAddress ) 
		{
				ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
			ScRange aRange;
			aRange.ParseRows( sAddress, getDocumentFromRange( mxRange ), dDetails );
			aAddress.StartRow = aRange.aStart.Row();
			aAddress.EndRow = aRange.aEnd.Row();
		}
		else
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Illegal param" ) ), uno::Reference< uno::XInterface >() );

		return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, mxRange->getCellRangeByPosition(
						aAddress.StartColumn, aAddress.StartRow,
						aAddress.EndColumn, aAddress.EndRow ), true ) 	); 	
	}
	// Questionable return, I'm just copying the invalid Any::value path
	// above. Would seem to me that this is an internal error and 
	// warrants an exception thrown
	return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, mxRange, true ) );
}	

uno::Reference< vba::XRange >
ScVbaRange::Columns( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	// #TODO code within the test below "if ( m_Areas.... " can be removed
	// Test is performed only because m_xRange is NOT set to be
	// the first range in m_Areas ( to force failure while
	// the implementations for each method are being updated )
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->Columns( aIndex );
	}
	if ( aIndex.hasValue() )
	{
		uno::Reference< vba::XRange > xRange;
		sal_Int32 nValue;
		rtl::OUString sAddress;
		RangeHelper thisRange( mxRange );
		uno::Reference< sheet::XCellRangeAddressable > xThisRangeAddress = thisRange.getCellRangeAddressable();
		uno::Reference< table::XCellRange > xRanges = thisRange.getCellRangeFromSheet();		
		table::CellRangeAddress thisRangeAddress = xThisRangeAddress->getRangeAddress();
		uno::Reference< table::XCellRange > xReferrer = xRanges->getCellRangeByPosition( thisRangeAddress.StartColumn, thisRangeAddress.StartRow, MAXCOL, thisRangeAddress.EndRow );
	
		if ( aIndex >>= nValue )
		{
			--nValue;
			// col value can expand outside this range
			// rows however cannot

			thisRangeAddress.StartColumn = nValue;	
			thisRangeAddress.EndColumn = nValue;	
		}
		else if ( aIndex >>= sAddress )
		{
			ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
			ScRange aRange;
			aRange.ParseCols( sAddress, getDocumentFromRange( mxRange ), dDetails );
			thisRangeAddress.StartColumn = aRange.aStart.Col();
			thisRangeAddress.EndColumn = aRange.aEnd.Col();
		}
		else
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Illegal param" ) ), uno::Reference< uno::XInterface >() );
		return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, xReferrer->getCellRangeByPosition( thisRangeAddress.StartColumn, thisRangeAddress.StartRow, thisRangeAddress.EndColumn, thisRangeAddress.EndRow ), false, true ) );
	}
	// otherwise return this object ( e.g for columns property with no
	// params
	return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, mxRange, false, true ) );
}

void
ScVbaRange::setMergeCells( sal_Bool bIsMerged ) throw (uno::RuntimeException)
{
	uno::Reference< util::XMergeable > xMerge( mxRange, ::uno::UNO_QUERY_THROW );
	//FIXME need to check whether all the cell contents are retained or lost by popping up a dialog 
	xMerge->merge( bIsMerged );
}
                                                                                                                             
sal_Bool
ScVbaRange::getMergeCells() throw (uno::RuntimeException)
{
	uno::Reference< util::XMergeable > xMerge( mxRange, ::uno::UNO_QUERY_THROW );
	return xMerge->getIsMerged();
}
                                                                                                                             
void
ScVbaRange::Copy(const ::uno::Any& Destination) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
	if ( Destination.hasValue() )
	{
		uno::Reference< vba::XRange > xRange( Destination, uno::UNO_QUERY_THROW );
		uno::Any aRange = xRange->getCellRange();
		uno::Reference< table::XCellRange > xCellRange;
		aRange >>= xCellRange;
		uno::Reference< sheet::XSheetCellRange > xSheetCellRange(xCellRange, ::uno::UNO_QUERY_THROW);
		uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
		uno::Reference< table::XCellRange > xDest( xSheet, uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XCellRangeMovement > xMover( xSheet, uno::UNO_QUERY_THROW);
		uno::Reference< sheet::XCellAddressable > xDestination( xDest->getCellByPosition(
												xRange->getColumn()-1,xRange->getRow()-1), uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XCellRangeAddressable > xSource( mxRange, uno::UNO_QUERY);
		xMover->copyRange( xDestination->getCellAddress(), xSource->getRangeAddress() );
	}
	else
	{
		Select();
		implnCopy();
	}
}

void
ScVbaRange::Cut(const ::uno::Any& Destination) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
	if (Destination.hasValue())
	{
		uno::Reference< vba::XRange > xRange( Destination, uno::UNO_QUERY_THROW );
		uno::Reference< table::XCellRange > xCellRange( xRange->getCellRange(), uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XSheetCellRange > xSheetCellRange(xCellRange, ::uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
		uno::Reference< table::XCellRange > xDest( xSheet, uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XCellRangeMovement > xMover( xSheet, uno::UNO_QUERY_THROW);
		uno::Reference< sheet::XCellAddressable > xDestination( xDest->getCellByPosition(
												xRange->getColumn()-1,xRange->getRow()-1), uno::UNO_QUERY);
		uno::Reference< sheet::XCellRangeAddressable > xSource( mxRange, uno::UNO_QUERY);
		xMover->moveRange( xDestination->getCellAddress(), xSource->getRangeAddress() );
	}
	{
		Select();
		implnCut();
	}
}
                                                                                                                             
void
ScVbaRange::setNumberFormat( const uno::Any& aFormat ) throw (uno::RuntimeException)
{
	rtl::OUString sFormat;
	aFormat >>= sFormat;
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			xRange->setNumberFormat( aFormat );	
		}
		return;
	}
	NumFormatHelper numFormat( mxRange );
	numFormat.setNumberFormat( sFormat );
}
                                                                                                                             
uno::Any
ScVbaRange::getNumberFormat() throw (uno::RuntimeException)
{

	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		uno::Any aResult = aNULL();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			// if the numberformat of one area is different to another
			// return null
			if ( index > 1 )
				if ( aResult != xRange->getNumberFormat() )
					return aNULL();
			aResult = xRange->getNumberFormat();	
			if ( aNULL() == aResult ) 
				return aNULL();
		}
		return aResult;
	}
	NumFormatHelper numFormat( mxRange );
	rtl::OUString sFormat = numFormat.getNumberFormatString();
	if ( sFormat.getLength() > 0 )
		return uno::makeAny( sFormat );
	return aNULL();
}

uno::Reference< vba::XRange >
ScVbaRange::Resize( const uno::Any &RowSize, const uno::Any &ColumnSize ) throw (uno::RuntimeException)
{
	long nRowSize, nColumnSize;
	sal_Bool bIsRowChanged = ( RowSize >>= nRowSize ), bIsColumnChanged = ( ColumnSize >>= nColumnSize );
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, ::uno::UNO_QUERY_THROW);
	uno::Reference< sheet::XSheetCellRange > xSheetRange(mxRange, ::uno::UNO_QUERY_THROW);
	uno::Reference< sheet::XSheetCellCursor > xCursor( xSheetRange->getSpreadsheet()->createCursorByRange(xSheetRange), ::uno::UNO_QUERY_THROW );

	if( !bIsRowChanged )
		nRowSize = xColumnRowRange->getRows()->getCount();
	if( !bIsColumnChanged )
		nColumnSize = xColumnRowRange->getColumns()->getCount();

	xCursor->collapseToSize( nColumnSize, nRowSize );
	uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xCursor, ::uno::UNO_QUERY_THROW );
	uno::Reference< table::XCellRange > xRange( xSheetRange->getSpreadsheet(), ::uno::UNO_QUERY_THROW );
	return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext,xRange->getCellRangeByPosition(
										xCellRangeAddressable->getRangeAddress().StartColumn,
										xCellRangeAddressable->getRangeAddress().StartRow,
										xCellRangeAddressable->getRangeAddress().EndColumn,
										xCellRangeAddressable->getRangeAddress().EndRow ) ) );
}
                                                                                                                             
void
ScVbaRange::setWrapText( const uno::Any& aIsWrapped ) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		uno::Any aResult;
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			xRange->setWrapText( aIsWrapped );	
		}
		return;
	}

	uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW );
	xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTextWrapped" ) ), aIsWrapped );
}
                                                                                                                             
uno::Any
ScVbaRange::getWrapText() throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		uno::Any aResult;
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			if ( index > 1 )
				if ( aResult != xRange->getWrapText() )
					return aNULL();
			aResult = xRange->getWrapText(); 
		}
		return aResult;
	}

	SfxItemSet* pDataSet = getCurrentDataSet();
	
	SfxItemState eState = pDataSet->GetItemState( ATTR_LINEBREAK, TRUE, NULL);
	if ( eState == SFX_ITEM_DONTCARE )
		return aNULL();
	
	uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW );
	uno::Any aValue = xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTextWrapped" ) ) );
	return aValue;
}

uno::Reference< vba::XInterior > ScVbaRange::Interior( ) throw (uno::RuntimeException)
{
	uno::Reference< beans::XPropertySet > xProps( mxRange, uno::UNO_QUERY_THROW );
        return uno::Reference<vba::XInterior> (new ScVbaInterior ( m_xContext, xProps, getDocumentFromRange( mxRange ) ));
}                                                                                                                             
uno::Reference< vba::XRange >
ScVbaRange::Range( const uno::Any &Cell1, const uno::Any &Cell2 ) throw (uno::RuntimeException)
{
    return Range( Cell1, Cell2, false );
}
uno::Reference< vba::XRange >
ScVbaRange::Range( const uno::Any &Cell1, const uno::Any &Cell2, bool bForceUseInpuRangeTab ) throw (uno::RuntimeException)

{
	RangeHelper thisRange( mxRange );
	uno::Reference< table::XCellRange > xRanges = thisRange.getCellRangeFromSheet();
	uno::Reference< sheet::XCellRangeAddressable > xAddressable( xRanges, uno::UNO_QUERY_THROW );

	uno::Reference< table::XCellRange > xReferrer = 
		xRanges->getCellRangeByPosition( getColumn()-1, getRow()-1, 
				xAddressable->getRangeAddress().EndColumn, 
				xAddressable->getRangeAddress().EndRow );
	// xAddressable now for this range	
	xAddressable.set( xReferrer, uno::UNO_QUERY_THROW );

	
	if( !Cell1.hasValue() )
		throw uno::RuntimeException(
			rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " Invalid Argument " ) ),
			uno::Reference< XInterface >() );

	table::CellRangeAddress resultAddress;
	table::CellRangeAddress parentRangeAddress = xAddressable->getRangeAddress();

	ScRange aRange;
	// Cell1 defined only
	if ( !Cell2.hasValue() )
	{
		rtl::OUString sName;
		Cell1 >>= sName;
		RangeHelper referRange( xReferrer );		
		table::CellRangeAddress referAddress = referRange.getCellRangeAddressable()->getRangeAddress();
		return getRangeForName( m_xContext, sName, getDocShellFromRange( mxRange ), referAddress );
 
	}
	else
	{
		table::CellRangeAddress  cell1, cell2;
		cell1 = getCellRangeAddress( Cell1, thisRange.getSpreadSheet() ); 	
		// Cell1 & Cell2 defined
		// Excel seems to combine the range as the range defined by
		// the combination of Cell1 & Cell2
	
		cell2 = getCellRangeAddress( Cell2, thisRange.getSpreadSheet() ); 	

		resultAddress.StartColumn = ( cell1.StartColumn <  cell2.StartColumn ) ? cell1.StartColumn : cell2.StartColumn;
		resultAddress.StartRow = ( cell1.StartRow <  cell2.StartRow ) ? cell1.StartRow : cell2.StartRow;
		resultAddress.EndColumn = ( cell1.EndColumn >  cell2.EndColumn ) ? cell1.EndColumn : cell2.EndColumn;
		resultAddress.EndRow = ( cell1.EndRow >  cell2.EndRow ) ? cell1.EndRow : cell2.EndRow;
		if ( bForceUseInpuRangeTab )
		{
			// this is a call from Application.Range( x,y )
			// its possiblefor x or y to specify a different sheet from
			// the current or active on ( but they must be the same )
			if ( cell1.Sheet != cell2.Sheet )
				throw uno::RuntimeException();
			parentRangeAddress.Sheet = cell1.Sheet;
		}
		else
		{
			// this is not a call from Application.Range( x,y )
			// if a different sheet from this range is specified it's
			// an error
			if ( parentRangeAddress.Sheet != cell1.Sheet 
			|| parentRangeAddress.Sheet != cell2.Sheet 
			)
				throw uno::RuntimeException();

		}
		ScUnoConversion::FillScRange( aRange, resultAddress );
	}
	ScRange parentAddress;
	ScUnoConversion::FillScRange( parentAddress, parentRangeAddress);	
	uno::Reference< table::XCellRange > xCellRange;
	if ( aRange.aStart.Col() >= 0 && aRange.aStart.Row() >= 0 && aRange.aEnd.Col() >= 0 && aRange.aEnd.Row() >= 0 )
	{
		sal_Int32 nStartX = parentAddress.aStart.Col() + aRange.aStart.Col();
		sal_Int32 nStartY = parentAddress.aStart.Row() + aRange.aStart.Row();
		sal_Int32 nEndX = parentAddress.aStart.Col() + aRange.aEnd.Col();
		sal_Int32 nEndY = parentAddress.aStart.Row() + aRange.aEnd.Row();

		if ( nStartX <= nEndX && nEndX <= parentAddress.aEnd.Col() &&
			 nStartY <= nEndY && nEndY <= parentAddress.aEnd.Row() )
		{
			ScRange aNew( (SCCOL)nStartX, (SCROW)nStartY, parentAddress.aStart.Tab(),
						  (SCCOL)nEndX, (SCROW)nEndY, parentAddress.aEnd.Tab() );
			xCellRange = new ScCellRangeObj( getDocShellFromRange( mxRange ), aNew );
		}
	}
		
	return uno::Reference< vba::XRange > ( new ScVbaRange( m_xContext, xCellRange )  );

}

// Allow access to underlying openoffice uno api ( useful for debugging
// with openoffice basic ) 
::com::sun::star::uno::Any SAL_CALL
ScVbaRange::getCellRange(  ) throw (::com::sun::star::uno::RuntimeException)
{
	uno::Any aAny;
	if ( mxRanges.is() )
		aAny <<= mxRanges;
	else if ( mxRange.is() )
		aAny <<= mxRange;
	return aAny;
}

static USHORT 
getPasteFlags (sal_Int32 Paste)
{
	USHORT nFlags = IDF_NONE;	
	switch (Paste) {
        case vba::xlPasteType::xlPasteComments: 
		nFlags = IDF_NOTE;break;
        case vba::xlPasteType::xlPasteFormats: 
		nFlags = IDF_ATTRIB;break;
        case vba::xlPasteType::xlPasteFormulas: 
		nFlags = IDF_FORMULA;break;
        case vba::xlPasteType::xlPasteFormulasAndNumberFormats : 
        case vba::Excel::XlFindLookIn::xlValues: 
        case vba::xlPasteType::xlPasteValues: 
		nFlags = ( IDF_VALUE | IDF_DATETIME | IDF_STRING | IDF_SPECIAL_BOOLEAN ); break;
        case vba::xlPasteType::xlPasteValuesAndNumberFormats:
		nFlags = IDF_VALUE | IDF_ATTRIB; break;
        case vba::xlPasteType::xlPasteColumnWidths:
        case vba::xlPasteType::xlPasteValidation: 
		nFlags = IDF_NONE;break;
	case vba::xlPasteType::xlPasteAll: 
        case vba::xlPasteType::xlPasteAllExceptBorders: 
	default:
		nFlags = IDF_ALL;break;
	};
return nFlags;
}

static USHORT 
getPasteFormulaBits( sal_Int32 Operation)
{
	USHORT nFormulaBits = PASTE_NOFUNC ;
	switch (Operation)
	{
	case vba::xlPasteSpecialOperation::xlPasteSpecialOperationAdd: 
		nFormulaBits = PASTE_ADD;break;
	case vba::xlPasteSpecialOperation::xlPasteSpecialOperationSubtract: 
		nFormulaBits = PASTE_SUB;break;
	case vba::xlPasteSpecialOperation::xlPasteSpecialOperationMultiply: 
		nFormulaBits = PASTE_MUL;break;
	case vba::xlPasteSpecialOperation::xlPasteSpecialOperationDivide:
		nFormulaBits = PASTE_DIV;break;

	case vba::xlPasteSpecialOperation::xlPasteSpecialOperationNone: 
	case vba::Excel::Constants::xlNone:
	default:
		nFormulaBits = PASTE_NOFUNC; break;
	};
	
return nFormulaBits;
}
void SAL_CALL 
ScVbaRange::PasteSpecial( const uno::Any& Paste, const uno::Any& Operation, const uno::Any& SkipBlanks, const uno::Any& Transpose ) throw (::com::sun::star::uno::RuntimeException) 
{
	if ( m_Areas->getCount() > 1 )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
	// set up defaults	
	sal_Int32 nPaste = vba::xlPasteType::xlPasteAll;
	sal_Int32 nOperation = vba::xlPasteSpecialOperation::xlPasteSpecialOperationNone;
	sal_Bool bTranspose = sal_False;
	sal_Bool bSkipBlanks = sal_False;

	if ( Paste.hasValue() )
		Paste >>= nPaste;
	if ( Operation.hasValue() )
		Operation >>= nOperation;
	if ( SkipBlanks.hasValue() )
		SkipBlanks >>= bSkipBlanks;
	if ( Transpose.hasValue() )
		Transpose >>= bTranspose;

	USHORT nFlags = getPasteFlags(nPaste);
	USHORT nFormulaBits = getPasteFormulaBits(nOperation);
	implnPasteSpecial(nFlags,nFormulaBits,bSkipBlanks,bTranspose);
}

uno::Reference< vba::XRange > 
ScVbaRange::getEntireColumnOrRow( bool bColumn ) throw (uno::RuntimeException)
{
	ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
	// copy the range list
	ScRangeList aCellRanges = pUnoRangesBase->GetRangeList(); 

	for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
	{
		if ( bColumn ) 
		{
			pRange->aStart.SetRow( 0 );
			pRange->aEnd.SetRow( MAXROW );
		}	
		else
		{
			pRange->aStart.SetCol( 0 );
			pRange->aEnd.SetCol( MAXCOL );
		}
	}
	if ( aCellRanges.Count() > 1 ) // Multi-Area
	{
		uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );
		
		return uno::Reference< vba::XRange >( new ScVbaRange( m_xContext, xRanges, !bColumn, bColumn ) );
	}
	uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.First() ) );
	return new ScVbaRange( m_xContext, xRange, !bColumn, bColumn  );
}

uno::Reference< vba::XRange > SAL_CALL 
ScVbaRange::getEntireRow() throw (uno::RuntimeException)
{
	return getEntireColumnOrRow(false);
}

uno::Reference< vba::XRange > SAL_CALL 
ScVbaRange::getEntireColumn() throw (uno::RuntimeException)
{
	return getEntireColumnOrRow();
}

uno::Reference< vba::XComment > SAL_CALL 
ScVbaRange::AddComment( const uno::Any& Text ) throw (uno::RuntimeException)
{
	uno::Reference< vba::XComment > xComment( new ScVbaComment( m_xContext, mxRange ) );
	// if you don't pass a valid text or if there is already a comment
	// associated with the range then return NULL
	if ( !xComment->Text( Text, uno::Any(), uno::Any() ).getLength() 
	||   xComment->Text( uno::Any(), uno::Any(), uno::Any() ).getLength() )
		return NULL;
	return xComment;
}

uno::Reference< vba::XComment > SAL_CALL
ScVbaRange::getComment() throw (uno::RuntimeException)
{
	// intentional behavior to return a null object if no
	// comment defined
	uno::Reference< vba::XComment > xComment( new ScVbaComment( m_xContext, mxRange ) );
	if ( !xComment->Text( uno::Any(), uno::Any(), uno::Any() ).getLength() )
		return NULL;
	return xComment;
	
}

uno::Reference< beans::XPropertySet >
getRowOrColumnProps( const uno::Reference< table::XCellRange >& xCellRange, bool bRows ) throw ( uno::RuntimeException )
{
	uno::Reference< table::XColumnRowRange > xColRow( xCellRange, uno::UNO_QUERY_THROW );
	uno::Reference< beans::XPropertySet > xProps;
	if ( bRows )
		xProps.set( xColRow->getRows(), uno::UNO_QUERY_THROW );
	else
		xProps.set( xColRow->getColumns(), uno::UNO_QUERY_THROW );
	return xProps;	
}

uno::Any SAL_CALL 
ScVbaRange::getHidden() throw (uno::RuntimeException)
{
	// if multi-area result is the result of the 
	// first area
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(sal_Int32(1)) ), uno::UNO_QUERY_THROW );
		return xRange->getHidden();	
	}
	bool bIsVisible = false;
	try
	{
		uno::Reference< beans::XPropertySet > xProps = getRowOrColumnProps( mxRange, mbIsRows );
		if ( !( xProps->getPropertyValue( ISVISIBLE ) >>= bIsVisible ) )
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to get IsVisible property")), uno::Reference< uno::XInterface >() );
	}
	catch( uno::Exception& e )
	{
		throw uno::RuntimeException( e.Message, uno::Reference< uno::XInterface >() );
	}
	return uno::makeAny( !bIsVisible ); 
}

void SAL_CALL 
ScVbaRange::setHidden( const uno::Any& _hidden ) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			xRange->setHidden( _hidden );	
		}
		return;
	}

	sal_Bool bHidden;
	if ( !(_hidden >>= bHidden) )
		throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to extract param for Hidden property" ) ), uno::Reference< uno::XInterface >() ); 

	try
	{
		uno::Reference< beans::XPropertySet > xProps = getRowOrColumnProps( mxRange, mbIsRows );
		xProps->setPropertyValue( ISVISIBLE, uno::makeAny( !bHidden ) );
	}
	catch( uno::Exception& e )
	{
		throw uno::RuntimeException( e.Message, uno::Reference< uno::XInterface >() );
	}	
}

::sal_Bool SAL_CALL 
ScVbaRange::Replace( const ::rtl::OUString& What, const ::rtl::OUString& Replacement, const uno::Any& LookAt, const uno::Any& SearchOrder, const uno::Any& MatchCase, const uno::Any& MatchByte, const uno::Any& SearchFormat, const uno::Any& ReplaceFormat  ) throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		for ( sal_Int32 index = 1; index <= m_Areas->getCount(); ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny( index ) ), uno::UNO_QUERY_THROW );
			xRange->Replace( What, Replacement,  LookAt, SearchOrder, MatchCase, MatchByte, SearchFormat, ReplaceFormat );
		}
		return sal_True; // seems to return true always ( or at least I haven't found the trick of 
	}	
	// sanity check required params
	if ( !What.getLength() || !Replacement.getLength() )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, missing params" )) , uno::Reference< uno::XInterface >() );

	// #TODO #FIXME SearchFormat & ReplacesFormat are not processed
	// What do we do about MatchByte.. we don't seem to support that
	const SvxSearchItem& globalSearchOptions = ScGlobal::GetSearchItem();
	SvxSearchItem newOptions( globalSearchOptions );

	sal_Int16 nLook =  globalSearchOptions.GetWordOnly() ?  vba::Excel::XlLookAt::xlPart : vba::Excel::XlLookAt::xlWhole; 
	sal_Int16 nSearchOrder = globalSearchOptions.GetRowDirection() ? vba::Excel::XlSearchOrder::xlByRows : vba::Excel::XlSearchOrder::xlByColumns;

	sal_Bool bMatchCase = sal_False;

	uno::Reference< util::XReplaceable > xReplace( mxRange, uno::UNO_QUERY );
	if ( xReplace.is() )
	{
		uno::Reference< util::XReplaceDescriptor > xDescriptor =
			xReplace->createReplaceDescriptor();
		
		xDescriptor->setSearchString( What); 		
		xDescriptor->setReplaceString( Replacement); 		
		if ( LookAt.hasValue() )
		{
			// sets SearchWords ( true is Cell match )
			nLook =  ::comphelper::getINT16( LookAt );
			sal_Bool bSearchWords = sal_False;
			if ( nLook == vba::Excel::XlLookAt::xlPart )
				bSearchWords = sal_False;
			else if ( nLook == vba::Excel::XlLookAt::xlWhole )
				bSearchWords = sal_True;
			else
				throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, illegal value for LookAt" )) , uno::Reference< uno::XInterface >() );
			// set global search props ( affects the find dialog
			// and of course the defaults for this method
			newOptions.SetWordOnly( bSearchWords );
			xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHWORDS ) ), uno::makeAny( bSearchWords ) ); 	
		}
		// sets SearchByRow ( true for Rows )
		if ( SearchOrder.hasValue() )
		{
			nSearchOrder =  ::comphelper::getINT16( SearchOrder );
			sal_Bool bSearchByRow = sal_False;
			if ( nSearchOrder == vba::Excel::XlSearchOrder::xlByColumns )
				bSearchByRow = sal_False;
			else if ( nSearchOrder == vba::Excel::XlSearchOrder::xlByRows )
				bSearchByRow = sal_True;
			else
				throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, illegal value for SearchOrder" )) , uno::Reference< uno::XInterface >() );
			
			newOptions.SetRowDirection( bSearchByRow ); 
			xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHBYROW ) ), uno::makeAny( bSearchByRow ) ); 	
		}			
		if ( MatchCase.hasValue() )
		{
			// SearchCaseSensitive
			MatchCase >>= bMatchCase;	
			xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHCASE ) ), uno::makeAny( bMatchCase ) ); 	
		}			

		ScGlobal::SetSearchItem( newOptions );	
		// ignore MatchByte for the moment, its not supported in
		// OOo.org afaik

		uno::Reference< util::XSearchDescriptor > xSearch( xDescriptor, uno::UNO_QUERY );
		xReplace->replaceAll( xSearch );
	}
	return sal_True; // always
}

uno::Reference< table::XCellRange > 
ScVbaRange::getCellRangeForName(  const rtl::OUString& sRangeName, const uno::Reference< sheet::XSpreadsheet >& xDoc, ScAddress::Convention aConv )
{
	uno::Reference< table::XCellRange > xRanges( xDoc, uno::UNO_QUERY_THROW );
	ScCellRangeObj* pRanges = dynamic_cast< ScCellRangeObj* >( xRanges.get() );
    ScAddress::Convention eConv = aConv;//ScAddress::CONV_XL_A1;   the default. 

	ScAddress::Details dDetails( eConv, 0, 0 );
		
	uno::Reference< table::XCellRange > xRange;
	if ( pRanges )
		xRange = pRanges->getCellRangeByName( sRangeName, dDetails );
	return xRange;	
}

uno::Reference< table::XCellRange > processKey( const uno::Any& Key, uno::Reference< table::XCellRange >& xRange )
{
	uno::Reference< table::XCellRange > xKey;
	if ( Key.getValueType() == vba::XRange::static_type() )
	{
		uno::Reference< vba::XRange > xKeyRange( Key, uno::UNO_QUERY_THROW );
		xKey.set( xKeyRange->getCellRange(), uno::UNO_QUERY_THROW );
	}
	else if ( Key.getValueType() == ::getCppuType( static_cast< const rtl::OUString* >(0) )  )
			
	{
		rtl::OUString sRangeName = ::comphelper::getString( Key );
		RangeHelper dRange( xRange );
		xKey = ScVbaRange::getCellRangeForName( sRangeName,  dRange.getSpreadSheet() );
	}
	else
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort illegal type value for key param")), uno::Reference< uno::XInterface >() );
	return xKey;
}

// helper method for Sort
sal_Int32 findSortPropertyIndex( const uno::Sequence< beans::PropertyValue >& props,
const rtl::OUString& sPropName ) throw( uno::RuntimeException )
{
	const beans::PropertyValue* pProp = props.getConstArray();
	sal_Int32 nItems = props.getLength();

	 sal_Int32 count=0;
	for ( ; count < nItems; ++count, ++pProp )
		if ( pProp->Name.equals( sPropName ) )
			return count;
	if ( count == nItems )
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort unknown sort property")), uno::Reference< uno::XInterface >() );
	return -1; //should never reach here ( satisfy compiler )
}

// helper method for Sort
void updateTableSortField( const uno::Reference< table::XCellRange >& xParentRange,
	const uno::Reference< table::XCellRange >& xColRowKey, sal_Int16 nOrder, 
	table::TableSortField& aTableField, sal_Bool bIsSortColumn, sal_Bool bMatchCase ) throw ( uno::RuntimeException )
{
		RangeHelper parentRange( xParentRange );	
		RangeHelper colRowRange( xColRowKey );	

		table::CellRangeAddress parentRangeAddress = parentRange.getCellRangeAddressable()->getRangeAddress();

		table::CellRangeAddress colRowKeyAddress = colRowRange.getCellRangeAddressable()->getRangeAddress();

		// make sure that upper left poing of key range is within the
		// parent range
		if (  colRowKeyAddress.StartColumn >= parentRangeAddress.StartColumn &&
			colRowKeyAddress.StartColumn <= parentRangeAddress.EndColumn  &&
			colRowKeyAddress.StartRow >= parentRangeAddress.StartRow &&
			colRowKeyAddress.StartRow <= parentRangeAddress.EndRow  )
		{
			//determine col/row index
			if ( bIsSortColumn )
				aTableField.Field = colRowKeyAddress.StartRow - parentRangeAddress.StartRow;			 
			else
				aTableField.Field = colRowKeyAddress.StartColumn - parentRangeAddress.StartColumn;			 
			aTableField.IsCaseSensitive = bMatchCase;

			if ( nOrder ==  vba::Excel::XlSortOrder::xlAscending ) 
				aTableField.IsAscending = sal_True; 
			else	
				aTableField.IsAscending = sal_False; 
		}
		else
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal Key param" ) ), uno::Reference< uno::XInterface >() );

						
} 

void SAL_CALL
ScVbaRange::Sort( const uno::Any& Key1, const uno::Any& Order1, const uno::Any& Key2, const uno::Any& /*Type*/, const uno::Any& Order2, const uno::Any& Key3, const uno::Any& Order3, const uno::Any& Header, const uno::Any& OrderCustom, const uno::Any& MatchCase, const uno::Any& Orientation, const uno::Any& SortMethod,  const uno::Any& DataOption1, const uno::Any& DataOption2, const uno::Any& DataOption3  ) throw (uno::RuntimeException)
{
	// #TODO# #FIXME# can we do something with Type
	if ( m_Areas->getCount() > 1 )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );

	sal_Int16 nDataOption1 = vba::Excel::XlSortDataOption::xlSortNormal;
	sal_Int16 nDataOption2 = vba::Excel::XlSortDataOption::xlSortNormal;;
	sal_Int16 nDataOption3 = vba::Excel::XlSortDataOption::xlSortNormal;

	ScDocument* pDoc = getDocumentFromRange( mxRange );
	if ( !pDoc )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

	RangeHelper thisRange( mxRange );
	table::CellRangeAddress thisRangeAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
	SCTAB nTab = thisRangeAddress.Sheet;

	ScSortParam aSortParam;
	pDoc->GetSortParam( aSortParam, nTab );

	if ( DataOption1.hasValue() )
		DataOption1 >>= nDataOption1;
	if ( DataOption2.hasValue() )
		DataOption2 >>= nDataOption2;
	if ( DataOption3.hasValue() )
		DataOption3 >>= nDataOption3;

	// 1) #TODO #FIXME need to process DataOption[1..3] not used currently
	// 2) #TODO #FIXME need to refactor this ( below ) into a IsSingleCell() method
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
			
	// 'Fraid I don't remember what I was trying to achieve here ???
/* 
	if (  isSingleCellRange() )
	{
		uno::Reference< vba::XRange > xCurrent = CurrentRegion();
		xCurrent->Sort( Key1, Order1, Key2, Type, Order2, Key3, Order3, Header, OrderCustom, MatchCase, Orientation, SortMethod, DataOption1, DataOption2, DataOption3 );
		return; 
	}
*/
	// set up defaults

	sal_Int16 nOrder1 = aSortParam.bAscending[0] ? vba::Excel::XlSortOrder::xlAscending : vba::Excel::XlSortOrder::xlDescending;
	sal_Int16 nOrder2 = aSortParam.bAscending[1] ? vba::Excel::XlSortOrder::xlAscending : vba::Excel::XlSortOrder::xlDescending;
	sal_Int16 nOrder3 = aSortParam.bAscending[2] ? vba::Excel::XlSortOrder::xlAscending : vba::Excel::XlSortOrder::xlDescending;

	sal_Int16 nCustom = aSortParam.nUserIndex;
	sal_Int16 nSortMethod = vba::Excel::XlSortMethod::xlPinYin;
	sal_Bool bMatchCase = aSortParam.bCaseSens;

	// seems to work opposite to expected, see below
	sal_Int16 nOrientation = aSortParam.bByRow ?  vba::Excel::XlSortOrientation::xlSortColumns :  vba::Excel::XlSortOrientation::xlSortRows;

	if ( Orientation.hasValue() )
	{
		// Documentation says xlSortRows is default but that doesn't appear to be 
		// the case. Also it appears that xlSortColumns is the default which 
		// strangely enought sorts by Row
		nOrientation = ::comphelper::getINT16( Orientation );
		// persist new option to be next calls default
		if ( nOrientation == vba::Excel::XlSortOrientation::xlSortRows )
			aSortParam.bByRow = FALSE;
		else
			aSortParam.bByRow = TRUE;

	}

	sal_Bool bIsSortColumns=sal_False; // sort by row

	if ( nOrientation == vba::Excel::XlSortOrientation::xlSortRows )
		bIsSortColumns = sal_True;

	sal_Int16 nHeader = aSortParam.nCompatHeader;
	sal_Bool bContainsHeader = sal_False;

	if ( Header.hasValue() )
	{
		nHeader = ::comphelper::getINT16( Header );
		aSortParam.nCompatHeader = nHeader;
	}			

	if ( nHeader == vba::Excel::XlYesNoGuess::xlGuess )
	{
		bool bHasColHeader = pDoc->HasColHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ));
		bool bHasRowHeader = pDoc->HasRowHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ) );
		if ( bHasColHeader || bHasRowHeader )
			nHeader =  vba::Excel::XlYesNoGuess::xlYes; 
		else
			nHeader =  vba::Excel::XlYesNoGuess::xlNo; 
		// save set param as default
		aSortParam.nCompatHeader = nHeader;
	}

	if ( nHeader == vba::Excel::XlYesNoGuess::xlYes )
		bContainsHeader = sal_True;

	if ( SortMethod.hasValue() )
	{
		nSortMethod = ::comphelper::getINT16( SortMethod );
	}
	
	if ( OrderCustom.hasValue() )
	{
		OrderCustom >>= nCustom;
		--nCustom; // 0-based in OOo
		aSortParam.nUserIndex = nCustom;
	}

	if ( MatchCase.hasValue() )
	{
		MatchCase >>= bMatchCase;
		aSortParam.bCaseSens = bMatchCase;
	}

	if ( Order1.hasValue() )
	{
		nOrder1 = ::comphelper::getINT16(Order1);
		if (  nOrder1 == vba::Excel::XlSortOrder::xlAscending ) 
			aSortParam.bAscending[0]  = TRUE;
		else
			aSortParam.bAscending[0]  = FALSE;

	}
	if ( Order2.hasValue() )
	{
		nOrder2 = ::comphelper::getINT16(Order2);
		if ( nOrder2 == vba::Excel::XlSortOrder::xlAscending ) 
			aSortParam.bAscending[1]  = TRUE;
		else
			aSortParam.bAscending[1]  = FALSE;
	}
	if ( Order3.hasValue() )
	{
		nOrder3 = ::comphelper::getINT16(Order3);
		if ( nOrder3 == vba::Excel::XlSortOrder::xlAscending ) 
			aSortParam.bAscending[2]  = TRUE;
		else
			aSortParam.bAscending[2]  = FALSE;
	}

	uno::Reference< table::XCellRange > xKey1;	
	uno::Reference< table::XCellRange > xKey2;	
	uno::Reference< table::XCellRange > xKey3;	

	xKey1 = processKey( Key1, mxRange );
	if ( !xKey1.is() )
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort needs a key1 param")), uno::Reference< uno::XInterface >() );

	if ( Key2.hasValue() )
		xKey2 = processKey( Key2, mxRange );
	if ( Key3.hasValue() )
		xKey3 = processKey( Key3, mxRange );

	uno::Reference< util::XSortable > xSort( mxRange, uno::UNO_QUERY_THROW );
	uno::Sequence< beans::PropertyValue > sortDescriptor = xSort->createSortDescriptor();
	sal_Int32 nTableSortFieldIndex = findSortPropertyIndex( sortDescriptor, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SortFields") ) );

	uno::Sequence< table::TableSortField > sTableFields(1);
	sal_Int32 nTableIndex = 0;
	updateTableSortField(  mxRange, xKey1, nOrder1, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );

	if ( xKey2.is() ) 
	{
		sTableFields.realloc( sTableFields.getLength() + 1 );
		updateTableSortField(  mxRange, xKey2, nOrder2, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );
	}
	if ( xKey3.is()  ) 
	{
		sTableFields.realloc( sTableFields.getLength() + 1 );
		updateTableSortField(  mxRange, xKey3, nOrder3, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );
	}
	sortDescriptor[ nTableSortFieldIndex ].Value <<= sTableFields;

	sal_Int32 nIndex = 	findSortPropertyIndex( sortDescriptor,  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSortColumns")) );
	sortDescriptor[ nIndex ].Value <<= bIsSortColumns;

	nIndex = 	findSortPropertyIndex( sortDescriptor,  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContainsHeader")) );
	sortDescriptor[ nIndex ].Value <<= bContainsHeader;

	pDoc->SetSortParam( aSortParam, nTab );
	xSort->sort( sortDescriptor );

	// #FIXME #TODO
	// The SortMethod param is not processed ( not sure what its all about, need to

}

uno::Reference< vba::XRange > SAL_CALL 
ScVbaRange::End( ::sal_Int32 Direction )  throw (uno::RuntimeException)
{
	if ( m_Areas->getCount() > 1 )
	{
		uno::Reference< vba::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
		return xRange->End( Direction );
	}

	
	// #FIXME #TODO
	// euch! found my orig implementation sucked, so 
	// trying this even suckier one ( really need to use/expose code in
	// around  ScTabView::MoveCursorArea(), thats the bit that calcutes
	// where the cursor should go ) 
	// Main problem with this method is the ultra hacky attempt to preserve
	// the ActiveCell, there should be no need to go to these extreems
	
	// Save ActiveCell pos ( to restore later )
	uno::Any aDft;
	rtl::OUString sActiveCell =	ScVbaGlobals::getGlobalsImpl(
                       m_xContext )->getApplication()->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

	// position current cell upper left of this range
	Cells( uno::makeAny( (sal_Int32) 1 ), uno::makeAny( (sal_Int32) 1 ) )->Select();

	SfxViewFrame* pViewFrame = getCurrentViewFrame();
	if ( pViewFrame )
	{
		SfxAllItemSet aArgs( SFX_APP()->GetPool() );
		// Hoping this will make sure this slot is called
		// synchronously
		SfxBoolItem sfxAsync( SID_ASYNCHRON, sal_False );
		aArgs.Put( sfxAsync, sfxAsync.Which() );
		SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();

		USHORT nSID = 0;
	
		switch( Direction )
		{
			case vba::Excel::XlDirection::xlDown:
				nSID = SID_CURSORBLKDOWN;
				break;
			case vba::Excel::XlDirection::xlUp:
				nSID = SID_CURSORBLKUP;
				break;
			case vba::Excel::XlDirection::xlToLeft:
				nSID = SID_CURSORBLKLEFT;
				break;
			case vba::Excel::XlDirection::xlToRight:
				nSID = SID_CURSORBLKRIGHT;
				break;
			default:
				throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": Invalid ColumnIndex" ) ), uno::Reference< uno::XInterface >() ); 
		}
		if ( pDispatcher )
		{
			pDispatcher->Execute( nSID, (SfxCallMode)SFX_CALLMODE_SYNCHRON, aArgs );
		}
	}

	// result is the ActiveCell		
	rtl::OUString sMoved =	ScVbaGlobals::getGlobalsImpl(
                       m_xContext )->getApplication()->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

	// restore old ActiveCell		
	uno::Any aVoid;
	uno::Reference< vba::XRange > xOldActiveCell( ScVbaGlobals::getGlobalsImpl(
		m_xContext )->getActiveSheet()->Range( uno::makeAny( sActiveCell ), aVoid ), uno::UNO_QUERY_THROW );
	xOldActiveCell->Select();

	uno::Reference< vba::XRange > resultCell;
	resultCell.set( ScVbaGlobals::getGlobalsImpl(
		m_xContext )->getActiveSheet()->Range( uno::makeAny( sMoved ), aVoid ), uno::UNO_QUERY_THROW );

	// return result
	
	return resultCell;
}

bool
ScVbaRange::isSingleCellRange()
{
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
	if ( xColumnRowRange->getRows()->getCount() == 1 && xColumnRowRange->getColumns()->getCount() == 1 )
		return true;
	return false;
}

uno::Reference< vba::XCharacters > SAL_CALL 
ScVbaRange::characters( const uno::Any& Start, const uno::Any& Length ) throw (uno::RuntimeException)
{
	if ( !isSingleCellRange() )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Can't create Characters property for multicell range ") ), uno::Reference< uno::XInterface >() );
	uno::Reference< text::XSimpleText > xSimple(mxRange->getCellByPosition(0,0) , uno::UNO_QUERY_THROW );
	ScDocument* pDoc = getDocumentFromRange(mxRange);
	if ( !pDoc )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

	ScVbaPalette aPalette( pDoc->GetDocumentShell() );
	return uno::Reference< vba::XCharacters >( new ScVbaCharacters( m_xContext, aPalette, xSimple, Start, Length ) );
}

 void SAL_CALL 
ScVbaRange::Delete( const uno::Any& Shift ) throw (uno::RuntimeException)
{
	
	if ( m_Areas->getCount() > 1 )
	{
		sal_Int32 nItems = m_Areas->getCount();
		for ( sal_Int32 index=1; index <= nItems; ++index )
		{
			uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny(index) ), uno::UNO_QUERY_THROW );
			xRange->Delete( Shift );	
		}
		return;
	}
	sheet::CellDeleteMode mode = sheet::CellDeleteMode_NONE ; 
	if ( Shift.hasValue() )		
	{
		sal_Int32 nShift;
		Shift >>= nShift;
		switch ( nShift )
		{
			case vba::Excel::XlDeleteShiftDirection::xlShiftUp:
				mode = sheet::CellDeleteMode_UP;
				break;
			case vba::Excel::XlDeleteShiftDirection::xlShiftToLeft:
				mode = sheet::CellDeleteMode_LEFT;
				break;
			default:
				throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("Illegal paramater ") ), uno::Reference< uno::XInterface >() );
		}
	}
	else
		if ( getRow() >  getColumn() )
			mode = sheet::CellDeleteMode_UP;
		else
			mode = sheet::CellDeleteMode_LEFT;
	RangeHelper thisRange( mxRange );
	uno::Reference< sheet::XCellRangeMovement > xCellRangeMove( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
	xCellRangeMove->removeRange( thisRange.getCellRangeAddressable()->getRangeAddress(), mode ); 
	
}

//XElementAccess
sal_Bool SAL_CALL 
ScVbaRange::hasElements() throw (uno::RuntimeException)
{
	uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY );
	if ( xColumnRowRange.is() )
		if ( xColumnRowRange->getRows()->getCount() ||
			xColumnRowRange->getColumns()->getCount() )
			return sal_True;
	return sal_False;
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL 
ScVbaRange::createEnumeration() throw (uno::RuntimeException)
{
	return new CellsEnumeration( m_xContext, mxRange );
}

// XInterface

IMPLEMENT_FORWARD_XINTERFACE2( ScVbaRange, ScVbaRange_BASE, OPropertyContainer )

// XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScVbaRange, ScVbaRange_BASE, OPropertyContainer )

// OPropertySetHelper

::cppu::IPropertyArrayHelper& 
ScVbaRange::getInfoHelper(  )
{
    static ::cppu::IPropertyArrayHelper* sProps = 0;
    if ( !sProps )
        sProps = createArrayHelper();
    return *sProps;
}


::cppu::IPropertyArrayHelper* 
ScVbaRange::createArrayHelper(  ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > 
ScVbaRange::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::rtl::OUString SAL_CALL 
ScVbaRange::getDefaultMethodName(  ) throw (uno::RuntimeException)
{
	const static rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM("Cells") );
	return sName;
}


uno::Reference< awt::XDevice > 
getDeviceFromDoc( const uno::Reference< frame::XModel >& xModel ) throw( uno::RuntimeException )
{
	uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
	uno::Reference< frame::XFrame> xFrame( xController->getFrame(), uno::UNO_QUERY_THROW );
	uno::Reference< awt::XDevice > xDevice( xFrame->getComponentWindow(), uno::UNO_QUERY_THROW );
	return xDevice;
}

// returns calc internal col. width ( in points )
double 
ScVbaRange::getCalcColWidth( const table::CellRangeAddress& rAddress) throw (uno::RuntimeException)
{
	ScDocument* pDoc = getDocumentFromRange( mxRange );
	USHORT nWidth = pDoc->GetOriginalWidth( static_cast< SCCOL >( rAddress.StartColumn ), static_cast< SCTAB >( rAddress.Sheet ) );
	double nPoints = lcl_TwipsToPoints( nWidth );
	nPoints = lcl_Round2DecPlaces( nPoints );
	return nPoints;
}

double
ScVbaRange::getCalcRowHeight( const table::CellRangeAddress& rAddress ) throw (uno::RuntimeException)
{
	ScDocument* pDoc = getDocumentFromRange( mxRange );
	USHORT nWidth = pDoc->GetOriginalHeight( rAddress.StartRow, rAddress.Sheet );
	double nPoints = lcl_TwipsToPoints( nWidth );
	nPoints = lcl_Round2DecPlaces( nPoints );
	return nPoints;	
}

// return Char Width in points
double getDefaultCharWidth( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
	const static rtl::OUString sDflt( RTL_CONSTASCII_USTRINGPARAM("Default")); 
	const static rtl::OUString sCharFontName( RTL_CONSTASCII_USTRINGPARAM("CharFontName")); 
	const static rtl::OUString sPageStyles( RTL_CONSTASCII_USTRINGPARAM("PageStyles")); 
	// get the font from the default style
	uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW );
	uno::Reference< container::XNameAccess > xNameAccess( xStyleSupplier->getStyleFamilies(), uno::UNO_QUERY_THROW );
	uno::Reference< container::XNameAccess > xNameAccess2( xNameAccess->getByName( sPageStyles ), uno::UNO_QUERY_THROW );
	uno::Reference< beans::XPropertySet > xProps( xNameAccess2->getByName( sDflt ), uno::UNO_QUERY_THROW );
	rtl::OUString sFontName;
	xProps->getPropertyValue( sCharFontName ) >>= sFontName;

	uno::Reference< awt::XDevice > xDevice = getDeviceFromDoc( xModel );
	awt::FontDescriptor aDesc;
	aDesc.Name = sFontName;
	uno::Reference< awt::XFont > xFont( xDevice->getFont( aDesc ), uno::UNO_QUERY_THROW );
	double nCharPixelWidth =  xFont->getCharWidth( (sal_Int8)'0' );	

	double nPixelsPerMeter = xDevice->getInfo().PixelPerMeterX;
	double nCharWidth = nCharPixelWidth /  nPixelsPerMeter;
	nCharWidth = nCharWidth * (double)56700;// in twips
	return lcl_TwipsToPoints( (USHORT)nCharWidth );	
}

uno::Any SAL_CALL 
ScVbaRange::getColumnWidth() throw (uno::RuntimeException)
{
	double nColWidth = 	0;
	ScDocShell* pShell = getDocShellFromRange( mxRange );
	if ( pShell )
	{
		RangeHelper thisRange( mxRange );
		table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();	
		uno::Reference< table::XColumnRowRange > xColRowRange( mxRange, uno::UNO_QUERY_THROW );			
		uno::Reference< beans::XPropertySet > xProps( xColRowRange->getColumns(), uno::UNO_QUERY_THROW ); 
		uno::Reference< frame::XModel > xModel = pShell->GetModel();
		if ( xModel.is() )
			nColWidth = getCalcColWidth(thisAddress) / getDefaultCharWidth( xModel );
	}
	nColWidth = lcl_Round2DecPlaces( nColWidth );
	return uno::makeAny( nColWidth );
}

void SAL_CALL 
ScVbaRange::setColumnWidth( const uno::Any& _columnwidth ) throw (uno::RuntimeException)
{
	double nColWidth = 0;
	_columnwidth >>= nColWidth;
	nColWidth = lcl_Round2DecPlaces( nColWidth );
        ScDocShell* pDocShell = getDocShellFromRange( mxRange );
        if ( pDocShell )
        {
                uno::Reference< frame::XModel > xModel = pDocShell->GetModel();
                if ( xModel.is() )
                {

			nColWidth = ( nColWidth * getDefaultCharWidth( xModel ) );
			RangeHelper thisRange( mxRange );	
			table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
			USHORT nTwips = lcl_pointsToTwips( nColWidth );
			
			ScDocFunc aFunc(*pDocShell);
			SCCOLROW nColArr[2];
			nColArr[0] = thisAddress.StartColumn;
			nColArr[1] = thisAddress.EndColumn;
			aFunc.SetWidthOrHeight( TRUE, 1, nColArr, thisAddress.Sheet, SC_SIZE_ORIGINAL,
		                                                                        nTwips, TRUE, TRUE );		
			
		}
	}
}

uno::Any SAL_CALL 
ScVbaRange::getWidth() throw (uno::RuntimeException)
{
	uno::Reference< table::XColumnRowRange > xColRowRange( mxRange, uno::UNO_QUERY_THROW );			
	uno::Reference< container::XIndexAccess > xIndexAccess( xColRowRange->getColumns(), uno::UNO_QUERY_THROW ); 
	sal_Int32 nElems = xIndexAccess->getCount();	
	double nWidth = 0;
	for ( sal_Int32 index=0; index<nElems; ++index )
	{
		uno::Reference< sheet::XCellRangeAddressable > xAddressable( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW ); 
		double nTmpWidth = getCalcColWidth( xAddressable->getRangeAddress() );
		nWidth += nTmpWidth;
	}
	return uno::makeAny( nWidth );
}

uno::Any SAL_CALL 
ScVbaRange::Areas( const uno::Any& item) throw (uno::RuntimeException)
{
	if ( !item.hasValue() )
		return uno::makeAny( m_Areas );
	return m_Areas->Item( item );	
}

uno::Reference< vba::XRange >
ScVbaRange::getArea( sal_Int32 nIndex ) throw( css::uno::RuntimeException )
{
	if ( !m_Areas.is() )
		throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No areas available")), uno::Reference< uno::XInterface >() );
	uno::Reference< vba::XRange > xRange( m_Areas->Item( uno::makeAny( ++nIndex ) ), uno::UNO_QUERY_THROW );
	return xRange;
}

uno::Any
ScVbaRange::Borders( const uno::Any& item ) throw( css::uno::RuntimeException )
{
	if ( !item.hasValue() )
		return uno::makeAny( m_Borders );
	return m_Borders->Item( item );
}

uno::Any SAL_CALL
ScVbaRange::BorderAround( const css::uno::Any& LineStyle, const css::uno::Any& Weight,
                const css::uno::Any& ColorIndex, const css::uno::Any& Color ) throw (css::uno::RuntimeException)
{
    sal_Int32 nCount = m_Borders->getCount();
    uno::Reference< vba::XBorders > xBorders( m_Borders, uno::UNO_QUERY_THROW);
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        const sal_Int32 nLineType = supportedIndexTable[i];
        switch( nLineType )
        {
            case XlBordersIndex::xlEdgeLeft:
            case XlBordersIndex::xlEdgeTop:
            case XlBordersIndex::xlEdgeBottom:
            case XlBordersIndex::xlEdgeRight:
            {
                uno::Reference< vba::XBorder > xBorder( m_Borders->Item( uno::makeAny( nLineType ) ), uno::UNO_QUERY_THROW );
                if( LineStyle.hasValue() )
                {
                    xBorder->setLineStyle( LineStyle );
                }
                if( Weight.hasValue() )
                {
                    xBorder->setWeight( Weight );
                }
                if( ColorIndex.hasValue() )
                {
                    xBorder->setColorIndex( ColorIndex );
                }
                if( Color.hasValue() )
                {
                    xBorder->setColor( Color );
                }
                break;
            }
            case XlBordersIndex::xlInsideVertical:
            case XlBordersIndex::xlInsideHorizontal:
            case XlBordersIndex::xlDiagonalDown:
            case XlBordersIndex::xlDiagonalUp:
                break;
            default:
                return uno::makeAny( sal_False );
        }
    }
    return uno::makeAny( sal_True );
}

uno::Any SAL_CALL 
ScVbaRange::getRowHeight() throw (uno::RuntimeException)
{
	RangeHelper thisRange( mxRange );	
	table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
	double nHeight = getCalcRowHeight( thisAddress );
	return uno::makeAny( nHeight );
}

void SAL_CALL 
ScVbaRange::setRowHeight( const uno::Any& _rowheight) throw (uno::RuntimeException)
{
	 double nHeight; // Incomming height is in points
        _rowheight >>= nHeight;
	nHeight = lcl_Round2DecPlaces( nHeight );
	RangeHelper thisRange( mxRange );	
	table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
	USHORT nTwips = lcl_pointsToTwips( nHeight );
	
	ScDocShell* pDocShell = getDocShellFromRange( mxRange );
	ScDocFunc aFunc(*pDocShell);
	SCCOLROW nRowArr[2];
	nRowArr[0] = thisAddress.StartRow;
	nRowArr[1] = thisAddress.EndRow;
	aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, thisAddress.Sheet, SC_SIZE_ORIGINAL,
                                                                        nTwips, TRUE, TRUE );		
}

uno::Any SAL_CALL 
ScVbaRange::getPageBreak() throw (uno::RuntimeException)
{
	sal_Int32 nPageBreak = XlPageBreak::XlPageBreakNone;
	ScDocShell* pShell = getDocShellFromRange( mxRange );
	if ( pShell )
	{
		RangeHelper thisRange( mxRange );
		table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
		BOOL bColumn = FALSE;
		
		if (thisAddress.StartRow==0)
		    bColumn = TRUE;
		
		uno::Reference< frame::XModel > xModel = pShell->GetModel();
		if ( xModel.is() )
		{
	        ScDocument* pDoc =  getDocumentFromRange( mxRange );
	        
			BYTE nFlag = 0;
			if ( !bColumn )
			    nFlag = pDoc -> GetRowFlags(thisAddress.StartRow, thisAddress.Sheet);
			else
			    nFlag = pDoc -> GetColFlags(thisAddress.StartColumn, thisAddress.Sheet);
			    
			if ( nFlag & CR_PAGEBREAK)
			    nPageBreak = XlPageBreak::XlPageBreakAutomatic;
			    
			if ( nFlag & CR_MANUALBREAK)
			    nPageBreak = XlPageBreak::XlPageBreakManual;
		}		
	}

	return uno::makeAny( nPageBreak );
}

void SAL_CALL 
ScVbaRange::setPageBreak( const uno::Any& _pagebreak) throw (uno::RuntimeException)
{
	sal_Int32 nPageBreak; 
    _pagebreak >>= nPageBreak;
		
	ScDocShell* pShell = getDocShellFromRange( mxRange );
	if ( pShell )
	{
		RangeHelper thisRange( mxRange );
		table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
		if ((thisAddress.StartColumn==0) && (thisAddress.StartRow==0))
		    return;
		BOOL bColumn = FALSE;
		
		if (thisAddress.StartRow==0)
		    bColumn = TRUE;
		
		ScAddress aAddr( thisAddress.StartColumn, thisAddress.StartRow, thisAddress.Sheet );	
		uno::Reference< frame::XModel > xModel = pShell->GetModel();
		if ( xModel.is() )
		{
			ScTabViewShell* pViewShell = getBestViewShell( xModel );
			if ( nPageBreak == XlPageBreak::XlPageBreakManual )
			    pViewShell->InsertPageBreak( bColumn, TRUE, &aAddr);
			else if ( nPageBreak == XlPageBreak::XlPageBreakNone )
			    pViewShell->DeletePageBreak( bColumn, TRUE, &aAddr);
		}
	}
}

uno::Any SAL_CALL 
ScVbaRange::getHeight() throw (uno::RuntimeException)
{
	uno::Reference< table::XColumnRowRange > xColRowRange( mxRange, uno::UNO_QUERY_THROW );			
	uno::Reference< container::XIndexAccess > xIndexAccess( xColRowRange->getRows(), uno::UNO_QUERY_THROW ); 
	sal_Int32 nElems = xIndexAccess->getCount();
	double nHeight = 0;
	for ( sal_Int32 index=0; index<nElems; ++index )
	{
        	uno::Reference< sheet::XCellRangeAddressable > xAddressable( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW ); 
		nHeight += getCalcRowHeight(xAddressable->getRangeAddress() );
	}
	return uno::makeAny( nHeight );
}

uno::Reference< vba::XWorksheet >
ScVbaRange::getWorksheet() throw (uno::RuntimeException)
{
	ScDocShell* pDocShell =  getDocShellFromRange(mxRange);
	RangeHelper* rHelper = new RangeHelper(mxRange);
        return new ScVbaWorksheet(m_xContext,rHelper->getSpreadSheet(),pDocShell->GetModel());
}

ScCellRangesBase*
ScVbaRange::getCellRangesBase() throw( uno::RuntimeException )
{
	ScCellRangesBase* pUnoRangesBase = NULL;
	if ( mxRanges.is() )
		pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( mxRanges.get() );
	else if ( mxRange.is() )
		pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( mxRange.get() );
	else
		throw uno::RuntimeException( rtl::OUString::createFromAscii("General Error creating range - Unknown" ), uno::Reference< uno::XInterface >() );
	return pUnoRangesBase;
}

// #TODO remove this ugly application processing
// Process an application Range request e.g. 'Range("a1,b2,a4:b6")
uno::Reference< vba::XRange >
ScVbaRange::ApplicationRange( const uno::Reference< uno::XComponentContext >& xContext, const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException)
{
	// Althought the documentation seems clear that Range without a 
	// qualifier then its a shortcut for ActiveSheet.Range
	// however, similarly Application.Range is apparently also a 
	// shortcut for ActiveSheet.Range
	// The is however a subtle behavioural difference I've come across 
	// wrt to named ranges.
	// If a named range "test" exists { Sheet1!$A1 } and the active sheet
	// is Sheet2 then the following will fail
	// msgbox ActiveSheet.Range("test").Address ' failes
	// msgbox WorkSheets("Sheet2").Range("test").Address
	// but !!!
	// msgbox Range("test").Address ' works
	// msgbox Application.Range("test").Address ' works

	// Single param Range 
	rtl::OUString sRangeName;
	Cell1 >>= sRangeName;
	if ( Cell1.hasValue() && !Cell2.hasValue() && sRangeName.getLength() )
	{
		const static rtl::OUString sNamedRanges( RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
		uno::Reference< beans::XPropertySet > xPropSet( getCurrentDocument(), uno::UNO_QUERY_THROW );
		
		uno::Reference< container::XNameAccess > xNamed( xPropSet->getPropertyValue( sNamedRanges ), uno::UNO_QUERY_THROW );
		uno::Reference< sheet::XCellRangeReferrer > xReferrer;
		try
		{
			xReferrer.set ( xNamed->getByName( sRangeName ), uno::UNO_QUERY );
		}
		catch( uno::Exception& /*e*/ )
		{
			// do nothing
		}
		if ( xReferrer.is() )
		{
			uno::Reference< table::XCellRange > xRange = xReferrer->getReferredCells();
			if ( xRange.is() )
			{
				uno::Reference< vba::XRange > xVbRange =  new  ScVbaRange( xContext, xRange );
				return xVbRange;
			}
		}
	}
	uno::Reference< sheet::XSpreadsheetView > xView( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY );
	uno::Reference< table::XCellRange > xSheetRange( xView->getActiveSheet(), uno::UNO_QUERY_THROW ); 
	ScVbaRange* pRange = new ScVbaRange( xContext, xSheetRange );
	uno::Reference< vba::XRange > xVbSheetRange( pRange );
	return pRange->Range( Cell1, Cell2, true ); 
}


/***************************************************************************************
 * interface for text: 
 * com.sun.star.text.XText, com.sun.star.table.XCell, com.sun.star.container.XEnumerationAccess
 * com.sun.star.text.XTextRange, 
 * the main problem is to recognize the numeric and date, which assosiate with DecimalSeparator, ThousandsSeparator, 
 * TrailingMinusNumbers and FieldInfo.
***************************************************************************************/
void SAL_CALL
ScVbaRange::TextToColumns( const css::uno::Any& Destination, const css::uno::Any& DataType, const css::uno::Any& TextQualifier,
        const css::uno::Any& ConsecutinveDelimiter, const css::uno::Any& Tab, const css::uno::Any& Semicolon, const css::uno::Any& Comma,
        const css::uno::Any& Space, const css::uno::Any& Other, const css::uno::Any& OtherChar, const css::uno::Any& FieldInfo,
        const css::uno::Any& DecimalSeparator, const css::uno::Any& ThousandsSeparator, const css::uno::Any& TrailingMinusNumbers  ) throw (css::uno::RuntimeException)
{
    OSL_TRACE("nJust for test\n");
    uno::Reference< vba::XRange > xRange;
    if( Destination.hasValue() )
    {
        if( !( Destination >>= xRange ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Destination parameter should be a range" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set range\n");
    }
    else
    {
        //set as current
        xRange = this;
        OSL_TRACE("set range as himself\n");
    }

   sal_Int16 xlTextParsingType = vba::Excel::XlTextParsingType::xlDelimited;
    if ( DataType.hasValue() )
    {
        if( !( DataType >>= xlTextParsingType ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "DataType parameter should be a short" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Datatype\n" );
    }
    sal_Bool bDilimited = ( xlTextParsingType == vba::Excel::XlTextParsingType::xlDelimited );

    sal_Int16 xlTextQualifier = vba::Excel::XlTextQualifier::xlTextQualifierDoubleQuote; 
    if( TextQualifier.hasValue() )
    {
        if( !( TextQualifier >>= xlTextQualifier ))
             throw uno::RuntimeException( rtl::OUString::createFromAscii( "TextQualifier parameter should be a short" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set TextQualifier\n");
    }

    sal_Bool bConsecutinveDelimiter = sal_False;
    if( ConsecutinveDelimiter.hasValue() )
    {
        if( !( ConsecutinveDelimiter >>= bConsecutinveDelimiter ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "ConsecutinveDelimiter parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set ConsecutinveDelimiter\n");
    }

    sal_Bool bTab = sal_False;
    if( Tab.hasValue() && bDilimited )
    {
        if( !( Tab >>= bTab ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Tab parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Tab\n");
    }

    sal_Bool bSemicolon = sal_False;
    if( Semicolon.hasValue() && bDilimited )
    {
        if( !( Semicolon >>= bSemicolon ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Semicolon parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Semicolon\n");
    }
    sal_Bool bComma = sal_False;
    if( Comma.hasValue() && bDilimited )
    {
        if( !( Comma >>= bComma ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Comma parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Comma\n");
    }
    sal_Bool bSpace = sal_False;
    if( Space.hasValue() && bDilimited )
    {
        if( !( Space >>= bSpace ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Space parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Space\n");
    }
    sal_Bool bOther = sal_False;
    rtl::OUString sOtherChar;
    if( Other.hasValue() && bDilimited )
    {
        if( Other >>= bOther )
        {
            if( OtherChar.hasValue() )
                if( !( OtherChar >>= sOtherChar ) )
                    throw uno::RuntimeException( rtl::OUString::createFromAscii( "OtherChar parameter should be a String" ),
                        uno::Reference< uno::XInterface >() );
        OSL_TRACE("set OtherChar\n" );
        }
     else if( bOther )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Other parameter should be a True" ),
                    uno::Reference< uno::XInterface >() );
    }
 //TODO* FieldInfo   Optional Variant. An array containing parse information for the individual columns of data. The interpretation depends on the value of DataType. When the data is delimited, this argument is an array of two-element arrays, with each two-element array specifying the conversion options for a particular column. The first element is the column number (1-based), and the second element is one of the xlColumnDataType  constants specifying how the column is parsed.

    rtl::OUString sDecimalSeparator;
    if( DecimalSeparator.hasValue() )
    {
        if( !( DecimalSeparator >>= sDecimalSeparator ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "DecimalSeparator parameter should be a String" ),
                uno::Reference< uno::XInterface >() );
        OSL_TRACE("set DecimalSeparator\n" );
    }
    rtl::OUString sThousandsSeparator;
    if( ThousandsSeparator.hasValue() )
    {
        if( !( ThousandsSeparator >>= sThousandsSeparator ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "ThousandsSeparator parameter should be a String" ),
                uno::Reference< uno::XInterface >() );
        OSL_TRACE("set ThousandsSpeparator\n" );
    }
 //TODO* TrailingMinusNumbers  Optional Variant. Numbers that begin with a minus character.
}

