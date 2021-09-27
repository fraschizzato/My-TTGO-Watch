/****************************************************************************
 *   Sep 3 23:05:42 2020
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

#include "omero_app.h"
#include "omero_main.h"
#include "omero_setup.h"
#include "config/omero_config.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"

#include "utils/json_psram_allocator.h"

omero_config_t omero_config;

// app and widget icon
icon_t *omero_app = NULL;
icon_t *omero_widget = NULL;

uint32_t omero_app_main_tile_num;
uint32_t omero_app_setup_tile_num;

// declare you images or fonts you need
LV_IMG_DECLARE(omero_64px);

// declare callback functions
static void enter_omero_app_event_cb( lv_obj_t * obj, lv_event_t event );

// setup routine for example app
void omero_app_setup( void ) {

    omero_config.load();

    // register 2 vertical tiles and get the first tile number and save it for later use
    omero_app_main_tile_num = mainbar_add_app_tile( 1, 3, "OMERO App" );
    omero_app_setup_tile_num = omero_app_main_tile_num + 1;

    omero_app = app_register( "omero", &omero_64px, enter_omero_app_event_cb );

    if ( omero_config.widget ) {
        omero_add_widget();
    }

    omero_main_tile_setup( omero_app_main_tile_num );
    omero_setup_tile_setup( omero_app_setup_tile_num );
}

uint32_t omero_get_app_main_tile_num( void ) {
    return( omero_app_main_tile_num );
}

uint32_t omero_get_app_setup_tile_num( void ) {
    return( omero_app_setup_tile_num );
}

icon_t *omero_get_app_icon( void ) {
    return( omero_app );
}

icon_t *omero_get_widget_icon( void ) {
    return( omero_widget );
}

static void enter_omero_app_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( omero_app_main_tile_num, LV_ANIM_OFF );
                                        statusbar_hide( true );
                                        break;
    }    
}


omero_config_t *omero_get_config( void ) {
    return( &omero_config );
}

bool omero_add_widget( void ) {
    if ( omero_widget == NULL ) {
        omero_widget = widget_register( "Omero", &omero_64px, enter_omero_app_event_cb );
        widget_hide_indicator( omero_widget );
        if ( omero_widget != NULL ) {
            return( true );
        }
        else {
            return( false );
        }
    }
    return( true );
}

bool omero_remove_widget( void ) {
    omero_widget = widget_remove( omero_widget );
    return( true );
}
