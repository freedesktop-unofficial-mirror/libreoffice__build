/*************************************************************************
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Kohei Yoshida.
 *    1039 Kingsway Dr., Apex, NC 27502, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include "solver.hxx"
#include "tool/global.hxx"
#include "dialog.hxx"
#include "lpbuilder.hxx"
#include "xcalc.hxx"
#include "option.hxx"
#include "solvemodel.hxx"
#include "numeric/matrix.hxx"

#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/frame/XDispatch.hpp"

#include <memory>

using namespace std;

namespace scsolver {

//--------------------------------------------------------------------------
// Component operations

static uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_SolverImpl();
static rtl::OUString getImplementationName_SolverImpl();
static Reference< uno::XInterface > SAL_CALL create_SolverImpl(
    Reference< uno::XComponentContext > const & xContext )
    SAL_THROW( () );


SolverImpl::SolverImpl( Reference< uno::XComponentContext > const & xContext ) :
	m_pResMgr( NULL ),
	m_pDlg( NULL ), m_pCalc( new CalcInterface( xContext ) ),
	m_pOption( new OptionData )
{
}

SolverImpl::~SolverImpl()
{
	delete m_pResMgr;
}

//--------------------------------------------------------------------------
// UNO Component Interface Methods

void SolverImpl::initialize( const Sequence< Any >& aArgs ) throw( Exception )
{
}

rtl::OUString SolverImpl::getImplementationName()
    throw( RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLEMENTATION_NAME) );
}

sal_Bool SolverImpl::supportsService( rtl::OUString const & serviceName )
    throw( RuntimeException )
{
    return serviceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SERVICE_NAME) );
}

Sequence< rtl::OUString > SolverImpl::getSupportedServiceNames()
    throw( RuntimeException )
{
    return getSupportedServiceNames_SolverImpl();
}

Reference< frame::XDispatch > SAL_CALL SolverImpl::queryDispatch(
    const util::URL& aURL, const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags )
    throw ( RuntimeException )
{
    Reference< frame::XDispatch > xRet;
    if ( aURL.Protocol.compareToAscii( "scsolver.SolverImpl:" ) == 0 )
    {
        if ( aURL.Path.compareToAscii( "execute" ) == 0 )
            xRet = this;
    }
    return xRet;
}

Sequence< Reference< frame::XDispatch > > SAL_CALL SolverImpl::queryDispatches(
    const Sequence< frame::DispatchDescriptor >& seqDescripts )
    throw ( RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    Sequence< Reference< frame::XDispatch > > lDispatcher( nCount );
    
    for ( sal_Int32 i = 0; i < nCount; ++i )
        lDispatcher[i] = queryDispatch( 
            seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );
    
    return lDispatcher;
}

void SAL_CALL SolverImpl::dispatch( 
    const util::URL& aURL, const Sequence< beans::PropertyValue >& lArgs )
    throw ( RuntimeException )
{
    if ( aURL.Protocol.compareToAscii( "scsolver.SolverImpl:" ) == 0 )
    {
        if ( aURL.Path.compareToAscii( "execute" ) == 0 )
        {
            execute();
        }
    }
}

void SAL_CALL SolverImpl::addStatusListener( 
    const Reference< frame::XStatusListener >& xControl, const util::URL& aURL )
    throw ( RuntimeException )
{
}

void SAL_CALL SolverImpl::removeStatusListener( 
    const Reference< frame::XStatusListener >& xControl, const util::URL& aURL )
    throw ( RuntimeException )
{
}

void SAL_CALL SolverImpl::dispatchWithNotification(
    const util::URL& aURL, const Sequence< beans::PropertyValue >& lArgs,
    const Reference< frame::XDispatchResultListener >& xDRL )
    throw ( RuntimeException )
{
}

SolverDialog* SolverImpl::getMainDialog()
{
	if ( m_pDlg.get() == NULL )
		m_pDlg.reset( new SolverDialog( this ) );

	return m_pDlg.get();
}

CalcInterface* SolverImpl::getCalcInterface() const
{
	return m_pCalc.get();
}

OptionData* SolverImpl::getOptionData() const
{
	return m_pOption.get();
}

void SolverImpl::setTitle( const ::rtl::OUString& aTitle )
		throw (uno::RuntimeException)
{
}

sal_Int16 SolverImpl::execute()
		throw (::com::sun::star::uno::RuntimeException)
{
	getMainDialog()->setVisible( true );
	return 0;
}

sal_Bool SolverImpl::solveModel()
{	
	Debug("solveModel --------------------------------------------------------");

	std::auto_ptr<SolveModel> p( new SolveModel( this ) );
	try
	{
		p->solve();
	}
	catch( const RuntimeError& e )
	{
		getMainDialog()->showSolveError( e.getMessage() );
	}

	if ( p->isSolved() )
	{
		Debug( "solution available" );
		return true;
	}
	
	return false;
}

void SolverImpl::initLocale()
{
	rtl::OString aModName( "scsolver" );
	aModName += rtl::OString::valueOf( sal_Int32( SUPD ) );

	m_pResMgr = ResMgr::CreateResMgr( aModName.getStr(), m_eLocale );
}

ResMgr* SolverImpl::getResMgr()
{
	if( !m_pResMgr )
		initLocale();
	return m_pResMgr;
}

rtl::OUString SolverImpl::getResStr( int resid )
{
   	ResMgr *pResMgr = getResMgr();
	if ( pResMgr )
		return rtl::OUString( String( ResId( resid, *getResMgr() ) ) );
	
	return rtl::OUString();
}

// XLocalizable

void SAL_CALL SolverImpl::setLocale( const lang::Locale& eLocale )
	throw(::com::sun::star::uno::RuntimeException)
{
	m_eLocale = eLocale;
	initLocale();
}

lang::Locale SAL_CALL SolverImpl::getLocale()
	throw(::com::sun::star::uno::RuntimeException)
{
	return m_eLocale;
}

//---------------------------------------------------------------------------
// Component operations

static Sequence< rtl::OUString > getSupportedServiceNames_SolverImpl()
{
    static Sequence < rtl::OUString > *pNames = 0;
    if( !pNames )
    {
        static Sequence< rtl::OUString > seqNames(1);
        seqNames.getArray()[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME));
        pNames = &seqNames;
    }
    return *pNames;
}

static rtl::OUString getImplementationName_SolverImpl()
{
    static rtl::OUString *pImplName = 0;
    if( !pImplName )
    {
        static rtl::OUString implName(
            RTL_CONSTASCII_USTRINGPARAM(IMPLEMENTATION_NAME));
        pImplName = &implName;
    }
    return *pImplName;
}

static Reference< uno::XInterface > SAL_CALL create_SolverImpl(
    Reference< uno::XComponentContext > const & xContext )
    SAL_THROW( () )
{
    return static_cast< lang::XTypeProvider * >( new SolverImpl( xContext ) );
}


static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    {
        create_SolverImpl, getImplementationName_SolverImpl,
        getSupportedServiceNames_SolverImpl, ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

   
}


//------------------------------------------------------------------------------
// Shared library symbol exports

extern "C"
{
    void SAL_DLLPUBLIC_EXPORT component_getImplementationEnvironment(
        sal_Char const ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
    
    sal_Bool SAL_DLLPUBLIC_EXPORT component_writeInfo(
        lang::XMultiServiceFactory * xMgr, registry::XRegistryKey * xRegistry )
    {		
        return ::cppu::component_writeInfoHelper(
            xMgr, xRegistry, ::scsolver::s_component_entries );
    }

    void SAL_DLLPUBLIC_EXPORT *component_getFactory(
        sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
        registry::XRegistryKey * xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
            implName, xMgr, xRegistry, ::scsolver::s_component_entries );
    }
}
