#include "vbapivottables.hxx"
#include "vbapivottable.hxx"
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <org/openoffice/vba/XPivotTable.hpp>


using namespace ::com::sun::star;
using namespace ::org::openoffice;

uno::Any DataPilotToPivotTable( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext )
{
	uno::Reference< sheet::XDataPilotTable > xTable( aSource, uno::UNO_QUERY_THROW );
	return uno::makeAny( uno::Reference< vba::XPivotTable > ( new ScVbaPivotTable( xContext, xTable ) ) );
}

class PivotTableEnumeration : public EnumerationHelperImpl
{
public:
	PivotTableEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ) {}

	virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException) 
	{ 
		return DataPilotToPivotTable( m_xEnumeration->nextElement(),  m_xContext );
	}

};

ScVbaPivotTables::ScVbaPivotTables( const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess  ):  ScVbaPivotTables_BASE( xContext, xIndexAccess ) 
{
}

uno::Reference< container::XEnumeration >
ScVbaPivotTables::createEnumeration() throw (uno::RuntimeException)
{
	uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
	return new PivotTableEnumeration( m_xContext, xEnumAccess->createEnumeration() );
}

uno::Any
ScVbaPivotTables::createCollectionObject( const css::uno::Any& aSource )
{
	return DataPilotToPivotTable( aSource,  m_xContext );
}

uno::Type 
ScVbaPivotTables::getElementType() throw (uno::RuntimeException)
{
	return vba::XPivotTables::static_type(0);
}
