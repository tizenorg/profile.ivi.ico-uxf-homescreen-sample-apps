/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   Sound Sample APP
 *          Test use with which sound is sounded
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

#include <unistd.h>
#include <glib.h>
//#include "app_log.h"
#include "ico_apf.h"
#include "ico_apf_ecore.h"
#include "ico_apf_log.h"
#include "soundsample.h"

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
//#define WIDTH  (700)    /* Background width  */
//#define HEIGHT (934)    /* Background height */
//#define WIDTH  (400)    /* Background width  */
//#define HEIGHT (400)    /* Background height */
#define WIDTH  (600)            /* Background width  */
#define HEIGHT (600)            /* Background height */

//#define LOG_NAME       "/home/rpf/var/log/uifw/audioTP.log"
#define LOG_NAME       "/tmp/ico-app-soundsample.log"
//#define CONFIG_FILE    "/home/rpf/src/app/audioTP/soundsample_config.txt"
#define CONFIG_FILE    "/opt/apps/org.tizen.ico.app-soundsample/res/soundsample_config.txt"

#define MAX_BUTTON_NUM 3
#define MAX_DRAW_LEM   20

#define GROUP_DATA        "data"
#define KEY_WAVFILE_PATH  "wavfile_path"
#define KEY_SERVER_IP     "server_ip"
#define KEY_DEVICE_NAME   "device_name"
#define KEY_VOLUME1       "volume1"
#define KEY_VOLUME2       "volume2"
#define KEY_APP_NAME      "app_name"
#define KEY_STREAM_NAME   "stream_name"

/*============================================================================*/
/* Define data types                                                          */
/*============================================================================*/
struct color_val_t
{
    int r;                      /* red   */
    int g;                      /* green */
    int b;                      /* blue  */
    int a;                      /* alpha */
};

struct size_val_t
{
    Evas_Coord w;               /* width  */
    Evas_Coord h;               /* height */
};

struct font_val_t
{
    char name[255];             /* font name */
    Evas_Font_Size size;        /* font size */
};

struct pos_val_t
{
    Evas_Coord x;               /* X position */
    Evas_Coord y;               /* Y position */
};

struct object_text_val_t
{
    struct color_val_t color;
    struct size_val_t size;
    struct font_val_t font;
    struct pos_val_t pos;
    char text[255];
    void *func;
};

struct object_figure_val_t
{
    struct color_val_t color;
    struct size_val_t size;
    struct pos_val_t pos;
    void *func;
};

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static void _canvas_resize_cb(Ecore_Evas *ee);
static void _on_destroy(Ecore_Evas *ee __UNUSED__);
static Eina_Bool _timer_cb(void *data);
static void _on_mousedown1(void *data, Evas *evas, Evas_Object *o,
                           void *einfo);
static void _on_mousedown2(void *data, Evas *evas, Evas_Object *o,
                           void *einfo);
static void _on_mousedown3(void *data, Evas *evas, Evas_Object *o,
                           void *einfo);
static int start_audio(void);
static Eina_Bool stop_audio(void *data, Ecore_Fd_Handler *fd_handler);
static void rcv_event(int event_num);
static void send_event_req(int event_num);
static void recv_event_res(int ret);
static void chg_state(int state_num);
static void conf_check_gerror(char *para_num, GError ** error);
static int read_config(void);
static void draw_text(Evas_Object *obj, struct object_text_val_t *text);
static void draw_figr(Evas_Object *obj, struct object_figure_val_t *figr);

/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
extern int pulse_main(struct audio_config_t *, int);

/* state values */
static int StateNum = STATE_STOP;
static Evas_Object *State_text;

/* file descriptor */
static int Filedes[2];
static int Filedes2[2];

/* Child process ID */
static pid_t ChPid = 0;

/* Config data */
static struct audio_config_t AudioConfig = {
    NULL, NULL, NULL, -1, -1, NULL, NULL
};

static struct object_text_val_t ButtonText[MAX_BUTTON_NUM] = {
    {{255, 0, 0, 255}, {180, 50}, {"Sans", 48}, {15 + 12, 70}, "START", NULL},
    {{0, 255, 0, 255}, {180, 50}, {"Sans", 48}, {210 + 24, 70}, "STOP", NULL},
    {{0, 0, 255, 255}, {180, 50}, {"Sans", 48}, {405 + 12, 70}, "PAUSE", NULL}
};

