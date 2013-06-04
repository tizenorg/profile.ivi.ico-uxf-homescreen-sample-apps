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

//#include <VehicleInfo.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include <dbus/dbus.h>          /* dbus */
#include <string.h>             /* test log output */
#include <stdbool.h>            // bool type define //TEST
//#include "app_log.h"
#include "ico_apf.h"
#include "ico_apf_ecore.h"
#include "ico_apf_log.h"
//#include    "ico_uxf_conf_common.h"
/* macro for debug */
//#define DEBUG_PRINT(...)    ICO_UXF_DEBUG(__VA_ARGS__)
//#define ERROR_PRINT(...)    ICO_UXF_WARN(__VA_ARGS__)

#if 0
// log output define
#include <unistd.h>

#define DEBUG_PRINT(...)    LOG_PRINT(__VA_ARGS__)
#define LOG_PRINT(fmt,...)  \
    {log_print("%s> "fmt" (%s,%s:%d)\n", log_curtime("DBG"), ##__VA_ARGS__,\
               __func__, __FILE__, __LINE__); }

static FILE *sDbgFd = (FILE *) 0;
void log_print(const char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vfprintf(sDbgFd, fmt, list);
    va_end(list);
    fflush(sDbgFd);
}

/* local time difference(sec) */
static int sTimeZone = (99 * 60 * 60);

char *log_curtime(const char *level)
{

    struct timeval NowTime;
    extern long timezone;
    static char sBuf[28];

    gettimeofday(&NowTime, (struct timezone *) 0);
    if (sTimeZone > (24 * 60 * 60)) {
        tzset();
        sTimeZone = timezone;
    }
    NowTime.tv_sec -= sTimeZone;

    sprintf(sBuf, "%02d:%02d:%02d.%03d[%s]@%d",
            (int) ((NowTime.tv_sec / 3600) % 24),
            (int) ((NowTime.tv_sec / 60) % 60),
            (int) (NowTime.tv_sec % 60),
            (int) NowTime.tv_usec / 1000, level, getpid());

    return (sBuf);
}
#endif

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
#define WIDTH  (700)            /* Background width  */
#define HEIGHT (934)            /* Background height */

/* Count vehicle information */
static int property_num = 0;

/* max size of vehicle information */
#define MAX_PARA_NUM 32

/* vehicle information ID */
#define VEHICLESPEED  0
#define ACCELERATIONX 1
#define SHIFTPOSITION 2
#define ENGINESPEED   3
#define LATITUDE      4
#define ALTITUDE      5
#define GEARPOSITION  6
#define LONGITUDE     7
#define MODE          8

#define DIRECTION     9
#define BRAKEPRESSURE 10
#define LEFTTURN      11
#define RIGHTTURN     12
#define BRAKESIGNAL   13

/* Definition for use with D-Bus */
#define DBUS_SERVICE   "org.automotive.message.broker"
#define DBUS_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_METHOD    "Get"

/* Definition for files */
//#define CONFIG_FILE    "/home/rpf/src/app/vic_inf_dspTP/vicsample_config.txt"
//#define TMP_LOG        "/home/rpf/var/log/uifw/tmp_vic_inf_dspTP.log"
//#define LOG_NAME       "/home/rpf/var/log/uifw/vic_inf_dspTP.log"
#define CONFIG_FILE    "/opt/apps/org.tizen.ico.app-vicsample/res/vicsample_config.txt"
#define TMP_LOG        "/tmp/tmp_ico-app-vicsample.log"
#define LOG_NAME       "/tmp/ico-app-vicsample.log"


/* coordinates of  vehicle information table frame */
#define VTX_SX         50.0F    /* X position of the upper left corner */
#define VTX_SY         30.0F    /* Y position of the upper left corner */
#define VTX_EX         650.0F   /* X position of the lower right */
#define VTX_EY         830.0F   /* Y position of the lower right */
#define FIX_LINE_NUM   6        /* The number of lines of the table */
                                /* Percentage of the column 13 = 1+4+4+4 */
                                /* No.[1]:Name(JP)[4]:Name(EN)[4]:Value[4] */
#define FIX_SEQ_BASE   (VTX_EX - VTX_SX) / 13
                                /* X position of the 1st frame vertical line */
#define FIX_SEQ_LINE1  VTX_SX + FIX_SEQ_BASE
                                /* X position of the 2nd frame vertical line */
