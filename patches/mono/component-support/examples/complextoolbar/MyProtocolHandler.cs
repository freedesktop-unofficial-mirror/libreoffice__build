using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;
using unoidl.com.sun.star.frame;
using unoidl.com.sun.star.util;
using unoidl.com.sun.star.awt;
using unoidl.com.sun.star.beans;
using unoidl.com.sun.star.system;
using unoidl.com.sun.star.text;
using unoidl.com.sun.star.sheet;

// #TODO #FIXME really use some proper Debugging class for 
// trace messages
using unoidl.com.sun.star.container;

namespace ooo.mono.comp {

public static class ServiceInfoHelper
{
    static private String[] supportedServices = {
        "vnd.demo.ProtocolHandler",
        "com.sun.star.frame.ProtocolHandler"
    };
    static public String getImplementationName() { return typeof( MyProtocolHandler ).ToString(); }
    static public String[] getSupportedServiceNames() { return supportedServices; }
    static public bool supportsService(String serviceName) 
    {
        String[] supportedServices = getSupportedServiceNames();
        for ( int i = 0; i < supportedServices.Length; i++ ) {
            if ( supportedServices[i] == serviceName )
                return true;
        }
        return false;
    }

}

public class MyProtocolHandler : uno.util.WeakBase, XDispatchProvider, XInitialization, XServiceInfo
{
    private XMultiServiceFactory mxMSF;
    private XFrame mxFrame; 
/*
    public MyProtocolHandler( XMultiServiceFactory rxMSF)
    {
        mxMSF = rxMSF;
    }
*/
    public MyProtocolHandler( XComponentContext rxMSF)
    {
        Console.WriteLine( "***  MyProtocolHandler( XComponentContext ) ");
        mxMSF = ( XMultiServiceFactory )rxMSF.getServiceManager();
    }
    
