using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;
using unoidl.org.openoffice.sheet.addin;

namespace ooo.mono.comp {

static class ServiceInfoHelper
{
    static private String __serviceName = "org.openoffice.sheet.addin.CalcAddins";
    static private String[] supportedServices = { 
       "com.sun.star.sheet.AddIn", 
        __serviceName
    };
    static public String getImplementationName() { return typeof( CalcAddins ).ToString();  }
    static public String getServiceName() { return __serviceName; }
    static public String[] getSupportedServiceNames() { return supportedServices; }
}


public class CalcAddins : uno.util.WeakBase, unoidl.org.openoffice.sheet.addin.XCalcAddins, unoidl.com.sun.star.sheet.XAddIn, unoidl.com.sun.star.lang.XServiceInfo, unoidl.com.sun.star.lang.XServiceName

{
        
        private Locale aFuncLoc;
        
        private static String[] stringFunctionName = {
/** TO DO:
 * You should replace these method names by the method names of your interface.
 */
            "getMyFirstValue",
            "getMySecondValue"
        };
        
        private static short shortINVALID = -1;
        
/** TO DO:
 * For each of your methods you should make up a new constant with a different value.
 */
        private const short shortGETMYFIRSTVALUE = 0;
        private const short shortGETMYSECONDVALUE = 1;
        
    private  unoidl.com.sun.star.uno.XComponentContext mCtx;
    // ctor
    public CalcAddins( unoidl.com.sun.star.uno.XComponentContext xCtx ) 
    {  
        mCtx = xCtx; 
    }         
/** TO DO:
 * This is where you implement all methods of your interface. The parameters have to
 * be the same as in your IDL file and their types have to be the correct 
 * IDL-to-Java mappings of their types in the IDL file.
 */
        public int getMyFirstValue(
unoidl.com.sun.star.beans.XPropertySet xOptions
        ) {
            return (int) 1;
        }
        
        public int getMySecondValue(
            unoidl.com.sun.star.beans.XPropertySet xOptions,
            int intDummy
        ) {
            return( (int) ( 2 + intDummy ) );
        }
        
        
        // Implement methods from interface XAddIn
        public String getDisplayArgumentName(String stringProgrammaticFunctionName,int intArgument) {
            String stringReturn = "";
            
            switch( this.getFunctionID( stringProgrammaticFunctionName ) ) {
/** TO DO:
 * You should list all argument names for each of your methods, here.
 */
                case shortGETMYFIRSTVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                    }
                    break;
                case shortGETMYSECONDVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                        case 1:
                            stringReturn = "intDummy";
                            break;
                    }
                    break;
            }
            return( stringReturn );
        }
        
        public String getDisplayFunctionName(String stringProgrammaticName) {
            String stringReturn = "";
            
            switch( this.getFunctionID( stringProgrammaticName ) ) {
/** TO DO:
 * Assign the name of each of your methods.
 */
                case shortGETMYFIRSTVALUE:
                    stringReturn = "(Mono)getMyFirstValue";
                    break;
                case shortGETMYSECONDVALUE:
                    stringReturn = "(Mono)getMySecondValue";
                    break;
            }
            
            return( stringReturn );
        }
        
        public String getProgrammaticCategoryName(String p1) {
            return( "(Mono)Add-In" );
        }
        
        public String getDisplayCategoryName(String p1) {
            return( "(Mono)Add-In" );
        }
        
        public String getFunctionDescription(String stringProgrammaticName) {
            String stringReturn = "";
            
            switch( this.getFunctionID( stringProgrammaticName ) ) {
/** TO DO:
 * Enter a description for each of your methods that office users will understand.
 */
                case shortGETMYFIRSTVALUE:
                    stringReturn = "(message from mono) This is your first method.";
                    break;
                case shortGETMYSECONDVALUE:
                    stringReturn = "(message from mono) This is your second method.";
                    break;
            }
            
            return( stringReturn );
        }
        
        public String getArgumentDescription(String stringProgrammaticFunctionName,int intArgument) {
            String stringReturn = "";
            
            switch( this.getFunctionID( stringProgrammaticFunctionName ) ) {
/** TO DO:
 * Enter a description for every argument of every method. Make them so that office users will understand.
 */
                case shortGETMYFIRSTVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                    }
                    break;
                case shortGETMYSECONDVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                        case 1:
                            stringReturn = "You can add this value.";
                            break;
                    }
                    break;
            }
            return( stringReturn );
        }
        
        public String getProgrammaticFuntionName(String p1) {
            return( "" );
        }
        
        // Implement methods from interface XLocalizable
        public Locale getLocale() {
            return( aFuncLoc );
        }
        
        public void setLocale(Locale p1) {
            aFuncLoc = p1;
        }
        
        // Auxiliary functions
        private short getFunctionID( String stringProgrammaticName ) {
            for ( int i = 0; i < stringFunctionName.Length; i++ ) {
                if ( stringProgrammaticName == stringFunctionName[ i ] ) {
                    return( ( short ) i );
                }
            }
            
            return( -1 );
        }
        // Implement method from interface XServiceName
        public String getServiceName() {
            return ServiceInfoHelper.getServiceName();
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
        unoidl.com.sun.star.lang.XSingleComponentFactory xFactory = null;
        //if ( implName == ServiceInfoHelper.getImplementationName() )
        if ( implName == ServiceInfoHelper.getServiceName() )
        {

            xFactory = uno.util.Factory.createComponentFactory( typeof ( CalcAddins ),
                                             ServiceInfoHelper.getSupportedServiceNames());
        }
        return xFactory;
    }
    public static bool __writeRegistryServiceInfo(unoidl.com.sun.star.registry.XRegistryKey regKey)
    {
        return uno.util.Factory.writeRegistryServiceInfo( ServiceInfoHelper.getServiceName(),
                                                ServiceInfoHelper.getSupportedServiceNames(),
                                                regKey);
    }
}

}

// Hack to discover the name of the component
namespace component {
static public class RegistrationClass
{
    static public String name = "ooo.mono.comp.CalcAddins";
}

}
