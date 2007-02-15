#ifndef SC_VBA_WINDOWS_HXX
#define SC_VBA_WINDOWS_HXX

#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/excel/XWindows.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
 
#include "vbahelper.hxx"
#include "vbacollectionimpl.hxx"


typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::excel::XWindows > ScVbaWindows_BASE;

class ScVbaWindows : public ScVbaWindows_BASE
{
protected:
public:
	ScVbaWindows( const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );
	virtual ~ScVbaWindows() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);


	// XWindows
	virtual void SAL_CALL Arrange( ::sal_Int32 ArrangeStyle, const css::uno::Any& ActiveWorkbook, const css::uno::Any& SyncHorizontal, const css::uno::Any& SyncVertical ) throw (::com::sun::star::uno::RuntimeException);
	// ScVbaCollectionBaseImpl	
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 

	static css::uno::Reference< oo::vba::XCollection > Windows(  const css::uno::Reference< css::uno::XComponentContext >& xContext );

};

#endif //SC_VBA_WINDOWS_HXX

