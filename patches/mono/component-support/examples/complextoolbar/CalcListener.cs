using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;
using unoidl.com.sun.star.frame;
using unoidl.com.sun.star.util;
using unoidl.com.sun.star.awt;
using unoidl.com.sun.star.beans;
using unoidl.com.sun.star.system;
using unoidl.com.sun.star.document;

class CalcListener : uno.util.WeakBase, unoidl.com.sun.star.document.XEventListener
{
    private XMultiServiceFactory mxMSF;
    public CalcListener( XMultiServiceFactory rxMSF ) 
    { 
        mxMSF = rxMSF;
    }

    // document.XEventListener
    public void notifyEvent(unoidl.com.sun.star.document. EventObject aEvent)
    {
    }
    public void disposing( unoidl.com.sun.star.lang.EventObject aEvent)
    {
    }
}
