
using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;

namespace ooo.mono.comp {

public static class ServiceInfoHelper
{
    static private String[] supportedServices = {
        "mono.uno.comp.SimpleNamed"
    };
    static public String getImplementationName() { return typeof( SimpleXNamedComponent ).ToString(); }
    static public String[] getSupportedServiceNames() { return supportedServices; }
}

public class SimpleXNamedComponent : uno.util.WeakBase, unoidl.com.sun.star.container.XNamed, unoidl.com.sun.star.lang.XServiceInfo, unoidl.com.sun.star.lang.XInitialization
{
    private String msName;
    private  unoidl.com.sun.star.uno.XComponentContext mCtx;
    // ctor
    public SimpleXNamedComponent( unoidl.com.sun.star.uno.XComponentContext xCtx ) 
    {  
        Console.WriteLine( "*** In SimpleXNamedComponent ctor" );
        mCtx = xCtx; 
    } 
    // XNamed
    public void setName( String sName ) { msName = sName; }
    public String getName() { return msName; }

    // XIntitialize
    public void initialize( uno.Any[] args )
    {
        if ( args.Length == 0 )
            // probably need to change to uno.Exception
            throw new System.Exception("No arguments passed to initialize");
        msName = (String)args[0].Value;
    }

    // XServiceInfo
    public String getImplementationName() 
    {
        return ServiceInfoHelper.getImplementationName();
    }    
    public bool supportsService(String serviceName) 
    {
        String[] supportedServices = ServiceInfoHelper.getSupportedServiceNames();
        for ( int i = 0; i < supportedServices.Length; i++ ) {
            if ( supportedServices[i] == serviceName )
                return true;
        }
        return false;
    }
    public String[] getSupportedServiceNames() 
    {
        return ServiceInfoHelper.getSupportedServiceNames();
    }    

    // Special methods used for component loading and 
    public static unoidl.com.sun.star.lang.XSingleComponentFactory __getComponentFactory(String implName )
    {
        // Anounce we got here
        Console.WriteLine( "*** In __getComponentFactory for " + implName + " where the expected implementation name is " + ServiceInfoHelper.getImplementationName() );
        unoidl.com.sun.star.lang.XSingleComponentFactory xFactory = null;
        if ( implName == ServiceInfoHelper.getImplementationName() )
        {

            Console.WriteLine( "*** attempting to create component factory");
            xFactory = uno.util.Factory.createComponentFactory( typeof ( SimpleXNamedComponent ),
                                             ServiceInfoHelper.getSupportedServiceNames());
        }
        return xFactory; // return a null one 
    }
    public static bool __writeRegistryServiceInfo(unoidl.com.sun.star.registry.XRegistryKey regKey)
    {
        // Anounce we got here
        Console.WriteLine( "*** In ____writeRegistryServiceInfo about to call factory" );
        return uno.util.Factory.writeRegistryServiceInfo( ServiceInfoHelper.getImplementationName(),
                                                ServiceInfoHelper.getSupportedServiceNames(),
                                                regKey);
    }
}

}
namespace component {
static public class RegistrationClass
{
    static public String name = "ooo.mono.comp.SimpleXNamedComponent";
}

}