static struct object_figure_val_t ButtonFigr[MAX_BUTTON_NUM] = {
    {{255, 0, 0, 100}, {180, 95}, {15, 120}, (void *) _on_mousedown1},
    {{0, 255, 0, 100}, {180, 95}, {210, 120}, (void *) _on_mousedown2},
    {{0, 0, 255, 100}, {180, 95}, {405, 120}, (void *) _on_mousedown3}
};

static struct object_figure_val_t BgFigr = {
    {255, 255, 255, 255}, {WIDTH, HEIGHT}, {0, 0}, NULL
};

static struct object_text_val_t TimeText = {
    {0, 0, 0, 255}, {150, 50}, {"Sans", 24}, {0, 0}, {}, (void *) _timer_cb
};

static struct object_text_val_t StateInfoText = {
    {0, 0, 0, 255}, {150, 50}, {"Sans", 48}, {140, 300}, "STATE : STOP", NULL
};

static struct object_text_val_t AppNameText = {
    {0, 0, 0, 255}, {150, 50}, {"Sans", 32}, {25, 440}, "App Name :  ", NULL
};

static struct object_text_val_t StreamNameText = {
    {0, 0, 0, 255}, {150, 50}, {"Sans", 32}, {25, 480}, "Stream Name :  ",
        NULL
};

static struct object_text_val_t PidText = {
    {0, 0, 0, 255}, {150, 50}, {"Sans", 36}, {50, 530}, "PID :  ", NULL
};

static Ecore_Evas *ee;
static Evas_Object *text, *bg;
static char ssndtype[32];

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
    snprintf(buf, sizeof(buf), "%s %d x %d", ssndtype, w, h);
    evas_object_text_text_set(text, buf);
    evas_object_move(text, 50, 25);
    evas_object_resize(bg, w, h);
}

static void _on_destroy(Ecore_Evas *ee __UNUSED__)
{
    ecore_main_loop_quit();
}

static Eina_Bool _timer_cb(void *data)
{
    char str[32];
    time_t timer;
    struct tm *date;
    timer = time(NULL);
    date = localtime(&timer);
    sprintf(str, "%s", asctime(date));
    evas_object_text_text_set((Evas_Object *) data, str);

    return EINA_TRUE;
}

static void _on_mousedown1(void *data, Evas *evas, Evas_Object *o,
                           void *einfo)
{
    uim_debug("_on_mousedown1: Enter");
    uim_debug("Input event   : START");

    rcv_event(START_REQ);

    uim_debug("_on_mousedown1: Leave");
}

static void _on_mousedown2(void *data, Evas *evas, Evas_Object *o,
                           void *einfo)
{
    uim_debug("_on_mousedown2: Enter");
    uim_debug("Input event  : STOP");

    rcv_event(STOP_REQ);

    uim_debug("_on_mousedown2: Leave");
}

static void _on_mousedown3(void *data, Evas *evas, Evas_Object *o,
                           void *einfo)
{
    uim_debug("_on_mousedown3: Enter");
    uim_debug("Input event  : PAUSE");

    rcv_event(PAUSE_REQ);

    uim_debug("_on_mousedown3: Leave");
}

static int start_audio(void)
{
    uim_debug("start_audio: Enter");

    int ret = 0;

    signal(SIGCLD, SIG_IGN);

    if ((ChPid = fork()) < 0) {
        uim_debug("Err fork");
        return -1;
    }
    else if (ChPid == 0) {
        /* Processing of child processes to start */
        close(Filedes[1]);
        close(Filedes2[0]);

        uim_debug("Voice response : START");
        ret = pulse_main(&AudioConfig, Filedes[0]);
        uim_debug("Voice response : END");

        uim_debug("pulse_main: ret = [%d]", ret);
        recv_event_res(ret);

        close(Filedes[0]);
        close(Filedes2[1]);
        /* Termination of child processes */
        exit(0);
    }

    /* Processing of parent process */
    uim_debug("Child process: ChPid = [%d]", ChPid);

    uim_debug("start_audio: Leave");

    return 0;
}

