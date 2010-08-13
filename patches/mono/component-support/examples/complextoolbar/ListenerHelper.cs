using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;
using unoidl.com.sun.star.frame;
using unoidl.com.sun.star.util;
using unoidl.com.sun.star.awt;
using unoidl.com.sun.star.beans;
using System.Collections;

namespace ooo.mono.comp {

public class ListenerItem
{
    // String -> ArrayList ( where ArrayList is a list of XStatusListener )
    public Hashtable aContainer = Hashtable.Synchronized(new Hashtable());
    public XDispatch xDispatch;
    public XFrame xFrame;
}

public class ListenerHelper
{
    static public ArrayList aListeners = new ArrayList(); // list of ListenerItem

    public void AddListener(
        XFrame xFrame,
        XStatusListener xControl,
        String aCommand )
    {
        ListenerItem item = null;
        for (int i = 0; i < aListeners.Count; i++)
        {
            item = aListeners[i] as ListenerItem;
            if ( item.xFrame == xFrame )
                break;
        }

        ArrayList tmp = (ArrayList)item.aContainer[ aCommand ];
        if ( tmp == null )
        {
            tmp = new ArrayList();
            item.aContainer.Add( aCommand, tmp );
        }
        tmp.Add( xControl );
    }
    public void RemoveListener(
        XFrame xFrame,
        XStatusListener xControl,
        String aCommand )
    {
        ListenerItem item = null;
        for (int i = 0; i < aListeners.Count; i++)
        {
            item = aListeners[i] as ListenerItem;
            if ( item.xFrame == xFrame )
            {
                ArrayList aL = item.aContainer[ aCommand ] as ArrayList;
                foreach( XStatusListener aIter in aL )
                {
                    if ( aIter == xControl )
                    {
                        aL.Remove( aIter );
                        break;
                    }
                } 
            }
        }
    }
    public void Notify(
        XFrame xFrame,
        String aCommand,
        FeatureStateEvent rEvent )
    {
        ListenerItem item = null;
        for (int i = 0; i < aListeners.Count; i++)
        {
            item = aListeners[i] as ListenerItem;
            if ( item.xFrame == xFrame )
            {
                ArrayList aL = item.aContainer[ aCommand ] as ArrayList;
                foreach( XStatusListener aIter in aL )
                    aIter.statusChanged( rEvent );
            }
        }
    }

    public XDispatch GetDispatch(
        XFrame xFrame,
        String aCommand )
    {
        XDispatch xRet = null;
        ListenerItem item = null;
        for (int i = 0; i < aListeners.Count; i++)
        {
            item = aListeners[i] as ListenerItem;
            if ( item.xFrame == xFrame )
            {
                xRet = item.xDispatch;
            }
        }
        return xRet;
    }

    public void AddDispatch(
        XDispatch xDispatch,
        XFrame xFrame,
        String aCommand )
    {
        ListenerItem aItem = new ListenerItem(); 
        aItem.xFrame = xFrame;
        aItem.xDispatch = xDispatch;
        aListeners.Add( aItem );			
        xFrame.addEventListener( new ListenerItemEventListener( xFrame ) );
    }
}

public class ListenerItemEventListener : uno.util.WeakBase, XEventListener
{
    private XFrame mxFrame;
    public ListenerItemEventListener( XFrame xFrame)
    {
        mxFrame = xFrame;
    }
    public void disposing( EventObject aEvent )
    {
        ListenerItem item = null;
        for (int i = 0; i < ListenerHelper.aListeners.Count; i++)
        {
            item = ListenerHelper.aListeners[i] as ListenerItem;
            if ( item.xFrame == mxFrame )
            {
                ListenerHelper.aListeners.Remove( item );
            }
        }
    }
}

}
