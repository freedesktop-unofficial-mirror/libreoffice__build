#include "cppuhelper/bootstrap.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>

#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#include <memory>
#include <iostream>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sheet;

using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiComponentFactory;
using ::com::sun::star::frame::XComponentLoader;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::ucb::XSimpleFileAccess;
using ::rtl::OUString;

using ::std::auto_ptr;

const OUString EXTN = rtl::OUString::createFromAscii(".xls"); 

OUString convertToURL( const OUString& rPath )
{
        rtl::OUString aURL;
        INetURLObject aObj;
        aObj.SetURL( rPath );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        if ( bIsURL )
                aURL = rPath;
        else
		{
                osl::FileBase::getFileURLFromSystemPath( rPath, aURL );
				if ( aURL.equals( rPath ) )
					throw uno::RuntimeException( rtl::OUString::createFromAscii( "could'nt convert " ).concat( rPath ).concat( rtl::OUString::createFromAscii( " to a URL, is it a fully qualified path name? " ) ), Reference< uno::XInterface >() );
		}
		return aURL;
}

OUString ascii(const sal_Char* cstr)
{
	return OUString::createFromAscii(cstr);
}

const sal_Char* getStr(const OUString& ou)
{
    return OUStringToOString(ou, RTL_TEXTENCODING_UTF8).getStr();
}


int usage( const char* pName )
{
	std::cerr << "usage: " << pName << "<path to testdocument dir> <output_directory>" << std::endl;
        return 1;

}

