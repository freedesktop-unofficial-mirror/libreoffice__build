
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <org/openoffice/excel/XOLEObject.hpp>

#include "vbaoleobject.hxx"
#include "vbaoleobjects.hxx"

using namespace com::sun::star;

ScVbaOLEObjects::ScVbaOLEObjects( const uno::Reference< uno::XComponentContext >& xContext,
                const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess )
            : OLEObjectsImpl_BASE( xContext, xIndexAccess )
{
}
uno::Reference< container::XEnumeration >
ScVbaOLEObjects::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumAccess->createEnumeration();
}

uno::Any
ScVbaOLEObjects::createCollectionObject( const css::uno::Any& aSource )
{
    if( aSource.hasValue() )
    {
        uno::Reference< drawing::XControlShape > xControlShape( aSource, uno::UNO_QUERY_THROW );
        return makeAny( uno::Reference< oo::excel::XOLEObject >( new ScVbaOLEObject( m_xContext, xControlShape ) ) );
    }
    return uno::Any();
}

uno::Any
ScVbaOLEObjects::getItemByStringIndex( const rtl::OUString& sIndex ) throw (uno::RuntimeException)
{
    try
    {
        return ScVbaCollectionBaseImpl::getItemByStringIndex( sIndex );
    }
    catch( uno::RuntimeException )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
        sal_uInt16 nCount = xIndexAccess->getCount();
        for( int index = 0; index < nCount; index++ )
        {
            uno::Any aUnoObj =  xIndexAccess->getByIndex( index );
            uno::Reference< drawing::XControlShape > xControlShape( aUnoObj, uno::UNO_QUERY_THROW );
            uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl() );
            uno::Reference< container::XNamed > xNamed( xControlModel, uno::UNO_QUERY_THROW );
            if( sIndex.equals( xNamed->getName() ))
            {
                return createCollectionObject( aUnoObj );
            }

        }
        return uno::Any();
    }
}

uno::Type
ScVbaOLEObjects::getElementType() throw (uno::RuntimeException)
{
    return org::openoffice::excel::XOLEObjects::static_type(0);
}

