#ifndef SC_VBA_CHARTOBJECTS_HXX
#define SC_VBA_CHARTOBJECTS_HXX
#include <org/openoffice/excel/XChartObjects.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include"vbacollectionimpl.hxx"
#include "vbahelper.hxx"


typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::excel::XChartObjects > ChartObjects_BASE;

class ScVbaChartObjects : public ChartObjects_BASE
{		

	css::uno::Reference< css::uno::XComponentContext > m_xContext;
	css::uno::Reference< css::table::XTableCharts > m_xTableCharts;
public:
	ScVbaChartObjects( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XTableCharts >& xTableCharts ) : ChartObjects_BASE(xContext, css::uno::Reference< css::container::XIndexAccess >( xTableCharts, css::uno::UNO_QUERY ) ), m_xContext(xContext), m_xTableCharts( xTableCharts ) {}
	// XEnumerationAccess
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
	// XElementAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	// ScVbaCollectionBaseImpl	
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 
};

#endif //SC_VBA_WINDOW_HXX
