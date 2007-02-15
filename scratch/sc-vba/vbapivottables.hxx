#ifndef SC_VBA_PIVOTTABLES_HXX
#define SC_VBA_PIVOTTABLES_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/excel/XPivotTables.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
 
#include "vbahelper.hxx"
#include "vbacollectionimpl.hxx"


typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::excel::XPivotTables > ScVbaPivotTables_BASE;

class ScVbaPivotTables : public ScVbaPivotTables_BASE
{
protected:
public:
	ScVbaPivotTables( const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );
	virtual ~ScVbaPivotTables() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);


	// XPivotTables

	// ScVbaCollectionBaseImpl	
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 

};

#endif //SC_VBA_PIVOTTABLES
