#include "vbaseriescollection.hxx"
#include <org/openoffice/vba/XSeries.hpp>

#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

uno::Reference< oo::vba::XApplication > 
ScVbaSeriesCollection::getApplication() throw (uno::RuntimeException)
{
	return ScVbaGlobals::getGlobalsImpl( m_xContext )->getApplication();
}
::sal_Int32
ScVbaSeriesCollection::getCount() throw (uno::RuntimeException)
{
	return 0;
}

uno::Any
ScVbaSeriesCollection::Item( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return uno::Any();
}

// XEnumerationAccess

uno::Reference< container::XEnumeration >
ScVbaSeriesCollection::createEnumeration() throw (uno::RuntimeException)
{
	uno::Reference< container::XEnumeration > xEnum;
	return xEnum;
}

// XElementAccess

uno::Type 
ScVbaSeriesCollection::getElementType() throw (uno::RuntimeException)
{
	return vba::XSeries::static_type(0);
}
::sal_Bool
ScVbaSeriesCollection::hasElements() throw (uno::RuntimeException)
{
	// #TODO #TOFIX Really?, how can we say that!
	// needs to delegate to xIndex
	return sal_True;
}

uno::Any SAL_CALL 
ScVbaSeriesCollection::getParent() throw (uno::RuntimeException)
{
	uno::Reference< vba::XApplication > xApplication =
		getApplication();
	uno::Reference< vba::XWorkbook > xWorkbook;
	if ( xApplication.is() )
	{
		xWorkbook = xApplication->getActiveWorkbook();
	}
	return uno::Any( xWorkbook );
}

sal_Int32 SAL_CALL 
ScVbaSeriesCollection::getCreator() throw (uno::RuntimeException)
{
	SC_VBA_STUB();
	return 0;
}





