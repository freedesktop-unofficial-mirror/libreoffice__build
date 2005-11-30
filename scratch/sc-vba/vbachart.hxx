#ifndef SC_VBA_CHART_HXX
#define SC_VBA_CHART_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <org/openoffice/vba/XChart.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::vba::XChart > ChartImpl_BASE;

class ScVbaChart : public ChartImpl_BASE
{		

	css::uno::Reference< css::uno::XComponentContext > m_xContext;
	css::uno::Reference< css::chart::XChartDocument > m_xChartDoc;
public:
	ScVbaChart( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::chart::XChartDocument >& xChartDoc ) : m_xContext(xContext), m_xChartDoc( xChartDoc ) {}
	virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
	virtual css::uno::Any SeriesCollection(const css::uno::Any&) throw (css::uno::RuntimeException);

};

#endif //SC_VBA_WINDOW_HXX
