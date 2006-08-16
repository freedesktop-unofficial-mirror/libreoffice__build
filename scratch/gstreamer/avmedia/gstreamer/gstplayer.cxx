/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

#include <math.h>

#ifndef __RTL_USTRING_
#include <rtl/string.hxx>
#endif

#include "gstplayer.hxx"
#include "gstframegrabber.hxx"
#include "gstwindow.hxx"

#include <gst/interfaces/xoverlay.h>

#define AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer"
#define AVMEDIA_GST_PLAYER_SERVICENAME "com.sun.star.media.Player_GStreamer"

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {

// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr ),
    mpPlaybin( NULL ),
    mnUnmutedVolume( 0 ),
    mbMuted( false ),
    mbLooping( false ),
    mbInitialized( false ),
    mnWindowID (0),
    mnDuration (0)
{
    // Initialize GStreamer library
    int argc = 1;
    char *arguments[] = { "openoffice.org" };
    char** argv = arguments;
    GError* pError = NULL;

    mbInitialized = gst_init_check( &argc, &argv, &pError );

    if (pError != NULL)
        // TODO: thow an exception?
        g_error_free (pError);
}

// ------------------------------------------------------------------------------

Player::~Player()
{
    // Release the elements and pipeline
    if( mbInitialized )
    {
        if( mpPlaybin )
        {
            gst_element_set_state( mpPlaybin, GST_STATE_NULL );
            gst_object_unref( GST_OBJECT( mpPlaybin ) );

            mpPlaybin = NULL;
        }
    }
}

// ------------------------------------------------------------------------------

static gboolean gst_pipeline_bus_callback( GstBus *, GstMessage *message, gpointer data )
{
    Player* pPlayer = (Player *) data;

    pPlayer->processMessage( message );

    return TRUE;
}

static GstBusSyncReply gst_pipeline_bus_sync_handler( GstBus *, GstMessage * message, gpointer data )
{
    Player* pPlayer = (Player *) data;

    return pPlayer->processSyncMessage( message );
}

void Player::processMessage( GstMessage *message )
{
    //OSL_TRACE ( "gst message received: src name: %s structure type: %s",
    //            gst_object_get_name (message->src),
    //            message->structure ? gst_structure_get_name (message->structure) : "<none>");

    switch( GST_MESSAGE_TYPE( message ) ) {
    case GST_MESSAGE_EOS:
        //OSL_TRACE( "EOS, reset state to NULL" );
        gst_element_set_state( mpPlaybin, GST_STATE_READY );
        break;
    case GST_MESSAGE_STATE_CHANGED:
        if( message->src == GST_OBJECT( mpPlaybin ) ) {
            GstState newstate, pendingstate;

            gst_message_parse_state_changed (message, NULL, &newstate, &pendingstate);

            //OSL_TRACE( "state change received, new state %d", newstate );
            if( newstate == GST_STATE_PAUSED &&
                pendingstate == GST_STATE_VOID_PENDING ) {
                
                //OSL_TRACE( "change to paused received" );

                if( mnDuration == 0) {
                    GstFormat format = GST_FORMAT_TIME;
                    gint64 gst_duration = 0L;

                    if( gst_element_query_duration( mpPlaybin, &format, &gst_duration) && format == GST_FORMAT_TIME && gst_duration > 0L )
                        mnDuration = gst_duration;
                }
            }
        }
        break;
    default:
        break;
    }
}

GstBusSyncReply Player::processSyncMessage( GstMessage *message )
{
    if (message->structure) {
        if( !strcmp( gst_structure_get_name( message->structure ), "prepare-xwindow-id" ) && mnWindowID != 0 ) {
            gst_x_overlay_set_xwindow_id( GST_X_OVERLAY( GST_MESSAGE_SRC( message ) ), mnWindowID );
            return GST_BUS_DROP;
        }
    }

    return GST_BUS_PASS;
}

bool Player::create( const ::rtl::OUString& rURL )
{
    bool    bRet = false;

    // create all the elements and link them     
    
    if( mbInitialized )
    {
        GstBus *pBus;

        mpPlaybin = gst_element_factory_make( "playbin", "player" );
        rtl::OString ascURL = OUStringToOString( rURL, RTL_TEXTENCODING_ASCII_US );
        g_object_set( G_OBJECT( mpPlaybin ), "uri", ascURL.getStr() , NULL );

        pBus = gst_element_get_bus( mpPlaybin );
        gst_bus_add_watch( pBus, gst_pipeline_bus_callback, this );
        gst_bus_set_sync_handler( pBus, gst_pipeline_bus_sync_handler, this );
        g_object_unref( pBus );

        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );

        bRet = true;
    }


    if( bRet )
        maURL = rURL;
    else
        maURL = ::rtl::OUString();

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start(  )
    throw (uno::RuntimeException)
{
    //OSL_TRACE ("Player::start");

    // set the pipeline state to READY and run the loop
    if( mbInitialized && NULL != mpPlaybin )
    {
        gst_element_set_state( mpPlaybin, GST_STATE_PLAYING );
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
    // set the pipeline in PAUSED STATE
    if( mpPlaybin )
        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    bool            bRet = false;

    // return whether the pipeline is in PLAYING STATE or not
    if( mbInitialized && mpPlaybin )
    {
        bRet = GST_STATE_PLAYING == GST_STATE( mpPlaybin );
    }

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration(  )
    throw (uno::RuntimeException)
{
    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.01;

    if( mpPlaybin && mnDuration > 0 ) {
        duration = mnDuration / 1E9;

        //OSL_TRACE( "gst duration: %lld ns duration: %lf s", gst_duration, duration );
    }

    return duration;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    if( mpPlaybin ) {
        gint64 gst_position = llround (fTime * 1E9);

        gst_element_seek( mpPlaybin, 1.0,
                          GST_FORMAT_TIME,
                          GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET, gst_position,
                          GST_SEEK_TYPE_NONE, 0 );


        //OSL_TRACE( "seek to: %lld ns original: %lf s", gst_position, fTime );
    }
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime(  )
    throw (uno::RuntimeException)
{
    double position = 0.0;

    if( mpPlaybin ) {
        // get current position in the stream
        GstFormat format = GST_FORMAT_TIME;
        gint64 gst_position;
        if( gst_element_query_position( mpPlaybin, &format, &gst_position ) && format == GST_FORMAT_TIME && gst_position > 0L )
            position = gst_position / 1E9;
    }
    
    return position; 
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
    throw (uno::RuntimeException)
{
    // TODO implement
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getStopTime(  )
    throw (uno::RuntimeException)
{
    // Get the time at which to stop

    return 0; 
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate )
    throw (uno::RuntimeException)
{
    // TODO set the window rate
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
    double rate = 0.0;

    // TODO get the window rate
    if( mbInitialized )
    {
        
    }
    
    return rate;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    // TODO check how to do with GST
    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
    // TODO check how to do with GST
    return mbLooping;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    OSL_TRACE( "set mute: %d muted: %d unmuted volume: %lf", bSet, mbMuted, mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if(  mpPlaybin && mbMuted != bSet )
    {
        double nVolume = mnUnmutedVolume;
        if( bSet )
        {
            nVolume = 0.0;
        }

        g_object_set( G_OBJECT( mpPlaybin ), "volume", nVolume, NULL );

        mbMuted = bSet;
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) 
    throw (uno::RuntimeException)
{
    mnUnmutedVolume = pow10 ( nVolumeDB / 20.0 );

    OSL_TRACE( "set volume: %d gst volume: %lf", nVolumeDB, mnUnmutedVolume );

    // change volume
     if( !mbMuted && mpPlaybin )
     {
         g_object_set( G_OBJECT( mpPlaybin ), "volume", (gdouble) mnUnmutedVolume, NULL );
     }
}

// ------------------------------------------------------------------------------
    
sal_Int16 SAL_CALL Player::getVolumeDB(  ) 
    throw (uno::RuntimeException)
{
    sal_Int16 nVolumeDB;

    if( mpPlaybin ) {
        double nGstVolume = 0.0;

        g_object_get( G_OBJECT( mpPlaybin ), "volume", &nGstVolume, NULL );

        nVolumeDB = (sal_Int16) ( 20.0*log10 ( nGstVolume ) );

        //OSL_TRACE( "get volume: %d gst volume: %lf", nVolumeDB, nGstVolume );
    }

    return nVolumeDB;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
    awt::Size aSize( 0, 0 );
    
    long nWidth = 320, nHeight = 240;

    OSL_TRACE( "Player::getPreferredPlayerWindowSize" );

    // TODO fill nWidth and nHeight with the current player size

    aSize.Width = nWidth;
    aSize.Height = nHeight;

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
    throw (uno::RuntimeException)
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    OSL_TRACE( "Player::createPlayerWindow %d %d", aSize.Width, aSize.Height );

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::gstreamer::Window* pWindow = new ::avmedia::gstreamer::Window( mxMgr, *this );

        xRet = pWindow;

        if( rArguments.getLength() > 2 ) {
            rArguments[ 2 ] >>= mnWindowID;
            OSL_TRACE( "window ID: %ld", mnWindowID );
        }

        //if( !pWindow->create( aArguments ) )
        //xRet = uno::Reference< ::media::XPlayerWindow >();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
    throw (uno::RuntimeException)
{
    uno::Reference< media::XFrameGrabber > xRet;

    /*if( maURL.getLength() > 0 )
    {
        FrameGrabber* pGrabber = new FrameGrabber( mxMgr );
        
        xRet = pGrabber;
        
        if( !pGrabber->create( maURL ) )
            xRet.clear();
            }*/
    
    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_GST_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_GST_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia
