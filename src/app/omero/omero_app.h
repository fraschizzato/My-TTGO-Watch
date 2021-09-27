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
#ifndef _OMERO_APP_H
    #define _OMERO_APP_H

    #include "gui/icon.h"
    #include "config/omero_config.h"

    #define OMERO_JSON_CONFIG_FILE        "/omero.json"

    void omero_app_setup( void );
    uint32_t omero_get_app_main_tile_num( void );
    uint32_t omero_get_app_setup_tile_num( void );
    omero_config_t *omero_get_config( void );
    void omero_save_config( void );
    void omero_load_config( void );
    bool omero_add_widget( void );
    bool omero_remove_widget( void );
    icon_t *omero_get_app_icon( void );
    icon_t *omero_get_widget_icon( void );

#endif // _OMERO_APP_H