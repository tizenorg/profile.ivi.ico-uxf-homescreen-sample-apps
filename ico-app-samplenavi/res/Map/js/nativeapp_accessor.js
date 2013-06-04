/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

/**
 *  Navigation Application(sample navi) Accessor Class
 *
 */
var CarInfo = function(lat_val, lon_val) {
    this.lat_val = lat_val;
    this.lon_val = lon_val;
}

var NativeAppAccessor = function() {
    var _socket;                                                // WebSocket
    
    /**
     *  Connection WebSocket
     *
     */
    this.connectWebSocket = function() {
        if ( navigator.userAgent.indexOf('Firefox') != -1 ) {
            _socket = new MozWebSocket('ws://' + IP_ADDR + ':' + WEBSOCKET_PORT + '/');
        }
        else {
            _socket = new WebSocket('ws://' + IP_ADDR + ':' + WEBSOCKET_PORT + '/', 'http-only');
        }
        
        // socket event
        _socket.onopen = function() {
            _socket.send('OPEN ' + EDJE_PATH + EDJE_CONF_NAME);
        };

        _socket.onmessage = function(e) { recvMessage(e); };
        _socket.onerror = function(e) { };
        _socket.onclose = function() { };
    }

    /**
     *  Disconnection WebSocket
     *
     */
    this.disconnectWebSocket = function() {
        if ( _socket ) {
            _socket.send('Close_Socket');
            _socket.close();
        }
    }

    /**
     * Request Navigation Start
     */
    this.sendReqNaviStart = function() {
        if (_socket) {
            $('#address').text('Send Req Navi Start.');
            _socket.send('CHG SEQ REQ_NAV');
        }
    }

    /**
     * Request Navigation End
     */
    this.sendReqNaviEnd = function() {
        if (_socket) {
            $('#address').text('Send Req Navi End.');
            _socket.send('CHG SEQ END_NAV');
        }
    }

    /**
     * Send Route
     */
    this.sendRoute = function( routeStr ) {
        if( _socket ) {
            _socket.send( routeStr );
        }
    }

    /**
     *  Receive Message
     *
     *  @param  event   Receive Event
     *
     */
    function recvMessage(event) {
        var message = event.data;
        var info;

        var messageType = getMessageType(message);
        if ( messageType == MSG_TYPE.CHG ) {
            // type is carinfo change message
            info = getCarInfo(message);
            if ( info ) {
                $(document).trigger('recvCarInfo', info);
            }
        }
        else
        {
            $(document).trigger('recvSeqNxt', message);
        }
    }

    /**
     *  get message type
     *
     *  @param  message     Receive message
     *
     *  @return message kind (MSG_TYPE)
     *
     */
    function getMessageType(message) {
        var cmd = message.slice(0, 7);
        var type;

        if ( cmd == 'CHG VAL') {
            type = MSG_TYPE.CHG;
        } else if (cmd == 'CHG SEQ') {
            type = MSG_TYPE.SEQ;
        } else {
            type = MSG_TYPE.ERR;
        }

        return type;
    }

    /**
     *  Get CarInfo
     *
     *  @param  message     Receive message
     *
     *  @return CarInfo data
     *
     */
    function getCarInfo(message) {
        var tmp = message.slice(17);
        var tmp_info;
        var info;
        var lat_index;
        var lon_index;

        lat_index = tmp.indexOf(KEY_LAT);
        lon_index = tmp.indexOf(KEY_LON);

        if ((lat_index != -1) && (lon_index != -1) ){
            info = new CarInfo();

            // Latitude
            tmp_info = tmp.slice(lat_index + KEY_LAT.length + 1, lon_index);
            info.lat_val =  parseFloat(tmp_info);

            // Longitude
            tmp_info = tmp.slice(lon_index + KEY_LON.length + 1);
            info.lon_val =  parseFloat(tmp_info);
        }

        return info;
    }

    /**
     * Request DIST ONSCREEN
     */
    this.sendReqDistOnScreen = function() {
        if (_socket) {
            $('#address').text("SEND REQ DIST OnScreen.");
            try {
                _socket.send('OPEN ' + EDJE_PATH + EDJE_DIST_NAME);
            } catch (e) {
                $('#address').text("SEND REQ DIST OnScreen Error. :" + e);
            }
        }
    }
    /**
     * Request CLOSE ONSCREEN
     */
    this.sendReqCloseOnScreen = function() {
        if (_socket) {
            $('#address').text("SEND REQ Close OnScreen.");
            _socket.send('CLOSE');
        }
    }
}
