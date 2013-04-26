/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   sample navigation application main
 *
 * @date    Apr-25-2013
 */

#include "define.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stddef.h>
#include <getopt.h>
#include <dbus/dbus.h>
#include <libwebsockets.h>
#include <EWebKit2.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
#include "samplenavi.h"
#include "e3d.h"
#include "ico_apf.h"
#include "ico_apf_ecore.h"
#include "ico_apf_log.h"

#ifdef _USE_OPENCV_
#include <highgui.h>
#endif


/* DEBUG.s */
//#define TEST_TIME 0.001
//#define TEST_TIME 0.066
#define TEST_TIME 0.5

//#define DEBUG_METER_TEXT_DSP

//#define DEBUG_VIC_INFO_SET
#ifdef DEBUG_VIC_INFO_SET
extern int route_data_count;
extern CsvRoute csv_route[MAX_ROUTE_POINTS];
static double test_lat = 35.47945;
static double test_lon = 139.40026;
static int test_cnt = 0;
#endif
/* DEBUG.e */

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
/* AMB */
#define CONFIG_FILE    "/opt/apps/org.tizen.ico.app-samplenavi/res/config.txt"
#define DBUS_SERVICE   "org.automotive.message.broker"
#define DBUS_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_METHOD    "Get"
#define MAX_PARA_NUM   4 /* Lat,Lon,Dir,VicSpeed */
#define LATITUDE       0
#define LONGITUDE      1
#define DIRECTION      2
#define VEHICLESPEED   3

/* Setting screen */
#define ICO_APP_BUF_SIZE    (1024)

/*============================================================================*/
/* Define data types                                                          */
/*============================================================================*/
/* AMB */
union VicVal_t{
    dbus_int32_t i32_val;
    dbus_int32_t i16_val;
    dbus_uint32_t u32_val;
    dbus_uint16_t u16_val;
    uint8_t byte_val;
    dbus_bool_t b_val;
    double d_val;
    char *s_val;
};

struct vic_data_t{
    int  property;
    char name[32];
    char path_name[64];
    char interface_name[64];
    char property_name[32];
};

struct vic_key_data_t{
    int  id;
    char name[32];
};

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static double get_time();
static void set_default_data();
static void load_config_file();
static void e_ui_draw();
static void e_ui_init();
static void e_map_init();
static void e_map_draw();
#ifdef _USE_OPENCV_
static void convert_data_evas_cv(Evas_Object *eo, IplImage *iplimage);
static void evas_object_image_from_cv(Evas_Object *eo, const char* filepath);
#endif
static Eina_Bool _time_interval_navi_cb(void *cam);
static Eina_Bool _time_interval_map_renew_cb(void *data);
static Eina_Bool callback_listener(void *data);
static int callback_http(
  struct libwebsocket_context *context, struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);
static bool parse_elementary_value(
  union VicVal_t *vic_val_p, DBusMessageIter *iter);
static bool parse_value(union VicVal_t *vic_val_p, DBusMessageIter *iter);
static int getAmbDbus(int key, union VicVal_t *vic_val_p);
static void getLocationFromAmb();
static int get_config();
static char* edje_parse_str(void *in, int arg_num);
static void event_message(struct libwebsocket *wsi, char *format, ...);
static void _touch_up_edje(
  void *data, Evas *evas , Evas_Object *obj, void *event_info);
static int loading_edje_file(const char *edje_file);
static void res_callback(
  ico_apf_resource_notify_info_t* info, void *user_data);

/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
int conf_data[CONF_DATA_MAX];

/* Screen right */
static char     sscrntype[32];

/* Setting screen */
static Evas *g_evas;
static Eina_List *g_img_list = NULL;
static Evas_Object *g_edje; /* loaded edje objects */
static unsigned char edje_str[ICO_APP_BUF_SIZE];

/* Meter Display */
static int ui_vicspeed = 0;
static Evas_Object *ui_vicspeed_text;
static Evas_Object *ui_meter_l;
static Evas_Object *ui_meter_c[10];
static Evas_Object *ui_meter_r[10];
static int fig_l = 0;
static int fig_c = 0;
static int fig_r = 0;

const static char *meter_l_path = {
IMAGES_DIR "/Meter/Meters_SpeedNum_1l.png"};

const static char *meter_c_path[10] = {
IMAGES_DIR "/Meter/Meters_SpeedNum_0c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_1c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_2c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_3c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_4c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_5c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_6c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_7c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_8c.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_9c.png"};

const static char *meter_r_path[10] = {
IMAGES_DIR "/Meter/Meters_SpeedNum_0r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_1r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_2r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_3r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_4r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_5r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_6r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_7r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_8r.png",
IMAGES_DIR "/Meter/Meters_SpeedNum_9r.png"};

/* AMB */
static DBusConnection *g_connection = NULL; /* D-Bus Connection */
static int property_num = 0;
const static char Bus_name[] = DBUS_SERVICE;
static struct vic_data_t vic_data[MAX_PARA_NUM];

const struct vic_key_data_t vic_key_data[]={
{ LATITUDE,     "Latitude"    },
{ LONGITUDE,    "Longitude"   },
{ DIRECTION,    "Direction"   },
{ VEHICLESPEED, "VehicleSpeed"},
{ -1,           "END"         }};

/* BASE */
static Ecore_Evas *window;
static Evas *e_cam;
static Evas *e_3d;
static Evas *e_ui;
static Evas *e_map;

#ifdef _USE_OPENCV_
IplImage *captureImage;
static CvCapture *capture;
#endif

Evas_Object *cam;

Evas_Object *ui_bg;
Evas_Object *ui_btn;
Evas_Object *ui_goal_text;
Evas_Object *ui_address;

#ifdef _SHOW_FPS_
Evas_Object *ui_fps;
Evas_Object *ui_fps_bg;
Evas_Object *ui_polygon_count;
Evas_Object *ui_polygon_count_bg;
double fps;
#endif

Evas_Object *browser;

/* CAMERA */
static const char *test_map_path = IMAGES_DIR "/test_map2.png";
static const char *test_camera_path = IMAGES_DIR "/test_cameraview.png";

/* WEBSOCKET */
static int event_type = 0;
static int connected = 0;
static struct libwebsocket *socket_val = NULL;
static struct libwebsocket_context *context;
static struct libwebsocket_protocols protocols[] = {
    {
        "http-only",                // name
        callback_http,              // callback
        0                           // per_session_data_size
    },
    {
            NULL, NULL, 0
    }
};

static int port = 50414;
static char addr[16] = "127.0.0.1";
static char proxy_uri[512] = "";

/*============================================================================*/
/* functions                                                                  */
/*============================================================================*/
/*--------------------------------------------------------------------------*/
/*
 * @brief   get_time
 *          Time acquisition(second).
 *
 * @param       none
 * @return      time(second)
 */
