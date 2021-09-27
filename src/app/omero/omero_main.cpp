/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "omero_app.h"
#include "omero_main.h"
#include "config/omero_config.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"
#include "hardware/motor.h"

#include "hardware/wifictl.h"

#include "utils/json_psram_allocator.h"
#include "utils/alloc.h"


lv_obj_t *omero_main_tile = NULL;
lv_style_t omero_main_style;
lv_style_t omero_id_style;

lv_task_t * _omero_main_task;

lv_obj_t *id_omero_cont = NULL;
lv_obj_t *id_omero_label = NULL;
lv_obj_t *degree_cont = NULL;
lv_obj_t *degree_label = NULL;
lv_obj_t *inpath_cont = NULL;
lv_obj_t *inpath_label = NULL;
lv_obj_t *comando_cont = NULL;
lv_obj_t *comando_label = NULL;
lv_obj_t *distance_cont = NULL;
lv_obj_t *distance_label_omero = NULL;
lv_obj_t *amplitude_cont = NULL;
lv_obj_t *amplitude_label = NULL;
lv_obj_t *duration_cont = NULL;
lv_obj_t *duration_label = NULL;
lv_obj_t *lastmsg_cont = NULL;
lv_obj_t *lastmsg_label = NULL;

WiFiClient espClientOmero;
PubSubClient omero_mqtt_client( espClientOmero );

LV_IMG_DECLARE(refresh_32px);
LV_FONT_DECLARE(Ubuntu_16px);
LV_FONT_DECLARE(Ubuntu_48px);

