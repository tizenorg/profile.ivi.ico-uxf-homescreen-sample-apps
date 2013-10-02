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

#ifdef HAVE_CONFIG_H

#include "config.h"
#define __UNUSED__
#else
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
//#include <Ecore_X.h>
#include <app.h>
#include <dbus/dbus.h>
#include <string.h>
#include <stdbool.h>
#include <bundle.h>
//#include "ico_apf.h"
//#include "ico_apf_ecore.h"
//#include "ico_apf_log.h"
#include <aul.h>
#include "ico_log.h"

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
#define STATUS_BAR_HEIGHT (64)
#define CTRL_BAR_HEIGHT   (128)
#define WIDTH  (1080) /* Base Background width  */
#define HEIGHT (1920 - STATUS_BAR_HEIGHT - CTRL_BAR_HEIGHT) /* Base Background height */

/* Base */
#define SCRN_TYPE_W  800
#define SCRN_TYPE_H  20
#define CTG_BX_W     530
#define CTG_BX_H     250
//#define VIC_LI_W     1000
#define VIC_LI_W     1060
//#define VIC_LI_H     700
#define VIC_LI_H     1050
#define GET_BTN_W    800
#define GET_BTN_H    160

#define SCRN_TYPE_X  20
#define SCRN_TYPE_Y  20
#define CTG_BX_X     10
#define CTG_BX_Y     40
//#define VIC_LI_X     900
//#define VIC_LI_Y     40
#define VIC_LI_X     10
//#define VIC_LI_Y     800
#define VIC_LI_Y     450
//#define GET_BTN_X    1000
//#define GET_BTN_Y    800
#define GET_BTN_X    140
#define GET_BTN_Y    1530

/* font */
#define FONT_SIZE       48
#define FONT_FAMILY     (char *)"Sans"  // Mono, Sans, Serif

/* Text class name */
#define TEXT_BUTTON     (char *)"button"
#define TEXT_LIST_ITEM  (char *)"list_item"
#define TEXT_LABEL      (char *)"label"

/* Count vehicle information */
static int property_num = 0;

/* max size of vehicle information */
#define MAX_PARA_NUM 32

/* vehicle information ID */
#define VEHICLESPEED       0
#define ACCELERATIONX      1
#define SHIFTPOSITION      2
#define ENGINESPEED        3
#define LATITUDE           4
#define ALTITUDE           5
#define GEARPOSITION       6
#define LONGITUDE          7
#define MODE               8
#define DIRECTION          9
#define WHEELBRAKEPRESSURE 10
#define LEFTTURN           11
#define RIGHTTURN          12
#define BRAKESIGNAL        13
#define ACCELERATIONY      14
#define ACCELERATIONZ      15
#define EXTERIORBRIGHTNESS 16

/* maximum categories */
#define PACKAG "ico-app-vicsample"
#define MAX_CATEGORY_NUM 8

/* vehicle information Parameter Category */
#define DRIVINGSAFETY   0
#define ELECTRICVEHICLE 1
#define ENVIRONMENT     2
#define MAINTENANCE     3
#define PARKING         4
#define PERSONALIZATION 5
#define RUNNINGSTATUS   6
#define VEHICLEINFO     7

/* Definition for use with D-Bus */
#define DBUS_SERVICE   "org.automotive.message.broker"
#define DBUS_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_METHOD    "Get"

/* Definition for files */
#define CONFIG_FILE    "/usr/apps/org.tizen.ico.app-vicsample/res/vicsample_config.txt"
#define BG_IMAGE_FILE  "/usr/apps/org.tizen.ico.app-vicsample/res/images/vicinfo_bg.png"
/*============================================================================*/
/* Define data types                                                          */
/*============================================================================*/
union VicVal_t
{
    dbus_int32_t i32_val;
    dbus_int32_t i16_val;
    dbus_uint32_t u32_val;
    dbus_uint16_t u16_val;
    uint8_t byte_val;
    dbus_bool_t b_val;
    double d_val;
    char *s_val;
};

struct vic_data_t
{
    int property;
    int category;
    char name[32];
    char path_name[64];
    char interface_name[64];
    char property_name[32];
};

struct vic_key_data_t
{
    int id;
    int category;
    char name[32];
};

struct vic_category_data_t
{
    int category;
    char name[32];
};

struct appdata_t
{
    Evas_Object *win;           //main window
    Evas_Object *bg;