static Eina_Bool stop_audio(void *data, Ecore_Fd_Handler *fd_handler)
{
    uim_debug("stop_audio: Enter");

    char buff[255];
    memset(buff, 0x00, sizeof(buff));

    /* Reading from a pipe */
    read(Filedes2[0], buff, sizeof(buff));
    uim_debug("buff :%s", buff);

    if (atoi(buff) != 0) {
        uim_debug("pulse_main Err: ret= [%d]", atoi(buff));
        _on_destroy(NULL);
    }

    /* State change */
    chg_state(STATE_STOP);

    /* Process id initialization */
    ChPid = 0;

    uim_debug("stop_audio: Leave");

    return ECORE_CALLBACK_RENEW;
}

static void rcv_event(int event_num)
{
    uim_debug("rcv_event: Enter");

    int ret = 0;

    switch (StateNum) {
    case STATE_START:
        switch (event_num) {
        case STOP_REQ:
            /* event send */
            send_event_req(event_num);
            /* State change */
            chg_state(STATE_STOP);
            break;

        case PAUSE_REQ:
            /* event send */
            send_event_req(event_num);
            /* State change */
            chg_state(STATE_PAUSE);
            break;

        default:
            uim_debug("Not receive: StateNum = [%d],event_num = [%d]",
                        StateNum, event_num);
            break;
        }

        break;

    case STATE_STOP:
        switch (event_num) {
        case START_REQ:
            /* Voice reproduction start */
            ret = start_audio();
            if (ret == 0) {
                /* State change */
                chg_state(STATE_START);
            }
            else {
                uim_debug("start_audio Err: ret= [%d]", ret);
                _on_destroy(NULL);
            }
            break;

        default:
            uim_debug("Not receive: StateNum = [%d],event_num = [%d]",
                        StateNum, event_num);
            break;
        }

        break;

    case STATE_PAUSE:
        switch (event_num) {
        case STOP_REQ:
            /* event send */
            send_event_req(event_num);
            /* State change */
            chg_state(STATE_STOP);
            break;

        case PAUSE_REQ:
            /* event send */
            send_event_req(event_num);
            /* State change */
            chg_state(STATE_START);
            break;

        default:
            uim_debug("Not receive: StateNum = [%d],event_num = [%d]",
                        StateNum, event_num);
            break;
        }

        break;

    default:
        uim_debug("Not support StateNum: StateNum = [%d]", StateNum);
        break;
    }

    uim_debug("rcv_event: Leave");
}

static void send_event_req(int event_num)
{
    uim_debug("send_event_req: Enter");

    char msg[32];
    memset(msg, 0x00, sizeof(msg));

    snprintf(msg, sizeof(msg), "%d", event_num);
    uim_debug("msg = %s", msg);

    write(Filedes[1], msg, strlen(msg) + 1);
    uim_debug("send_event_req: Leave");
}

static void recv_event_res(int ret)
{
    uim_debug("recv_event_res: Enter");

    char msg[32];
    memset(msg, 0x00, sizeof(msg));

    snprintf(msg, sizeof(msg), "%d", ret);
    uim_debug("msg = %s", msg);

    write(Filedes2[1], msg, strlen(msg) + 1);
    uim_debug("recv_event_res: Leave");
}

static void chg_state(int state_num)
{
    uim_debug("chg_state: Enter");
    uim_debug("chg_state state_num = [%d]", state_num);

    char buf[ICO_UXF_MAX_PROCESS_NAME + 1];
    memset(buf, 0x00, sizeof(buf));

    switch (state_num) {
    case STATE_START:
        snprintf(buf, sizeof(buf), "%s", "STATE : START");
        evas_object_text_text_set(State_text, buf);
        StateNum = state_num;
        break;

    case STATE_STOP:
        snprintf(buf, sizeof(buf), "%s", "STATE : STOP");
        evas_object_text_text_set(State_text, buf);
        StateNum = state_num;
        break;

    case STATE_PAUSE:
        snprintf(buf, sizeof(buf), "%s", "STATE : PAUSE");
        evas_object_text_text_set(State_text, buf);
        StateNum = state_num;
        break;

    default:
        uim_debug("Not support State: state_num = [%d]", state_num);
        break;
    }

    uim_debug("chg_state StateNum = [%d]", StateNum);
    uim_debug("chg_state: Leave");
}

