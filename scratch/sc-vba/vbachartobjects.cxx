#include "vbachart.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>


#include "vbachartobjects.hxx"
#include "vbachartobject.hxx"
#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;


class ChartObjectEnumerationImpl : public EnumerationHelperImpl
{
public:

	ChartObjectEnumerationImpl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ) {}
	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		uno::Reference< table::XTableChart > xTableChart( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
		return makeAny( uno::Reference< vba::XChartObject > ( new ScVbaChartObject( m_xContext, xTableChart ) ) );
	}
};



// XEnumerationAccess

uno::Reference< container::XEnumeration >
ScVbaChartObjects::createEnumeration() throw (uno::RuntimeException)
{
	css::uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xTableCharts, uno::UNO_QUERY_THROW );
	return new ChartObjectEnumerationImpl( m_xContext, xEnumAccess->createEnumeration() );
}

// XElementAccess

uno::Type 
ScVbaChartObjects::getElementType() throw (uno::RuntimeException)
{
	return vba::XChartObject::static_type(0);
}

// ScVbaCollectionBaseImpl
uno::Any
ScVbaChartObjects::createCollectionObject( const css::uno::Any& aSource )
{	
	uno::Reference< table::XTableChart > xTableChart( aSource, uno::UNO_QUERY_THROW );
	return makeAny( uno::Reference< vba::XChartObject > ( new ScVbaChartObject( m_xContext, xTableChart ) ) );
}