    Evas_Object *scrn_type;
    Evas_Object *ctg_bx;
    Evas_Object *ctg_bx2;
    Evas_Object *ctg_btn[MAX_CATEGORY_NUM];
    Evas_Object *vic_ini_li;
    Evas_Object *vic_li[MAX_CATEGORY_NUM];
    Evas_Object *vic_val_text[MAX_PARA_NUM];
    Evas_Object *vic_val_dmy_text[MAX_CATEGORY_NUM];
    Evas_Object *get_btn;
};
/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static double Width = 0;        /* Background width  */
static double Height = 0;       /* Background height */
static void drawVehicleInfo(void);
static void _on_mousedown(void *data, Evas_Object *obj, void *event_info);
static void _on_ctg_mousedown(void *data, Evas_Object *obj,
                              void *event_info);
static int getVehicleInfo(int key, union VicVal_t *vic_val_p);
static bool parse_elementary_value(union VicVal_t *vic_val_p,
                                   DBusMessageIter *iter);
// TEST
static bool parse_value(union VicVal_t *vic_val_p, DBusMessageIter *iter);
// TEST
static bool parse_dict_entry(union VicVal_t *vic_val_p,
                             DBusMessageIter *iter);
// TEST
static bool parse_array(union VicVal_t *vic_val_p, DBusMessageIter *iter);
// TEST
static bool parse_struct(union VicVal_t *vic_val_p, DBusMessageIter *iter);

static int get_config(void);
static void _winCreate(void);
static void elmListCreate(void);
/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
const static char Bus_name[] = DBUS_SERVICE;
static struct vic_data_t vic_data[MAX_PARA_NUM];
static DBusConnection *g_connection = NULL;
static struct appdata_t Ad;
static char SscrnType[32];
static int ListDispSts = -1;

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
    {VEHICLESPEED, RUNNINGSTATUS, "VehicleSpeed"},
    {ACCELERATIONX, RUNNINGSTATUS, "AccelerationX"},
    {SHIFTPOSITION, RUNNINGSTATUS, "ShiftPosition"},
    {ENGINESPEED, RUNNINGSTATUS, "EngineSpeed"},
    {LATITUDE, RUNNINGSTATUS, "Latitude"},
    {ALTITUDE, RUNNINGSTATUS, "Altitude"},
    {GEARPOSITION, RUNNINGSTATUS, "GearPosition"},
    {LONGITUDE, RUNNINGSTATUS, "Longitude"},
    {MODE, RUNNINGSTATUS, "Mode"},
    {DIRECTION, RUNNINGSTATUS, "Direction"},
    {WHEELBRAKEPRESSURE, RUNNINGSTATUS, "WheelBrakePressure"},
    {LEFTTURN, RUNNINGSTATUS, "LeftTurn"},
    {RIGHTTURN, RUNNINGSTATUS, "RightTurn"},
    {BRAKESIGNAL, RUNNINGSTATUS, "BrakeSignal"},
    {ACCELERATIONY, RUNNINGSTATUS, "AccelerationY"},
    {ACCELERATIONZ, RUNNINGSTATUS, "AccelerationZ"},
    {EXTERIORBRIGHTNESS, ENVIRONMENT, "ExteriorBrightness"},
    {-1, -1, "END"}
};

/*============================================================================*/
/* Function                                                                   */
/*============================================================================*/
static void drawVehicleInfo()
{
    union VicVal_t vic_val[32];
    int result = 0;
    int i;
    char vic_str[256];

    for (i = 0; i < property_num; i++) {
        result = getVehicleInfo(i, vic_val);
        memset(vic_str, 0x00, sizeof(vic_str));

        if (result != 0) {
            ICO_DBG("Err getVehicleInfo : %s", vic_data[i].name);
            continue;
        }

        switch (vic_data[i].property) {
        case VEHICLESPEED:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].i32_val);
            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].i32_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case ACCELERATIONX:
        case ACCELERATIONZ:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].u16_val);
            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].u16_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case SHIFTPOSITION:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].byte_val);
            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].byte_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case ENGINESPEED:
        case DIRECTION:
        case WHEELBRAKEPRESSURE:
        case ACCELERATIONY:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].i32_val);
            sprintf(vic_str, "%d", vic_val[0].i32_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case LATITUDE:
        case ALTITUDE:
        case LONGITUDE:
            ICO_DBG("%s(D-bus I/F Result) = %f", vic_data[i].name,
                      vic_val[0].d_val);
            sprintf(vic_str, "%f", vic_val[0].d_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case GEARPOSITION:
        case MODE:
        case EXTERIORBRIGHTNESS:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].byte_val);
            sprintf(vic_str, "%d", vic_val[0].byte_val);
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        case LEFTTURN:
        case RIGHTTURN:
        case BRAKESIGNAL:
            ICO_DBG("%s(D-bus I/F Result) = %d", vic_data[i].name,
                      vic_val[0].b_val);
            if (vic_val[0].b_val == TRUE) {
                sprintf(vic_str, "%s", "true");
            }
            else {
                sprintf(vic_str, "%s", "false");
            }
            elm_object_text_set(Ad.vic_val_text[i], vic_str);
            break;

        default:
            ICO_DBG("Err no property : %s\n", vic_data[i].name);
            break;
        }
    }
    return;
}