#define FIX_SEQ_LINE2  FIX_SEQ_LINE1 + (FIX_SEQ_BASE * 4)
                                /* X position of the 3rd frame vertical line */
#define FIX_SEQ_LINE3  FIX_SEQ_LINE2 + (FIX_SEQ_BASE * 4)

/* Horizontal line interval size */
#define UFIX_LINE_BASE (VTX_EY - VTX_SY)/MAX_PARA_NUM

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
    char name[32];
    char path_name[64];
    char interface_name[64];
    char property_name[32];
};

struct vic_key_data_t
{
    int id;
    char name[32];
};

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static void _canvas_resize_cb(Ecore_Evas *ee);
static void _on_destroy(Ecore_Evas *ee __UNUSED__);
static Eina_Bool _timer_cb(void *data);
static void cmdlog_output(int key);
static void drawVehicleInfo(void);
static void _on_mousedown(void *data, Evas *evas, Evas_Object *o,
                          void *einfo);
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

/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
const static char Bus_name[] = DBUS_SERVICE;
static struct vic_data_t vic_data[MAX_PARA_NUM];

const struct vic_key_data_t vic_key_data[] = {
    {VEHICLESPEED, "VehicleSpeed"},
    {ACCELERATIONX, "AccelerationX"},
    {SHIFTPOSITION, "ShiftPosition"},
    {ENGINESPEED, "EngineSpeed"},
    {LATITUDE, "Latitude"},
    {ALTITUDE, "Altitude"},
    {GEARPOSITION, "GearPosition"},
    {LONGITUDE, "Longitude"},
    {MODE, "Mode"},
    {DIRECTION, "Direction"},
    {BRAKEPRESSURE, "BrakePressure"},
    {LEFTTURN, "LeftTurn"},
    {RIGHTTURN, "RightTurn"},
    {BRAKESIGNAL, "BrakeSignal"},
    {-1, "END"}
};

/* Storage area of vehicle information */
static union VicVal_t s_vic_val[MAX_PARA_NUM];
/* Object for displaying vehicle information */
static Evas_Object *vic_val_text[MAX_PARA_NUM];

/* Table border line */
const static Evas_Coord f_vtx[FIX_LINE_NUM][4] = {
    {VTX_SX, VTX_SY, VTX_SX, VTX_EY},   // Border of the Left frame
    {VTX_EX, VTX_SY, VTX_EX, VTX_EY},   // Border of the Right frame
    {VTX_SX, VTX_SY, VTX_EX, VTX_SY},   // Border of the upper frame
    {VTX_SX, VTX_EY, VTX_EX, VTX_EY},   // Border of the lower frame

    {FIX_SEQ_LINE1, VTX_SY, FIX_SEQ_LINE1, VTX_EY},     // Border of the column 1
    {FIX_SEQ_LINE2, VTX_SY, FIX_SEQ_LINE2, VTX_EY},     // Border of the column 2
//{FIX_SEQ_LINE3, VTX_SY, FIX_SEQ_LINE3, VTX_EY}  // Border of the column 3
};

//#define WIDTH  (300)
//#define HEIGHT (300)

static Ecore_Evas *ee;
static Evas_Object *text, *bg;
static char sscrntype[32];

static const char commands[] =
    "commands are:\n"
    "\tm - impose a minumum size to the window\n"
    "\tx - impose a maximum size to the window\n"
    "\tb - impose a base size to the window\n"
    "\ts - impose a step size (different than 1 px) to the window\n"
    "\th - print help\n";

/* to inform current window's size */
static void _canvas_resize_cb(Ecore_Evas *ee)
{
    int w, h;
    char buf[256];

    ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
    snprintf(buf, sizeof(buf), "%s %d x %d", sscrntype, w, h);
    evas_object_text_text_set(text, buf);
    evas_object_move(text, VTX_SX, VTX_SY - 22);
    evas_object_resize(bg, w, h);
}

static void _on_destroy(Ecore_Evas *ee __UNUSED__)
{
    ecore_main_loop_quit();
}

