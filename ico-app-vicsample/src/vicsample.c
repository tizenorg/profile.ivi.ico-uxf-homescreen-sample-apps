/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   Vehicles Sample APP
 *          Vehicles information is displayed.
 *
 * @date    Mar-04-2013
 */

#include <Elementary.h>
#include <E_DBus.h>
#include <app.h>
#include <dbus/dbus.h>
#include <string.h>
#include <stdbool.h>
#include <bundle.h>
#include <aul.h>
#include "ico_log.h"
#include "ico_dbus_amb_efl.h"

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
#define STATUS_BAR_HEIGHT   (64)
#define CTRL_BAR_HEIGHT     (128)
#define BAR_HEIGHT          (STATUS_BAR_HEIGHT + CTRL_BAR_HEIGHT)
#define WIDTH               (1080)      /* Base Background width  */
#define HEIGHT              (1920 - BAR_HEIGHT) /* Base Background height */

/* Base */
#define CTG_BX_W            530
#define CTG_BX_H            250
#define VIC_LI_W            1060
#define VIC_LI_H            1240

#define CTG_BX_X            10
#define CTG_BX_Y            40
#define VIC_LI_X            10
#define VIC_LI_Y            450

/* font */
#define FONT_SIZE           48
#define FONT_FAMILY         (char *)"Sans"      /* Mono, Sans, Serif */

/* Text class name */
#define TEXT_BUTTON         (char *)"button"
#define TEXT_LIST_ITEM      (char *)"list_item"
#define TEXT_LABEL          (char *)"label"

/* max size of vehicle information */
#define MAX_PARA_NUM        32

/* vehicle information ID */
#define VEHICLESPEED             0
#define ACCELERATIONX            1
#define SHIFTPOSITION            2
#define ENGINESPEED              3
#define LATITUDE                 4
#define ALTITUDE                 5
#define GEARPOSITION             6
#define LONGITUDE                7
#define MODE                     8
#define DIRECTION                9
#define WHEELBRAKEPRESSURE       10
#define LEFTTURNLIGHT            11
#define RIGHTTURNLIGHT           12
#define BRAKESIGNAL              13
#define ACCELERATIONY            14
#define ACCELERATIONZ            15
#define EXTERIORBRIGHTNESS       16
#define TURNSIGNAL               17
#define ENGINECOOLANTTEMPERATURE 18
#define STEERINGWHEELANGLE       19
#define HEADLIGHT                20
#define PARKINGLIGHT             21

/* maximum categories */
#define MAX_CATEGORY_NUM    8

/* vehicle information Parameter Category */
#define DRIVINGSAFETY       0
#define ELECTRICVEHICLE     1
#define ENVIRONMENT         2
#define MAINTENANCE         3
#define PARKING             4
#define PERSONALIZATION     5
#define RUNNINGSTATUS       6
#define VEHICLEINFO         7

/* Definition of files */
#define BASE_DIR            "/usr/apps/org.tizen.ico.app-vicsample"
#define IMAGE_DIR           BASE_DIR"/res/images"
#define CONFIG_FILE         BASE_DIR"/res/vicsample_config.txt"
#define BG_IMAGE_FILE       IMAGE_DIR"/vicinfo_bg.png"

/* Package name */
#define PACKAGE             "org.tizen.ico.app-vicsample"

#define DMY_DATA            "000000000000000"
#define DIGIT_NUM_OVER_DATA "############"
#define MAX_DRAW_VAL_LEM    15
#define MAX_NAME_LEM        32

/*============================================================================*/
/* Define data types                                                          */
/*============================================================================*/
struct vic_data_t
{
    int idx;
    int property;
    int category;
    dbus_type type;
    int zone;
    char name[MAX_NAME_LEM];
    char objpath_name[MAX_NAME_LEM];
    char property_name[MAX_NAME_LEM];
};

struct vic_key_data_t
{
    int id;
    int category;
    int type;
    char name[MAX_NAME_LEM];
};

struct vic_category_data_t
{
    int category;
    char name[MAX_NAME_LEM];
};

struct appdata_t
{
    Evas_Object *win;           //main window
    Evas_Object *bg;

