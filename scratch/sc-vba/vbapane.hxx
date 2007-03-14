#ifndef SC_VBA_PANE_HXX
#define SC_VBA_PANE_HXX

#include<cppuhelper/implbase1.hxx>
#include<com/sun/star/sheet/XViewPane.hpp>
#include<org/openoffice/excel/XPane.hpp>

#include"vbahelper.hxx"

typedef cppu::WeakImplHelper1< oo::excel::XPane > PaneImpl_Base;

class ScVbaPane : public PaneImpl_Base
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::sheet::XViewPane > m_xViewPane;
public:
    ScVbaPane( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XViewPane > xViewPane ) : m_xContext( xContext ), m_xViewPane( xViewPane ) {}

    css::uno::Reference< css::sheet::XViewPane > getViewPane() { return m_xViewPane; }

    //Attribute
    virtual sal_Int32 SAL_CALL getScrollColumn() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScrollColumn( sal_Int32 _scrollcolumn ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getScrollRow() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScrollRow( sal_Int32 _scrollrow ) throw (css::uno::RuntimeException);

    //Method
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);

};

#endif//SC_VBA_PANE_HXX
