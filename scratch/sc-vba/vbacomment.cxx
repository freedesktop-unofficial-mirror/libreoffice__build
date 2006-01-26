#include "vbacomment.hxx"

#include <org/openoffice/vba/Excel/XlCreator.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>

#include "vbaglobals.hxx"
#include "vbacomments.hxx"


using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaComment::ScVbaComment( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ) throw( lang::IllegalArgumentException )
: m_xContext( xContext ), mxRange( xRange )
{
	if  ( !xContext.is() )
		throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "context is not set " ) ), uno::Reference< uno::XInterface >() , 1 );
	if  ( !xRange.is() )
		throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "range is not set " ) ), uno::Reference< uno::XInterface >() , 1 );
	uno::Reference< text::XSimpleText > xAnnoText( getAnnotation(), uno::UNO_QUERY );
}

// private helper functions

uno::Reference< sheet::XSheetAnnotation > SAL_CALL
ScVbaComment::getAnnotation() throw (uno::RuntimeException)
{
	uno::Reference< table::XCell > xCell( mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
	uno::Reference< sheet::XSheetAnnotationAnchor > xAnnoAnchor( xCell, uno::UNO_QUERY_THROW );
	return uno::Reference< sheet::XSheetAnnotation > ( xAnnoAnchor->getAnnotation(), uno::UNO_QUERY_THROW );
}

uno::Reference< sheet::XSheetAnnotations > SAL_CALL
ScVbaComment::getAnnotations() throw (uno::RuntimeException)
{
	uno::Reference< sheet::XSheetCellRange > xSheetCellRange(mxRange, ::uno::UNO_QUERY_THROW );
	uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
	uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnosSupp( xSheet, uno::UNO_QUERY_THROW );

	return uno::Reference< sheet::XSheetAnnotations > ( xAnnosSupp->getAnnotations(), uno::UNO_QUERY_THROW );
}

sal_Int32 SAL_CALL
ScVbaComment::getAnnotationIndex() throw (uno::RuntimeException)
{
	uno::Reference< sheet::XSheetAnnotations > xAnnos = getAnnotations();
	table::CellAddress aAddress = getAnnotation()->getPosition();

	sal_Int32 aIndex = 0;
	sal_Int32 aCount = xAnnos->getCount();

	for ( ; aIndex < aCount ; aIndex++ )
	{
		uno::Reference< sheet::XSheetAnnotation > xAnno( xAnnos->getByIndex( aIndex ), uno::UNO_QUERY_THROW );
		table::CellAddress xAddress = xAnno->getPosition();
	
		if ( xAddress.Column == aAddress.Column && xAddress.Row == aAddress.Row && xAddress.Sheet == aAddress.Sheet )
		{
			OSL_TRACE("** terminating search, index is %d", aIndex );
			break;
		}
	}
	OSL_TRACE("** returning index is %d", aIndex );

       return aIndex;
}

uno::Reference< vba::XComment > SAL_CALL
ScVbaComment::getCommentByIndex( sal_Int32 Index ) throw (uno::RuntimeException)
{
	uno::Reference< container::XIndexAccess > xIndexAccess( getAnnotations(), uno::UNO_QUERY_THROW );
	uno::Reference< vba::XCollection > xColl( uno::Reference< vba::XComments > ( new ScVbaComments( m_xContext, xIndexAccess ) ), uno::UNO_QUERY_THROW );

	return uno::Reference< vba::XComment > ( xColl->Item( uno::makeAny( Index ) ), uno::UNO_QUERY_THROW );
 }

// public vba functions

uno::Reference< vba::XApplication > SAL_CALL
ScVbaComment::getApplication() throw (uno::RuntimeException)
{
	uno::Reference< vba::XApplication > xApplication =
		ScVbaGlobals::getGlobalsImpl( m_xContext )->getApplication();
	if ( xApplication.is() )
		return xApplication;
	return uno::Reference< vba::XApplication >(NULL);
}

rtl::OUString SAL_CALL
ScVbaComment::getAuthor() throw (uno::RuntimeException)
{
	return getAnnotation()->getAuthor();
}

void SAL_CALL
ScVbaComment::setAuthor( const rtl::OUString& _author ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL
ScVbaComment::getCreator() throw (uno::RuntimeException)
{
	return vba::Excel::XlCreator::xlCreatorCode;
}

uno::Reference< vba::XRange > SAL_CALL
ScVbaComment::getParent() throw (uno::RuntimeException)
{
	uno::Reference< vba::XApplication > xApplication =
		ScVbaGlobals::getGlobalsImpl( m_xContext )->getApplication();
	if ( xApplication.is() )
		return xApplication->getActiveCell();
	return uno::Reference< vba::XRange >(NULL);
}

sal_Bool SAL_CALL
ScVbaComment::getVisible() throw (uno::RuntimeException)
{
	return getAnnotation()->getIsVisible();
}

void SAL_CALL
ScVbaComment::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
	getAnnotation()->setIsVisible( _visible );
}

void SAL_CALL
ScVbaComment::Delete() throw (uno::RuntimeException)
{
	getAnnotations()->removeByIndex( getAnnotationIndex() );
}

uno::Reference< vba::XComment > SAL_CALL
ScVbaComment::Next() throw (uno::RuntimeException)
{
	// index: uno = 0, vba = 1
	return getCommentByIndex( getAnnotationIndex() + 2 );
}

uno::Reference< vba::XComment > SAL_CALL
ScVbaComment::Previous() throw (uno::RuntimeException)
{
	// index: uno = 0, vba = 1
	return getCommentByIndex( getAnnotationIndex() );
}

rtl::OUString SAL_CALL
ScVbaComment::Text( const uno::Any& Text, const uno::Any& Start, const uno::Any& Overwrite ) throw (uno::RuntimeException)
{
	rtl::OUString sText;
	Text >>= sText;

	uno::Reference< text::XSimpleText > xAnnoText( getAnnotation(), uno::UNO_QUERY_THROW );
	rtl::OUString sAnnoText = xAnnoText->getString();

	if ( Start.hasValue() )
	{
		sal_Int16 nStart;
		sal_Bool bOverwrite = sal_True;
		Overwrite >>= bOverwrite;

		if ( Start >>= nStart )
		{
			uno::Reference< text::XTextCursor > xTextCursor( xAnnoText->createTextCursor(), uno::UNO_QUERY_THROW );

			if ( bOverwrite )
			{
				xTextCursor->collapseToStart();
				xTextCursor->gotoStart( sal_False );
				xTextCursor->goRight( nStart - 1, sal_False );
				xTextCursor->gotoEnd( sal_True );
			}
			else
			{
				xTextCursor->collapseToStart();
				xTextCursor->gotoStart( sal_False );
				xTextCursor->goRight( nStart - 1 , sal_True );
			}

			uno::Reference< text::XTextRange > xRange( xTextCursor, uno::UNO_QUERY_THROW );
			xAnnoText->insertString( xRange, sText, bOverwrite );
		}
		else
			throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScVbaComment::Text - bad Start value " ) ), uno::Reference< uno::XInterface >() );
	}
	else if ( Text.hasValue() )
		xAnnoText->setString( sText );

	return sAnnoText;
}