    Evas_Object *ctg_bx;
    Evas_Object *ctg_bx2;
    Evas_Object *ctg_btn[MAX_CATEGORY_NUM];
    Evas_Object *vic_ini_li;
    Evas_Object *vic_li[MAX_CATEGORY_NUM];
    Evas_Object *vic_val_text[MAX_PARA_NUM];
    Evas_Object *vic_val_dmy_text[MAX_CATEGORY_NUM];
};

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static void on_ctg_mousedown(void *data, Evas_Object *obj, void *event_info);
static void amb_get_property_cb(const char *objectname, const char *property,
                                dbus_type type,
                                union dbus_value_variant value,
                                void *user_data);
static void amb_subscribe_property_cb(const char *objectname,
                                      const char *property, dbus_type type,
                                      union dbus_value_variant value,
                                      int sequence, struct timeval tv,
                                      void *user_data);
static void set_vic_text_data(Evas_Object *obj, const char *text);
static void set_vic_data(union dbus_value_variant value, void *user_data);
static int amb_get_property(int key);
static int amb_subscribe_property(int key);
static Eina_Bool amb_init(void);
static int get_config(void);
static void win_del(void *data, Evas_Object *obj, void *event_info);
static void elmListCreate(void);
static void winCreate(void);
static Evas_Object *create_win(const char *name);

/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
static struct vic_data_t vic_data[MAX_PARA_NUM];
static int property_num = 0;
static int ListDispSts = -1;
static struct appdata_t Ad;

const struct vic_category_data_t vic_category_data[MAX_CATEGORY_NUM] = {
    {DRIVINGSAFETY, "Driving safety"},
    {ELECTRICVEHICLE, "Electric Vehicle"},
    {ENVIRONMENT, "Environment"},
    {MAINTENANCE, "Maintenance"},
    {PARKING, "Parking"},
    {PERSONALIZATION, "Personalization"},
    {RUNNINGSTATUS, "Running Status"},
    {VEHICLEINFO, "Vehicle Info"}
};

const struct vic_key_data_t vic_key_data[] = {
    {VEHICLESPEED, RUNNINGSTATUS, DBUS_TYPE_UINT16, "VehicleSpeed"},
    {ACCELERATIONX, RUNNINGSTATUS, DBUS_TYPE_UINT16, "AccelerationX"},
    {SHIFTPOSITION, RUNNINGSTATUS, DBUS_TYPE_INT32, "ShiftPosition"},
    {ENGINESPEED, RUNNINGSTATUS, DBUS_TYPE_UINT16, "EngineSpeed"},
    {LATITUDE, RUNNINGSTATUS, DBUS_TYPE_DOUBLE, "Latitude"},
    {ALTITUDE, RUNNINGSTATUS, DBUS_TYPE_DOUBLE, "Altitude"},
    {GEARPOSITION, RUNNINGSTATUS, DBUS_TYPE_INT32, "GearPosition"},
    {LONGITUDE, RUNNINGSTATUS, DBUS_TYPE_DOUBLE, "Longitude"},
    {MODE, RUNNINGSTATUS, DBUS_TYPE_INT32, "Mode"},
    {DIRECTION, RUNNINGSTATUS, DBUS_TYPE_UINT16, "Direction"},
    {WHEELBRAKEPRESSURE, RUNNINGSTATUS, DBUS_TYPE_UINT16,
     "WheelBrakePressure"},
    {LEFTTURNLIGHT, RUNNINGSTATUS, DBUS_TYPE_BOOLEAN, "LeftTurnLight"},
    {RIGHTTURNLIGHT, RUNNINGSTATUS, DBUS_TYPE_BOOLEAN, "RightTurnLight"},
    {BRAKESIGNAL, RUNNINGSTATUS, DBUS_TYPE_BOOLEAN, "BrakeSignal"},
    {ACCELERATIONY, RUNNINGSTATUS, DBUS_TYPE_UINT16, "AccelerationY"},
    {ACCELERATIONZ, RUNNINGSTATUS, DBUS_TYPE_UINT16, "AccelerationZ"},
    {EXTERIORBRIGHTNESS, ENVIRONMENT, DBUS_TYPE_UINT16, "ExteriorBrightness"},
    {TURNSIGNAL, RUNNINGSTATUS, DBUS_TYPE_INT32, "TurnSignal"},
    {ENGINECOOLANTTEMPERATURE, RUNNINGSTATUS, DBUS_TYPE_INT32,
     "EngineCoolantTemperature"},
    {STEERINGWHEELANGLE, RUNNINGSTATUS, DBUS_TYPE_UINT16,
     "SteeringWheelAngle"},
    {HEADLIGHT, RUNNINGSTATUS, DBUS_TYPE_BOOLEAN, "HeadLight"},
    {PARKINGLIGHT, RUNNINGSTATUS, DBUS_TYPE_BOOLEAN, "ParkingLight"},
    {-1, -1, -1, "END"}
};