/* drawing current time */
static Eina_Bool _timer_cb(void *data)
{
    char str[32];
    time_t timer;
    struct tm *date;
    timer = time(NULL);         /* get time in seconds */
    date = localtime(&timer);   /* Converted to calendar time */
    sprintf(str, "%s", asctime(date));
    evas_object_text_text_set((Evas_Object *) data, str);

//    drawVehicleInfo(); /* Get the vehicle information than AMB */ TEST

    return EINA_TRUE;           /* Continuation of the timer */
}

/* log output for DBUS command result (Debug functin) */
static void cmdlog_output(int key)
{
#if 0
    char mes1[] =
        "dbus-send --system --dest=org.automotive.message.broker --type=method_call --print-reply";
    char mes2[] = "org.freedesktop.DBus.Properties.Get";
    char mes3[] = "string:";
    char str[1024];
    sprintf(str, "%s %s %s %s\"%s\" %s\"%s\" > %s",
            mes1, vic_data[key].path_name, mes2, mes3,
            vic_data[key].interface_name, mes3, vic_data[key].property_name,
            TMP_LOG);

    DEBUG_PRINT("%s(D-bus  Command Result)", vic_data[key].name);
    system(str);

    /* logout */
    log_output();
#endif
    return;
}

static void drawVehicleInfo()
{
    union VicVal_t vic_val[32];
    int result = 0;
    int i;
    char vic_str[256];

    for (i = 0; i < property_num; i++) {
        result = getVehicleInfo(i, vic_val);

        if (result != 0) {
            uim_debug("Err getVehicleInfo : %s", vic_data[i].name);
            continue;
        }

        switch (vic_data[i].property) {
        case VEHICLESPEED:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].i32_val);
            cmdlog_output(i);

//            if (vic_val[0].i32_val != s_vic_val[i].i32_val) {
//                uim_debug("%s update Front:%d Back:%d", vic_data[i].name,
//                            s_vic_val[i].i32_val, vic_val[0].i32_val);
            s_vic_val[i].i32_val = vic_val[0].i32_val;

            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].i32_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);

//            }
//            else {
//                uim_debug("%s no update:%d", vic_data[i].name,
//                            vic_val[0].i32_val);
//            }

            break;

        case ACCELERATIONX:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].u16_val);
            cmdlog_output(i);

//            if (vic_val[0].u16_val != s_vic_val[i].u16_val) {
//                uim_debug("%s update Front:%d Back:%d", vic_data[i].name,
//                            s_vic_val[i].u16_val, vic_val[0].u16_val);
            s_vic_val[i].u16_val = vic_val[0].u16_val;

            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].i32_val);
            sprintf(vic_str, "%d", vic_val[0].u16_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);
//            }
//            else {
//                uim_debug("%s no update:%d", vic_data[i].name,
//                            vic_val[0].u16_val);
//            }

            break;

        case SHIFTPOSITION:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].byte_val);
            cmdlog_output(i);

//            if (vic_val[0].byte_val != s_vic_val[i].byte_val) {
//                uim_debug("%s update Front:%d Back:%d", vic_data[i].name,
//                            s_vic_val[i].byte_val, vic_val[0].byte_val);
            s_vic_val[i].byte_val = vic_val[0].byte_val;

            /* Drawing update */
            sprintf(vic_str, "%d", vic_val[0].byte_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);
//            }
//            else {
//                uim_debug("%s noupdate:%d", vic_data[i].name,
//                            vic_val[0].byte_val);
//            }

            break;

        case ENGINESPEED:
        case DIRECTION:
        case BRAKEPRESSURE:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].i32_val);
            cmdlog_output(i);
            s_vic_val[i].i32_val = vic_val[0].i32_val;
            sprintf(vic_str, "%d", vic_val[0].i32_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);
            break;

        case LATITUDE:
        case ALTITUDE:
        case LONGITUDE:
            uim_debug("%s(D-bus I/F Result) = %f", vic_data[i].name,
                        vic_val[0].d_val);
            cmdlog_output(i);
            s_vic_val[i].d_val = vic_val[0].d_val;
            sprintf(vic_str, "%f", vic_val[0].d_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);
            break;

        case GEARPOSITION:
        case MODE:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].byte_val);
            cmdlog_output(i);
            s_vic_val[i].byte_val = vic_val[0].byte_val;
            sprintf(vic_str, "%d", vic_val[0].byte_val);
            evas_object_text_text_set(vic_val_text[i], vic_str);
            break;

        case LEFTTURN:
        case RIGHTTURN:
        case BRAKESIGNAL:
            uim_debug("%s(D-bus I/F Result) = %d", vic_data[i].name,
                        vic_val[0].b_val);
            cmdlog_output(i);
            s_vic_val[i].b_val = vic_val[0].b_val;
            if (vic_val[0].b_val == TRUE) {
                sprintf(vic_str, "%s", "true");
            }
            else {
                sprintf(vic_str, "%s", "false");
            }
            evas_object_text_text_set(vic_val_text[i], vic_str);
            break;

        default:
            uim_debug("Err no property : %s\n", vic_data[i].name);
            break;
        }
    }
    return;
}

