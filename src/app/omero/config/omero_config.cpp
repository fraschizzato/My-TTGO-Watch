/****************************************************************************
 *   Aug 11 17:13:51 2020
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
#include "omero_config.h"

omero_config_t::omero_config_t() : BaseJsonConfig( OMERO_JSON_CONFIG_FILE ) {}

bool omero_config_t::onSave(JsonDocument& doc) {
    doc["omero"]["server"] = server;
    doc["omero"]["port"] = port;
    doc["omero"]["ssl"] = ssl;
    doc["omero"]["user"] = user;
    doc["omero"]["password"] = password;
    doc["omero"]["topic"] = topic;
    doc["omero"]["idTag"] = idTag;
    doc["omero"]["autoconnect"] = autoconnect;
    doc["omero"]["widget"] = widget;

    return true;
}

bool omero_config_t::onLoad(JsonDocument& doc) {
    strncpy( server, doc["omero"]["server"] | "", sizeof( server ) );
    port = doc["omero"]["port"] | 1883;
    ssl = doc["omero"]["ssl"] | false;
    strncpy( user, doc["omero"]["user"] | "", sizeof( user ) );
    strncpy( password, doc["omero"]["password"] | "", sizeof( password ) );
    strncpy( topic, doc["omero"]["topic"] | "", sizeof( topic ) );
    strncpy( idTag, doc["omero"]["idTag"] | "", sizeof( idTag ) );
    autoconnect = doc["omero"]["autoconnect"] | false;
    widget = doc["omero"]["widget"] | false;
    
    return true;
}

bool omero_config_t::onDefault( void ) {
    strncpy( server, "", sizeof( server ) );
    port = 1883;
    ssl = false;
    strncpy( user, "", sizeof( user ) );
    strncpy( password, "", sizeof( password ) );
    strncpy( topic, "", sizeof( topic ) );
    strncpy( idTag, "", sizeof( idTag ) );
    autoconnect = false;
    widget = false;

    return true;
}