/*============================================================================*/
/* Function                                                                   */
/*============================================================================*/
/**
 * @brief on_ctg_mousedown
 */
static void on_ctg_mousedown(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("on_ctg_mousedown Enter");

    int category = -1;
    if (data != NULL) {
        category = *((int *) data);
    }

    if ((category != -1) && (category != ListDispSts)) {
        if (ListDispSts != -1) {
            evas_object_color_set(Ad.ctg_btn[ListDispSts], 255, 255, 255,
                                  255);
            evas_object_hide(Ad.vic_li[ListDispSts]);
        }
        else {
            evas_object_hide(Ad.vic_ini_li);
            elm_list_clear(Ad.vic_ini_li);
        }
        evas_object_color_set(Ad.ctg_btn[category], 0, 255, 255, 255);
        elm_list_go(Ad.vic_li[category]);
        evas_object_show(Ad.vic_li[category]);
        ListDispSts = category;
    }

    ICO_DBG("on_ctg_mousedown Leave");
    return;
}

/**
 * @brief amb_get_property_cb
 */
static void amb_get_property_cb(const char *objectname, const char *property,
                                dbus_type type,
                                union dbus_value_variant value,
                                void *user_data)
{
    ICO_DBG("amb_get_property_cb Enter");

    if (user_data == NULL) {
        ICO_ERR("No user data");
        return;
    }

    set_vic_data(value, user_data);

    ICO_DBG("amb_get_property_cb Leave");
    return;
}

/**
 * @brief amb_subscribe_property_cb
 */
static void amb_subscribe_property_cb(const char *objectname,
                                      const char *property, dbus_type type,
                                      union dbus_value_variant value,
                                      int sequence, struct timeval tv,
                                      void *user_data)
{
    ICO_DBG("amb_subscribe_property_cb Enter");

    if (user_data == NULL) {
        ICO_ERR("No user data");
        return;
    }

    set_vic_data(value, user_data);

    ICO_DBG("amb_subscribe_property_cb Leave");
    return;
}

/**
 * @brief set_vic_text_data
 */
static void set_vic_text_data(Evas_Object *obj, const char *text)
{
    ICO_DBG("CHG_VIC_INF set_vic_text_data Enter");

    if (obj == NULL) {
        ICO_ERR("Parameter NG obj NULL");
        return;
    }

    if (text == NULL) {
        ICO_ERR("Parameter NG text NULL");
        return;
    }

    if (strlen(text) <= MAX_DRAW_VAL_LEM) {
        elm_object_text_set(obj, text);
    }
    else {
        elm_object_text_set(obj, DIGIT_NUM_OVER_DATA);
    }

    ICO_DBG("CHG_VIC_INF set_vic_text_data Leave");
    return;
}

/**
 * @brief set_vic_data
 */