static void _on_mousedown(void *data, Evas *evas, Evas_Object *o,
                          void *einfo)
{
    drawVehicleInfo();
    return;
}

/* Get the vehicle information than AMB */
DBusConnection *g_connection = NULL;
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
            uim_debug("Err dbus_bus_get");

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
        uim_debug("Err dbus_message_new_method_call");

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
        uim_debug("Err dbus_message_append_args");

        /* Release the connection */
        dbus_connection_unref(g_connection);
        g_connection = NULL;
        /* Release the message */
        dbus_message_unref(message);

        return -1;
    }

    /* Gets the error name */
    if ((dest) && (!dbus_message_set_destination(message, dest))) {
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
                                                      reply_timeout, &error);
    if (reply == NULL) {
        uim_debug("Err dbus_connection_send_with_reply_and_block");

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
        uim_debug("Err parse_elementary_value");

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
        uim_debug("Err Parameter NG ");
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
            uim_debug("Err malloc");
            return FALSE;
        }
        strncpy(w_s_val, s_val, strlen(s_val));
        vic_val_p->s_val = w_s_val;
        vic_val_p++;
//        vic_val_p->s_val = s_val;
        break;
    default:
        uim_debug("Err parse_elementary_value: unknown type");
        return FALSE;
    }

    return TRUE;
}

static bool parse_value(union VicVal_t *vic_val_p, DBusMessageIter *iter)
{
    char curr;

    if ((vic_val_p == NULL) || (iter == NULL)) {
        uim_debug("Err Parameter NG ");
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
        uim_debug("Err Parameter NG ");
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
        uim_debug("Err Parameter NG ");
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
        uim_debug("Err Parameter NG ");
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

    FILE *fp;
    int k = 0;
    int j, m;
    char buff[512];
    char *tp;
    char *clm = " \n";

    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        uim_debug("File open error");
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
                            strcpy(vic_data[k].name, tp);
                            strcpy(vic_data[k].path_name, strtok(NULL, clm));
                            strcpy(vic_data[k].interface_name,
                                   strtok(NULL, clm));
                            strcpy(vic_data[k].property_name,
                                   strtok(NULL, clm));
                           uim_debug("vic_data[%d].property=%d", k,
                                   vic_data[k].property);
                           uim_debug("vic_data[%d].name=%s", k,
                                   vic_data[k].name);
                           uim_debug("vic_data[%d].path_name=%s", k,
                                   vic_data[k].path_name);
                           uim_debug("vic_data[%d].interface_name=%s", k,
                                   vic_data[k].interface_name);
                           uim_debug("vic_data[%d].property_name=%s", k,
                                   vic_data[k].property_name);

                            k++;
                            break;
                        }
                    }
                    if (vic_key_data[j].id == -1) {
                        uim_debug("Err vicsample_config.txt Line:%d Unregistered"
                                    " parameter name", m + 1);
                    }

                }
                else {
                    uim_debug
                        ("vicsample_config.txt Line:%d Comment out  '#'Discovery",
                         m + 1);
                }
            }
            else {
                uim_debug("vicsample_config.txt Line:%d Comment out  Null line",
                            m + 1);
            }
        }
        else {
            uim_debug("vicsample_config.txt The end of data reading");
            break;
        }
    }
    fclose(fp);

    property_num = k;
    if (property_num == 0) {
        uim_debug("vicsample_config.txt No valid data");
        return -1;
    }
    return 0;
}