bool omero_wifictl_event_cb( EventBits_t event, void *arg );
static void enter_omero_setup_event_cb( lv_obj_t * obj, lv_event_t event );
void omero_main_task( lv_task_t * task );

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    time_t now;
    struct tm info;
    char buf[64];
    String buf2 = String(millis());

    char *msg = NULL;
    msg = (char*)CALLOC( length + 1, 1 );
    int amplitude = 0;
    int duration = 0;
    
    if ( msg == NULL ) {
        log_e("calloc failed");
        return;
    }
    memcpy( msg, payload, length );

   
    SpiRamJsonDocument doc( strlen( msg ) * 2 );

    DeserializationError error = deserializeJson( doc, msg );

    //log_e("omero message: %s", error.c_str() );

    if ( error ) {
        log_e("omero message deserializeJson() failed: %s", error.c_str());
    }
    else  {
        if ( doc["idTag"] ) {
            String temp =  doc["idTag"];
            //lv_label_set_text( id_omero_label, doc["idTag"]);
            //log_e("idTag: %s", temp);
        }
        if ( doc["degreeDifferenceFromTarget"] ) {
            char temp[16] = "";
            float degree = doc["degreeDifferenceFromTarget"];
            //log_e("degree: %20.15f", degree);
            snprintf( temp, sizeof( temp ), " %20.15f ", degree);
            lv_label_set_text( degree_label, temp );
        }
        if ( doc["inPath"] != "X") {
            //log_e("inPath: %s", "Ricevuto");
            bool flag = doc["inPath"];
            if (flag){
               lv_label_set_text( inpath_label, "SI");
               //log_e("inPath: %s", "SI");
            }else{
               lv_label_set_text( inpath_label, "NO");
               //log_e("inPath: %s", "NO");
            }
        }
        if ( doc["comando"] ) {
            String tcom = doc["comando"];
            lv_label_set_text( comando_label, doc["comando"]);
            //log_e("Comando: %s", tcom.c_str() );
        }
        if ( doc["distanceToIdealLine"] ) {
            char temp[16] = "";
            float dist = doc["distanceToIdealLine"];
            //log_e("distance: %f", dist);
            snprintf( temp, sizeof( temp ), " %5.1f ", dist);
            lv_label_set_text( distance_label_omero, temp );
        }
        if ( doc["vibrationAmplitude"] ) {
            char temp[16] = "";
            amplitude = doc["vibrationAmplitude"];
            snprintf( temp, sizeof( temp ), "%u", amplitude);
            lv_label_set_text( amplitude_label, temp );
            //log_e("amplitude: %d", amplitude);
        }
        if ( doc["vibrationTime"] ) {
            char temp[16] = "";
            duration = doc["vibrationTime"];
            snprintf( temp, sizeof( temp ), "%ums", duration);
            lv_label_set_text( duration_label, temp );
            //log_e("time: %d", duration);           
        }
        if (duration != 0){
            time( &now );
            localtime_r( &now, &info );
            strftime( buf, sizeof(buf), "%d/%m/%y %H:%M:%S", &info );
            log_e("Timestamp %s - %s", buf, buf2);
            String timestamp = String(buf) + "-" + buf2;
            lv_label_set_text( lastmsg_label, timestamp.c_str());
            motor_vibe(duration, true); 
            //log_e("vibroooooooooo");
        }

        lv_obj_align( id_omero_label, id_omero_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( degree_label, degree_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( inpath_label, inpath_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( comando_label, comando_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( distance_label_omero, distance_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( amplitude_label, amplitude_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( duration_label, duration_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
        lv_obj_align( lastmsg_label, lastmsg_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    }
    doc.clear();
    free( msg );
}

void omero_main_tile_setup( uint32_t tile_num ) {

    omero_main_tile = mainbar_get_tile_obj( tile_num );

    lv_style_copy( &omero_id_style, ws_get_app_opa_style() );
    lv_style_set_text_font( &omero_id_style, LV_STATE_DEFAULT, &Ubuntu_48px);
    lv_obj_add_style( omero_main_tile, LV_OBJ_PART_MAIN, &omero_id_style );

    lv_style_copy( &omero_id_style, ws_get_app_opa_style() );
    lv_style_set_text_font( &omero_id_style, LV_STATE_DEFAULT, &Ubuntu_16px);

    lv_obj_t * exit_btn = wf_add_exit_button( omero_main_tile, &omero_id_style );
    lv_obj_align(exit_btn, omero_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );

    lv_obj_t * setup_btn = wf_add_setup_button( omero_main_tile, enter_omero_setup_event_cb, &omero_id_style );
    lv_obj_align(setup_btn, omero_main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10 );

    //ID 
    id_omero_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( id_omero_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( id_omero_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( id_omero_cont, omero_main_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
    lv_obj_t * id_info_label = lv_label_create( id_omero_cont, NULL );
    lv_obj_add_style( id_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( id_info_label, "ID:" );
    lv_obj_align( id_info_label, id_omero_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    id_omero_label = lv_label_create( id_omero_cont, NULL );
    lv_obj_add_style( id_omero_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( id_omero_label, "n/a" );
    lv_obj_align( id_omero_label, id_omero_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //lastMSG
    lastmsg_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( lastmsg_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( lastmsg_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( lastmsg_cont, id_omero_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * lastmsg_info_label = lv_label_create( lastmsg_cont, NULL );
    lv_obj_add_style( lastmsg_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( lastmsg_info_label, "TS:" );
    lv_obj_align( lastmsg_info_label, lastmsg_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    lastmsg_label = lv_label_create( lastmsg_cont, NULL );
    lv_obj_add_style( lastmsg_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( lastmsg_label, "n/a" );
    lv_obj_align( lastmsg_label, lastmsg_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //Degree
    degree_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( degree_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( degree_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( degree_cont, lastmsg_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * degree_info_label = lv_label_create( degree_cont, NULL );
    lv_obj_add_style( degree_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( degree_info_label, "Angolo:" );
    lv_obj_align( degree_info_label, degree_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    degree_label = lv_label_create( degree_cont, NULL );
    lv_obj_add_style( degree_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( degree_label, "n/a" );
    lv_obj_align( degree_label, degree_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //inPath
    inpath_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( inpath_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( inpath_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( inpath_cont, degree_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * inpath_info_label = lv_label_create( inpath_cont, NULL );
    lv_obj_add_style( inpath_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( inpath_info_label, "InPath:" );
    lv_obj_align( inpath_info_label, inpath_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    inpath_label = lv_label_create( inpath_cont, NULL );
    lv_obj_add_style( inpath_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( inpath_label, "n/a" );
    lv_obj_align( inpath_label, inpath_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //Comando
    comando_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( comando_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( comando_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( comando_cont, inpath_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * comando_info_label = lv_label_create( comando_cont, NULL );
    lv_obj_add_style( comando_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( comando_info_label, "Cmnd:" );
    lv_obj_align( comando_info_label, comando_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    comando_label = lv_label_create( comando_cont, NULL );
    lv_obj_add_style( comando_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( comando_label, "n/a" );
    lv_obj_align( comando_label, comando_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //Distance
    distance_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( distance_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( distance_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( distance_cont, comando_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * distance_info_label = lv_label_create( distance_cont, NULL );
    lv_obj_add_style( distance_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( distance_info_label, "Dist:" );
    lv_obj_align( distance_info_label, distance_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    distance_label_omero = lv_label_create( distance_cont, NULL );
    lv_obj_add_style( distance_label_omero, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( distance_label_omero, "n/a" );
    lv_obj_align( distance_label_omero, distance_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //Amplitude
    amplitude_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( amplitude_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( amplitude_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( amplitude_cont, distance_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * amplitude_info_label = lv_label_create( amplitude_cont, NULL );
    lv_obj_add_style( amplitude_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( amplitude_info_label, "Potenza:" );
    lv_obj_align( amplitude_info_label, amplitude_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    amplitude_label = lv_label_create( amplitude_cont, NULL );
    lv_obj_add_style( amplitude_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( amplitude_label, "n/a" );
    lv_obj_align( amplitude_label, amplitude_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    //Duration
    duration_cont = lv_obj_create( omero_main_tile, NULL );
    lv_obj_set_size( duration_cont, lv_disp_get_hor_res( NULL ), 25 );
    lv_obj_add_style( duration_cont, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_obj_align( duration_cont, amplitude_cont , LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * duration_info_label = lv_label_create( duration_cont, NULL );
    lv_obj_add_style( duration_info_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( duration_info_label, "Durata:" );
    lv_obj_align( duration_info_label, duration_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    duration_label = lv_label_create( duration_cont, NULL );
    lv_obj_add_style( duration_label, LV_OBJ_PART_MAIN, &omero_id_style );
    lv_label_set_text( duration_label, "n/a" );
    lv_obj_align( duration_label, duration_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    

    omero_mqtt_client.setCallback( mqtt_callback );
    omero_mqtt_client.setBufferSize( 512 );

    wifictl_register_cb( WIFICTL_CONNECT_IP | WIFICTL_OFF_REQUEST | WIFICTL_OFF | WIFICTL_DISCONNECT , omero_wifictl_event_cb, "omero" );
    // create an task that runs every secound
    _omero_main_task = lv_task_create( omero_main_task, 50, LV_TASK_PRIO_HIGHEST, NULL );
}

bool omero_wifictl_event_cb( EventBits_t event, void *arg ) {
    omero_config_t *omero_config = omero_get_config();
    switch( event ) {
        case WIFICTL_CONNECT_IP:    if ( omero_config->autoconnect ) {
                                        omero_mqtt_client.setServer( omero_config->server, omero_config->port );
                                        if ( !omero_mqtt_client.connect( omero_config->idTag, omero_config->user, omero_config->password ) ) {
                                            log_e("connect to mqtt server %s failed", omero_config->server );
                                            app_set_indicator( omero_get_app_icon(), ICON_INDICATOR_FAIL );
                                            widget_set_indicator( omero_get_widget_icon() , ICON_INDICATOR_FAIL );
                                        }
                                        else {
                                            log_i("connect to mqtt server %s success", omero_config->server );
                                            omero_mqtt_client.subscribe( omero_config->topic );
                                            app_set_indicator( omero_get_app_icon(), ICON_INDICATOR_OK );
                                            widget_set_indicator( omero_get_widget_icon(), ICON_INDICATOR_OK );
                                        }
                                    } 
                                    break;
        case WIFICTL_OFF_REQUEST:
        case WIFICTL_OFF:
        case WIFICTL_DISCONNECT:    if ( omero_mqtt_client.connected() ) {
                                        log_i("disconnect from mqtt server %s", omero_config->server );
                                        omero_mqtt_client.disconnect();
                                        app_hide_indicator( omero_get_app_icon() );
                                        widget_hide_indicator( omero_get_widget_icon() );
                                        widget_set_label( omero_get_widget_icon(), "n/a" );
                                    }
                                    break;
    }
    return( true );
}

static void enter_omero_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( omero_get_app_setup_tile_num(), LV_ANIM_ON );
                                        statusbar_hide( true );
                                        break;
    }
}

void omero_main_task( lv_task_t * task ) {
    // put your code here
    omero_mqtt_client.loop();
}