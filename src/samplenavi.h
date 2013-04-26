/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   header file of sample navigation application
 *
 * @date    Apr-25-2013
 */

#ifndef SAMPLENAVI_H_
#define SAMPLENAVI_H_

/* enum */
enum ConfDataID {
    USB_CAMERA_ID,
    LANDMARK_POSITION,
    LANDMARK_ROTATION,
    CONF_DATA_MAX
};

extern int conf_data[CONF_DATA_MAX];

#endif /* SAMPLENAVI_H_ */