static void _on_mousedown(void *data, Evas_Object *obj, void *event_info)
{
    drawVehicleInfo();
    return;
}

static void _on_ctg_mousedown(void *data, Evas_Object *obj, void *event_info)
{
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
    return;
}

/* Get the vehicle information than AMB */
static int getVehicleInfo(int key, union VicVal_t *vic_val_p)
{
    /* local variable */
    DBusMessage *message;
    DBusError error;
    int result = 0;
    const char *v_string[] = {
        vic_data[key].interface_name,
        vic_data[key].property_name
    };
    const char *dest = Bus_name;
    DBusMessage *reply;
    int reply_timeout = 1000;   /* Millisecond */
    DBusMessageIter iter;
    DBusMessageIter iter_array;

    /* initialize */
    dbus_error_init(&error);

    if (NULL == g_connection) {
        /* obtain the right to use dbus */
        g_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

        if (g_connection == NULL) {
            ICO_DBG("Err dbus_bus_get");

            /* Release err parameter */
            dbus_error_free(&error);
            return -1;
        }
    }
    /* Constructs a new message */
    message = dbus_message_new_method_call(DBUS_SERVICE,
                                           vic_data[key].path_name,
                                           DBUS_INTERFACE, DBUS_METHOD);
    if (message == NULL) {
        ICO_DBG("Err dbus_message_new_method_call");

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;
        return -1;
    }

    /* Gets the type of a message */
    result = dbus_message_append_args(message,
                                      DBUS_TYPE_STRING,
                                      &v_string[0],
                                      DBUS_TYPE_STRING,
                                      &v_string[1], DBUS_TYPE_INVALID);
    if (!result) {
        ICO_DBG("Err dbus_message_append_args");

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;
        /* Release the message */
        dbus_message_unref(message);

        return -1;
    }

    /* Gets the error name */
    if ((dest) && (!dbus_message_set_destination(message, dest))) {
        ICO_DBG("Err dbus_message_new_method_call");

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
                                                      reply_timeout, &error);
    if (reply == NULL) {
        ICO_DBG("Err dbus_connection_send_with_reply_and_block");

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
//    result = parse_elementary_value(vic_val_p, &iter_array);
    union VicVal_t *tmp_vic_val_p = vic_val_p;  //TEST
    result = parse_value(tmp_vic_val_p, &iter_array);   //TEST

    if (result != TRUE) {
        ICO_DBG("Err parse_elementary_value");

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

/* Parse of the value */
static bool parse_elementary_value(union VicVal_t *vic_val_p,
                                   DBusMessageIter *iter)
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

    if ((vic_val_p == NULL) || (iter == NULL)) {
        ICO_DBG("Err Parameter NG ");
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
        dbus_message_iter_get_basic(iter, &s_val);
        w_s_val = (char *) malloc(strlen(s_val) + 1);   // Release required
        if (w_s_val == NULL) {
            ICO_DBG("Err malloc");
            return FALSE;
        }
        strncpy(w_s_val, s_val, strlen(s_val));
        vic_val_p->s_val = w_s_val;
        vic_val_p++;
//        vic_val_p->s_val = s_val;
        break;
    default:
        ICO_DBG("Err parse_elementary_value: unknown type");
        return FALSE;
    }

    return TRUE;
}

static bool parse_value(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    char curr;

    if ((vic_val_p == NULL) || (iter == NULL)) {
        ICO_DBG("Err Parameter NG ");
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
        return parse_array(vic_val_p, iter);
    case DBUS_TYPE_STRUCT:
        return parse_struct(vic_val_p, iter);
    case DBUS_TYPE_DICT_ENTRY:
//            goto error; /* these are handled from parse_array */
        return FALSE;
    case DBUS_TYPE_INVALID:
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

static bool parse_array(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    DBusMessageIter new_iter;
    int element_type;

    if ((vic_val_p == NULL) || (iter == NULL)) {
        ICO_DBG("Err Parameter NG ");
        return FALSE;
    }

    /* the lua array */
//    lua_newtable(L);

    element_type = dbus_message_iter_get_element_type(iter);

    dbus_message_iter_recurse(iter, &new_iter);

    /* the problem: if the value inside array is a dict entry, the
     * indexing of elements need to be done with dict keys instead
     * of numbers. */

    if (element_type == DBUS_TYPE_DICT_ENTRY) {
        while (dbus_message_iter_get_arg_type(&new_iter) != DBUS_TYPE_INVALID) {

            parse_dict_entry(vic_val_p, &new_iter);
            dbus_message_iter_next(&new_iter);
        }
    }

    else {
        while (dbus_message_iter_get_arg_type(&new_iter) != DBUS_TYPE_INVALID) {

            /* array index */
//            lua_pushinteger(L, i++);

            parse_value(vic_val_p, &new_iter);
            dbus_message_iter_next(&new_iter);

            /* put the values to the table */
//            lua_settable(L, -3);
        }
    }

    return TRUE;
}

static bool parse_dict_entry(union VicVal_t *vic_val_p,
                             DBusMessageIter *iter)
{
    DBusMessageIter new_iter;

    if ((vic_val_p == NULL) || (iter == NULL)) {
        ICO_DBG("Err Parameter NG ");
        return FALSE;
    }

    dbus_message_iter_recurse(iter, &new_iter);

    while (dbus_message_iter_get_arg_type(&new_iter) != DBUS_TYPE_INVALID) {

        /* key must be elementary, value can be anything */

        parse_elementary_value(vic_val_p, &new_iter);
        dbus_message_iter_next(&new_iter);

        parse_value(vic_val_p, &new_iter);
        dbus_message_iter_next(&new_iter);

        /* put the values to the table */
//        lua_settable(L, -3);
    }

    return TRUE;
}

static bool parse_struct(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    DBusMessageIter new_iter;

    if ((vic_val_p == NULL) || (iter == NULL)) {
        ICO_DBG("Err Parameter NG ");
        return FALSE;
    }

    /* initialize the table */
//    lua_newtable(L);

    dbus_message_iter_recurse(iter, &new_iter);

    while (dbus_message_iter_get_arg_type(&new_iter) != DBUS_TYPE_INVALID) {

        /* struct "index" */
//        lua_pushinteger(L, i++);

        parse_value(vic_val_p, &new_iter);
        dbus_message_iter_next(&new_iter);

        /* put the values to the table */
//        lua_settable(L, -3);
    }

    return TRUE;
}

/* Read configuration file */
static int get_config(void)
{
    ICO_DBG("ENTER get_config");

    FILE *fp;
    int k = 0;
    int j, m;
    char buff[512];
    char *tp;
    char *clm = " \n";

    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        ICO_DBG("File open error");
        return -1;
    }

    for (m = 0; k < MAX_PARA_NUM; m++) {
        if (fgets(buff, sizeof(buff) - 2, fp) != NULL) {
            tp = strtok(buff, clm);
            if (tp != NULL) {
                if (tp[0] != '#') {
                    for (j = 0; vic_key_data[j].id != -1; j++) {
                        if (strcmp(tp, vic_key_data[j].name) == 0) {
                            vic_data[k].property = vic_key_data[j].id;
                            vic_data[k].category = vic_key_data[j].category;
                            strcpy(vic_data[k].name, tp);
                            strcpy(vic_data[k].path_name, strtok(NULL, clm));
                            strcpy(vic_data[k].interface_name,
                                   strtok(NULL, clm));
                            strcpy(vic_data[k].property_name,
                                   strtok(NULL, clm));
                            ICO_DBG("vic_data[%d].property=%d", k,
                                      vic_data[k].property);
                            ICO_DBG("vic_data[%d].name=%s", k,
                                      vic_data[k].name);
                            ICO_DBG("vic_data[%d].path_name=%s", k,
                                      vic_data[k].path_name);
                            ICO_DBG("vic_data[%d].interface_name=%s", k,
                                      vic_data[k].interface_name);
                            ICO_DBG("vic_data[%d].property_name=%s", k,
                                      vic_data[k].property_name);

                            k++;
                            break;
                        }
                    }
                    if (vic_key_data[j].id == -1) {
                        ICO_DBG
                            ("Err vicsample_config.txt Line:%d Unregistered"
                             " parameter name", m + 1);
                    }

                }
                else {
                    ICO_DBG
                        ("vicsample_config.txt Line:%d Comment out  '#'Discovery",
                         m + 1);
                }
            }
            else {
                ICO_DBG
                    ("vicsample_config.txt Line:%d Comment out  Null line",
                     m + 1);
            }
        }
        else {
            ICO_DBG("vicsample_config.txt The end of data reading");
            break;
        }
    }
    fclose(fp);

    property_num = k;
    if (property_num == 0) {
        ICO_DBG("vicsample_config.txt No valid data");
        return -1;
    }

    ICO_DBG("LEAVE get_config");
    return 0;
}
#if 0
static void res_callback(ico_apf_resource_notify_info_t *info,
                         void *user_data)
{
    int ret;

    ICO_DBG("##==> Callbacked evt=%d res=%d id=%d bid=%d appid=%s dev=%s"
              " user_data=%d", info->state, info->resid, info->id,
              info->bid, info->appid, info->device, (int) user_data);
    switch (info->state) {
    case ICO_APF_RESOURCE_STATE_ACQUIRED:
    case ICO_APF_RESOURCE_STATE_DEPRIVED:
    case ICO_APF_RESOURCE_STATE_WAITTING:
    case ICO_APF_RESOURCE_STATE_RELEASED:
        if (info->resid == ICO_APF_RESID_INT_SCREEN) {
            ret = ico_apf_resource_reply_int_screen_mode(info->device,
                                                         info->bid, info->id,
                                                         1);
            ICO_DBG("##==> callback reply int_screen(%s,%d,%d,1) = %d",
                      info->device, info->bid, info->id, ret);
        }
        else if (info->resid == ICO_APF_RESID_ON_SCREEN) {
            ret = ico_apf_resource_reply_int_screen_mode_disp(info->device,
                                                              info->id, 1);
            ICO_DBG("##==> callback reply on_screen(%s,%d,1) = %d",
                      info->device, info->id, ret);
        }
        else {
            ret =
                ico_apf_resource_reply_screen_mode(info->device, info->id, 1);
            ICO_DBG("##==> callback reply screen(%s,%d,1) = %d",
                      info->device, info->id, ret);
        }
        break;
    default:
        /* NOP  */
        break;
    }
}
#endif
/**
 * @breif _winCreate
 */
static void _winCreate(void)
{
    ICO_DBG("ENTER _winCreate");

    int i;
    double w_mag;
    double h_mag;

    if (NULL == Ad.win) {
        ICO_DBG("Err Param NG");
        return;
    }

    w_mag = Width / WIDTH;
    h_mag = Height / HEIGHT;
    ICO_DBG("Width =%f,Height=%f", Width, Height);
    ICO_DBG("w_mag =%f,h_magh=%f", w_mag, h_mag);

    Ad.scrn_type = elm_label_add(Ad.win);
    elm_object_text_set(Ad.scrn_type, SscrnType);
    evas_object_resize(Ad.scrn_type, SCRN_TYPE_W * w_mag,
                       SCRN_TYPE_H * h_mag);
    evas_object_move(Ad.scrn_type, SCRN_TYPE_X * w_mag, SCRN_TYPE_Y * h_mag);
    evas_object_show(Ad.scrn_type);

    Ad.ctg_bx = elm_box_add(Ad.win);
    evas_object_resize(Ad.ctg_bx, CTG_BX_W * w_mag, CTG_BX_H * h_mag);
    evas_object_move(Ad.ctg_bx, CTG_BX_X * w_mag, CTG_BX_Y * h_mag);
    evas_object_show(Ad.ctg_bx);

    Ad.ctg_bx2 = elm_box_add(Ad.win);
    evas_object_resize(Ad.ctg_bx2, CTG_BX_W * w_mag , CTG_BX_H * h_mag);
    evas_object_move(Ad.ctg_bx2, (CTG_BX_X + CTG_BX_W) * w_mag, CTG_BX_Y * h_mag);
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
                                           _on_ctg_mousedown,
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
        evas_object_resize(Ad.vic_li[i], VIC_LI_W * w_mag, VIC_LI_H * h_mag);
        evas_object_move(Ad.vic_li[i], VIC_LI_X * w_mag, VIC_LI_Y * h_mag);
    }

    /* Initial list display */
    Ad.vic_ini_li = elm_list_add(Ad.win);
    elm_list_select_mode_set(Ad.vic_ini_li, ELM_OBJECT_SELECT_MODE_NONE);
    evas_object_resize(Ad.vic_ini_li, VIC_LI_W * w_mag, VIC_LI_H * h_mag);
    evas_object_move(Ad.vic_ini_li, VIC_LI_X * w_mag, VIC_LI_Y * h_mag);
    elm_list_item_append(Ad.vic_ini_li, NULL, NULL, NULL, NULL, NULL);
    elm_list_go(Ad.vic_ini_li);
    evas_object_show(Ad.vic_ini_li);

    Ad.get_btn = elm_button_add(Ad.win);
    elm_object_text_set(Ad.get_btn, "Get VehicleInfo");
    evas_object_smart_callback_add(Ad.get_btn, "clicked", _on_mousedown,
                                   NULL);
    evas_object_resize(Ad.get_btn, GET_BTN_W * w_mag, GET_BTN_H * h_mag);
    evas_object_move(Ad.get_btn, GET_BTN_X * w_mag, GET_BTN_Y * h_mag);
    evas_object_show(Ad.get_btn);

    ICO_DBG("LEAVE _winCreate");
    return;
}

/**
 * @brief elmListCreate
 */
static void elmListCreate(void)
{
    ICO_DBG("ENTER elmListCreate");

    int i;

    if (NULL == Ad.win) {
        ICO_DBG("Err Param NG");
        return;
    }

    for (i = 0; vic_key_data[i].id != -1 && i < MAX_PARA_NUM; i++) {
        Ad.vic_val_text[i] = elm_label_add(Ad.win);
        elm_list_item_append(Ad.vic_li[vic_data[i].category],
                             vic_data[i].name, NULL, Ad.vic_val_text[i], NULL,
                             NULL);
    }

    /* dummy set */
    for (i = 0; i < MAX_CATEGORY_NUM; i++) {
        Ad.vic_val_dmy_text[i] = elm_label_add(Ad.win);
        elm_list_item_append(Ad.vic_li[i], NULL, NULL, NULL, NULL, NULL);
    }

    ICO_DBG("LEAVE elmListCreate");
    return;
}

/**
 * @brief app_terminate
 */
static void app_terminate(void *data)
{
    ICO_DBG("ENTER app_terminate");
    // Release all resources
    int i;

    if (Ad.win) {
        evas_object_del(Ad.win);
        Ad.win = NULL;
    }

    if (Ad.bg) {
        evas_object_del(Ad.bg);
        Ad.bg = NULL;
    }

    if (Ad.scrn_type) {
        evas_object_del(Ad.scrn_type);
        Ad.scrn_type = NULL;
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

    if (Ad.get_btn) {
        evas_object_del(Ad.get_btn);
        Ad.get_btn = NULL;
    }
    ICO_DBG("LEAVE app_terminate");
    return;
}

/**
 * @brief _win_del
 */
static void _win_del(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("ENTER _win_del");

    elm_exit();

    ICO_DBG("LEAVE _win_del");
    return;
}

/**
 * @brief _create_win
 */
static Evas_Object *_create_win(const char *name)
{
    ICO_DBG("ENTER _create_win");

    Evas_Object *eo;
//    int w, h;
    eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
    if (eo) {
        elm_win_title_set(eo, name);
        evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
//        ecore_x_window_size_get(ecore_x_window_root_first_get(), &w,
//                                &h);
//      ICO_DBG("window size w=%d,h=%d",w,h);
//        evas_object_resize(eo, w, h - STATUS_BAR_HEIGHT);
    }
    ICO_DBG("LEAVE _create_win");

    return eo;
}

/**
 * @brief app_create
 */
static bool app_create(void *data)
{
    ICO_DBG("ENTER app_create");

#if 0 //TEST.s
    int w, h;

    /* get display size */
    ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
    ICO_DBG("window size w=%d,h=%d", w, h);

    /* set app screen size */
    Width = w;
    Height = h - STATUS_BAR_HEIGHT;
#else
    Width = WIDTH;
    Height = HEIGHT;
#endif //TEST.e

    /* main widnow */
    Ad.win = _create_win(PACKAGE);
    if (Ad.win == NULL) {
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

    evas_object_resize(Ad.win, Width, Height);

    /* set font size */
    (void)elm_config_font_overlay_set(TEXT_LIST_ITEM, FONT_FAMILY, FONT_SIZE);
    (void)elm_config_font_overlay_set(TEXT_BUTTON, FONT_FAMILY, FONT_SIZE);
    (void)elm_config_font_overlay_set(TEXT_LABEL, FONT_FAMILY, FONT_SIZE);
    (void)elm_config_font_overlay_apply();

    _winCreate();

    elmListCreate();

    ICO_DBG("LEAVE app_create");

    return TRUE;                /* EXIT_SUCCESS */
}
#if 0
/**
 * @brief get_screen
 */
static int get_screen(int argc, char **argv)
{
    ICO_DBG("ENTER get_screen");

    int getscreen;
    bundle *b;
    const char *val;

    b = bundle_import_from_argv(argc, argv);
    getscreen = 0;
    SscrnType[0] = 0;
    if (b != NULL) {
        val = bundle_get_val(b, "rightoption");
        if (val != NULL) {
            if (strcasecmp(val, "-basescreen") == 0) {
                getscreen = 1;  /* get base screen */
                strcpy(SscrnType, "BasicScreen");
                ICO_DBG("BasicScreen");
            }
            else if (strcasecmp(val, "-intscreen") == 0) {
                getscreen = 2;  /* get interrupt screen */
                strcpy(SscrnType, "IntScreen");
            }
            else if (strcasecmp(val, "-onscreen") == 0) {
                getscreen = 3;  /* get on screen */
                strcpy(SscrnType, "OnScreen");
            }
        }
    }

    if (getscreen > 0) {
        /* initialize resource control for Ecore */
        if (ico_apf_ecore_init(NULL) != ICO_APF_E_NONE) {
            ICO_DBG("ico_apf_ecore_init() Error");
            ecore_evas_shutdown();
            return -1;
        }

        /* set resource request callback */
        ico_apf_resource_set_event_cb(res_callback, NULL);

        /* acquire a right to display a screen */
        if (getscreen == 1) {
            ico_apf_resource_get_screen_mode(NULL, 0);
        }
        else if (getscreen == 2) {
            ico_apf_resource_get_int_screen_mode(NULL, 0, 0);
        }
        else {
            ico_apf_resource_get_int_screen_mode_disp(NULL, 0);
        }
    }
    ICO_DBG("LEAVE get_screen");
    return 0;
}
#endif

int main(int argc, char *argv[])
{
//    char appid[ICO_UXF_MAX_PROCESS_NAME + 1];
    char appid[256];
    int pid;
    app_event_callback_s event_callback;
    int result = 0;

    if (!ecore_evas_init()) {
        return EXIT_FAILURE;
    }

    /* Setting the log output */
//    if (ico_apf_get_app_id(0, appid) == ICO_APP_CTL_E_NONE) {
//        ico_apf_log_open(appid);
//    }

    /* Setting the log output */
    memset(appid, 0, sizeof(appid));
    pid = getpid();
    if (aul_app_get_appid_bypid(pid, appid, sizeof(appid)) == AUL_R_OK) {
        ico_log_open(appid);
    }
    else {
        ico_log_open("org.tizen.ico.app-vicsample");
    }

    ICO_DBG("ENTER main");

    /* Read configuration file */
    if (get_config() != 0) {
        ICO_DBG("Err get_config");
        return EXIT_FAILURE;
    }

    /* get argument */
//    if (get_screen(argc, argv) != 0) {
//        ICO_DBG("Err get_argument");
//        return EXIT_FAILURE;
//    }

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

//    ico_apf_ecore_term();
    ecore_evas_shutdown();

    if (NULL != g_connection) {
        dbus_connection_unref(g_connection);
        g_connection = NULL;
    }
    ICO_DBG("EXIT main");

    return result;
}
