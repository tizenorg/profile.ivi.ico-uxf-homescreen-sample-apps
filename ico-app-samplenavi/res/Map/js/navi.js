/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

/**
 * Navigation Map Class
 */

var Navigation = function() {
    var _myLatLng;                                          // Current Location
    var _lat = TMP_LAT;                                     // Current Latitude
    var _lng = TMP_LON;                                     // Current Longitude
    var _dir;                                               // Current Direction
    var _map;                                               // map
    var _dirDisplay;                                        // DirectionsRenderer Object
    var _dirService;                                        // DirectionsService Object
    var _carMarker;                                         // Current Location Marker
    var _nativeAccessor;                                    // WebSocket accessor (samplenavi)
    var _seqnum = SEQUENCE_MODE.START;                      // Sequence
    var _routeStr = '';                                     // Route Info
    
    /*
     *  initialize
     */
    this.initialize = function() {
        $('#address').text('For samplenaviConnection');

        // event bind
        $(document).bind('recvSeqNxt', function(event, data) {
            recvSeqNxt(data);
        });
        $(document).bind('recvCarInfo', function(event, data) {
            recvCarinfo(data);
        });
        // Initialize Location
        _myLatLng = new google.maps.LatLng(_lat, _lng);

        // MAP create
        var mapOptions = {
            zoom: DEFAULT_ZOOM,                                 // zoom value
            mapTypeId: google.maps.MapTypeId.ROADMAP,           // MAP TYPE
            center: _myLatLng                                   // Initial Position
        };
            
        _map = new google.maps.Map($('#map_canvas').get(0), mapOptions);
        
        // Create current position marker
        _carMarker = new google.maps.Marker({
            map: _map,
            position: _myLatLng,
            zIndex: 3
        });
        
        // Set DirectionRendererOption
        var rendererOptions = {
            draggable: false,
            preserveViewport:true
        };
        
        _dirDisplay = new google.maps.DirectionsRenderer(rendererOptions);
        _dirDisplay.setMap(_map);
        _dirDisplay.setOptions( {suppressMarkers: true} );      // Hide markers
        _dirService = new google.maps.DirectionsService();
        
        // Create WebSocket Accessor(samplenavi)
        _nativeAccessor = new NativeAppAccessor();
        _nativeAccessor.connectWebSocket();

        // Request CONF ONSCREEN
        _seqnum = SEQUENCE_MODE.CONF;
    }

    /**
     *  finalize
     *
     */
    this.finalize = function() {
        _nativeAccessor.disconnectWebSocket();
    }

    /**
     *  Move Current Location
     *
     */
    function changeLatLng() {
        _myLatLng = new google.maps.LatLng(_lat, _lng);
        _map.panTo(_myLatLng);
        
        // Move Marker
        _carMarker.setPosition(_myLatLng);
    }

    /**
     *  Set Route
     */
    function setRoute() {
        if (SHOWING_ROUTE == 1) {
            setRoute1();
        } else if (SHOWING_ROUTE == 2) {
            setRoute2();
        } else if (SHOWING_ROUTE == 3) {
            setRoute3();
        }
    }

    /**
     *  Set Route(Route 1)
     *
     */
    function setRoute1() {
        // Local variable
        var startStr = String(START_LAT) + ',' + String(START_LNG);
        var goalStr = String(GOAL_LAT) + ',' + String(GOAL_LNG);
        
        // Route request
        var request = {
                origin: startStr,
                destination: goalStr,
                waypoints: [
                    ],
                travelMode: google.maps.DirectionsTravelMode.DRIVING,   // Set DrivingMode(Car)
                unitSystem: google.maps.DirectionsUnitSystem.METRIC,    // Set Unit
                optimizeWaypoints: true,                                // optimize route
                avoidHighways: true,                                    // Avoid highways
                avoidTolls: true                                        // Avoid tall road
        };
        
        _dirService.route(request, function(response, status) {
            if (status == google.maps.DirectionsStatus.OK) {
                _dirDisplay.setDirections(response);

                _routeStr = "<route>";
                
                // Set route info
                for( var i = 0; i < (response.routes[0].overview_path.length); i++ ) {
                    var lat = response.routes[0].overview_path[i].lat();
                    var lng = response.routes[0].overview_path[i].lng();
                    _routeStr += String( lng ) + ',' + String( lat ) + "<BR>";
                }
                _routeStr += "</route>";
                
                // Send route
                _nativeAccessor.sendRoute( _routeStr );
            }
            else {
                alert('google maps error:' + status);
            }
        })
    }
    
    /**
     *  Set Route(Route 2)
     *
     */
    function setRoute2() {
        // Route request
        var request = {
                origin: "35.47945,139.40026",
                destination: "35.61689,139.44791",
                waypoints: [
                    { location: "35.534845,139.437477" }
                    ],
                travelMode: google.maps.DirectionsTravelMode.DRIVING,   // Set DrivingMode(Car)
                unitSystem: google.maps.DirectionsUnitSystem.METRIC,    // Set Unit
                optimizeWaypoints: false,                               // optimize route
                avoidHighways: true,                                    // Avoid highways
                avoidTolls: true                                        // Avoid tall road
        };
        
        _dirService.route(request, function(response, status) {
            if (status == google.maps.DirectionsStatus.OK) {
                _dirDisplay.setDirections(response);
                _routeStr = "<route>";
                
                // Set route info
                for( var i = 0; i < (response[0].overview_path.length); i++ ) {
                    var lat = response[0].overview_path[i].lat();
                    var lng = response[0].overview_path[i].lng();
                    _routeStr += String( lng ) + ',' + String( lat ) + "<BR>";
                }
                _routeStr += "</route>";
                
                // Send route
                _nativeAccessor.sendRoute( _routeStr );
            }
            else {
                alert('google maps error:' + status);
            }
        })
    }
    
    /**
     *  Set Route(Route 3)
     *
     */
    function setRoute3() {
        // Route request
        var request = {
            origin: "35.45153,139.2125",
            destination: "35.43037,139.21579",
                waypoints: [
                    ],
                travelMode: google.maps.DirectionsTravelMode.DRIVING,   // Set DrivingMode(Car)
                unitSystem: google.maps.DirectionsUnitSystem.METRIC,    // Set Unit
                optimizeWaypoints: false,                               // optimize route
                avoidHighways: true,                                    // Avoid highways
                avoidTolls: true                                        // Avoid tall road
        };
        
        _dirService.route(request, function(response, status) {
            if (status == google.maps.DirectionsStatus.OK) {
                _dirDisplay.setDirections(response);
                _routeStr = "<route>";
                
                // Set root info
                for( var i = 0; i < (response[0].overview_path.length); i++ ) {
                    var lat = response[0].overview_path[i].lat();
                    var lng = response[0].overview_path[i].lng();
                    _routeStr += String( lng ) + ',' + String( lat ) + "<BR>";
                }
                _routeStr += "</route>";
                
                // Send route
                _nativeAccessor.sendRoute( _routeStr );
            }
            else {
                alert('google maps error:' + status);
            }
        })
    }

    /**
     *  Receive CarInfo
	 *
     *  @param  data    CarInfo data
	 *
     */
    function recvCarinfo(data) {
        _lat = data.lat_val;
        _lng = data.lon_val;
        changeLatLng();
    }

    /**
     *  Receive Change Sequence
     *
     *  @param  message     Receive message
	 *
     */
    function recvSeqNxt(data) {
        if (data == 'TOUCH ' + EDJE_PATH + EDJE_CONF_NAME + ' nai_01' ) {
            _nativeAccessor.sendReqDistOnScreen();
            _seqnum = SEQUENCE_MODE.DIST;
        } else if (data == 'TOUCH ' + EDJE_PATH + EDJE_DIST_NAME + ' nai_12') {
            _nativeAccessor.sendReqNaviStart();
            _nativeAccessor.sendReqCloseOnScreen();
            _seqnum = SEQUENCE_MODE.NAVI;

            setRoute();
        } else if (data == 'RESULT SUCCESS') {
            $('#address').text("OnScreen Request Success.");
        } else if (data == 'RESULT FAILED') {
            $('#address').text("OnScreen Request Failed.");
        }
    }
}

