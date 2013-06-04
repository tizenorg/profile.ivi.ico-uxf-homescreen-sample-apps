/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

/**
 *  Defines
 */

const MSG_TYPE = 
{
    CHG : 0,                                        // Change Message
    SEQ : 1,                                        // Sequence Message
    ERR : 2,                                        // Error Message
    
};

const SEQUENCE_MODE =                               // Sequence
{
    START : 0,                                      // Start Application
    CONF : 1,                                       // Display Conf.
    DIST : 2,                                       // Display Dist.
    NAVI : 3,                                       // Run Navigation
    NAVI_END : 4                                    // End Navigation(no use)
};

const SHOWING_ROUTE         = 1;                    // Current Route number
const TMP_LAT               = 35.47945;             // Initialize Latitude
const TMP_LON               = 139.40026;            // Initialize Longitude
const DEFAULT_ZOOM          = 17;                   // Zoom value for Map
const EDJE_PATH             = '/opt/apps/org.tizen.ico.app-samplenavi/data';
const EDJE_CONF_NAME        = '/samplenavi_conf.edj';
const EDJE_DIST_NAME        = '/samplenavi_conf_dist.edj';
const APP_NO                = '300';
const IP_ADDR               = '127.0.0.1';
const WEBSOCKET_PORT        = '50414';

const KEY_LAT = "LAT";
const KEY_LON = "LON";