    // XInitialise
    public void initialize( uno.Any[] aArguments )
    {
        Console.WriteLine( "***  MyProtocolHandler.Initialise ");
        XFrame xFrame = null;
        if ( aArguments.Length > 0 )
        {
            xFrame =  ( XFrame )aArguments[0].Value;
            mxFrame = xFrame;
            XStatusListener xList = null;
            XNamed xNamed = null;
            if (  aArguments.Length > 1 )
            {
                try
                {
                    xList = ( XStatusListener ) aArguments[1].Value;
                }
                catch( System.Exception e )
                {
                    Console.WriteLine( "***  MyProtocolHandler.Initialise, caught exception " + e);
                }
                try
                {
                    xNamed = ( XNamed ) aArguments[1].Value;
                }
                catch( System.Exception e )
                {
                }
                if ( xNamed != null )
                    xNamed.setName("foo");

                if ( xList != null )
                {
                    FeatureStateEvent aEvent = new FeatureStateEvent();
                    xList.statusChanged( aEvent );
                }
            }
        }
    }
    // XDispatchProvider
    public XDispatch queryDispatch( URL aURL, String sTargetFrameName, int nSearchFlags )
    {
        Console.WriteLine( "***  MyProtocolHandler.queryDispatch ( URL.Protocol ) " + aURL.Protocol + " URL.Path " + aURL.Path );
        XDispatch xRet = null ;
        if ( mxFrame == null )
            return null;

        XController xCtrl = mxFrame.getController();

        if ( xCtrl != null && aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
        {
            XTextViewCursorSupplier xCursor = null;
            XSpreadsheetView xView = null;
            try 
            {
               xView = (XSpreadsheetView )xCtrl;
               xCursor = ( XTextViewCursorSupplier )xCtrl;
            }
            catch ( System.Exception )
            {
            }
            if ( xCursor == null && xView == null )
                // ohne ein entsprechendes Dokument funktioniert der Handler nicht
                return xRet;
            

            if ( aURL.Path == "Command1" ||
                 aURL.Path == "Command2" ||
                 aURL.Path == "Command3" ||
                 aURL.Path == "Command4" ||
                 aURL.Path == "Command5" ||
                 aURL.Path == "Command6" ||
                 aURL.Path == "Command7" )
            {
                xRet = BaseDispatch.aListenerHelper.GetDispatch( mxFrame, aURL.Path );
                if ( xRet == null )
                {
                    if ( xCursor != null )
                        xRet = new WriterDispatch( mxMSF, mxFrame );
                    else
                        xRet = new CalcDispatch( mxMSF, mxFrame ); 
                    BaseDispatch.aListenerHelper.AddDispatch( xRet, mxFrame, aURL.Path );
                }
            }
        }
        return xRet;
    }
    public XDispatch[] queryDispatches( DispatchDescriptor[] seqDescripts )
    { 
        Console.WriteLine( "***  MyProtocolHandler.queryDispatches ");
        int nCount = seqDescripts.Length;
        XDispatch[] lDispatcher = new XDispatch[ nCount ];

        for( int i=0; i<nCount; ++i )
            lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

        return lDispatcher;
    }
    // XServiceInfo
    public String getImplementationName( )
    {
        return ServiceInfoHelper.getImplementationName();
    }

    public bool supportsService( String rServiceName )
    {
        return ServiceInfoHelper.supportsService( rServiceName );
    }

    public String[] getSupportedServiceNames(  )
    {
        return ServiceInfoHelper.getSupportedServiceNames();
    }

    // Special methods used for component loading and 
    public static unoidl.com.sun.star.lang.XSingleComponentFactory __getComponentFactory(String implName )
    {
        // Announce we got here
        Console.WriteLine( "*** In __getComponentFactory for " + implName + " where the expected implementation name is " + ServiceInfoHelper.getImplementationName() );
        unoidl.com.sun.star.lang.XSingleComponentFactory xFactory = null;
        if ( implName == ServiceInfoHelper.getImplementationName() )
        {

            Console.WriteLine( "*** attempting to create component factory");
            xFactory = uno.util.Factory.createComponentFactory( typeof ( MyProtocolHandler ),
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

public class BaseDispatch : uno.util.WeakBase, XDispatch, XControlNotificationListener
{
    protected XFrame mxFrame;
    protected XMultiServiceFactory mxMSF;
    protected XToolkit mxToolkit;
    protected String msDocService;
    protected String maComboBoxText="";
    protected bool mbButtonEnabled=false;
    public static ListenerHelper aListenerHelper= new ListenerHelper();
   
    public BaseDispatch( XMultiServiceFactory rxMSF,
        XFrame xFrame, String rServiceName ) 
    {
        mxMSF = rxMSF;
        mxFrame = xFrame; 
        msDocService = rServiceName;
        mbButtonEnabled = true;
    }
    public void ShowMessageBox( XFrame rFrame, String aTitle, String aMsgText )
    {
        Console.WriteLine("ShowMessageBox");
        if ( mxToolkit == null )
            mxToolkit = ( XToolkit )mxMSF.createInstance( "com.sun.star.awt.Toolkit" );
        if ( rFrame != null && mxToolkit != null )
        {
            // describe window properties.
            WindowDescriptor aDescriptor = new WindowDescriptor();
            aDescriptor.Type = WindowClass.MODALTOP;
            aDescriptor.WindowServiceName = "infobox";
            aDescriptor.ParentIndex = -1;
            aDescriptor.Parent = ( XWindowPeer )rFrame.getContainerWindow();
            aDescriptor.Bounds = new Rectangle(0,0,300,200);
            aDescriptor.WindowAttributes  = WindowAttribute.BORDER | WindowAttribute.MOVEABLE | WindowAttribute.CLOSEABLE;

            XWindowPeer xPeer = mxToolkit.createWindow( aDescriptor );
            if ( xPeer != null )
            {
                XMessageBox xMsgBox = ( XMessageBox ) xPeer;
                if ( xMsgBox != null  )
                {
                    xMsgBox.setCaptionText( aTitle );
                    xMsgBox.setMessageText( aMsgText );
                    xMsgBox.execute();
                }
            }
        }
    }
    public void SendCommand( URL aURL, String rCommand, NamedValue[] rArgs, bool bEnabled )
    {
        
        Console.WriteLine( "*** BaseDispatch.SendCommand " + rCommand );
        XDispatch xDispatch = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
    
        FeatureStateEvent aEvent = new FeatureStateEvent();
    
        aEvent.FeatureURL = aURL;
        aEvent.Source     = xDispatch;
        aEvent.IsEnabled  = bEnabled;
        aEvent.Requery    = false;

        ControlCommand aCtrlCmd = new ControlCommand();
        aCtrlCmd.Command   = rCommand;
        aCtrlCmd.Arguments = rArgs;
    
        aEvent.State = new uno.Any( aCtrlCmd.GetType(), aCtrlCmd );
        aListenerHelper.Notify( mxFrame, aEvent.FeatureURL.Path, aEvent ); 

    }
    public void SendCommandTo( XStatusListener xControl, URL aURL, String rCommand, NamedValue[] rArgs, bool bEnabled )
    {
        Console.WriteLine( "*** BaseDispatch.SendCommandTo " + rCommand );
        FeatureStateEvent aEvent = new FeatureStateEvent();
    
        aEvent.FeatureURL = aURL;
        aEvent.Source     = this;
        aEvent.IsEnabled  = bEnabled;
        aEvent.Requery    = false;

        ControlCommand aCtrlCmd = new ControlCommand();
        aCtrlCmd.Command   = rCommand;
        aCtrlCmd.Arguments = rArgs;
    
        aEvent.State = new uno.Any( aCtrlCmd.GetType(), aCtrlCmd );
        xControl.statusChanged( aEvent );
    }
    // XDispatch
    public void dispatch( URL aURL,
        PropertyValue[] lArgs )
    {
        Console.WriteLine( "*** BaseDispatch.dispatch URL.Protocol " + aURL.Protocol + " URL.Path " + aURL.Path );
        if ( aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
        {
            if ( aURL.Path == "Command1" )
            {
                // open the OpenOffice.org web page
                String sURL = "http://www.mono-project.com";
                XSystemShellExecute xSystemShellExecute = ( XSystemShellExecute ) mxMSF.createInstance( "com.sun.star.system.SystemShellExecute" );
                if ( xSystemShellExecute != null )
                {        
                    try
                    {
                        xSystemShellExecute.execute( sURL, "", SystemShellExecuteFlags.DEFAULTS );
                    }    
                    catch( unoidl.com.sun.star.uno.Exception )
                    {
                    } 
                }
            }
            else if ( aURL.Path == "Command2" )
            {
                // remove the text if it's in our list
                NamedValue[] aRemoveArgs = new NamedValue[ 1 ];
                aRemoveArgs[0] = new NamedValue();
                aRemoveArgs[0].Name  = "Text";
                aRemoveArgs[0].Value = new uno.Any( maComboBoxText.GetType(), maComboBoxText );
                SendCommand( aURL, "RemoveEntryText", aRemoveArgs, true );
                  
                // add the new text to the start of the list
                NamedValue[] aInsertArgs = new NamedValue[ 2 ];
                aInsertArgs[0] = new NamedValue();
                aInsertArgs[0].Name = "Pos";
                aInsertArgs[0].Value = new uno.Any( typeof(int), 0 );
                aInsertArgs[1] = new NamedValue();
                aInsertArgs[1].Name = "Text";
                aInsertArgs[1].Value = new uno.Any( maComboBoxText.GetType(), maComboBoxText );
                SendCommand( aURL, "InsertEntry", aInsertArgs, true );
            }
            else if ( aURL.Path == "Command3" )
            {
                // Retrieve the text argument from the sequence property value
                String aText = "";
                for ( int i = 0; i < lArgs.Length; i++ )
                {
                    if ( String.Compare( lArgs[i].Name,0, "Text",0, 4 ) == 0 )
                    {
                        aText = (String) lArgs[i].Value.Value;
                        break;
                    }
                }
                if ( aText == null ) 
                    aText = "null";
                // create new URL to address the combox box
                URL aCmdURL = new URL();
                aCmdURL.Path = "Command2";
                aCmdURL.Protocol = "vnd.demo.complextoolbarcontrols.demoaddon:";
                aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;
                    
                // set the selected item as text into the combobox
                NamedValue[] aArgs = new NamedValue[ 1 ];
                aArgs[0] = new NamedValue();
                aArgs[0].Name ="Text";
                aArgs[0].Value = new uno.Any( aText.GetType(), aText );
                SendCommand( aCmdURL, "SetText", aArgs, true );
            }
            else if ( aURL.Path == "Command4" )
            {
                // Retrieve the text argument from the sequence property value
                String aText = null;
                for ( int i = 0; i < lArgs.Length; i++ )
                {
                    if ( String.Compare( lArgs[i].Name,0, "Text",0, 4 ) == 0 )
                    {
                        aText = (String) lArgs[i].Value.Value;
                        break;
                    }
                }
                
                // just enable this command
                
                // set enable flag according to selection
                if ( aText == "Button Disabled" )
                    mbButtonEnabled = false;
                else
                    mbButtonEnabled = true;

                // create new URL to address the image button
                URL aCmdURL = new URL();
                aCmdURL.Path = "Command1";
                aCmdURL.Protocol = "vnd.demo.complextoolbarcontrols.demoaddon:";
                aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;
                    
                // create and initialize FeatureStateEvent with IsEnabled
                FeatureStateEvent aEvent = new FeatureStateEvent();
                aEvent.FeatureURL = aCmdURL;
                aEvent.Source = this;
                aEvent.IsEnabled = mbButtonEnabled;
                aEvent.Requery = false;
                aEvent.State = new uno.Any();
            
                // Notify listener about new state
                XDispatch xDispatch = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
                aListenerHelper.Notify( mxFrame, aEvent.FeatureURL.Path, aEvent ); 
            }
            else if ( aURL.Path == "Command5" )
            {
            }
        }
    }
    public void addStatusListener( XStatusListener xControl,
        URL aURL )
    {
         Console.WriteLine( "***  BaseDispatch.addStatusListener aURL.Path " +  aURL.Path);
        if ( aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
        {
            if ( aURL.Path == "Command1" )
            {
                // just enable this command
                FeatureStateEvent aEvent = new FeatureStateEvent();
                aEvent.FeatureURL = aURL;
                aEvent.Source = this;
                aEvent.IsEnabled = mbButtonEnabled;
                aEvent.Requery = false;
                aEvent.State = new uno.Any();
                xControl.statusChanged( aEvent );
            }
            else if ( aURL.Path == "Command2" )
            {
                // just enable this command
                FeatureStateEvent aEvent = new FeatureStateEvent();
                aEvent.FeatureURL = aURL;
                aEvent.Source = this;
                aEvent.IsEnabled = true;
                aEvent.Requery = false;
                aEvent.State = new uno.Any();
                xControl.statusChanged( aEvent );
            }
            else if ( aURL.Path == "Command3" )
            {
                // A toggle dropdown box is normally used for a group of commands
                // where the user can select the last issued command easily.
                // E.g. a typical command group would be "Insert shape"
                NamedValue[] aArgs = new NamedValue[ 1 ];
                
                // send command to set context menu content
                String[] aContextMenu = new String[ 3 ];
                aContextMenu[0] = "Command 1";
                aContextMenu[1] = "Command 2";
                aContextMenu[2] = "Command 3";
                
                aArgs[0] = new NamedValue();
                aArgs[0].Name = "List";
                aArgs[0].Value = new uno.Any( aContextMenu.GetType(), aContextMenu );
                SendCommandTo( xControl, aURL, "SetList", aArgs, true );
                
                // send command to check item on pos=0
                aArgs[0].Name = "Pos";
                aArgs[0].Value = new uno.Any( typeof(int), 0 );            
                SendCommandTo( xControl, aURL, "CheckItemPos", aArgs, true );
            }
            else if ( aURL.Path == "Command4" )
            {
                // A dropdown box is normally used for a group of dependent modes, where
                // the user can only select one. The modes cannot be combined.
                // E.g. a typical group would be left,right,center,block.
                NamedValue[] aArgs = new NamedValue[ 1 ];
                
                // send command to set context menu content
                String[] aContextMenu = new String[ 2 ];
                aContextMenu[0] = "Button Enabled";
                aContextMenu[1] = "Button Disabled";
                
                aArgs[0] = new NamedValue();
                aArgs[0].Name = "List";
                aArgs[0].Value = new uno.Any( aContextMenu.GetType(), aContextMenu );
                SendCommandTo( xControl, aURL, "SetList", aArgs, true );            
    
                // set position according to enable/disable state of button
                int nPos =  ( mbButtonEnabled ? 0 : 1 );
                
                // send command to check item on pos=0
                aArgs[0].Name = "Pos";
                aArgs[0].Value = new uno.Any( typeof(int), nPos );
                SendCommandTo( xControl, aURL, "CheckItemPos", aArgs, true );
            }
            else if ( aURL.Path == "Command5" )
            {
                // A spin button
                NamedValue[] aArgs = new NamedValue[ 5 ];
                
                // send command to initialize spin button
                aArgs[0] = new NamedValue();
                aArgs[0].Name = "Value";
                aArgs[0].Value = new uno.Any( typeof( double ),  0.0 );
                aArgs[1] = new NamedValue();
                aArgs[1].Name = "UpperLimit";
                aArgs[1].Value = new uno.Any( typeof( double ),  10.0 );
                aArgs[2] = new NamedValue();
                aArgs[2].Name = "LowerLimit";
                aArgs[2].Value =  new uno.Any( typeof( double ),  0.0 );
                aArgs[3] = new NamedValue();
                aArgs[3].Name = "Step";
                aArgs[3].Value = new uno.Any( typeof( double ), 0.1 );
                aArgs[4] = new NamedValue();
                aArgs[4].Name = "OutputFormat";
                aArgs[4].Value =  new uno.Any( typeof( String ), "%.2f cm" );
    
                SendCommandTo( xControl, aURL, "SetValues", aArgs, true );
            }
            else if ( aURL.Path == "Command7" )
            {
                // A dropdown box is normally used for a group of commands
                // where the user can select one of a defined set.
                NamedValue[] aArgs = new NamedValue[ 1 ];
                
                // send command to set context menu content
                String[] aList = new String[ 10 ];
                aList[0] = "White";
                aList[1] = "Black";
                aList[2] = "Red";
                aList[3] = "Blue";
                aList[4] = "Green";
                aList[5] = "Grey";
                aList[6] = "Yellow";
                aList[7] = "Orange";
                aList[8] = "Brown";
                aList[9] = "Pink";
                
                aArgs[0] = new NamedValue();
                aArgs[0].Name = "List";
                aArgs[0].Value = new uno.Any( aList.GetType(), aList );
                SendCommandTo( xControl, aURL, "SetList", aArgs, true );
            }
            aListenerHelper.AddListener( mxFrame, xControl, aURL.Path );
        }
    
    }
    public void removeStatusListener( XStatusListener xControl,
        URL aURL )
    {
        aListenerHelper.RemoveListener( mxFrame, xControl, aURL.Path );
    }
    // XControlNotificationListener
    public void controlEvent( ControlEvent Event ) 
    {
        if ( Event.aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
        {
            if ( Event.aURL.Path == "Command2" )
            {
                // We get notifications whenever the text inside the combobox has been changed.
                // We store the new text into a member.
                if ( Event.Event == "TextChanged" )
                {
                    String aNewText = "";
                    bool  bHasText = false;
                    for ( int i = 0; i < Event.aInformation.Length; i++ )
                    {
                        if ( String.Compare( Event.aInformation[i].Name, 0,  "Text", 0, 4 ) == 0 )
                        {
                            try
                            {
                                aNewText = ( String ) Event.aInformation[i].Value.Value;
                                bHasText = (aNewText.Length > 0 );
                            }
                            catch( unoidl.com.sun.star.uno.Exception ) {}
                            catch( System.Exception ) {}
 
                            break;
                        }
                    }
                    
                    if ( bHasText )
                        maComboBoxText = aNewText;
                }
            }
        }
    }
}

public class WriterDispatch : BaseDispatch
{
    public WriterDispatch( XMultiServiceFactory rxMSF,
        XFrame xFrame ) : base( rxMSF, xFrame, "com.sun.star.text.TextDocument" )
    {
        Console.WriteLine( "   WriterDispatch - ctor ");
    }
}

public class CalcDispatch : BaseDispatch
{
    public CalcDispatch( XMultiServiceFactory rxMSF, XFrame xFrame ) : base( rxMSF, xFrame, "com.sun.star.sheet.SpreadSheetDocument" )
    { 
        Console.WriteLine( "   CalcDispatch - ctor ");
    }
}

}
namespace component {
static public class RegistrationClass
{
    static public String name = "ooo.mono.comp.MyProtocolHandler";
}
}