static void conf_check_gerror(char *para_num, GError ** error)
{
    uim_debug("conf_check_gerror: Enter");

    if (*error != NULL) {
        uim_debug("Config [%s] : %s", para_num, (*error)->message);
    }
    g_clear_error(error);

    uim_debug("conf_check_gerror: Leave");
}

static int read_config(void)
{
    uim_debug("read_config: Enter");

    GKeyFile *keyfile;
    GKeyFileFlags flags;
    GError *error = NULL;

    keyfile = g_key_file_new();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
    if (!g_key_file_load_from_file(keyfile, CONFIG_FILE, flags, &error)) {
        conf_check_gerror(CONFIG_FILE, &error);
        g_key_file_free(keyfile);
        return -1;
    }

    AudioConfig.wavfile_path = g_key_file_get_string(keyfile, GROUP_DATA,
                                                     KEY_WAVFILE_PATH,
                                                     &error);
    if ((error) || (strlen(AudioConfig.wavfile_path) <= 0)) {
        uim_debug("No config data [%s]", KEY_WAVFILE_PATH);
        conf_check_gerror(KEY_WAVFILE_PATH, &error);
        g_key_file_free(keyfile);
        return -1;
    }

    AudioConfig.server_ip = g_key_file_get_string(keyfile, GROUP_DATA,
                                                  KEY_SERVER_IP, &error);
    if ((error) || (strlen(AudioConfig.server_ip) <= 0)) {
        uim_debug("No config data [%s]", KEY_SERVER_IP);
        conf_check_gerror(KEY_SERVER_IP, &error);
        AudioConfig.server_ip = NULL;
    }

    AudioConfig.device_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_DEVICE_NAME, &error);
    if ((error) || (strlen(AudioConfig.device_name) <= 0)) {
        uim_debug("No config data [%s]", KEY_DEVICE_NAME);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.device_name = NULL;
    }

    AudioConfig.volume1 = g_key_file_get_integer(keyfile, GROUP_DATA,
                                                 KEY_VOLUME1, &error);
    if (error) {
        uim_debug("No config data [%s]", KEY_VOLUME1);
        conf_check_gerror(KEY_VOLUME1, &error);
        AudioConfig.volume1 = -1;
    }

    AudioConfig.volume2 = g_key_file_get_integer(keyfile, GROUP_DATA,
                                                 KEY_VOLUME2, &error);
    if (error) {
        uim_debug("No config data [%s]", KEY_VOLUME2);
        conf_check_gerror(KEY_VOLUME2, &error);
        AudioConfig.volume2 = -1;
    }

    AudioConfig.app_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                 KEY_APP_NAME, &error);
    if ((error) || (strlen(AudioConfig.app_name) <= 0)) {
        uim_debug("No config data [%s]", KEY_APP_NAME);
        conf_check_gerror(KEY_APP_NAME, &error);
        AudioConfig.app_name = "TP_PulseAudio";
    }

    AudioConfig.stream_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_STREAM_NAME, &error);
    if ((error) || (strlen(AudioConfig.stream_name) <= 0)) {
        uim_debug("No config data [%s]", KEY_STREAM_NAME);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.stream_name = "Pri0";
    }

    uim_debug("read_config: Leave");
    return 0;
}

static void draw_text(Evas_Object *obj, struct object_text_val_t *text)
{
    uim_debug("draw_text: Enter");

    evas_object_color_set(obj, text->color.r, text->color.g, text->color.b,
                          text->color.a);
    evas_object_resize(obj, text->size.w, text->size.h);
    evas_object_text_font_set(obj, text->font.name, text->font.size);
    evas_object_move(obj, text->pos.x, text->pos.y);
    evas_object_show(obj);
    evas_object_text_text_set(obj, text->text);

    uim_debug("draw_text: Leave");
}

static void draw_figr(Evas_Object *obj, struct object_figure_val_t *figr)
{
    uim_debug("draw_figr: Enter");

    evas_object_color_set(obj, figr->color.r, figr->color.g, figr->color.b,
                          figr->color.a);
    evas_object_resize(obj, figr->size.w, figr->size.h);
    evas_object_move(obj, figr->pos.x, figr->pos.y);
    evas_object_show(obj);

    uim_debug("draw_figr: Leave");
}

