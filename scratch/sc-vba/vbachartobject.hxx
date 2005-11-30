#ifndef SC_VBA_CHARTOBJECT_HXX
#define SC_VBA_CHARTOBJECT_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <org/openoffice/vba/XChartObject.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::vba::XChartObject > ChartObjectImpl_BASE;

class ScVbaChartObject : public ChartObjectImpl_BASE
{		

	css::uno::Reference< css::uno::XComponentContext > m_xContext;
	css::uno::Reference< css::table::XTableChart  > m_xTableChart;
public:
	ScVbaChartObject( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XTableChart >& xTableChart ) : m_xContext(xContext), m_xTableChart( xTableChart ) {}
	virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::vba::XChart > SAL_CALL getChart() throw (css::uno::RuntimeException);
	
	virtual css::uno::Any SAL_CALL test(  ) throw (css::uno::RuntimeException); 
};

#endif //SC_VBA_WINDOW_HXX
