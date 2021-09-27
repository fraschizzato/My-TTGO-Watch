/****************************************************************************
 *   Tu May 22 21:23:51 2020
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
#include <config.h>

#include "omero_app.h"
#include "omero_setup.h"
#include "config/omero_config.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/statusbar.h"
#include "gui/keyboard.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

lv_obj_t *omero_setup_tile = NULL;
lv_obj_t *omero_setup_tile_2 = NULL;
lv_style_t omero_setup_style;
uint32_t omero_setup_tile_num;

lv_obj_t *omero_server_textfield = NULL;
lv_obj_t *omero_user_textfield = NULL;
lv_obj_t *omero_password_textfield = NULL;
lv_obj_t *omero_topic_textfield = NULL;
lv_obj_t *omero_idTag_textfield = NULL;
lv_obj_t *omero_port_textfield = NULL;
lv_obj_t *omero_autoconnect_onoff = NULL;
lv_obj_t *omero_widget_onoff = NULL;

static void omero_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void omero_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void omero_autoconnect_onoff_event_handler( lv_obj_t * obj, lv_event_t event );
static void omero_widget_onoff_event_handler( lv_obj_t *obj, lv_event_t event );
static void omero_setup_hibernate_callback ( void );

void omero_setup_tile_setup( uint32_t tile_num ) {

    omero_config_t *omero_config = omero_get_config();

    mainbar_add_tile_hibernate_cb( tile_num, omero_setup_hibernate_callback );

    omero_setup_tile_num = tile_num;
    omero_setup_tile = mainbar_get_tile_obj( omero_setup_tile_num );
    omero_setup_tile_2 = mainbar_get_tile_obj( omero_setup_tile_num + 1 );

    lv_style_copy( &omero_setup_style, ws_get_setup_tile_style() );
    lv_obj_add_style( omero_setup_tile, LV_OBJ_PART_MAIN, &omero_setup_style );
    lv_obj_add_style( omero_setup_tile_2, LV_OBJ_PART_MAIN, &omero_setup_style );

    lv_obj_t *header = wf_add_settings_header( omero_setup_tile, "omero setup" );
    //lv_obj_align( header, omero_setup_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );

    lv_obj_t *omero_server_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_server_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_server_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_server_cont, omero_setup_tile, LV_ALIGN_IN_TOP_MID, 0, 47 );
    lv_obj_t *omero_server_label = lv_label_create( omero_server_cont, NULL);
    lv_obj_add_style( omero_server_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_server_label, "server");
    lv_obj_align( omero_server_label, omero_server_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_server_textfield = lv_textarea_create( omero_server_cont, NULL);
    lv_textarea_set_text( omero_server_textfield, omero_config->server );
    lv_textarea_set_pwd_mode( omero_server_textfield, false);
    lv_textarea_set_one_line( omero_server_textfield, true);
    lv_textarea_set_cursor_hidden( omero_server_textfield, true);
    lv_obj_set_width( omero_server_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_server_textfield, omero_server_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_server_textfield, omero_textarea_event_cb );

    lv_obj_t *omero_port_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_port_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_port_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_port_cont, omero_server_cont, LV_ALIGN_OUT_BOTTOM_MID, 0,  0 );
    lv_obj_t *omero_port_label = lv_label_create( omero_port_cont, NULL);
    lv_obj_add_style( omero_port_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_port_label, "port");
    lv_obj_align( omero_port_label, omero_port_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_port_textfield = lv_textarea_create( omero_port_cont, NULL);
    char buf[10];
    sprintf(buf, "%d", omero_config->port );
    lv_textarea_set_text( omero_port_textfield, buf);
    lv_textarea_set_pwd_mode( omero_port_textfield, false);
    lv_textarea_set_one_line( omero_port_textfield, true);
    lv_textarea_set_cursor_hidden( omero_port_textfield, true);
    lv_obj_set_width( omero_port_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_port_textfield, omero_port_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_port_textfield, omero_num_textarea_event_cb );

    lv_obj_t *omero_user_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_user_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_user_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_user_cont, omero_port_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *omero_user_label = lv_label_create( omero_user_cont, NULL);
    lv_obj_add_style( omero_user_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_user_label, "user");
    lv_obj_align( omero_user_label, omero_user_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_user_textfield = lv_textarea_create( omero_user_cont, NULL);
    lv_textarea_set_text( omero_user_textfield, omero_config->user );
    lv_textarea_set_pwd_mode( omero_user_textfield, false);
    lv_textarea_set_one_line( omero_user_textfield, true);
    lv_textarea_set_cursor_hidden( omero_user_textfield, true);
    lv_obj_set_width( omero_user_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_user_textfield, omero_user_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_user_textfield, omero_textarea_event_cb );

    lv_obj_t *omero_password_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_password_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_password_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_password_cont, omero_user_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *omero_password_label = lv_label_create( omero_password_cont, NULL);
    lv_obj_add_style( omero_password_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_password_label, "pass");
    lv_obj_align( omero_password_label, omero_password_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_password_textfield = lv_textarea_create( omero_password_cont, NULL);
    lv_textarea_set_text( omero_password_textfield, omero_config->password );
    lv_textarea_set_pwd_mode( omero_password_textfield, false);
    lv_textarea_set_one_line( omero_password_textfield, true);
    lv_textarea_set_cursor_hidden( omero_password_textfield, true);
    lv_obj_set_width( omero_password_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_password_textfield, omero_password_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_password_textfield, omero_textarea_event_cb );

    lv_obj_t *omero_idTag_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_idTag_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_idTag_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_idTag_cont, omero_user_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *omero_idTag_label = lv_label_create( omero_idTag_cont, NULL);
    lv_obj_add_style( omero_idTag_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_idTag_label, "idTag");
    lv_obj_align( omero_idTag_label, omero_idTag_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_idTag_textfield = lv_textarea_create( omero_idTag_cont, NULL);
    lv_textarea_set_text( omero_idTag_textfield, omero_config->idTag );
    lv_textarea_set_pwd_mode( omero_idTag_textfield, false);
    lv_textarea_set_one_line( omero_idTag_textfield, true);
    lv_textarea_set_cursor_hidden( omero_idTag_textfield, true);
    lv_obj_set_width( omero_idTag_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_idTag_textfield, omero_idTag_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_idTag_textfield, omero_textarea_event_cb );

    lv_obj_t *omero_topic_cont = lv_obj_create( omero_setup_tile, NULL );
    lv_obj_set_size( omero_topic_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( omero_topic_cont, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_obj_align( omero_topic_cont, omero_password_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *omero_topic_label = lv_label_create( omero_topic_cont, NULL);
    lv_obj_add_style( omero_topic_label, LV_OBJ_PART_MAIN, &omero_setup_style  );
    lv_label_set_text( omero_topic_label, "topic");
    lv_obj_align( omero_topic_label, omero_topic_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    omero_topic_textfield = lv_textarea_create( omero_topic_cont, NULL);
    lv_textarea_set_text( omero_topic_textfield, omero_config->topic );
    lv_textarea_set_pwd_mode( omero_topic_textfield, false);
    lv_textarea_set_one_line( omero_topic_textfield, true);
    lv_textarea_set_cursor_hidden( omero_topic_textfield, true);
    lv_obj_set_width( omero_topic_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( omero_topic_textfield, omero_topic_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( omero_topic_textfield, omero_textarea_event_cb );

    lv_tileview_add_element( omero_setup_tile, omero_server_cont );
    lv_tileview_add_element( omero_setup_tile, omero_port_cont );
    lv_tileview_add_element( omero_setup_tile, omero_user_cont );
    lv_tileview_add_element( omero_setup_tile, omero_password_cont );
    lv_tileview_add_element( omero_setup_tile, omero_idTag_cont );
    lv_tileview_add_element( omero_setup_tile, omero_topic_cont );

    lv_obj_t *omero_autoconnect_onoff_cont = lv_obj_create( omero_setup_tile_2, NULL);
    lv_obj_set_size( omero_autoconnect_onoff_cont, lv_disp_get_hor_res( NULL ), 32);
    lv_obj_add_style( omero_autoconnect_onoff_cont, LV_OBJ_PART_MAIN, &omero_setup_style );
    lv_obj_align( omero_autoconnect_onoff_cont, omero_setup_tile_2, LV_ALIGN_IN_TOP_MID, 0, 49 );
    omero_autoconnect_onoff = wf_add_switch( omero_autoconnect_onoff_cont, false);
    lv_obj_align( omero_autoconnect_onoff, omero_autoconnect_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_set_event_cb( omero_autoconnect_onoff, omero_autoconnect_onoff_event_handler );
    lv_obj_t *omero_autoconnect_label = lv_label_create(omero_autoconnect_onoff_cont, NULL);
    lv_obj_add_style( omero_autoconnect_label, LV_OBJ_PART_MAIN, &omero_setup_style );
    lv_label_set_text( omero_autoconnect_label, "autoconnect");
    lv_obj_align( omero_autoconnect_label, omero_autoconnect_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0);

    lv_obj_t *omero_widget_onoff_cont = lv_obj_create( omero_setup_tile_2, NULL);
    lv_obj_set_size( omero_widget_onoff_cont, lv_disp_get_hor_res( NULL ), 32);
    lv_obj_add_style( omero_widget_onoff_cont, LV_OBJ_PART_MAIN, &omero_setup_style );
    lv_obj_align( omero_widget_onoff_cont, omero_autoconnect_onoff_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    omero_widget_onoff = wf_add_switch( omero_widget_onoff_cont, false);
    lv_obj_align( omero_widget_onoff, omero_widget_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_set_event_cb( omero_widget_onoff, omero_widget_onoff_event_handler );
    lv_obj_t *omero_widget_onoff_label = lv_label_create( omero_widget_onoff_cont, NULL);
    lv_obj_add_style( omero_widget_onoff_label, LV_OBJ_PART_MAIN, &omero_setup_style );
    lv_label_set_text( omero_widget_onoff_label, "mainbar widget");
    lv_obj_align( omero_widget_onoff_label, omero_widget_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0);

    if ( omero_config->autoconnect )
        lv_switch_on( omero_autoconnect_onoff, LV_ANIM_OFF);
    else
        lv_switch_off( omero_autoconnect_onoff, LV_ANIM_OFF);

    if ( omero_config->widget )
        lv_switch_on( omero_widget_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( omero_widget_onoff, LV_ANIM_OFF );
}

static void omero_setup_hibernate_callback ( void ) {
    keyboard_hide();
    omero_config_t *omero_config = omero_get_config();
    strlcpy( omero_config->server, lv_textarea_get_text( omero_server_textfield ), sizeof( omero_config->server ) );
    strlcpy( omero_config->user, lv_textarea_get_text( omero_user_textfield ), sizeof( omero_config->user ) );
    strlcpy( omero_config->password, lv_textarea_get_text( omero_password_textfield ), sizeof( omero_config->password ) );
    strlcpy( omero_config->topic, lv_textarea_get_text( omero_topic_textfield ), sizeof( omero_config->topic ) );
    strlcpy( omero_config->idTag, lv_textarea_get_text( omero_idTag_textfield ), sizeof( omero_config->idTag ) );
    omero_config->port = atoi(lv_textarea_get_text( omero_port_textfield ));
    omero_config->save();
}

static void omero_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        keyboard_set_textarea( obj );
    }
}

static void omero_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        num_keyboard_set_textarea( obj );
    }
}

static void omero_autoconnect_onoff_event_handler( lv_obj_t * obj, lv_event_t event ) {
    switch ( event ) {
        case (LV_EVENT_VALUE_CHANGED):      omero_config_t *omero_config = omero_get_config();
                                            omero_config->autoconnect = lv_switch_get_state( obj );
                                            break;
    }
}

static void omero_widget_onoff_event_handler(lv_obj_t *obj, lv_event_t event)
{
    switch ( event ) {
        case ( LV_EVENT_VALUE_CHANGED ):    omero_config_t *omero_config = omero_get_config();
                                            omero_config->widget = lv_switch_get_state( obj );
                                            if ( omero_config->widget ) {
                                                omero_add_widget();
                                            }
                                            else {
                                                omero_remove_widget();
                                            }
                                            break;
    }
}
