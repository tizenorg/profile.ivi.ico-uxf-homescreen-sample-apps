/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   definition header file for sample navigation application
 *
 * @date    Apr-25-2013
 */

#ifndef SAMPLENAVI_DEFINE_H_
#define SAMPLENAVI_DEFINE_H_

/* FUNCTION SELECT */
//#define _USE_CAMERA_
//#define _USE_OPENCV_  //USE
//#define _E3D_H_OUTPUT_PRINT_

#define _SHOW_FPS_
#define _USE_Z_LIMIT_FIX_


/* Default data (for samplenavi.conf) */
#define DEF_USB_CAMERA_ID				(0)
#define DEF_LANDMARK_POSITION			(-80)
#define DEF_LANDMARK_ROTATION			(90)


/* for 3D Layer */
#define ROUTE_PLANE_HEIGHT			(-190)
#define ROUTE_PLANE_WIDTH			(100)

#define FRONT_SIDE_Z				(-190)
#define Z0_POSITION					(0)
#define FOCUS_LENGTH				(512)

#define Z_LIMIT						(-150)
#define COORD_CONVERT_COEFFICIENT	(20)

#define CULLING_LENGTH_LANDMARKS	(5000)
#define CULLING_LENGTH_ROUTE		(20000)

#define GOAL_MESSAGE_LENGTH			(3500)


/* For route point */
#define MAX_DRAW_ROUTE_POINTS		(500)
#define ROUTE_DIVISION_NUMBER		(10)
#define BOX_FIXED_LENGTH			(150)

/* For Landmarks */
#define MAX_LANDMARK_POINTS			(1000)
#define LANDMARK_WIDTH				(100)

/* Timers */
#define TIME_INTERVAL_CAMERA		(0.10)
//#define TIME_INTERVAL_AR			(0.05)
//#define TIME_INTERVAL_AR			(0.033)
#define TIME_INTERVAL_AR			(0.066)
#define TIME_INTERVAL_MAP			(1.0)


/* Window Size */
#define W_WIDTH						(1920) 
#define W_HEIGHT					(1080-64) // StatusBar window height:64

#define W_NAVI_HEIGHT				(480)
#define W_MAP_HEIGHT				(536)

#define W_NAVI_ORIGINE_X			(0)
#define W_NAVI_ORIGINE_Y			(0)

#define W_MAP_ORIGINE_X				(0)
#define W_MAP_ORIGINE_Y				(480)

#define W_TEXTAREA_X				(0)
#define W_TEXTAREA_Y				(416)
#define W_TEXTAREA_HEIGHT			(64)
#define W_TEXTAREA_WIDTH			(W_WIDTH)

#define W_NAVIBUTTON_HEIGHT			(44)
#define W_NAVIBUTTON_WIDTH			(146)
#define W_NAVIBUTTON_X				(W_WIDTH - 10 - W_NAVIBUTTON_WIDTH)
#define W_NAVIBUTTON_Y				(W_TEXTAREA_Y + 10)

#define W_GOALMESSAGE_HEIGHT		(50)
#define W_GOALMESSAGE_WIDTH			(478)
#define W_GOALMESSAGE_X				(150)
#define W_GOALMESSAGE_Y				(283)

#define W_ADDRESS_HEIGHT			(50)
#define W_ADDRESS_WIDTH				(570)
#define W_ADDRESS_X					(32)
#define W_ADDRESS_Y					(437)

#define MAX_ROUTE_POINTS			(500)
#define LON_CONVERT					(90550.8)
#define LAT_CONVERT					(111240)

/* Directory */
#define RESOURCE_DIR				"/opt/apps/org.tizen.ico.app-samplenavi/res"
#define IMAGES_DIR					"/opt/apps/org.tizen.ico.app-samplenavi/res/images"

/* Common */
#define TRUE						(1)
#define FALSE						(0)

/* Layer */
#define LAYER_CAMERA				(0)
#define LAYER_ROUTE					(5)
#define LAYER_LANDMARK				(10)
#define LAYER_MAP					(30)
#define LAYER_UI					(30)

/* Resources */
static const char *config_path = RESOURCE_DIR "/samplenavi.conf";

static const char *setting_button_path = IMAGES_DIR "/na_bt3_off.png";

static const char *goal_img_path = IMAGES_DIR "/na_goal.png";
static const char *goal_text_img_path = IMAGES_DIR "/na_goal_text.png";

static const char *landmark_img_path_0 = IMAGES_DIR "/sg_m_01.png";
static const char *landmark_img_path_1 = IMAGES_DIR "/sg_m_02.png";
static const char *landmark_img_path_2 = IMAGES_DIR "/sg_m_03.png";
static const char *landmark_img_path_3 = IMAGES_DIR "/sg_m_04.png";
static const char *landmark_img_path_4 = IMAGES_DIR "/sg_m_05.png";
static const char *landmark_img_path_5 = IMAGES_DIR "/sg_m_06.png";
static const char *landmark_img_path_6 = IMAGES_DIR "/sg_m_07.png";
static const char *landmark_img_path_7 = IMAGES_DIR "/sg_m_08.png";
static const char *landmark_img_path_8 = IMAGES_DIR "/sg_l_01.png";
static const char *landmark_img_path_9 = IMAGES_DIR "/sg_l_02.png";
static const char *landmark_img_path_10 = IMAGES_DIR "/sg_l_03.png";
static const char *landmark_img_path_11 = IMAGES_DIR "/sg_l_04.png";
static const char *landmark_img_path_12 = IMAGES_DIR "/sg_l_05.png";
static const char *landmark_img_path_13 = IMAGES_DIR "/sg_l_06.png";
static const char *landmark_img_path_14 = IMAGES_DIR "/sg_l_07.png";
static const char *landmark_img_path_15 = IMAGES_DIR "/sg_l_08.png";
static const char *csv_landmark_path = RESOURCE_DIR "/landmark.csv";

static const char *default_url = "file:///opt/apps/org.tizen.ico.app-samplenavi/res/Map/map.html";

#endif /* SAMPLENAVI_DEFINE_H_ */

