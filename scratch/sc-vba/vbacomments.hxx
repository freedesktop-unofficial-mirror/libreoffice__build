#ifndef SC_VBA_COMMENTS_HXX
#define SC_VBA_COMMENTS_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/excel/XComments.hpp>

#include "vbahelper.hxx"
#include "vbacollectionimpl.hxx"
#include "vbacomment.hxx"

typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::excel::XComments > ScVbaComments_BASE;

class ScVbaComments : public ScVbaComments_BASE
{
public:
    ScVbaComments( const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );

    virtual ~ScVbaComments() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // ScVbaCollectionBaseImpl	
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 

};

#endif /* SC_VBA_COMMENTS_HXX */

