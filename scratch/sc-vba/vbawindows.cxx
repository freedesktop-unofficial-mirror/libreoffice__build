#include "vbawindows.hxx"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <cppuhelper/implbase3.hxx>

#include <tools/urlobj.hxx>
#include "vbawindow.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

typedef  std::hash_map< rtl::OUString,
sal_Int32, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameIndexHash;

uno::Any ComponentToWindow( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext )
{
	uno::Reference< frame::XModel > xModel( aSource, uno::UNO_QUERY_THROW );
	uno::Reference< vba::XWindow > xWin( new ScVbaWindow( xContext,xModel ) );
	return uno::makeAny( xWin );
}

typedef std::vector < uno::Reference< sheet::XSpreadsheetDocument > > Components;
// #TODO more or less the same as class in workwindows ( code sharing needed )
class WindowComponentEnumImpl : public EnumerationHelper_BASE
{
protected:
	uno::Reference< uno::XComponentContext > m_xContext;
	Components m_components;
	Components::const_iterator m_it;

public:
	WindowComponentEnumImpl( const uno::Reference< uno::XComponentContext >& xContext, const Components& components ) throw ( uno::RuntimeException ) :  m_xContext( xContext ), m_components( components )
	{
		m_it = m_components.begin();
	}

	WindowComponentEnumImpl( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException ) :  m_xContext( xContext )
	{
		uno::Reference< lang::XMultiComponentFactory > xSMgr(
			m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );

		uno::Reference< frame::XDesktop > xDesktop
			(xSMgr->createInstanceWithContext(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"), m_xContext), uno::UNO_QUERY_THROW );
		uno::Reference< container::XEnumeration > mxComponents = xDesktop->getComponents()->createEnumeration();
		while( mxComponents->hasMoreElements() )
		{
			uno::Reference< sheet::XSpreadsheetDocument > xNext( mxComponents->nextElement(), uno::UNO_QUERY );
			if ( xNext.is() )
				m_components.push_back( xNext );
		}
		m_it = m_components.begin();
	}
	// XEnumeration
	virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException) 
	{ 
		return m_it != m_components.end();
	}

	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		if ( !hasMoreElements() )
		{
			throw container::NoSuchElementException();
		}
		return makeAny( *(m_it++) );
	}
};

class WindowEnumImpl : public  WindowComponentEnumImpl 
{
public:
	WindowEnumImpl(const uno::Reference< uno::XComponentContext >& xContext, const Components& components ):WindowComponentEnumImpl( xContext, components ) {}
	WindowEnumImpl( const uno::Reference< uno::XComponentContext >& xContext ): WindowComponentEnumImpl( xContext ) {}
	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
	{
		return ComponentToWindow( WindowComponentEnumImpl::nextElement(), m_xContext );
	}
};

typedef ::cppu::WeakImplHelper3< container::XEnumerationAccess 
	, com::sun::star::container::XIndexAccess
	, com::sun::star::container::XNameAccess
	> WindowsAccessImpl_BASE;

class WindowsAccessImpl : public WindowsAccessImpl_BASE
{
	uno::Reference< uno::XComponentContext > m_xContext;
	Components m_windows;
	NameIndexHash namesToIndices;
public:	
	WindowsAccessImpl( const uno::Reference< uno::XComponentContext >& xContext ):m_xContext( xContext )
	{
		uno::Reference< container::XEnumeration > xEnum = new WindowComponentEnumImpl( m_xContext );
		sal_Int32 nIndex=0;
		while( xEnum->hasMoreElements() )
		{
			uno::Reference< sheet::XSpreadsheetDocument > xNext( xEnum->nextElement(), uno::UNO_QUERY );
			if ( xNext.is() )
			{
				m_windows.push_back( xNext );
				uno::Reference< frame::XModel > xModel( xNext, uno::UNO_QUERY_THROW ); // that the spreadsheetdocument is a xmodel is a given
				ScVbaWindow window( m_xContext, xModel );
				rtl::OUString sCaption;
				window.getCaption() >>= sCaption;
				namesToIndices[ sCaption ] = nIndex++;
			}
		}
			
	}
	
	//XEnumerationAccess
	virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
	{
		return new WindowComponentEnumImpl( m_xContext, m_windows ); 
	}
	// XIndexAccess
	virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException) 
	{ 
		return m_windows.size();
	}
	virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		if ( Index < 0 
			|| static_cast< Components::size_type >( Index ) >= m_windows.size() ) 
			throw lang::IndexOutOfBoundsException();
		return makeAny( m_windows[ Index ] ); // returns xspreadsheetdoc
	}

	//XElementAccess
	virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
	{ 
		return sheet::XSpreadsheetDocument::static_type(0); 
	}

	virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) 
	{ 
		return (m_windows.size() > 0);
	}

	//XNameAccess
	virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		NameIndexHash::const_iterator it = namesToIndices.find( aName );
		if ( it == namesToIndices.end() )
			throw container::NoSuchElementException();
		return makeAny( m_windows[ it->second ] );
		
	}

	virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException) 
	{ 
		uno::Sequence< ::rtl::OUString > names( namesToIndices.size() );
		::rtl::OUString* pString = names.getArray();
		NameIndexHash::const_iterator it = namesToIndices.begin();
		NameIndexHash::const_iterator it_end = namesToIndices.end();
		for ( ; it != it_end; ++it, ++pString )
			*pString = it->first;	
		return names;	
	}

	virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException) 
	{ 
		NameIndexHash::const_iterator it = namesToIndices.find( aName );
		return (it != namesToIndices.end());
	}

};


ScVbaWindows::ScVbaWindows( const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess  ):  ScVbaWindows_BASE( xContext, xIndexAccess ) 
{
}

uno::Reference< container::XEnumeration >
ScVbaWindows::createEnumeration() throw (uno::RuntimeException)
{
	return new WindowEnumImpl( m_xContext );
}

uno::Any
ScVbaWindows::createCollectionObject( const css::uno::Any& aSource )
{
	return ComponentToWindow( aSource,  m_xContext );
}

uno::Type 
ScVbaWindows::getElementType() throw (uno::RuntimeException)
{
	return vba::XWindows::static_type(0);
}

uno::Reference< vba::XCollection >
ScVbaWindows::Windows( const css::uno::Reference< css::uno::XComponentContext >& xContext )
{
	uno::Reference< container::XIndexAccess > xIndex( new WindowsAccessImpl( xContext ) );

	return uno::Reference< vba::XCollection >( new ScVbaWindows( xContext , xIndex ) );	
}

void SAL_CALL 
ScVbaWindows::Arrange( ::sal_Int32 /*ArrangeStyle*/, const uno::Any& /*ActiveWorkbook*/, const uno::Any& /*SyncHorizontal*/, const uno::Any& /*SyncVertical*/ ) throw (uno::RuntimeException)
{
	//#TODO #FIXME see what can be done for an implementation here
}


