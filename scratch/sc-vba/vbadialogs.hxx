#ifndef SC_VBA_DIALOGS_HXX
#define SC_VBA_DIALOGS_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/excel/XDialogs.hpp>
#include <org/openoffice/vba/XCollection.hpp>
 
class ScModelObj;

typedef ::cppu::WeakImplHelper1< oo::excel::XDialogs > ScVbaDialogs_BASE;

class ScVbaDialogs : public ScVbaDialogs_BASE
{
	css::uno::Reference< css::uno::XComponentContext > & m_xContext;
public:
	ScVbaDialogs( css::uno::Reference< css::uno::XComponentContext > &xContext ):
	m_xContext( xContext ) {} 
	virtual ~ScVbaDialogs() {}

	// XCollection
	virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() 
									throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);

	// XDialogs
	virtual void SAL_CALL Dummy() throw (css::uno::RuntimeException);
};

#endif /* SC_VBA_DIALOGS_HXX */
