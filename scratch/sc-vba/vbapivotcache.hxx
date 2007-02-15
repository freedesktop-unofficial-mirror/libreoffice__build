#ifndef SC_VBA_PIVOTCACHE_HXX
#define SC_VBA_PIVOTCACHE_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>

#include <org/openoffice/excel/XPivotCache.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::excel::XPivotCache > PivotCacheImpl_BASE;

class ScVbaPivotCache : public PivotCacheImpl_BASE
{		
	css::uno::Reference< css::uno::XComponentContext > m_xContext;
	css::uno::Reference< css::sheet::XDataPilotTable > m_xTable;

public:
	ScVbaPivotCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XDataPilotTable >& xTable );
	virtual void SAL_CALL Refresh() throw (css::uno::RuntimeException);
};

#endif //SC_VBA_PIVOTCACHE_HXX
