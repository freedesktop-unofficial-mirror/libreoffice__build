#ifndef SC_VBA_APPLICATION_HXX
#define SC_VBA_APPLICATION_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/vba/XWorksheetFunction.hpp>
#include <org/openoffice/vba/XApplication.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1< oo::vba::XApplication > ScVbaApplication_BASE;

class ScVbaApplication : public ScVbaApplication_BASE
{
private:
	css::uno::Reference< css::uno::XComponentContext > m_xContext;
	sal_Int32 m_xCalculation;
public:
	ScVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
	virtual ~ScVbaApplication();

    // XApplication
	virtual ::sal_Int32 SAL_CALL getCalculation() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::vba::XRange > SAL_CALL getSelection() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::vba::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::vba::XRange > SAL_CALL getActiveCell() throw ( css::uno::RuntimeException);
 virtual css::uno::Reference< oo::vba::XWindow > SAL_CALL getActiveWindow() throw (::com::sun::star::uno::RuntimeException);
 virtual css::uno::Reference< oo::vba::XWorksheet > SAL_CALL getActiveSheet() throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL getScreenUpdating() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setScreenUpdating(sal_Bool bUpdate) throw (css::uno::RuntimeException);
	virtual sal_Bool SAL_CALL getDisplayStatusBar() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (css::uno::RuntimeException);
	virtual css::uno::Reference< oo::vba::XWorkbook > SAL_CALL getThisWorkbook() throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Workbooks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL WorksheetFunction( ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Evaluate( const ::rtl::OUString& Name ) throw (css::uno::RuntimeException); 
	virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& DialogIndex ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getCutCopyMode() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setCutCopyMode( const css::uno::Any& _cutcopymode ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getStatusBar() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setStatusBar( const css::uno::Any& _statusbar ) throw (css::uno::RuntimeException);

	virtual double SAL_CALL CountA( const css::uno::Any& arg1 ) throw (css::uno::RuntimeException) ;

	virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL wait( double time ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL GoTo( const css::uno::Any& Reference, const css::uno::Any& Scroll ) throw (css::uno::RuntimeException);
};
#endif /* SC_VBA_APPLICATION_HXX */
