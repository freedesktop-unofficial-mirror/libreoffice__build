#ifndef SC_VBA_DIALOG_HXX
#define SC_VBA_DIALOG_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/vba/XGlobals.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <org/openoffice/excel/XDialog.hpp>
 
#include "vbahelper.hxx"
#include "vbadialog.hxx"

typedef ::cppu::WeakImplHelper1< oo::excel::XDialog > ScVbaDialog_BASE;

class ScVbaDialog : public ScVbaDialog_BASE
{
	sal_Int32 mnIndex;
	css::uno::Reference< css::uno::XComponentContext > m_xContext;
public:
	ScVbaDialog( sal_Int32 nIndex, css::uno::Reference< css::uno::XComponentContext > xContext ):
		mnIndex( nIndex ),
		m_xContext( xContext ) {}
	virtual ~ScVbaDialog() {}

	// Attributes
	virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
	virtual sal_Int32 SAL_CALL getCreator() throw(css::uno::RuntimeException);
	// Methods
	virtual void SAL_CALL Show() throw (css::uno::RuntimeException);
	rtl::OUString mapIndexToName( sal_Int32 nIndex );
};

#endif /* SC_VBA_DIALOG_HXX */
