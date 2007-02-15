#include "vbacharacters.hxx"

#include "vbaglobals.hxx"
#include "vbafont.hxx"


using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaCharacters::ScVbaCharacters( const uno::Reference< uno::XComponentContext >& xContext, const ScVbaPalette& dPalette, const uno::Reference< text::XSimpleText>& xRange,const css::uno::Any& Start, const css::uno::Any& Length  ) throw ( css::lang::IllegalArgumentException ) : m_xSimpleText(xRange), m_xContext( xContext ), m_aPalette( dPalette),  nLength(-1), nStart(1)
{
	Start >>= nStart;
	if ( nStart < 1 )
		nStart = 1; // silently correct user error ( as ms )
	nStart--; // OOo is 0 based
	Length >>=nLength;			
	uno::Reference< text::XTextCursor > xTextCursor( m_xSimpleText->createTextCursor(), uno::UNO_QUERY_THROW );
	xTextCursor->collapseToStart();
	if ( nStart )
		xTextCursor->goRight( nStart, sal_False );
	if ( nLength < 0 ) // expand to end
		xTextCursor->gotoEnd( sal_True );
	else
		xTextCursor->goRight( nLength, sal_True );
	m_xTextRange.set( xTextCursor, uno::UNO_QUERY_THROW );

}

::rtl::OUString SAL_CALL 
ScVbaCharacters::getCaption() throw (css::uno::RuntimeException)
{
	return m_xTextRange->getString();
}
void SAL_CALL 
ScVbaCharacters::setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException)
{
	m_xTextRange->setString( _caption );

}

::sal_Int32 SAL_CALL 
ScVbaCharacters::getCount() throw (css::uno::RuntimeException)
{
	return getCaption().getLength();	
}

::rtl::OUString SAL_CALL 
ScVbaCharacters::getText() throw (css::uno::RuntimeException)
{
	return getCaption();
}
void SAL_CALL 
ScVbaCharacters::setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException)
{
	setCaption( _text );
}
uno::Reference< excel::XFont > SAL_CALL 
ScVbaCharacters::getFont() throw (css::uno::RuntimeException) 
{
	uno::Reference< beans::XPropertySet > xProps( m_xTextRange, uno::UNO_QUERY_THROW );
	return uno::Reference< excel::XFont >( new ScVbaFont( m_aPalette, xProps ) );
}
void SAL_CALL 
ScVbaCharacters::setFont( const uno::Reference< excel::XFont >& /*_font*/ ) throw (css::uno::RuntimeException)
{
	// #TODO #FIXME needs implementation, or can't be done?
	throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Not Implemented") ), uno::Reference< XInterface >() );
}


// Methods
void SAL_CALL 
ScVbaCharacters::Insert( const ::rtl::OUString& String ) throw (css::uno::RuntimeException)
{
	m_xSimpleText->insertString( m_xTextRange, String, sal_False );
}

void SAL_CALL 
ScVbaCharacters::Delete(  ) throw (css::uno::RuntimeException)
{
	// #FIXME #TODO is this a bit suspect?, I wonder should the contents 
	// of the cell be deleted from the parent ( range )
	m_xSimpleText->setString(rtl::OUString()); 
}