static void set_vic_data(union dbus_value_variant value, void *user_data)
{
    ICO_DBG("CHG_VIC_INF set_vic_data Enter");

    int idx = -1;
    char vic_str[256];

    if (user_data == NULL) {
        ICO_ERR("No user data");
        return;
    }

    idx = *((int *) (user_data));

    if (idx < 0 || idx > property_num) {
        ICO_ERR("Inaccurate user data :idx = %d", idx);
        return;
    }

    switch (vic_data[idx].property) {
    case SHIFTPOSITION:
    case GEARPOSITION:
    case MODE:
    case TURNSIGNAL:
    case ENGINECOOLANTTEMPERATURE:
        ICO_DBG("CHG_VIC_INF %s(D-bus I/F Result) = %d", vic_data[idx].name,
                value.i32val);
        sprintf(vic_str, "%d", value.i32val);
        set_vic_text_data(Ad.vic_val_text[idx], vic_str);
        break;

    case VEHICLESPEED:
    case ENGINESPEED:
    case DIRECTION:
    case ACCELERATIONX:
    case ACCELERATIONY:
    case ACCELERATIONZ:
    case WHEELBRAKEPRESSURE:
    case EXTERIORBRIGHTNESS:
    case STEERINGWHEELANGLE:
        ICO_DBG("CHG_VIC_INF %s(D-bus I/F Result) = %d", vic_data[idx].name,
                value.ui16val);
        sprintf(vic_str, "%d", value.ui16val);
        set_vic_text_data(Ad.vic_val_text[idx], vic_str);
        break;

//    case XXXXXXXX:
//        ICO_DBG("CHG_VIC_INF %s(D-bus I/F Result) = %d", vic_data[idx].name,
//                  value.yval);
//        sprintf(vic_str, "%d", value.yval);
//        set_vic_text_data(Ad.vic_val_text[idx], vic_str);
//        break;

    case LATITUDE:
    case ALTITUDE:
    case LONGITUDE:
        ICO_DBG("CHG_VIC_INF %s(D-bus I/F Result) = %f", vic_data[idx].name,
                value.dval);
        sprintf(vic_str, "%f", value.dval);
        set_vic_text_data(Ad.vic_val_text[idx], vic_str);
        break;

    case HEADLIGHT:
    case LEFTTURNLIGHT:
    case RIGHTTURNLIGHT:
    case PARKINGLIGHT:
    case BRAKESIGNAL:
        ICO_DBG("CHG_VIC_INF %s(D-bus I/F Result) = %d", vic_data[idx].name,
                value.bval);
        if (value.bval == TRUE) {
            sprintf(vic_str, "%s", "true");
        }
        else {
            sprintf(vic_str, "%s", "false");
        }
        set_vic_text_data(Ad.vic_val_text[idx], vic_str);
        break;

    default:
        ICO_ERR("Err no property : vic_data[%d]property  = %d", idx,
                vic_data[idx].property);
        break;
    }

    ICO_DBG("CHG_VIC_INF set_vic_data Leave");
    return;
}

/**
 * @brief amb_get_property
 */
static int amb_get_property(int key)
{
    ICO_DBG("amb_get_property Enter");

    int result = 0;

    result = ico_dbus_amb_get(vic_data[key].objpath_name,
                              vic_data[key].property_name,
                              vic_data[key].zone,
                              vic_data[key].type,
                              amb_get_property_cb,
                              (void *) (&(vic_data[key].idx))
        );

    if (result != 0) {
        ICO_ERR("Can't make dbus get message.");
        return -1;
    }

    ICO_DBG("amb_get_property Leave");
    return 0;
}

/**
 * @brief amb_subscribe_property
 */
static int amb_subscribe_property(int key)
{
    ICO_DBG("amb_subscribe_property Enter");

    int result = 0;

    result = ico_dbus_amb_subscribe(vic_data[key].objpath_name,
                                    vic_data[key].property_name,
                                    vic_data[key].zone,
                                    vic_data[key].type,
                                    amb_subscribe_property_cb,
                                    (void *) (&(vic_data[key].idx))
        );

    if (result != 0) {
        ICO_ERR("Can't make dbus subscribe message.");
        return -1;
    }

    ICO_DBG("amb_subscribe_property Leave");
    return 0;
}

/**
 * @brief amb_init
 */
static Eina_Bool amb_init(void)
{
    ICO_DBG("amb_init Enter");

    int i = 0;
    int result = 0;

    result = ico_dbus_amb_start();
    if (result != 0) {
        ICO_ERR("Can't get dbus bus.");
        return EINA_FALSE;
    }

    /* First get properties */
    for (i = 0; i < property_num; i++) {
        result = amb_get_property(i);
        if (result != 0) {
            ICO_ERR("amb_get_property : get ng [%s]", vic_data[i].name);
        }
    }

    /* Signal registration */
    for (i = 0; i < property_num; i++) {
        result = amb_subscribe_property(i);
        if (result != 0) {
            ICO_ERR("amb_subscribe_property : subscribe ng [%s]",
                    vic_data[i].name);
        }
    }

    ICO_DBG("amb_init Leave");
    return EINA_TRUE;
}

