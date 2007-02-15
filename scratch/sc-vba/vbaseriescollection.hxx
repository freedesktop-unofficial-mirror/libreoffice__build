#ifndef SC_VBA_SERIESCOLLECTION_HXX
#define SC_VBA_SERIESCOLLECTION_HXX
#include <org/openoffice/excel/XSeriesCollection.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper2<oo::excel::XSeriesCollection,
css::container::XEnumerationAccess > SeriesCollectionImpl_BASE;

class ScVbaSeriesCollection : public SeriesCollectionImpl_BASE
{		

	css::uno::Reference< css::uno::XComponentContext > m_xContext;
public:
	ScVbaSeriesCollection( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : m_xContext(xContext) {}
	//XCollection
	virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);

	// XEnumerationAccess
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);	
	// XElementAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);
};

#endif //SC_VBA_WINDOW_HXX
