#ifndef SC_VBA_WORKBOOKS_HXX
#define SC_VBA_WORKBOOKS_HXX


#include "vbacollectionimpl.hxx"
#include <org/openoffice/vba/XGlobals.hpp>
#include <org/openoffice/vba/XWorkbooks.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include "vbahelper.hxx"


class ScModelObj;

typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::vba::XWorkbooks > ScVbaWorkbooks_BASE;

class ScVbaWorkbooks : public ScVbaWorkbooks_BASE
{
private:
	css::uno::Reference< oo::vba::XGlobals > getGlobals() throw (css::uno::RuntimeException);
	
public:
	//ScVbaWorkbooks( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );
	ScVbaWorkbooks( const css::uno::Reference< css::uno::XComponentContext >& xContext );
	virtual ~ScVbaWorkbooks() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

	// ScVbaCollectionBaseImpl	
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 

	// XWorkbooks
	virtual css::uno::Any SAL_CALL Add() throw (css::uno::RuntimeException);
	virtual void SAL_CALL Close(  ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL Open(const css::uno::Any &aFileName) throw (css::uno::RuntimeException);

// delete these
        //virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
        //virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
        //virtual ::sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
        //virtual css::uno::Reference< oo::vba::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);

        //virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);

};

#endif /* SC_VBA_WORKBOOKS_HXX */