/**
 * @brief get_config
 */
/* Read configuration file */
static int get_config(void)
{
    ICO_DBG("get_config Enter");

    FILE *fp;
    int k = 0;
    int j = 0;
    int m = 0;
    char buff[512];
    char *tp;
    char *clm = " \n";

    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        ICO_ERR("File open error");
        return -1;
    }

    for (m = 0; k < MAX_PARA_NUM; m++) {
        if (fgets(buff, sizeof(buff) - 2, fp) == NULL) {
            ICO_DBG("vicsample_config.txt The end of data reading");
            break;
        }

        tp = strtok(buff, clm);
        if (tp == NULL) {
            ICO_DBG("vicsample_config.txt Line:%d Comment out  Null line",
                    m + 1);
            continue;
        }

        if (tp[0] == '#') {
            ICO_DBG("vicsample_config.txt Line:%d Comment out  '#'Discovery",
                    m + 1);
            continue;
        }

        for (j = 0; vic_key_data[j].id != -1; j++) {
            if (strcmp(tp, vic_key_data[j].name) == 0) {
                vic_data[k].idx = k;
                vic_data[k].property = vic_key_data[j].id;
                vic_data[k].category = vic_key_data[j].category;
                vic_data[k].type = vic_key_data[j].type;
                strcpy(vic_data[k].name, tp);
                strcpy(vic_data[k].objpath_name, strtok(NULL, clm));
                strcpy(vic_data[k].property_name, strtok(NULL, clm));
                vic_data[k].zone = atoi(strtok(NULL, clm));

                ICO_DBG("vic_data[%d].idx=%d", k, vic_data[k].idx);
                ICO_DBG("vic_data[%d].property=%d", k, vic_data[k].property);
                ICO_DBG("vic_data[%d].category=%d", k, vic_data[k].category);
                ICO_DBG("vic_data[%d].type=%d", k, vic_data[k].type);
                ICO_DBG("vic_data[%d].zone=%d", k, vic_data[k].zone);
                ICO_DBG("vic_data[%d].name=%s", k, vic_data[k].name);
                ICO_DBG("vic_data[%d].objpath_name=%s", k,
                        vic_data[k].objpath_name);
                ICO_DBG("vic_data[%d].property_name=%s", k,
                        vic_data[k].property_name);
                k++;
                break;
            }
        }

        if (vic_key_data[j].id == -1) {
            ICO_ERR("Err vicsample_config.txt Line:%d Unregistered"
                    " parameter name", m + 1);
        }
    }

    fclose(fp);

    property_num = k;
    if (property_num == 0) {
        ICO_ERR("vicsample_config.txt No valid data");
        return -1;
    }

    ICO_DBG("get_config Leave");
    return 0;
}

/**
 * @brief win_del
 */
static void win_del(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("win_del Enter");

    elm_exit();

    ICO_DBG("win_del Leave");
    return;
}

/**
 * @brief app_terminate
 */
static void app_terminate(void *data)
{
    ICO_DBG("app_terminate Enter");

    // Release all resources
    int i = 0;
    int result = 0;

    if (Ad.win) {
        evas_object_del(Ad.win);
        Ad.win = NULL;
    }

    if (Ad.bg) {
        evas_object_del(Ad.bg);
        Ad.bg = NULL;
    }

    if (Ad.ctg_bx) {
        evas_object_del(Ad.ctg_bx);
        Ad.ctg_bx = NULL;
    }

    if (Ad.ctg_bx2) {
        evas_object_del(Ad.ctg_bx2);
        Ad.ctg_bx2 = NULL;
    }

    for (i = 0; i < MAX_CATEGORY_NUM; i++) {
        if (Ad.ctg_btn[i]) {
            evas_object_del(Ad.ctg_btn[i]);
            Ad.ctg_btn[i] = NULL;
        }

        if (Ad.vic_li[i]) {
            evas_object_del(Ad.vic_li[i]);
            Ad.vic_li[i] = NULL;
        }

        if (Ad.vic_val_dmy_text[i]) {
            evas_object_del(Ad.vic_val_dmy_text[i]);
            Ad.vic_val_dmy_text[i] = NULL;
        }
    }

    if (Ad.vic_ini_li) {
        evas_object_del(Ad.vic_ini_li);
        Ad.vic_ini_li = NULL;
    }

    for (i = 0; i < MAX_PARA_NUM; i++) {
        if (Ad.vic_val_text[i]) {
            evas_object_del(Ad.vic_val_text[i]);
            Ad.vic_val_text[i] = NULL;
        }
    }

    for (i = 0; i < property_num; i++) {
        result =
            ico_dbus_amb_unsubscribe(vic_data[i].objpath_name,
                                     vic_data[i].property_name,
                                     vic_data[i].zone);

        if (result != 0) {
            if (result == -1) {
                ICO_ERR("Valid vehicle information name is not entered.");
            }
            else if (result == -2) {
                ICO_ERR("property is not subscribe.");
            }
        }
    }
    result = ico_dbus_amb_end();

    ICO_DBG("app_terminate Leave");
    return;
}