static void res_callback(ico_apf_resource_notify_info_t *info,
                         void *user_data)
{
    int ret;

    uim_debug
        ("##==> Callbacked! evt=%d res=%d id=%d bid=%d appid=%s dev=%s "
         "user_data=%d", info->state, info->resid, info->id, info->bid,
         info->appid, info->device, (int) user_data);

    switch (info->state) {
    case ICO_APF_RESOURCE_STATE_ACQUIRED:
    case ICO_APF_RESOURCE_STATE_DEPRIVED:
    case ICO_APF_RESOURCE_STATE_WAITTING:
    case ICO_APF_RESOURCE_STATE_RELEASED:
        if (info->resid == ICO_APF_RESID_INT_SOUND) {
            ret =
                ico_apf_resource_reply_int_sound_mode(info->device, info->id,
                                                      1);
            uim_debug("##==> callback reply int_sound(%s,%d,1) = %d",
                        info->device, info->id, ret);
        }
        else {
            ret =
                ico_apf_resource_reply_sound_mode(info->device, info->id, 1);
            uim_debug("##==> callback reply sound(%s,%d,1) = %d",
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
    int getsound;
    char appid[64];

    /* Log output setup */
    if (ico_apf_get_app_id(0, appid) == ICO_APP_CTL_E_NONE) {
        ico_apf_log_open(appid);
    }

    uim_debug("main: Enter");

    Evas *evas;
    Evas_Object *app_name_text;
    Evas_Object *stream_name_text;
    Evas_Object *pid_text;
    Evas_Object *button_text[MAX_BUTTON_NUM];
    Evas_Object *button_figr[MAX_BUTTON_NUM];
    Evas_Object *time_text;
    char buf[256];
    char buf2[256];
    int i = 0;

    /* get resource control option  */
    getsound = 0;
    ssndtype[0] = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcasecmp(argv[i], "-basesound") == 0) {
                getsound = 1;   /* get base sound       */
                strcpy(ssndtype, "BasecSound");
            }
            else if (strcasecmp(argv[i], "-intsound") == 0) {
                getsound = 2;   /* get interrupt sound  */
                strcpy(ssndtype, "IntSound");
            }
            else if (strncasecmp(argv[i], "-int=", 5) == 0) {
                getsound = strtol(&argv[i][5], (char **) 0, 0);
                getsound += 2;  /* get interrupt sound  */
                sprintf(ssndtype, "IntSound.%d", getsound - 2);
            }
        }
    }

    StateNum = STATE_STOP;
    ChPid = 0;
    memset(buf, 0x00, sizeof(buf));
    memset(buf2, 0x00, sizeof(buf));

    if (pipe(Filedes) == -1) {
        uim_debug("Err pipe Filedes");
        return EXIT_FAILURE;
    }

    if (pipe(Filedes2) == -1) {
        uim_debug("Err pipe Filedes2");
        return EXIT_FAILURE;
    }

    if (read_config() == -1) {
        uim_debug("Err Config Read NG");
        return EXIT_FAILURE;
    }

    uim_debug("AudioConfig.wavfile_path = [%s]", AudioConfig.wavfile_path);
    uim_debug("AudioConfig.server_ip = [%s]", AudioConfig.server_ip);
    uim_debug("AudioConfig.device_name = [%s]", AudioConfig.device_name);
    uim_debug("AudioConfig.volume1 = [%d]", AudioConfig.volume1);
    uim_debug("AudioConfig.volume2 = [%d]", AudioConfig.volume2);
    uim_debug("AudioConfig.app_name = [%s]", AudioConfig.app_name);
    uim_debug("AudioConfig.stream_name = [%s]", AudioConfig.stream_name);

    if (!ecore_evas_init()) {
        return EXIT_FAILURE;
    }

    if (getsound > 0) {
        /* initialize resource control for Ecore */
        if (ico_apf_ecore_init(NULL) != ICO_APF_E_NONE) {
            uim_debug("ico_apf_ecore_init() Error");
            ecore_evas_shutdown();
            return -1;
        }

        /* set resource request callback */
        ico_apf_resource_set_event_cb(res_callback, NULL);

        /* acquire a right to display a screen      */
        if (getsound == 1) {
            ico_apf_resource_get_sound_mode(NULL, 0, 0);
        }
        else {
            ico_apf_resource_get_int_sound_mode(NULL, getsound - 2, 0);
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

    /* Background painting */
    bg = evas_object_rectangle_add(evas);
    draw_figr(bg, &BgFigr);

    evas_object_focus_set(bg, EINA_TRUE);

    /* App Name drawing */
    app_name_text = evas_object_text_add(evas);
    draw_text(app_name_text, &AppNameText);

    if (strlen(AudioConfig.app_name) > MAX_DRAW_LEM) {
        snprintf(buf2, MAX_DRAW_LEM, "%s", AudioConfig.app_name);
        snprintf(buf, sizeof(buf), "%s%s...", "App name :  ", buf2);
    }
    else {
        snprintf(buf, sizeof(buf), "%s%s", "App name :  ",
                 AudioConfig.app_name);
    }

    evas_object_text_text_set(app_name_text, buf);

    /* Drawing window */
    text = evas_object_text_add(evas);
    evas_object_color_set(text, 255, 0, 0, 255);
    evas_object_resize(text, 150, 50);
    evas_object_text_font_set(text, "Sans", 20);
    evas_object_show(text);

    /* Stream Name drawing */
    stream_name_text = evas_object_text_add(evas);
    draw_text(stream_name_text, &StreamNameText);

    if (strlen(AudioConfig.stream_name) > MAX_DRAW_LEM) {
        snprintf(buf2, MAX_DRAW_LEM, "%s", AudioConfig.stream_name);
        snprintf(buf, sizeof(buf), "%s%s...", "Stream name :  ", buf2);
    }
    else {
        snprintf(buf, sizeof(buf), "%s%s", "Stream name :  ",
                 AudioConfig.stream_name);
    }

    evas_object_text_text_set(stream_name_text, buf);

    /* PID drawing */
    pid_text = evas_object_text_add(evas);
    draw_text(pid_text, &PidText);
    snprintf(buf, sizeof(buf), "%s%d", "PID :  ", getpid());
    evas_object_text_text_set(pid_text, buf);

    /* Sound condition drawing */
    State_text = evas_object_text_add(evas);
    draw_text(State_text, &StateInfoText);

    /** Button drawing */
    for (i = 0; i < MAX_BUTTON_NUM; i++) {
        /* Button name drawing */
        button_text[i] = evas_object_text_add(evas);
        draw_text(button_text[i], &ButtonText[i]);

        /* Button Frame drawing */
        button_figr[i] = evas_object_rectangle_add(evas);
        draw_figr(button_figr[i], &ButtonFigr[i]);

        /* Callback function entry */
        evas_object_event_callback_add(button_figr[i],
                                       EVAS_CALLBACK_MOUSE_DOWN,
                                       (Evas_Object_Event_Cb) (ButtonFigr[i].
                                                               func),
                                       State_text);
    }

    /* Drawing of the current time */
    time_text = evas_object_text_add(evas);
    draw_text(time_text, &TimeText);

    /* Timer callback function entry */
    ecore_timer_add(0.1, (Ecore_Task_Cb) (TimeText.func), time_text);

    /* File descriptor to monitor callback function entry */
    ecore_main_fd_handler_add(Filedes2[0], ECORE_FD_READ, stop_audio, NULL,
                              NULL, NULL);

    _canvas_resize_cb(ee);
    fprintf(stdout, commands);
    ecore_main_loop_begin();

    ico_apf_ecore_term();

    ecore_evas_free(ee);
    ecore_evas_shutdown();

    close(Filedes[0]);
    close(Filedes[1]);
    close(Filedes2[0]);
    close(Filedes2[1]);

    /* Process check */
    if ((ChPid > 0) && (kill(ChPid, 0) != EOF)) {
        kill(ChPid, SIGKILL);
        uim_debug("END Process ChPid = [%d]", ChPid);
    }
    uim_debug("main: Leave");

    return 0;

  error:
    fprintf(stderr,
            "You got to have at least one Evas engine built and linked up"
            " to ecore-evas for this example to run properly.\n");
    ecore_evas_shutdown();
    return -1;
}