static void res_callback(ico_apf_resource_notify_info_t *info,
                         void *user_data)
{
    int ret;

    uim_debug("##==> Callbacked evt=%d res=%d id=%d bid=%d appid=%s dev=%s"
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
            uim_debug("##==> callback reply int_screen(%s,%d,%d,1) = %d",
                        info->device, info->bid, info->id, ret);
        }
        else if (info->resid == ICO_APF_RESID_ON_SCREEN) {
            ret = ico_apf_resource_reply_int_screen_mode_disp(info->device,
                                                              info->id, 1);
            uim_debug("##==> callback reply on_screen(%s,%d,1) = %d",
                        info->device, info->id, ret);
        }
        else {
            ret =
                ico_apf_resource_reply_screen_mode(info->device, info->id, 1);
            uim_debug("##==> callback reply screen(%s,%d,1) = %d",
                        info->device, info->id, ret);
        }
        break;
    default:
        /* NOP  */
        break;
    }
}

int main(int argc, char *argv[])
{
    int i;
    int getscreen;
    Evas *evas;
    char appid[ICO_UXF_MAX_PROCESS_NAME + 1];

    getscreen = 0;
    sscrntype[0] = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcasecmp(argv[i], "-basescreen") == 0) {
                getscreen = 1;  /* get base screen */
                strcpy(sscrntype, "BasicScreen");
            }
            else if (strcasecmp(argv[i], "-intscreen") == 0) {
                getscreen = 2;  /* get interrupt screen */
                strcpy(sscrntype, "IntScreen");
            }
            else if (strcasecmp(argv[i], "-onscreen") == 0) {
                getscreen = 3;  /* get on screen */
                strcpy(sscrntype, "OnScreen");
            }
        }
    }

    if (!ecore_evas_init()) {
        return EXIT_FAILURE;
    }
    /* Setting the log output */
    if (ico_apf_get_app_id(0, appid) == ICO_APP_CTL_E_NONE) {
        ico_apf_log_open(appid);
    }

    uim_debug("main ENTER");

    /* Read configuration file */
    if (get_config() != 0) {
        uim_debug("Err get_config");
        return -1;
    }

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
            ico_apf_resource_get_screen_mode(NULL, 0);
        }
        else if (getscreen == 2) {
            ico_apf_resource_get_int_screen_mode(NULL, 0, 0);
        }
        else {
            ico_apf_resource_get_int_screen_mode_disp(NULL, 0);
        }
    }

    /* this will give you a window with an Evas canvas under the first
     * engine available */
    ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, "frame=0");
    if (!ee) {
        goto error;
    }

    ecore_evas_callback_delete_request_set(ee, _on_destroy);
    ecore_evas_title_set(ee, "Ecore_Evas window sizes example");
    ecore_evas_callback_resize_set(ee, _canvas_resize_cb);
    ecore_evas_show(ee);

    evas = ecore_evas_get(ee);

    /* Background printing */
    bg = evas_object_rectangle_add(evas);
    evas_object_color_set(bg, 255, 255, 255, 255);      /* white bg */
    evas_object_move(bg, 0, 0); /* at canvas' origin */
    evas_object_resize(bg, WIDTH, HEIGHT);      /* covers full canvas */
    evas_object_show(bg);

    evas_object_focus_set(bg, EINA_TRUE);

    /* Drawing window */
    text = evas_object_text_add(evas);
    evas_object_color_set(text, 255, 0, 0, 255);
    evas_object_resize(text, 150, 50);
    evas_object_text_font_set(text, "Sans", 20);
    evas_object_show(text);

    /* Button on the output drawing vehicle information */
    static Evas_Object *sikaku;
    sikaku = evas_object_rectangle_add(evas);
    evas_object_color_set(sikaku, 255, 0, 0, 100);
    evas_object_move(sikaku, 50, ((int) VTX_EY) + 7);
    int hsz = HEIGHT - (int) VTX_EY - 7 - 7;
    evas_object_resize(sikaku, 200, hsz);
    evas_object_show(sikaku);

    /* draw table */
    static Evas_Object *fix_line[FIX_LINE_NUM];

    for (i = 0; i < FIX_LINE_NUM; i++) {
        fix_line[i] = evas_object_line_add(evas);
        evas_object_color_set(fix_line[i], 55, 55, 55, 255);
        evas_object_line_xy_set(fix_line[i], f_vtx[i][0], f_vtx[i][1],
                                f_vtx[i][2], f_vtx[i][3]);
        evas_object_show(fix_line[i]);
    }

    /* draw order line */
    static Evas_Object *ufix_line[MAX_PARA_NUM - 1];

    for (i = 0; i < MAX_PARA_NUM - 1; i++) {
        ufix_line[i] = evas_object_line_add(evas);
        evas_object_color_set(ufix_line[i], 55, 55, 55, 255);
        evas_object_line_xy_set(ufix_line[i],
                                VTX_SX, VTX_SY + UFIX_LINE_BASE * (i + 1),
                                VTX_EX, VTX_SY + UFIX_LINE_BASE * (i + 1));
        evas_object_show(ufix_line[i]);
    }

    /* draw table item */
    static Evas_Object *num_text[MAX_PARA_NUM];
    char str[11];
    static Evas_Object *pname_text2[MAX_PARA_NUM];

    for (i = 0; i < MAX_PARA_NUM; i++) {
        /* draw table item (No.) */
        num_text[i] = evas_object_text_add(evas);
        evas_object_color_set(num_text[i], 0, 0, 0, 255);
        evas_object_resize(num_text[i], FIX_SEQ_BASE, UFIX_LINE_BASE);
        evas_object_text_font_set(num_text[i], "Sans",
                                  (UFIX_LINE_BASE / 3) * 2);
        evas_object_show(num_text[i]);
        sprintf(str, "%d", i + 1);
        evas_object_text_text_set(num_text[i], str);
        evas_object_move(num_text[i], VTX_SX + FIX_SEQ_BASE / 4,
                         VTX_SY + UFIX_LINE_BASE * (i) + UFIX_LINE_BASE / 6);

        /* draw table item (Name of vehicle information) */
        pname_text2[i] = evas_object_text_add(evas);
        evas_object_color_set(pname_text2[i], 0, 0, 0, 255);
        evas_object_resize(pname_text2[i], 150, 50);
        evas_object_text_font_set(pname_text2[i], "Sans",
                                  (UFIX_LINE_BASE / 3) * 2);
        evas_object_show(pname_text2[i]);
        evas_object_text_text_set(pname_text2[i], vic_data[i].name);
        evas_object_move(pname_text2[i], FIX_SEQ_LINE1 + FIX_SEQ_BASE / 4,
                         VTX_SY + UFIX_LINE_BASE * (i) + UFIX_LINE_BASE / 6);

        /* draw table item (Value of vehicle information) */
        vic_val_text[i] = evas_object_text_add(evas);
        evas_object_color_set(vic_val_text[i], 0, 0, 0, 255);
        evas_object_resize(vic_val_text[i], FIX_SEQ_BASE, UFIX_LINE_BASE);
        evas_object_text_font_set(vic_val_text[i], "Sans",
                                  (UFIX_LINE_BASE / 3) * 2);
        evas_object_show(vic_val_text[i]);
        evas_object_move(vic_val_text[i], FIX_SEQ_LINE2 + FIX_SEQ_BASE / 4,
                         VTX_SY + UFIX_LINE_BASE * (i) + UFIX_LINE_BASE / 6);
    }

    /* entry of the function event callback Vehicle Information */
    evas_object_event_callback_add(sikaku, EVAS_CALLBACK_MOUSE_DOWN,
                                   _on_mousedown, vic_val_text);

    /* drawing current time */
    static Evas_Object *time_text;

    time_text = evas_object_text_add(evas);
    evas_object_color_set(time_text, 0, 0, 0, 255);
    evas_object_resize(time_text, 150, 50);
    evas_object_text_font_set(time_text, "Sans", 20);
    evas_object_show(time_text);
    evas_object_move(time_text, 400, VTX_SY - 22);

    /* entry of the function event Interval Timer */
    ecore_timer_add(0.1, _timer_cb, time_text);

    _canvas_resize_cb(ee);
    fprintf(stdout, commands);
    ecore_main_loop_begin();

    ico_apf_ecore_term();

    ecore_evas_free(ee);
    ecore_evas_shutdown();

    if (NULL != g_connection) {
        dbus_connection_unref(g_connection);
        g_connection = NULL;
    }
    uim_debug("main EXIT");
    return 0;

  error:
    fprintf(stderr, "You got to have at least one Evas engine built"
            " and linked up to ecore-evas for this example to run"
            " properly.\n");
    ecore_evas_shutdown();
    if (NULL != g_connection) {
        dbus_connection_unref(g_connection);
        g_connection = NULL;
    }
    return -1;
}