/**
 * @brief elmListCreate
 */
static void elmListCreate(void)
{
    ICO_DBG("elmListCreate Enter");

    int i;

    if (NULL == Ad.win) {
        ICO_ERR("main window is un-creating.");
        return;
    }

    for (i = 0; i < property_num; i++) {
        Ad.vic_val_text[i] = elm_label_add(Ad.win);
        elm_list_item_append(Ad.vic_li[vic_data[i].category],
                             vic_data[i].name, NULL, Ad.vic_val_text[i], NULL,
                             NULL);
    }

    /* dummy set */
    for (i = 0; i < MAX_CATEGORY_NUM; i++) {
        Ad.vic_val_dmy_text[i] = elm_label_add(Ad.win);
        elm_object_text_set(Ad.vic_val_dmy_text[i], DMY_DATA);
        evas_object_color_set(Ad.vic_val_dmy_text[i], 255, 255, 255, 0);
        elm_list_item_append(Ad.vic_li[i], NULL, NULL, Ad.vic_val_dmy_text[i],
                             NULL, NULL);
    }

    ICO_DBG("elmListCreate Leave");
    return;
}

/**
 * @breif winCreate
 */
static void winCreate(void)
{
    ICO_DBG("winCreate Enter");

    int i = 0;

    if (NULL == Ad.win) {
        ICO_ERR("main window is un-creating.");
        return;
    }

    Ad.ctg_bx = elm_box_add(Ad.win);
    evas_object_resize(Ad.ctg_bx, CTG_BX_W, CTG_BX_H);
    evas_object_move(Ad.ctg_bx, CTG_BX_X, CTG_BX_Y);
    evas_object_show(Ad.ctg_bx);

    Ad.ctg_bx2 = elm_box_add(Ad.win);
    evas_object_resize(Ad.ctg_bx2, CTG_BX_W, CTG_BX_H);
    evas_object_move(Ad.ctg_bx2, (CTG_BX_X + CTG_BX_W), CTG_BX_Y);
    evas_object_show(Ad.ctg_bx2);

    for (i = 0; i < MAX_CATEGORY_NUM; i++) {
        /* category buttn create */
        Ad.ctg_btn[i] = elm_button_add(Ad.win);
        elm_object_text_set(Ad.ctg_btn[i], vic_category_data[i].name);
        if (i < (MAX_CATEGORY_NUM / 2)) {
            elm_box_pack_end(Ad.ctg_bx, Ad.ctg_btn[i]);
        }
        else {
            elm_box_pack_end(Ad.ctg_bx2, Ad.ctg_btn[i]);
        }

        /* The present category to support "Running Status" "Eenvironment" */
        if ((vic_category_data[i].category == RUNNINGSTATUS)
            || (vic_category_data[i].category == ENVIRONMENT)) {
            evas_object_smart_callback_add(Ad.ctg_btn[i], "clicked",
                                           on_ctg_mousedown,
                                           &(vic_category_data[i].category));
        }
        else {
            /* Unsupported Grayout */
            evas_object_color_set(Ad.ctg_btn[i], 128, 128, 128, 255);
        }

        evas_object_size_hint_weight_set(Ad.ctg_btn[i], EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(Ad.ctg_btn[i], EVAS_HINT_FILL, 0);
        evas_object_show(Ad.ctg_btn[i]);

        /* list create */
        Ad.vic_li[i] = elm_list_add(Ad.win);
        elm_list_select_mode_set(Ad.vic_li[i], ELM_OBJECT_SELECT_MODE_NONE);
        evas_object_resize(Ad.vic_li[i], VIC_LI_W, VIC_LI_H);
        evas_object_move(Ad.vic_li[i], VIC_LI_X, VIC_LI_Y);
    }

    /* Initial list display */
    Ad.vic_ini_li = elm_list_add(Ad.win);
    elm_list_select_mode_set(Ad.vic_ini_li, ELM_OBJECT_SELECT_MODE_NONE);
    evas_object_resize(Ad.vic_ini_li, VIC_LI_W, VIC_LI_H);
    evas_object_move(Ad.vic_ini_li, VIC_LI_X, VIC_LI_Y);
    elm_list_item_append(Ad.vic_ini_li, NULL, NULL, NULL, NULL, NULL);
    elm_list_go(Ad.vic_ini_li);
    evas_object_show(Ad.vic_ini_li);

    ICO_DBG("winCreate Leave");
    return;
}

/**
 * @brief create_win
 */
static Evas_Object *create_win(const char *name)
{
    ICO_DBG("create_win Enter");

    Evas_Object *eo;
    eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
    if (eo) {
        elm_win_title_set(eo, name);
        evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
    }

    ICO_DBG("create_win Leave");
    return eo;
}

/**
 * @brief app_create
 */
static bool app_create(void *data)
{
    ICO_DBG("app_create Enter");

    /* main widnow */
    Ad.win = create_win(PACKAGE);
    if (Ad.win == NULL) {
        ICO_ERR("main window is un-creating.");
        return FALSE;
    }
    evas_object_show(Ad.win);

    elm_win_indicator_mode_set(Ad.win, ELM_WIN_INDICATOR_SHOW);
    elm_win_fullscreen_set(Ad.win, EINA_TRUE);

    Ad.bg = elm_bg_add(Ad.win);
    elm_win_resize_object_add(Ad.win, Ad.bg);
    evas_object_size_hint_weight_set(Ad.bg, EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    elm_bg_file_set(Ad.bg, BG_IMAGE_FILE, NULL);
    evas_object_show(Ad.bg);

    evas_object_resize(Ad.win, WIDTH, HEIGHT);

    /* set font size */
    (void) elm_config_font_overlay_set(TEXT_LIST_ITEM, FONT_FAMILY,
                                       FONT_SIZE);
    (void) elm_config_font_overlay_set(TEXT_BUTTON, FONT_FAMILY, FONT_SIZE);
    (void) elm_config_font_overlay_set(TEXT_LABEL, FONT_FAMILY, FONT_SIZE);
    (void) elm_config_font_overlay_apply();

    winCreate();

    elmListCreate();

    ICO_DBG("app_create Leave");
    return TRUE;                /* EXIT_SUCCESS */
}

int main(int argc, char *argv[])
{
    char appid[256];
    app_event_callback_s event_callback;
    int pid = 0;
    int result = 0;

    /* Setting the log output */
    memset(appid, 0x00, sizeof(appid));
    pid = getpid();
    if (aul_app_get_appid_bypid(pid, appid, sizeof(appid)) == AUL_R_OK) {
        ico_log_open(appid);
    }
    else {
        ico_log_open(PACKAGE);
    }

    ICO_DBG("main Enter");

    /* Read configuration file */
    if (get_config() != 0) {
        ICO_ERR("get_config NG");
        return EXIT_FAILURE;
    }

    /* Setting AMB */
    if (amb_init() != EINA_TRUE) {
        ICO_ERR("amb_init NG");
        return EXIT_FAILURE;
    }

    /* set callback fanc */
    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = NULL;
    event_callback.resume = NULL;
    event_callback.service = NULL;
    event_callback.low_memory = NULL;
    event_callback.low_battery = NULL;
    event_callback.device_orientation = NULL;
    event_callback.language_changed = NULL;
    event_callback.region_format_changed = NULL;

    memset(&Ad, 0x0, sizeof(struct appdata_t));

    result = app_efl_main(&argc, &argv, &event_callback, &Ad);

    ICO_DBG("main Leave");
    return result;
}