class TestVBA
{
private:
	Reference< XComponentContext >  mxContext;
	Reference< XMultiComponentFactory > mxMCF;
	Reference< XComponentLoader > mxCompLoader;
	Reference< XSimpleFileAccess > mxSFA;
	rtl::OUString msOutDirPath;
protected:
public:
	TestVBA( const Reference< XComponentContext >&  _xContext, 
		const Reference< XMultiComponentFactory >& _xMCF, 
		const Reference< XComponentLoader >& _xCompLoader, 
		const rtl::OUString& _outDirPath ) : mxContext( _xContext ), mxMCF( _xMCF ), 
mxCompLoader( _xCompLoader ), msOutDirPath( convertToURL( _outDirPath  ) ) 
	{
		mxSFA.set( mxMCF->createInstanceWithContext( rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ), mxContext), uno::UNO_QUERY_THROW );
	} 

	rtl::OUString getLogLocation() throw (  beans::UnknownPropertyException,  lang::IllegalArgumentException, lang::WrappedTargetException,  uno::Exception ) 
	{
		rtl::OUString sLogLocation;
		Reference< XPropertySet > pathSettings( mxMCF->createInstanceWithContext( rtl::OUString::createFromAscii( "com.sun.star.comp.framework.PathSettings" ), mxContext), uno::UNO_QUERY_THROW );
		pathSettings->getPropertyValue( rtl::OUString::createFromAscii( "Work" ) ) >>= sLogLocation;
		sLogLocation = sLogLocation.concat( rtl::OUString::createFromAscii( "/" ) ).concat( rtl::OUString::createFromAscii( "HelperAPI-test.log" ) );
		return sLogLocation;
	}

	void init() 
	{
		// blow away previous logs?
	}

	void traverse( const rtl::OUString& sFileDirectory ) 
	{
		rtl::OUString sFileDirectoryURL = convertToURL( sFileDirectory );	
		if ( !mxSFA->isFolder( sFileDirectoryURL) ) 
		{
			throw lang::IllegalArgumentException( rtl::OUString::createFromAscii( "not a directory: ").concat( sFileDirectoryURL ), Reference<uno::XInterface>(), 1 );
		}	
		// Getting all files and directories in the current directory
		Sequence<OUString> entries = mxSFA->getFolderContents( sFileDirectoryURL, sal_False );
	   	 
		// Iterating for each file and directory
		for ( sal_Int32 i = 0; i < entries.getLength(); ++i ) 
		{
		// Testing, if the entry in the list is a directory
		//
		//            if ( entries[ i ].isDirectory() ) 
		//            {
		//                // ignore Recursive call for the new directory
		//                // traverse( entries[ i ] );
		//            } 
		//            else 
			rtl::OUString sUrl = entries[ i ];
			if ( !mxSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCaseAsciiL( ".xls", 4 ) ) 
				
			{
				try 
				{
					OSL_TRACE( "processing %s",  rtl::OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
					// Loading the wanted document
					Sequence< PropertyValue > propertyValues(1);
					propertyValues[0].Name = rtl::OUString::createFromAscii( "Hidden" );
					propertyValues[0].Value <<= sal_False;
				
					Reference< uno::XInterface > xDoc =	
						mxCompLoader->loadComponentFromURL( sUrl, rtl::OUString::createFromAscii( "_blank" ), 0, propertyValues);
					OUString logFileURL = convertToURL( getLogLocation() );
					try
					{
						Reference< script::provider::XScriptProviderSupplier > xSupplier( xDoc, uno::UNO_QUERY_THROW ) ; 
						if ( mxSFA->exists( logFileURL ) )	
							mxSFA->kill( logFileURL );
					
						Reference< script::provider::XScriptProvider > xProv = xSupplier->getScriptProvider();
						Reference< script::provider::XScript > xScript = xProv->getScript( rtl::OUString::createFromAscii( "vnd.sun.star.script:Standard.TestMacros.Main?language=Basic&location=document" ));
						OSL_TRACE("Got script for doc %s", rtl::OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
						Sequence< uno::Any > aArgs;
						Sequence< sal_Int16 > aOutArgsIndex;
						Sequence< uno::Any > aOutArgs;
						
						xScript->invoke(aArgs, aOutArgsIndex, aOutArgs); 
		
						OUString fileName = sUrl.copy ( sUrl.lastIndexOf( '/' ) );
						OUString newLocation = msOutDirPath + fileName.copy ( 0, fileName.lastIndexOf( EXTN )  ) + rtl::OUString::createFromAscii( ".log" );
						mxSFA->move( logFileURL, newLocation );
						OSL_TRACE("new logfile location is %s ", rtl::OUStringToOString( newLocation, RTL_TEXTENCODING_UTF8 ).getStr() ); 
						
					}
					catch ( uno::Exception& e )
					{
						std::cerr << "Caught exception " << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
					}

					// interface is supported, otherwise use XComponent.dispose
					Reference< util::XCloseable > xCloseable ( xDoc, uno::UNO_QUERY );
		
					if ( xCloseable.is() ) 
					{
						xCloseable->close(sal_False);
					} 
					else 
					{
						Reference< XComponent > xComp( xDoc, uno::UNO_QUERY_THROW );
		                        
						xComp->dispose();
					}
				}
				catch( uno::Exception& e ) 
				{
					std::cerr << "Caught exception " << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
				}
		                
			}
		}
	}	
};

void tryDispose( Reference< uno::XInterface > xIF, const char* sComp )
{
	Reference< lang::XComponent > xComponent( xIF, uno::UNO_QUERY );
	if ( xComponent.is() )
	{
		try
		{
			xComponent->dispose();
		}	
		catch( uno::Exception& e )
		{
			std::cerr << "tryDispose caught exception " <<rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << " while disposing " <<  sComp << std::endl;
		}
	}
}
int main( int argv, char** argc )
{
	if ( !( argv > 2 ) )
		return usage( argc[0] );
	try
	{

		OSL_TRACE("Attempting to bootstrap normal");
		Reference<XComponentContext> xCC = ::cppu::bootstrap();
		Reference<XMultiComponentFactory> xFactory = xCC->getServiceManager();
		OSL_TRACE("got servicemanager");
		Reference<XInterface> desktop = xFactory->createInstanceWithContext(
		ascii("com.sun.star.frame.Desktop"), xCC);
		OSL_TRACE("got desktop");
		Reference<frame::XComponentLoader> xLoader(desktop, UNO_QUERY_THROW);
		TestVBA* dTest = new TestVBA( xCC, xFactory, xLoader, ascii( argc[ 2 ] ) );
		dTest->traverse( ascii( argc[ 1 ] ) );
		delete dTest;
//		tryDispose( xLoader, "desktop" );
//		tryDispose( xCC, "remote context" );

	}
	catch( uno::Exception& e )
	{
		std::cerr << "Caught Exception " << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
	}

}
