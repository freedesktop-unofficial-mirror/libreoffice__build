#include "vbaaxistitle.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

ScVbaAxisTitle::ScVbaAxisTitle( const uno::Reference< vba::XHelperInterface >& xParent,  const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& _xTitleShape ) :  AxisTitleBase( xParent, xContext, _xTitleShape )
{
}

rtl::OUString&
ScVbaAxisTitle::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAxisTitle") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
ScVbaAxisTitle::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		uno::Sequence< rtl::OUString > BaseServiceNames = AxisTitleBase::getServiceNames();
		aServiceNames.realloc( BaseServiceNames.getLength() + 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.AxisTitle" ) );
		for ( sal_Int32 index = 1; index < (BaseServiceNames.getLength() + 1); ++index )
			aServiceNames[ index ] = BaseServiceNames[ index ];
	}
	return aServiceNames;
}


