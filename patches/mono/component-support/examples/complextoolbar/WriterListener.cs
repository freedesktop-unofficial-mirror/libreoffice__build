using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.frame;
using unoidl.com.sun.star.util;
using unoidl.com.sun.star.awt;
using unoidl.com.sun.star.beans;
using unoidl.com.sun.star.system;
using unoidl.com.sun.star.document;

class WriterListener : uno.util.WeakBase, unoidl.com.sun.star.document.XEventListener
{
    private XMultiServiceFactory mxMSF;
    public WriterListener( XMultiServiceFactory rxMSF ) 
    { 
        mxMSF = rxMSF;
    }

    // document.XEventListener
    public void notifyEvent( unoidl.com.sun.star.document.EventObject aEvent)
    {
    }
    public void disposing( unoidl.com.sun.star.lang.EventObject aEvent)
    {
    }
}
