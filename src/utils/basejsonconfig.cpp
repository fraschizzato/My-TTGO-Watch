/****
 * QuickGLUI - Quick Graphical LVLGL-based User Interface development library
 * Copyright  2020  Skurydin Alexey
 * http://github.com/anakod
 * All QuickGLUI project files are provided under the MIT License.
 ****/
#ifdef NATIVE_64BIT
    #include <iostream>
    #include <fstream>
    #include <string.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#else
    #include <SPIFFS.h>
    #include <FS.h>
#endif

#include "basejsonconfig.h"
#include "utils/filepath_convert.h"
#include "json_psram_allocator.h"
#include "alloc.h"

BaseJsonConfig::BaseJsonConfig(const char* configFileName) {

#ifdef NATIVE_64BIT
    const char *homedir;
    char localpath[128] = "";
    /**
     * get local device path for config files
     */
    if ( configFileName[0] == '/') {
        snprintf( localpath, sizeof( localpath ), "spiffs%s", configFileName );
    }
    else {
        localpath[0] = '/';
        snprintf( localpath, sizeof( localpath ), "spiffs/%s", configFileName );
    }
    /**
     * convert from local device path to machine path
     */
    filepath_convert( fileName, sizeof( fileName ), localpath );
#else
    if (configFileName[0] == '/')
        strncpy( fileName, configFileName, MAX_CONFIG_FILE_NAME_LENGTH);
    else
    {
        fileName[0] = '/';
        strncpy(fileName+1, configFileName, MAX_CONFIG_FILE_NAME_LENGTH);
    }
#endif
}

bool BaseJsonConfig::load() {
    bool result = false;
    /*
     * load config if exsits
     */
#ifdef NATIVE_64BIT
    std::fstream file;

    file.open( fileName, std::fstream::in );
    /*
        * check if open was success
        */
    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        /*
            * get filesize
            */
        file.seekg( 0, file.end );
        int filesize = file.tellg();
        file.seekg( 0, file.beg );
        /*
            * create json structure
            */
        SpiRamJsonDocument doc( filesize*4 );
        DeserializationError error = deserializeJson( doc, file );
        /*
            * check if create json structure was successfull
            */
        if ( error || filesize == 0 ) {
            log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
        }
        else {
            log_i("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
            result = onLoad(doc);
        }
        doc.clear();
    }
    file.close();
#else
    if ( SPIFFS.exists(fileName) ) {
        /*
         * open file
         */
        fs::File file = SPIFFS.open(fileName, FILE_READ);
        /*
         * check if open was success
         */
        if (!file) {
            log_e("Can't open file: %s!", fileName);
        }
        else {
            /*
             * get filesize
             */
            int filesize = file.size();
            /*
             * create json structure
             */
            SpiRamJsonDocument doc( filesize*4 );
            DeserializationError error = deserializeJson( doc, file );
            /*
             * check if create json structure was successfull
             */
            if ( error || filesize == 0 ) {
                log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
            }
            else {
                log_i("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
                result = onLoad(doc);
            }
            doc.clear();
        }
        file.close();
    }
#endif
    /*
     * check if read from json is failed
     */
    if ( !result ) {
        log_i("reading json failed, call defaults, file: %s", fileName );
        result = onDefault();
    }

    return result;
}

bool BaseJsonConfig::load( uint32_t size ) {
    bool result = false;
    /*
     * load config if exsits
     */
#ifdef NATIVE_64BIT
    std::fstream file;
    /*
     * open file
     */
    file.open( fileName, std::fstream::in );
    /*
        * check if open was success
        */
    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        /*
            * create json structure
            */
        SpiRamJsonDocument doc( size );
        DeserializationError error = deserializeJson( doc, file );
        /*
            * check if create json structure was successfull
            */
        if ( error || size == 0 ) {
            log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
        }
        else {
            log_i("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
            result = onLoad(doc);
        }
        doc.clear();
    }
    file.close();

#else
    if ( SPIFFS.exists(fileName) ) {
        /*
         * open file
         */
        fs::File file = SPIFFS.open(fileName, FILE_READ);
        /*
         * check if open was success
         */
        if (!file) {
            log_e("Can't open file: %s!", fileName);
        }
        else {
            /*
             * create json structure
             */
            SpiRamJsonDocument doc( size );
            DeserializationError error = deserializeJson( doc, file );
            /*
             * check if create json structure was successfull
             */
            if ( error || size == 0 ) {
                log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
            }
            else {
                log_i("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
                result = onLoad(doc);
            }
            doc.clear();
        }
        file.close();
    }
#endif
    /*
     * check if read from json is failed
     */
    if ( !result ) {
        log_i("reading json failed, call defaults, file: %s", fileName );
        result = onDefault();
    }
    return result;
}

bool BaseJsonConfig::save( uint32_t size ) {
    bool result = false;
#ifdef NATIVE_64BIT
    std::fstream file;

    file.open(fileName, std::fstream::out );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        SpiRamJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use another size");
        }
        
        size_t outSize = 0;
        if (prettyJson)
            outSize = serializeJsonPretty(doc, file);
        else
            outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_i("json config serializeJson() success: %s", fileName );
        }
        
        doc.clear();
    }
    file.close();
#else
    fs::File file = SPIFFS.open(fileName, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        SpiRamJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use another size");
        }
        
        size_t outSize = 0;
        if (prettyJson)
        outSize = serializeJsonPretty(doc, file);
        else
        outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_i("json config serializeJson() success: %s", fileName );
        }
        
        doc.clear();
    }
    file.close();
#endif
    return result;
}

bool BaseJsonConfig::save() {
    bool result = false;
#ifdef NATIVE_64BIT
    std::fstream file;

    file.open(fileName, std::fstream::out );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        auto size = getJsonBufferSize();
        SpiRamJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use doc.save( uint32_t size )");
        }
        
        size_t outSize = 0;
        if (prettyJson)
            outSize = serializeJsonPretty(doc, file);
        else
            outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_i("json config serializeJson() success: %s", fileName );            
        }
        
        doc.clear();
    }
    file.close();
#else
    fs::File file = SPIFFS.open(fileName, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        auto size = getJsonBufferSize();
        SpiRamJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use doc.save( uint32_t size )");
        }
        
        size_t outSize = 0;
        if (prettyJson)
        outSize = serializeJsonPretty(doc, file);
        else
        outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_i("json config serializeJson() success: %s", fileName );            
        }
        
        doc.clear();
    }
    file.close();
#endif
    return result;
}

void BaseJsonConfig::debugPrint() {
/*
    auto size = getJsonBufferSize();
    SpiRamJsonDocument doc(size);
    bool result = onSave(doc);

    if ( result ) {
        serializeJsonPretty(doc, Serial );
    }
*/
}