/*--------------------------------------------------------------------------*/
static double
get_time()
{
    struct timeval sec_timeofday;
    gettimeofday(&sec_timeofday, NULL);
    return ((sec_timeofday.tv_sec) + 
      (sec_timeofday.tv_usec / 1000000.0));
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   set_default_data
 *          Set Default data without define in conf file.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
set_default_data()
{
    int i;

    for (i = 0; i < CONF_DATA_MAX; i++) {
        if (conf_data[i] == 0) {
            switch (i) {
                case USB_CAMERA_ID:
                    conf_data[USB_CAMERA_ID] = DEF_USB_CAMERA_ID;
                    break;
                case LANDMARK_POSITION:
                    conf_data[LANDMARK_POSITION] = DEF_LANDMARK_POSITION;
                    break;
                case LANDMARK_ROTATION:
                    conf_data[LANDMARK_ROTATION] = DEF_LANDMARK_ROTATION;
                    break;
            }
        }
    }
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   load_config_file
 *          A config file is loaded.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
load_config_file()
{
    FILE *fp;
    char str[512];
    char *valuestr;
    int i = 0;

    for (i = 0; i < CONF_DATA_MAX; i++) {
        conf_data[i] = 0;
    }

    if ((fp = fopen(config_path, "r")) == NULL) {
        fprintf(stderr, "%s\n", "Error : can't open file.(samplenavi.conf)\n");
        set_default_data();
        // TODO for test
        uim_debug("USB_CAMERA_ID = %d", conf_data[USB_CAMERA_ID]);
        uim_debug("LANDMARK_POSITION = %d", conf_data[LANDMARK_POSITION]);
        uim_debug("LANDMARK_ROTATION = %d", conf_data[LANDMARK_ROTATION]);
        return;
    }

    while (fscanf(fp, "%s", str) != EOF) {
        if (strncmp(str, "USBCAMERAID", 11) == 0) {
            valuestr = strtok((char *)(str) + 12, "=");
            if(valuestr != NULL)
            {
                conf_data[USB_CAMERA_ID] = atoi(valuestr);
            }
        } else if (strncmp(str, "LANDMARKPOSITION", 16) == 0) {
            valuestr = strtok((char *)(str) + 17, "=");
            if(valuestr != NULL)
            {
                conf_data[LANDMARK_POSITION] = atoi(valuestr);
            }
        } else if (strncmp(str, "LANDMARKROTATION", 16) == 0) {
            valuestr = strtok((char *)(str) + 17, "=");
            if(valuestr != NULL)
            {
                conf_data[LANDMARK_ROTATION] = atoi(valuestr);
            }
        } else if (strncmp(str, "WEBSOCKETPORT", 13) == 0) {
            valuestr = strtok((char *)(str) + 14, "=");
            if(valuestr != NULL)
            {
                port = atoi(valuestr);
            }
        } else if (strncmp(str, "IPADDR", 6) == 0) {
            valuestr = strtok((char *)(str) + 7, "=");
            if(valuestr != NULL)
            {
                strcpy( addr, valuestr);
            }
        } else if (strncmp(str, "PROXYURI", 8) == 0) {
            valuestr = strtok((char *)(str) + 9, "=");
            if(valuestr != NULL)
            {
                strcpy( proxy_uri, valuestr);
            }
        }
    }

    fclose(fp);

    set_default_data();

    // TODO for test
    uim_debug("USB_CAMERA_ID = %d", conf_data[USB_CAMERA_ID]);
    uim_debug("LANDMARK_POSITION = %d", conf_data[LANDMARK_POSITION]);
    uim_debug("LANDMARK_ROTATION = %d", conf_data[LANDMARK_ROTATION]);
    uim_debug("WEBSOKET_PORT  = %d", port);
    uim_debug("IP_ADDR = %s", addr);
    uim_debug("PROXY_URI = %s", proxy_uri);

    return;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   e_ui_draw
 *          Draw UI Layer.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
e_ui_draw()
{
#ifdef _SHOW_FPS_
    char str[15];
#endif

/* Meter */
    int value = 0;
    int w_fig_r = 0;
    int w_fig_c = 0;
    int w_fig_l = 0;

    evas_object_show(ui_bg);
    evas_object_show(ui_btn);

/* Meter */
#ifdef DEBUG_METER_TEXT_DSP
    sprintf(str, "[kph]  %d", ui_vicspeed);
    evas_object_text_text_set(ui_vicspeed_text, str);
    evas_object_show(ui_vicspeed_text);
#else
    evas_object_show(ui_vicspeed_text);
    value = ui_vicspeed;
    if(value > 199)
    {
        uim_debug("MAX SPEED OVER [%d]",value);
        value = 199;
    }

    w_fig_r = (value % 10); value /= 10;
    w_fig_c = (value % 10); value /= 10;
    w_fig_l = (value % 10); value /= 10;

    if(w_fig_r != fig_r)
    {
        evas_object_hide(ui_meter_r[fig_r]);
        evas_object_show(ui_meter_r[w_fig_r]);
        fig_r = w_fig_r;
    }

    if(w_fig_c != fig_c)
    {
        evas_object_hide(ui_meter_c[fig_c]);

        if(w_fig_c != 0 ||  w_fig_l == 1)
        {
            evas_object_show(ui_meter_c[w_fig_c]);
        }

        fig_c = w_fig_c;
    }

    if(w_fig_l != fig_l)
    {
        evas_object_hide(ui_meter_l);

        if(w_fig_l != 0)
        {
            evas_object_show(ui_meter_l);
        }
                
        fig_l = w_fig_l;
    }
#endif
    if (enable_navi == TRUE && set_route == TRUE) {
        if (goal_square_length <= (GOAL_MESSAGE_LENGTH * GOAL_MESSAGE_LENGTH)) {
            evas_object_show(ui_goal_text);
        } else {
            evas_object_hide(ui_goal_text);
        }
    }

#ifdef _SHOW_FPS_
    sprintf(str, "FPS : %.2f", fps);
    evas_object_text_text_set(ui_fps, str);
    sprintf(str, "Polygon : %d", polygon_count);
    evas_object_text_text_set(ui_polygon_count, str);
#endif
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   e_ui_init
 *          Initialize UI Layer.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
e_ui_init()
{
#ifdef _SHOW_FPS_
    char str[15];
#endif
    int i = 0;

    ui_bg = evas_object_rectangle_add(e_ui);
    evas_object_color_set(ui_bg, 0, 34, 119, 255);
    evas_object_move(ui_bg, W_TEXTAREA_X, W_TEXTAREA_Y);
    evas_object_resize(ui_bg, W_TEXTAREA_WIDTH, W_TEXTAREA_HEIGHT);
    evas_object_show(ui_bg);
    evas_object_layer_set(ui_bg, LAYER_UI);

    ui_btn = evas_object_image_add(e_ui);
    evas_object_image_file_set(ui_btn, setting_button_path, NULL);
    evas_object_image_fill_set(ui_btn, 0, 0, W_NAVIBUTTON_WIDTH, W_NAVIBUTTON_HEIGHT);
    evas_object_move(ui_btn, W_NAVIBUTTON_X, W_NAVIBUTTON_Y);
    evas_object_resize(ui_btn, W_NAVIBUTTON_WIDTH, W_NAVIBUTTON_HEIGHT);
    evas_object_show(ui_btn);
    evas_object_layer_set(ui_btn, LAYER_UI);

    ui_goal_text = evas_object_image_add(e_ui);
    evas_object_image_file_set(ui_goal_text, goal_text_img_path, NULL);
    evas_object_image_fill_set(ui_goal_text, 0, 0, W_GOALMESSAGE_WIDTH, W_GOALMESSAGE_HEIGHT);
    evas_object_move(ui_goal_text, W_GOALMESSAGE_X, W_GOALMESSAGE_Y);
    evas_object_resize(ui_goal_text, W_GOALMESSAGE_WIDTH, W_GOALMESSAGE_HEIGHT);
    evas_object_hide(ui_goal_text);
    evas_object_layer_set(ui_goal_text, LAYER_UI);

/* Meter */
#ifdef DEBUG_METER_TEXT_DSP
    ui_vicspeed_text = evas_object_text_add(e_ui);
    evas_object_text_style_set(ui_vicspeed_text, EVAS_TEXT_STYLE_PLAIN);
    evas_object_color_set(ui_vicspeed_text, 255, 255, 255, 255);
    evas_object_text_font_set(ui_vicspeed_text, "Utopia", 28);
    sprintf(str, "[kph]  %d", ui_vicspeed);
    evas_object_text_text_set(ui_vicspeed_text, str);
    evas_object_move(ui_vicspeed_text, 0, 430);
    evas_object_resize(ui_vicspeed_text, W_GOALMESSAGE_WIDTH, W_GOALMESSAGE_HEIGHT);
//    evas_object_hide(ui_vicspeed_text);
    evas_object_show(ui_vicspeed_text);
    evas_object_layer_set(ui_vicspeed_text, LAYER_UI);
#else
    ui_meter_l = evas_object_image_add(e_ui);
    evas_object_image_file_set(ui_meter_l, meter_l_path, NULL);
    evas_object_image_fill_set(ui_meter_l, 0, 0, 300, 320);
    evas_object_move(ui_meter_l, 0, 300);
    evas_object_resize(ui_meter_l, 300, 320);
    evas_object_hide(ui_meter_l);
    evas_object_layer_set(ui_meter_l, LAYER_UI);

    for(i = 0; i < 10; i++)
    {
        ui_meter_c[i] = evas_object_image_add(e_ui);
        evas_object_image_file_set(ui_meter_c[i], meter_c_path[i], NULL);
        evas_object_image_fill_set(ui_meter_c[i], 0, 0, 300, 320);
        evas_object_move(ui_meter_c[i], 0, 300);
        evas_object_resize(ui_meter_c[i], 300, 320);
        evas_object_hide(ui_meter_c[i]);
        evas_object_layer_set(ui_meter_c[i], LAYER_UI);

        ui_meter_r[i] = evas_object_image_add(e_ui);
        evas_object_image_file_set(ui_meter_r[i], meter_r_path[i], NULL);
        evas_object_image_fill_set(ui_meter_r[i], 0, 0, 300, 320);
        evas_object_move(ui_meter_r[i], 0, 300);
        evas_object_resize(ui_meter_r[i], 300, 320);
        evas_object_hide(ui_meter_r[i]);
        evas_object_layer_set(ui_meter_r[i], LAYER_UI);
    }
	evas_object_show(ui_meter_r[0]);

    ui_vicspeed_text = evas_object_text_add(e_ui);
    evas_object_text_style_set(ui_vicspeed_text, EVAS_TEXT_STYLE_PLAIN);
    evas_object_color_set(ui_vicspeed_text, 255, 255, 255, 255);
    evas_object_text_font_set(ui_vicspeed_text, "Utopia", 24);
    evas_object_text_text_set(ui_vicspeed_text, "kph");
    evas_object_move(ui_vicspeed_text, 180, 440);
    evas_object_resize(ui_vicspeed_text, W_GOALMESSAGE_WIDTH, W_GOALMESSAGE_HEIGHT);
//    evas_object_hide(ui_vicspeed_text);
    evas_object_show(ui_vicspeed_text);
    evas_object_layer_set(ui_vicspeed_text, LAYER_UI);
#endif

#ifdef _SHOW_FPS_
    sprintf(str, "FPS : %.2f", fps);

    ui_fps_bg = evas_object_rectangle_add(e_ui);
    evas_object_color_set(ui_fps_bg, 255, 255, 255, 255);
    evas_object_move(ui_fps_bg, 0, 0);
    evas_object_resize(ui_fps_bg, 150, 20);
    evas_object_show(ui_fps_bg);
    evas_object_layer_set(ui_fps_bg, LAYER_UI);

    ui_fps = evas_object_text_add(e_ui);
    evas_object_text_style_set(ui_fps, EVAS_TEXT_STYLE_PLAIN);
    evas_object_color_set(ui_fps, 0, 0, 0, 255);
    evas_object_text_font_set(ui_fps, "Utopia", 14);
    evas_object_text_text_set(ui_fps, str);
    evas_object_move(ui_fps, 0, 0);
    evas_object_resize(ui_fps, 250, 250);
    evas_object_show(ui_fps);
    evas_object_layer_set(ui_fps, LAYER_UI);

    sprintf(str, "Polygon : %d", polygon_count);

    ui_polygon_count_bg = evas_object_rectangle_add(e_ui);
    evas_object_color_set(ui_polygon_count_bg, 255, 255, 255, 255);
    evas_object_move(ui_polygon_count_bg, 0, 20);
    evas_object_resize(ui_polygon_count_bg, 150, 20);
    evas_object_show(ui_polygon_count_bg);
    evas_object_layer_set(ui_polygon_count_bg, LAYER_UI);

    ui_polygon_count = evas_object_text_add(e_ui);
    evas_object_text_style_set(ui_polygon_count, EVAS_TEXT_STYLE_PLAIN);
    evas_object_color_set(ui_polygon_count, 0, 0, 0, 255);
    evas_object_text_font_set(ui_polygon_count, "Utopia", 14);
    evas_object_text_text_set(ui_polygon_count, str);
    evas_object_move(ui_polygon_count, 0, 20);
    evas_object_resize(ui_polygon_count, 250, 250);
    evas_object_show(ui_polygon_count);
    evas_object_layer_set(ui_polygon_count, LAYER_UI);
#endif
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   e_map_init
 *          Initialize MAP Layer.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
e_map_init()
{
    e_map = ecore_evas_get(window);
    browser = ewk_view_add(e_map);
    ewk_context_proxy_uri_set(ewk_view_context_get(browser), proxy_uri);
    evas_object_move(browser, W_MAP_ORIGINE_X, W_MAP_ORIGINE_Y);
    evas_object_resize(browser, W_WIDTH, W_MAP_HEIGHT);
    evas_object_show(browser);
    ewk_view_uri_set(browser, default_url);
    evas_object_layer_set(browser, LAYER_MAP);
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   e_map_draw
 *          Draw Map Layer(EWebKit2)
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
e_map_draw()
{
    evas_object_show(browser);
}

#ifdef _USE_OPENCV_
/*--------------------------------------------------------------------------*/
/*
 * @brief   convert_data_evas_cv
 *          Convert IplImage object to Evas_Object.
 *
 * @param[out]  eo              Evas_Object 
 * @param[in]   iplimagenone    IplImage object
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
convert_data_evas_cv(Evas_Object *eo, IplImage *iplimage)
{
    int image_data_size = (iplimage->width) * iplimage->height * 4;
    unsigned char image[image_data_size];
    int i = 0;

    if (image_data_size == 0) {
        uim_debug("convert_data_evas_cv: IplImage empty error.");
        return;
    }

    for (i = 0; i < (iplimage->width * iplimage->height); i++) {
        image[i*4] = iplimage->imageData[(3*i)];
        image[(i*4)+1] = iplimage->imageData[(3*i)+1];
        image[(i*4)+2] = iplimage->imageData[(3*i)+2];
        image[(i*4)+3] = 255;
    }

    /* convert to Evas_Object */
    evas_object_image_colorspace_set(eo, EVAS_COLORSPACE_ARGB8888);
    evas_object_image_size_set(eo, iplimage->width , iplimage->height);
    evas_object_image_data_set(eo, image);
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   evas_object_image_from_cv
 *          Load the picture to an IplImage object. 
 *
 * @param[out]  eo          Evas_Object 
 * @param[in]   filepath    path of the file
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
evas_object_image_from_cv(Evas_Object *eo, const char* filepath)
{
    IplImage *iplimage;

    /* load picture */
    iplimage = cvLoadImage(filepath, CV_LOAD_IMAGE_COLOR);
    if (iplimage == 0) {
        uim_debug("cvLoadImageError.");
    }

    convert_data_evas_cv(eo, iplimage);
}
#endif

/*--------------------------------------------------------------------------*/
/*
 * @brief   _time_interval_navi_cb
 *          main timer for navigation. 
 *
 * @param[out]  cam    Evas_Object 
 * @return      result
 * @retval      ECORE_CALLBACK_RENEW    Return value to keep a callback
 * @retval      ECORE_CALLBACK_CANCEL   Return value to remove a callback
 */
/*--------------------------------------------------------------------------*/
static Eina_Bool
_time_interval_navi_cb(void *cam)
{
    static double webcam_timer = 0;
    double carinfo_lat, carinfo_lon;
    int carinfo_dir;

    /* for calc fps */
#ifdef _SHOW_FPS_
    static double total_frame_time = 0;
    static int frame_count = 0;
    static double t0;
    double t1;
#endif

#ifdef _USE_CAMERA_
    webcam_timer += TIME_INTERVAL_AR;

    if(webcam_timer >= TIME_INTERVAL_CAMERA) {
        webcam_timer = 0;

        /* Capture from Camera */
        cam = evas_object_image_add(e_cam);

        captureImage = cvQueryFrame(capture);
        convert_data_evas_cv(cam, captureImage);

        evas_object_image_fill_set(cam, 0, 0, W_WIDTH, W_NAVI_HEIGHT);
        evas_object_move(cam, W_NAVI_ORIGINE_X, W_NAVI_ORIGINE_Y);
        evas_object_resize(cam, W_WIDTH, W_NAVI_HEIGHT);
        evas_object_show(cam);

    }
#else
    evas_object_image_fill_set(cam, 0, 0, W_WIDTH, W_NAVI_HEIGHT);
    evas_object_move(cam, W_NAVI_ORIGINE_X, W_NAVI_ORIGINE_Y);
    evas_object_resize(cam, W_WIDTH, W_NAVI_HEIGHT);
    evas_object_show(cam);
#endif

    getLocationFromAmb();

    calc_camera_coord();

    /* render 3D view */
    draw_route(e_3d);
    draw_landmark(e_3d);

    /* render UI view */
    e_ui_draw();

    /* render Map View */
    e_map_draw();

#ifdef _SHOW_FPS_
    // Moving Average
    if(frame_count == 0) {
        t0 = get_time();
        frame_count++;
    } else if (frame_count < 5) {
        t1 = get_time();
        total_frame_time += t1 - t0;
        t0 = t1;
        frame_count++;
    } else {
        t1 = get_time();
        total_frame_time -= total_frame_time / 4;
        total_frame_time += t1 - t0;
        t0 = t1;

        fps = 4.0 * 1.0 / total_frame_time;

    }
#endif

    return ECORE_CALLBACK_RENEW;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   _time_interval_map_renew_cb
 *          timer for renewal of a map.
 *
 * @param[out]  data    user data(UNUSED) 
 * @return      result
 * @retval      ECORE_CALLBACK_RENEW    Return value to keep a callback
 * @retval      ECORE_CALLBACK_CANCEL   Return value to remove a callback
 */
/*--------------------------------------------------------------------------*/
static Eina_Bool
_time_interval_map_renew_cb(void *data)
{
/* DEBUG.s */
#ifdef DEBUG_VIC_INFO_SET
    if(test_cnt < route_data_count)
    {
        test_lat = csv_route[test_cnt].lat;
        test_lon = csv_route[test_cnt].lon;
        camera_geocode.lat = test_lat;
        camera_geocode.lon = test_lon;
        test_cnt++ ;
    }
#endif    
/* DEBUG.e */

    if((socket_val) && ((map_pos.lat != camera_geocode.lat) || (map_pos.lon != camera_geocode.lon)))
    {
        map_pos.lat = camera_geocode.lat;
        map_pos.lon = camera_geocode.lon;

        uim_debug ("map_pos.lat = [%f]",map_pos.lat);
        uim_debug ("map_pos.lon = [%f]",map_pos.lon);

        event_message(socket_val, "CHG VAL VIC_INFO LAT %f LON %f", map_pos.lat, map_pos.lon );
        uim_debug ("SEND LAT LON TO SINARIO(samplenavi)");
    }

    return ECORE_CALLBACK_RENEW;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   callback_listener
 *          timer for libwebsocket connection surveillance.
 *
 * @param[out]  data    libwebsocket context 
 * @return      result
 * @retval      ECORE_CALLBACK_RENEW    Return value to keep a callback
 * @retval      ECORE_CALLBACK_CANCEL   Return value to remove a callback
 */
/*--------------------------------------------------------------------------*/
static Eina_Bool
callback_listener(void *data)
{
    if(connected) {
        libwebsocket_service((struct libwebsocket_context *)data, 0);
    } else {
        uim_debug("WEBSOCKET CONNECTED ERROR");
        if(context != NULL) {
            libwebsocket_context_destroy(context);
        }
        /* Server */
        context = libwebsocket_create_context(port, NULL,
                            protocols, libwebsocket_internal_extensions,
                            NULL, NULL, -1, -1, 0);
        
        if(context == NULL) {
            uim_debug("libwebsocket_create_context failed. (line:%d)", __LINE__);
            sleep(1);
            return 0;
        }

        connected = 1;
    }

    return ECORE_CALLBACK_RENEW;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   callback_http
 *          this callback function is notified from libwebsockets
 *
 * @param[in]   context    libwebsocket context 
 * @param[in]   wsi        libwebsockets management table
 * @param[in]   reason     event type 
 * @param[in]   user       intact 
 * @param[in]   in         receive message 
 * @param[in]   len        message size[BYTE]
 * @return      result
 * @retval      =0         success
 * @retval      =1         error
 */
/*--------------------------------------------------------------------------*/
static int
callback_http(struct libwebsocket_context *context, struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    int n = 0;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
                LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    char data[512];

    // TODO for test
    uim_debug("call callback_http! size = %d", len);
    uim_debug("50414 callback_http REASON %d", reason);

    switch(reason) {
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        uim_debug("CONNECTION %x", wsi);
        break;
    case LWS_CALLBACK_ESTABLISHED:
        uim_debug("ESTABLISHED %x", wsi);
        socket_val = wsi;
        n = sprintf((char *)p, "%s", "ANS HELLO");
        break;
    case LWS_CALLBACK_ADD_POLL_FD:
        uim_debug("RECIEVE REASON LWS_CALLBACK_ADD_POLL_FD");
        break;
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED");
        break;
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER");
        break;
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLIENT_ESTABLISHED");
        break;
    case LWS_CALLBACK_SET_MODE_POLL_FD:
        uim_debug("RECIEVE REASON LWS_CALLBACK_SET_MODE_POLL_FD");
        break;
    case LWS_CALLBACK_CLEAR_MODE_POLL_FD:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLEAR_MODE_POLL_FD");
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLIENT_WRITEABLE");
        break;
    case LWS_CALLBACK_RECEIVE:

        if( (in != NULL) && (strncmp( (char *)in, "OPEN", strlen("OPEN") ) == 0 )) {

            uim_debug("OPEN loading_edje_file [%s]",(char *)in);
            strncpy(edje_str, edje_parse_str(in, 1), sizeof(edje_str));
            loading_edje_file(&edje_str[0]);
            if(socket_val)
            {
                event_message(socket_val, "RESULT SUCCESS");
                uim_debug ("SEND RESULT SUCCESS");
            }
        }
        else if ( (in != NULL) && (strncmp("CLOSE", in, strlen("CLOSE")) == 0)) {
            evas_object_del(g_edje);
            uim_debug ("ONSCREEN CLOSE");
        }

        /* Get route from navi.js */
        if( strncmp( (char *)in, "<route>", strlen( "<route>" ) ) == 0 ) {
            uim_debug("<route> recv");
            init_e3d(e_3d, in, len);
            set_route = TRUE;
            break;
        }
        
        memset(data, 0, sizeof(data));
        strncpy(data, (char *)in, len);
        uim_debug("RECIEVE[%d] %s", len, data);

        if (len == 0) {
            uim_debug("ERROR data=null (line:%d)", __LINE__);
            break;
        }
        if (strncmp((char *)data, "CHG SEQ REQ_NAV", 15) == 0) {
            uim_debug("RECIEVE COMMAND CHG SEQ REQ_NAV");

            enable_navi = TRUE;
        } else if (strncmp((char *)data, "CHG SEQ END_NAV", 15) == 0) {
            uim_debug("RECIEVE COMMAND CHG SEQ END_NAV");

            enable_navi = FALSE;
        } else if(strncmp((char *)data, "ERR", 3) == 0) {
        }
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        memset(data, 0, sizeof(data));
        strncpy(data, (char *)in, len);
        uim_debug("CLIENT_RECIEVE[%d] %s", len, data);
        break;
    case LWS_CALLBACK_CLOSED:
        uim_debug("RECIEVE REASON LWS_CALLBACK_CLOSED");
        connected = 0;
        break;
    default:
        uim_debug("REASON %d", reason);
        break;
    }

    return 0;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   parse_elementary_value
 *          The message content received from DBus is acquired. 
 *
 * @param[out]   vic_val_p    Vehicles information data
 * @param[in]    iter         message received from DBus
 * @return       result
 * @retval       TRUE     success
 * @retval       FALSE    error
 */
/*--------------------------------------------------------------------------*/
static bool
parse_elementary_value(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    dbus_int32_t i32_val;
    dbus_int32_t i16_val;
    dbus_uint32_t u32_val;
    dbus_uint16_t u16_val;
    uint8_t byte_val;
    dbus_bool_t b_val;
    double d_val;
    char *s_val;
    char *w_s_val;

    char sig;

    if (vic_val_p == NULL || iter == NULL){
        uim_debug( "Err Parameter NG " );
        return FALSE;
    }

    sig = dbus_message_iter_get_arg_type(iter);

    switch (sig) {
    case DBUS_TYPE_INT32:
        dbus_message_iter_get_basic(iter, &i32_val);
        vic_val_p->i32_val = i32_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_INT16:
        dbus_message_iter_get_basic(iter, &i16_val);
        vic_val_p->i16_val = i16_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_UINT32:
        dbus_message_iter_get_basic(iter, &u32_val);
        vic_val_p->u32_val = u32_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_UINT16:
        dbus_message_iter_get_basic(iter, &u16_val);
        vic_val_p->u16_val = u16_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_BOOLEAN:
        dbus_message_iter_get_basic(iter, &b_val);
        vic_val_p->b_val = b_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_BYTE:
        dbus_message_iter_get_basic(iter, &byte_val);
        vic_val_p->byte_val = byte_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_DOUBLE:
        dbus_message_iter_get_basic(iter, &d_val);
        vic_val_p->d_val = d_val;
        vic_val_p++;
        break;
    case DBUS_TYPE_STRING:
#if 0
        dbus_message_iter_get_basic(iter, &s_val);
        w_s_val = (char *)malloc(strlen(s_val) + 1); 
        if(w_s_val == NULL) {
            uim_debug( "Err malloc" );
            return FALSE ;
        }
        strncpy(w_s_val, s_val, strlen(s_val));
        vic_val_p->s_val = w_s_val;
        vic_val_p++;
        break;
#endif
    default:
        uim_debug("Err parse_elementary_value: unknown type");
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   parse_value
 *          parse message received from DBus
 *
 * @param[out]   vic_val_p    Vehicles information data
 * @param[in]    iter         message received from DBus
 * @return       result
 * @retval       TRUE     success
 * @retval       FALSE    error
 */
/*--------------------------------------------------------------------------*/
static bool
parse_value(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    char curr;

    if (vic_val_p == NULL || iter == NULL){
        uim_debug( "Err Parameter NG " );
        return FALSE;
    }

    curr = dbus_message_iter_get_arg_type(iter);

    switch (curr) {
        case DBUS_TYPE_BYTE:
        case DBUS_TYPE_BOOLEAN:
        case DBUS_TYPE_INT16:
        case DBUS_TYPE_INT32:
        case DBUS_TYPE_UINT16:
        case DBUS_TYPE_UINT32:
        case DBUS_TYPE_DOUBLE:
        case DBUS_TYPE_STRING:
            return parse_elementary_value(vic_val_p, iter);
        case DBUS_TYPE_ARRAY:
        case DBUS_TYPE_STRUCT:
        case DBUS_TYPE_DICT_ENTRY:
            return FALSE;
        case DBUS_TYPE_INVALID:
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   getAmbDbus
 *          Vehicle information is acquired from AMB.(D-Bus)
 *
 * @param[in]    key          vic_data index
 * @param[out]   vic_val_p    Vehicles information data
 * @return       result
 * @retval       = 0      success
 * @retval       =-1      error
 */
/*--------------------------------------------------------------------------*/
static int
getAmbDbus(int key, union VicVal_t *vic_val_p)
{
    /* local variable */
    DBusMessage *message;
    DBusError error;    
    int result = 0;
    const char *v_string[] = {vic_data[key].interface_name,
                              vic_data[key].property_name  };
    const char *dest = Bus_name;
    DBusMessage *reply;
    int reply_timeout = 1000; /* Millisecond */
    DBusMessageIter iter;
    DBusMessageIter iter_array;
    union VicVal_t *tmp_vic_val_p  = vic_val_p;

    /* initialize */
    dbus_error_init(&error);

    if (NULL == g_connection) {
        /* obtain the right to use dbus */
        g_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

        if (g_connection == NULL){
            uim_debug( "Err dbus_bus_get" );

            /* Release err parameter */
            dbus_error_free(&error);
            return -1;
        }
    }

    /* Constructs a new message */
    message = dbus_message_new_method_call(DBUS_SERVICE,
                                           vic_data[key].path_name,
                                           DBUS_INTERFACE,
                                           DBUS_METHOD);
    if(message == NULL){
        uim_debug( "Err dbus_message_new_method_call" );

        /* Release the connection */
        dbus_connection_unref (g_connection);
        g_connection = NULL;  
        return -1;
    }

    /* Gets the type of a message */
    result = dbus_message_append_args(message,
                                      DBUS_TYPE_STRING,
                                      &v_string[0],
                                      DBUS_TYPE_STRING,
                                      &v_string[1],
                                      DBUS_TYPE_INVALID);

	if ( !result ){
        uim_debug( "Err dbus_message_append_args" );

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;
        /* Release the message */
        dbus_message_unref(message);

        return -1;
    }

    /* Gets the error name */
    if (dest && !dbus_message_set_destination (message, dest)){
        uim_debug("Err dbus_message_new_method_call");

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;

        /* Release the message */
        dbus_message_unref(message);

        return -1;
    }

    /* Queues a message to send */
    reply = dbus_connection_send_with_reply_and_block(g_connection,
                                                      message,
                                                      reply_timeout,
                                                      &error);
    if (reply == NULL){
//DEBUG        uim_debug( "Err dbus_connection_send_with_reply_and_block" );

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;

        /* Release the message */
        dbus_message_unref(message);

        /* Release err parameter */
        dbus_error_free(&error);

        return -1;
    }

    /* Gets the result */
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_recurse(&iter, &iter_array);

    /* Type conversion of the resulting value */
    result = parse_value(tmp_vic_val_p, &iter_array);
    if (result != TRUE) {
        uim_debug( "Err parse_elementary_value" );

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;

        /* Release the message */
        dbus_message_unref(message);
        dbus_message_unref(reply);

        return -1;
    }

    /* Release the message */
    dbus_message_unref(message);   
    dbus_message_unref(reply);   

    return 0;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   getLocationFromAmb
 *          Location information acquisition from AMB.
 *
 * @param    none
 * @return   none
 */
/*--------------------------------------------------------------------------*/
static void
getLocationFromAmb()
{
    union VicVal_t vic_val[32];
    int result = 0;
    int i;
    char vic_str[256];

    for(i = 0; i < property_num ; i++){
        result = getAmbDbus(i, vic_val);

        if (result != 0){
//DEBUG            uim_debug("Err getAmbDbus : %s",vic_data[i].name);
            continue;
        }

        switch(vic_data[i].property){
        case LATITUDE :
            if((camera_geocode.lat != vic_val[0].d_val) &&
               (vic_val[0].d_val != 0))
            {
                uim_debug ("%s(%f)",vic_data[i].name, vic_val[0].d_val);
                camera_geocode.lat = vic_val[0].d_val;
            }

            break;
        case LONGITUDE :
            if((camera_geocode.lon != vic_val[0].d_val) &&
               (vic_val[0].d_val != 0))
            {
                uim_debug ("%s(%f)",vic_data[i].name, vic_val[0].d_val);
                camera_geocode.lon = vic_val[0].d_val;
            }

            break;
        case DIRECTION :
            if(camera_geocode.dir != vic_val[0].i32_val)
            {
                uim_debug ("%s(%d)",vic_data[i].name, vic_val[0].i32_val);
                camera_geocode.dir = vic_val[0].i32_val;
            }
            break;
        case VEHICLESPEED :
//DEBUG            uim_debug ("%s(%d)",vic_data[i].name, vic_val[0].i32_val);
            if(ui_vicspeed != vic_val[0].i32_val)
            {
                uim_debug ("%s(%d)",vic_data[i].name, vic_val[0].i32_val);
                ui_vicspeed = vic_val[0].i32_val;
            }
            break;
        default :
            uim_debug ("ERROR no property : %s", vic_data[i].name);
            break;
        }

    }
    return;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   get_config
 *          The config file for vehicle information acquisition is read.
 *
 * @param    none
 * @return   result
 * @retval   = 0      success
 * @retval   =-1      error
 */
/*--------------------------------------------------------------------------*/
static int
get_config()
{
    FILE *fp;
    int k = 0;
    int j, m;
    char buff[512];
    char *tp; 
    char *clm = " \n"; 

    fp = fopen( CONFIG_FILE , "r" );
    if( fp == NULL ) {
        uim_debug( "File open error" );
        return -1;
    }

    for(m = 0 ; k < MAX_PARA_NUM ; m++){
        if( fgets( buff, sizeof(buff) - 2, fp) != NULL){
            tp = strtok( buff, clm );
            if(tp != NULL){
                if(tp[0] != '#' ){
                    for(j = 0; vic_key_data[j].id != -1; j++ ){
                        if(strcmp(tp, vic_key_data[j].name) == 0){
                            vic_data[k].property = vic_key_data[j].id;
                            strcpy( vic_data[k].name, tp);
                            strcpy( vic_data[k].path_name, strtok( NULL, clm ));
                            strcpy( vic_data[k].interface_name, strtok( NULL, clm ));
                            strcpy( vic_data[k].property_name, strtok(NULL,clm ));

                            uim_debug( "vic_data[%d].property=%d",k,vic_data[k].property );
                            uim_debug( "vic_data[%d].name=%s",k,vic_data[k].name );
                            uim_debug( "vic_data[%d].path_name=%s", k, vic_data[k].path_name);
                            uim_debug( "vic_data[%d].interface_name=%s", k,vic_data[k].interface_name);
                            uim_debug( "vic_data[%d].property_name=%s", k,vic_data[k].property_name);

                            k++;
                            break;
                        }
                    }
                    if(vic_key_data[j].id == -1){
                        uim_debug("Err config.txt Line:%d Unregistered parameter name",m+1);
                    } 
   
                }else{
                    uim_debug("config.txt Line:%d Comment out  '#'Discovery",m+1);
                }
            }else{
                uim_debug("config.txt Line:%d Comment out  Null line",m+1);
            }
        }else{
            uim_debug("config.txt The end of data reading");
            break;
        } 
    }
    fclose(fp);

    property_num = k;
    if( property_num == 0 ) {
        uim_debug( "config.txt No valid data");
        return -1;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   edje_parse_str
 *          A character string is decomposed by a delimiter.
 *
 * @param[in]    key          vic_data index
 * @param[in]    vic_val_p    Vehicles information data
 * @return       The pointer address to a lexical token
 * @retval       > 0      success
 * @retval       NULL     error
 */
/*--------------------------------------------------------------------------*/
static char* edje_parse_str(void *in, int arg_num)
{
    int i;
    unsigned char *data;

    uim_debug("edje_parse_str %s, arg = %d", in, arg_num);
    data = strtok(in, " ");
    /* arg_num : 0 to n */
    for (i = 0; i < arg_num; i++) {
        data = strtok( NULL, " ");
    }
    uim_debug("edje_parse_str data: %s",data);
    return data;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   event_message
 *          send message
 *
 * @param[in]   wsi                 libwebsockets management table to send
 * @param[in]   fromat              message to send
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
event_message(struct libwebsocket *wsi, char *format, ...)
{
    va_list list;
    char message[256];

    va_start(list, format);
    vsnprintf(message, sizeof(message), format, list);
    va_end(list);

    uim_debug("OnScreen: event_message wsi = %p, %s", wsi, message);
    if (wsi) {
        int n = 0;
        unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
                      LWS_SEND_BUFFER_POST_PADDING];
        unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

        n = sprintf((char *)p, "%s", message);
        n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
        uim_debug("OnScreen: libwebsocket_write return = %d", n);
    }
    else {
        uim_debug("OnScreen: wsi is not initialized");
    }

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   _touch_up_edje
 *          Touch-up event processing.
 *
 * @param[in]   data                user data
 * @param[in]   obj                 evas object of the button
 * @param[in]   event_info          evas event infomation
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
_touch_up_edje(void *data, Evas *evas , Evas_Object *obj, void *event_info)
{
    /* get name from userdata */
    if (data != NULL) {
        uim_debug("OnScreen: user data is %s", (const char *)data);

        if(socket_val)
        {
            event_message(socket_val, "TOUCH %s %s", edje_str, data);
            uim_debug ("SEND TOUCH %s ",data);
        }
    }
    else
    {
        uim_debug("OnScreen: user data is NULL");
    }
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   loading_edje_file
 *          Loading of edje file.
 *
 * @param[in]   edje_file    edje file
 * @return      result
 * @retval      = 0      success
 * @retval      =-1      error
 */
/*--------------------------------------------------------------------------*/
static int
loading_edje_file(const char *edje_file)
{
    Evas_Object *part;      /* part handle */
    Eina_List *group;       /* edje group list */
    Eina_List *list;        /* part list in edje */
    int group_count = 0;    /* group counter */
    int name_count = 0;     /* name counter */

    g_evas = ecore_evas_get(window);
    if (!g_evas) {
        uim_debug("OnScreen: could not create evas.");
        return -1;
    }

    /* delete edge */
    evas_object_del(g_edje);

    /* create and add object in canvas from edje */
    g_edje = edje_object_add(g_evas);
    if (!g_edje) {
        uim_debug("OnScreen: could not create edje object!");
        return -1;
    }

    /* get group list */
    group = edje_file_collection_list(edje_file);
    while (group != NULL)
    {
        /* Set the edj file */
        if (!edje_object_file_set(g_edje, edje_file, (const char *)group->data)) {
            int err = edje_object_load_error_get(g_edje);
            const char *errmsg = edje_load_error_str(err);
            uim_debug("OnScreen: could not load %s: %s", edje_file, errmsg);

            edje_file_collection_list_free(group);
            evas_object_del(g_edje);
            return -1;
        }
        uim_debug("OnScreen: group[%d] data : %s", group_count, (const char *)group->data);

        /* get list */
        list = edje_object_access_part_list_get(g_edje);
        while (list != NULL) {
            uim_debug("OnScreen: list[%d] data : %s", name_count, (const char *)list->data);

            /* set callback for part name */
            part = edje_object_part_object_get((const Evas_Object *)g_edje, (const char *)list->data);
            if(part != NULL) {
                uim_debug("OnScreen: list[%d] name : %s", name_count, (const char *)list->data);
                evas_object_event_callback_add(part, EVAS_CALLBACK_MOUSE_UP, _touch_up_edje, list->data);
            }
            else {
                uim_debug("OnScreen: list[%d] is NULL", name_count);
            }

            /* to next list */
            list = list->next;
            name_count++;
        }

        /* to next group */
        group = group->next;
        group_count++;
    }
    uim_debug("OnScreen: group num is %d", group_count);
    uim_debug("OnScreen: name num is %d", name_count);

    /* Put in the image */
    evas_object_move(g_edje, 0, 0);
    /* Resize the image */
    evas_object_resize(g_edje, W_WIDTH, W_HEIGHT);
    /* Show the image */
    evas_object_show(g_edje);

    evas_object_layer_set(g_edje, LAYER_UI);

    /* Show the window */
    /* ecore_evas_show(g_window); */

    return 0;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   res_callback
 *          Notice event processing of a resource state.
 *
 * @param[in]   info                rsource information
 * @param[in]   user_data           user data(UNUSED)
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
res_callback(ico_apf_resource_notify_info_t* info, void *user_data)
{
    int ret;

    uim_debug("##==> Callbacked evt=%d res=%d id=%d bid=%d appid=%s dev=%s"
                " user_data=%d", info->state, info->resid, info->id, info->bid,
                info->appid, info->device, (int)user_data);

    switch (info->state)    {
    case ICO_APF_RESOURCE_STATE_ACQUIRED:
    case ICO_APF_RESOURCE_STATE_DEPRIVED:
    case ICO_APF_RESOURCE_STATE_WAITTING:
    case ICO_APF_RESOURCE_STATE_RELEASED:
        if (info->resid == ICO_APF_RESID_INT_SCREEN) {
            ret = ico_apf_resource_reply_int_screen_mode(info->device, info->bid, info->id, 1);
            uim_debug("##==> callback reply int_screen(%s,%d,%d,1) = %d",
                        info->device, info->bid, info->id, ret);
        }
        else if (info->resid == ICO_APF_RESID_ON_SCREEN) {
            ret = ico_apf_resource_reply_int_screen_mode_disp(info->device, info->id, 1);
            uim_debug("##==> callback reply on_screen(%s,%d,1) = %d",
                        info->device, info->id, ret);
        }
        else    {
            ret = ico_apf_resource_reply_screen_mode(info->device, info->id, 1);
            uim_debug("##==> callback reply screen(%s,%d,1) = %d",
                        info->device, info->id, ret);
        }
        break;
    default:
        /* NOP  */
        break;
    }
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   sample navigation application
 *          main routine
 *
 * @param   main() finction's standard parameter (argc,argv)
 * @return  result
 * @retval  =0       success
 * @retval  =-1      failed
 */
/*--------------------------------------------------------------------------*/
int
main(int argc, char *argv[])
{
    int i;
    int getscreen;
    char    appid[ICO_UXF_MAX_PROCESS_NAME+1];
    int ret = 0;
    static struct libwebsocket *wsi;
    connected = 0;

    /* Setting the log output */
    if (ico_apf_get_app_id(0, appid) == ICO_APP_CTL_E_NONE) {
        ico_apf_log_open(appid);
    }

    load_config_file();

    elm_init(argc, argv);

    // TODO
    enable_navi = FALSE;
    set_route = FALSE;

    /* initialize */
    if(0 != get_config()){
        uim_debug( "ERROR get_config()" );
        return -1;
    }

    getscreen = 0;
        sscrntype[0] = 0;
    for (i = 1; i < argc; i++)  {
        if (argv[i][0] == '-')  {
            if (strcasecmp(argv[i], "-basescreen") == 0) {
                getscreen = 1;              /* get base screen */
                                strcpy(sscrntype, "BasicScreen");
                uim_debug("BasicScreen");
            }
            else if (strcasecmp(argv[i], "-intscreen") == 0) {
                getscreen = 2;              /* get interrupt screen */
                                strcpy(sscrntype, "IntScreen");
            }
            else if (strcasecmp(argv[i], "-onscreen") == 0) {
                getscreen = 3;              /* get on screen */
                                strcpy(sscrntype, "OnScreen");
            }
        }
    }

    ecore_evas_init();

    if (getscreen > 0) {
        /* initialize resource control for Ecore */
        if (ico_apf_ecore_init(NULL) != ICO_APF_E_NONE) {
            uim_debug("ico_apf_ecore_init() Error");
            ecore_evas_shutdown();
            return -1;
        }

        /* set resource request callback */
        ico_apf_resource_set_event_cb(res_callback, NULL);

        /* acquire a right to display a screen */
        if (getscreen == 1) {
            ret = ico_apf_resource_get_screen_mode(NULL, 0);
        }
        else if (getscreen == 2) {
            ret = ico_apf_resource_get_int_screen_mode(NULL, 0, 0);
        }
        else {
            ret = ico_apf_resource_get_int_screen_mode_disp(NULL, 0);
        }
    }
    uim_debug("getscreen = %d, ret = %d",getscreen, ret);

    /* window setup */
    window = ecore_evas_new(NULL, 0, 0, W_WIDTH, W_HEIGHT, "frame=0");
    if (!window) goto error;

    ecore_evas_show(window);

    /* Camera Layer */
    e_cam = ecore_evas_get(window);
    cam = evas_object_image_add(e_cam);
#ifdef _USE_CAMERA_
    capture = cvCreateCameraCapture(conf_data[USB_CAMERA_ID]);
    if (!capture) {
        uim_debug("cvCaptureFromCAM failed.");
        return -1;
    }

    captureImage = cvQueryFrame(capture);
    uim_debug("camera width = %d, height = %d", captureImage->width, captureImage->height);
    convert_data_evas_cv(cam, captureImage);
#else

#ifdef _USE_OPENCV_
    evas_object_image_from_cv(cam, test_camera_path);
#else
	evas_object_image_file_set(cam, test_camera_path, NULL);
#endif

#endif
    evas_object_image_fill_set(cam, 0, 0, W_WIDTH, W_NAVI_HEIGHT);
    evas_object_move(cam, W_NAVI_ORIGINE_X, W_NAVI_ORIGINE_Y);
    evas_object_resize(cam, W_WIDTH, W_NAVI_HEIGHT);

    evas_object_show(cam);

    /* 3D Layer */
    e_3d = ecore_evas_get(window);

    /* UI Layer */
    e_ui = ecore_evas_get(window);
    e_ui_init();

    /* Server */
    context = libwebsocket_create_context(port, NULL, protocols, libwebsocket_internal_extensions,
                        NULL, NULL, -1, -1, 0);
    if (context == NULL) {
        fprintf(stderr, "libwebsocket_create_context failed.");     goto error;
    }

    connected = 1;
    ecore_timer_add(0.05, callback_listener, context);

    /* Browser(Map) Layer */
    e_map_init();

    edje_init();

    /* timer */
    ecore_timer_add(TIME_INTERVAL_AR, _time_interval_navi_cb, cam);

    ecore_timer_add(TEST_TIME, _time_interval_map_renew_cb, NULL);

    /* main loop */
    ecore_main_loop_begin();

    /* cleanup */
    e3d_cleanup();

#ifdef _USE_CAMERA_
    cvReleaseCapture(&capture);
#endif

    if (NULL != g_connection)
    {
        dbus_connection_unref(g_connection);
        g_connection = NULL;
    }

    ico_apf_ecore_term();

    ecore_evas_free(window);
    ecore_evas_shutdown();

    return 0;

error:
    fprintf(stderr, "Evas engine error.");
    ecore_evas_shutdown();
    return -1;
}

