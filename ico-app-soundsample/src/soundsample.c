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
#include <Elementary.h>
//#include <Ecore_X.h>
#include <app.h>
#include <unistd.h>
#include <glib.h>
#include <bundle.h>
//#include "ico_apf.h"
//#include "ico_apf_ecore.h"
//#include "ico_apf_log.h"
#include "soundsample.h"
#include <aul.h>
#include "ico_log.h"

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
#define STATUS_BAR_HEIGHT (64)
#define CTRL_BAR_HEIGHT   (128)
#define WIDTH  (1080) /* Base Background width  */
#define HEIGHT (1920 - STATUS_BAR_HEIGHT - CTRL_BAR_HEIGHT) /* Base Background height */

#define PACKAG            "ico-app-soundsample"
#define CONFIG_FILE       "/usr/apps/org.tizen.ico.app-soundsample/res/soundsample_config.txt"
#define BG_IMAGE_FILE     "/usr/apps/org.tizen.ico.app-soundsample/res/images/sound_bg.png"

#define MAX_BUTTON_NUM    (3)
#define MAX_DRAW_LEM      (20)

#define GROUP_DATA        "data"
#define KEY_WAVFILE_PATH  "wavfile_path"
#define KEY_SERVER_IP     "server_ip"
#define KEY_DEVICE_NAME   "device_name"
#define KEY_VOLUME1       "volume1"
#define KEY_VOLUME2       "volume2"
#define KEY_APP_NAME      "app_name"
#define KEY_STREAM_NAME   "stream_name"
#define KEY_REPEAT_FLG    "repeat_flg"
#define KEY_MEDIA_ROLE    "media_role"

/* font */
#define FONT_SIZE       48
#define FONT_FAMILY     (char *)"Sans"  // Mono, Sans, Serif

/* Text class name */
#define TEXT_BUTTON     (char *)"button"
#define TEXT_LABEL      (char *)"label"

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
    char icon_name[64];
    void *func;
};

struct appdata_t
{
    Evas_Object *win;           //main window
    Evas_Object *bg;

    Evas_Object *snd_type;
    Evas_Object *btn_name[MAX_BUTTON_NUM];
    Evas_Object *btn[MAX_BUTTON_NUM];
    Evas_Object *icon[MAX_BUTTON_NUM];
    Evas_Object *sts_txt;
    Evas_Object *app_name;
    Evas_Object *strm_name;
    Evas_Object *pid_txt;
};

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static void _on_mousedown1(void *data, Evas_Object *obj, void *event_info);
static void _on_mousedown2(void *data, Evas_Object *obj, void *event_info);
static void _on_mousedown3(void *data, Evas_Object *obj, void *event_info);
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
//static int get_sound(int argc, char **argv);
static bool app_create(void *data);
static Evas_Object *_create_win(const char *name);
static void _win_del(void *data, Evas_Object *obj, void *event_info);
static void app_terminate(void *data);
static void _winCreate(void);
/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
extern int pulse_main(struct audio_config_t *, int);

/* application data  */
static struct appdata_t Ad;

/* screen size  */
static double Width = 0;
static double Height = 0;
static double W_mag = 0;
static double H_mag = 0;

/* state values */
static int StateNum = STATE_STOP;

/* file descriptor */
static int Filedes[2];
static int Filedes2[2];

/* Child process ID */
static pid_t ChPid = 0;

/* Config data */
static struct audio_config_t AudioConfig = {
    NULL, NULL, NULL, -1, -1, NULL, NULL
};

static struct object_text_val_t SndTypeText = {
    {0, 0, 0, 255}, {500, 50}, {"Sans", 48}, {25, 25}, "", NULL
};

static struct object_text_val_t ButtonText[MAX_BUTTON_NUM] = {
    {{255, 0, 0, 255}, {220, 50}, {"Sans", 48}, {100 + 65, 90}, "START",
     NULL},
    {{0, 255, 0, 255}, {220, 50}, {"Sans", 48}, {370 + 75, 90}, "STOP", NULL},
    {{0, 0, 255, 255}, {220, 50}, {"Sans", 48}, {640 + 65, 90}, "PAUSE", NULL}
};

static struct object_figure_val_t ButtonFigr[MAX_BUTTON_NUM] = {
    {{255, 0, 0, 255}, {220, 95}, {130, 140}, "media_player/play",
     (void *) _on_mousedown1},
    {{0, 255, 0, 255}, {220, 95}, {400, 140}, "media_player/stop",
     (void *) _on_mousedown2},
    {{0, 0, 255, 255}, {220, 95}, {670, 140}, "media_player/pause",
     (void *) _on_mousedown3}
};

static struct object_text_val_t StateInfoText = {
    {0, 0, 0, 255}, {500, 50}, {"Sans", 48}, {330, 300}, "STATE : STOP", NULL
};

static struct object_text_val_t AppNameText = {
    {0, 0, 0, 255}, {900, 50}, {"Sans", 32}, {150, 410}, "App Name :  ", NULL
};

static struct object_text_val_t StreamNameText = {
    {0, 0, 0, 255}, {900, 50}, {"Sans", 32}, {150, 490}, "Stream Name :  ",
    NULL
};

static struct object_text_val_t PidText = {
    {0, 0, 0, 255}, {900, 50}, {"Sans", 36}, {150, 570}, "PID :  ", NULL
};

static char SsndType[32];

/*============================================================================*/
/* Function                                                                   */
/*============================================================================*/
static void _on_mousedown1(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("_on_mousedown1: Enter");
    ICO_DBG("Input event   : START");

    rcv_event(START_REQ);

    ICO_DBG("_on_mousedown1: Leave");
}

static void _on_mousedown2(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("_on_mousedown2: Enter");
    ICO_DBG("Input event  : STOP");

    rcv_event(STOP_REQ);

    ICO_DBG("_on_mousedown2: Leave");
}

static void _on_mousedown3(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("_on_mousedown3: Enter");
    ICO_DBG("Input event  : PAUSE");

    rcv_event(PAUSE_REQ);

    ICO_DBG("_on_mousedown3: Leave");
}

static int start_audio(void)
{
    ICO_DBG("start_audio: Enter");

    int ret = 0;

    signal(SIGCLD, SIG_IGN);

    if ((ChPid = fork()) < 0) {
        ICO_DBG("Err fork");
        return -1;
    }
    else if (ChPid == 0) {
        /* Processing of child processes to start */
        close(Filedes[1]);
        close(Filedes2[0]);

        ICO_DBG("Voice response : START");
        ret = pulse_main(&AudioConfig, Filedes[0]);
        ICO_DBG("Voice response : END");

        ICO_DBG("pulse_main: ret = [%d]", ret);
        recv_event_res(ret);

        close(Filedes[0]);
        close(Filedes2[1]);
        /* Termination of child processes */
        exit(0);
    }

    /* Processing of parent process */
    ICO_DBG("Child process: ChPid = [%d]", ChPid);

    ICO_DBG("start_audio: Leave");

    return 0;
}

static Eina_Bool stop_audio(void *data, Ecore_Fd_Handler *fd_handler)
{
    ICO_DBG("stop_audio: Enter");

    char buff[255];
    memset(buff, 0x00, sizeof(buff));

    /* Reading from a pipe */
    read(Filedes2[0], buff, sizeof(buff));
    ICO_DBG("buff :%s", buff);

    if (atoi(buff) != 0) {
        ICO_DBG("pulse_main Err: ret= [%d]", atoi(buff));
        _win_del(NULL, NULL, NULL);
    }

    /* State change */
    chg_state(STATE_STOP);

    /* Process id initialization */
    ChPid = 0;

    ICO_DBG("stop_audio: Leave");

    return ECORE_CALLBACK_RENEW;
}

static void rcv_event(int event_num)
{
    ICO_DBG("rcv_event: Enter");

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
            ICO_DBG("Not receive: StateNum = [%d],event_num = [%d]",
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
                ICO_DBG("start_audio Err: ret= [%d]", ret);
                _win_del(NULL, NULL, NULL);
            }
            break;

        default:
            ICO_DBG("Not receive: StateNum = [%d],event_num = [%d]",
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
            ICO_DBG("Not receive: StateNum = [%d],event_num = [%d]",
                      StateNum, event_num);
            break;
        }

        break;

    default:
        ICO_DBG("Not support StateNum: StateNum = [%d]", StateNum);
        break;
    }

    ICO_DBG("rcv_event: Leave");
}

static void send_event_req(int event_num)
{
    ICO_DBG("send_event_req: Enter");

    char msg[32];
    memset(msg, 0x00, sizeof(msg));

    snprintf(msg, sizeof(msg), "%d", event_num);
    ICO_DBG("msg = %s", msg);

    write(Filedes[1], msg, strlen(msg) + 1);
    ICO_DBG("send_event_req: Leave");
}

static void recv_event_res(int ret)
{
    ICO_DBG("recv_event_res: Enter");

    char msg[32];
    memset(msg, 0x00, sizeof(msg));

    snprintf(msg, sizeof(msg), "%d", ret);
    ICO_DBG("msg = %s", msg);

    write(Filedes2[1], msg, strlen(msg) + 1);
    ICO_DBG("recv_event_res: Leave");
}

static void chg_state(int state_num)
{
    ICO_DBG("chg_state: Enter");
    ICO_DBG("chg_state state_num = [%d]", state_num);

//    char buf[ICO_UXF_MAX_PROCESS_NAME + 1];
    char buf[64];
    memset(buf, 0x00, sizeof(buf));

    switch (state_num) {
    case STATE_START:
        snprintf(buf, sizeof(buf), "%s", "STATE : START");
        elm_object_text_set(Ad.sts_txt, buf);
        StateNum = state_num;
        break;

    case STATE_STOP:
        snprintf(buf, sizeof(buf), "%s", "STATE : STOP");
        elm_object_text_set(Ad.sts_txt, buf);
        StateNum = state_num;
        break;

    case STATE_PAUSE:
        snprintf(buf, sizeof(buf), "%s", "STATE : PAUSE");
        elm_object_text_set(Ad.sts_txt, buf);
        StateNum = state_num;
        break;

    default:
        ICO_DBG("Not support State: state_num = [%d]", state_num);
        break;
    }

    ICO_DBG("chg_state StateNum = [%d]", StateNum);
    ICO_DBG("chg_state: Leave");
}

static void conf_check_gerror(char *para_num, GError ** error)
{
    ICO_DBG("conf_check_gerror: Enter");

    if (*error != NULL) {
        ICO_DBG("Config [%s] : %s", para_num, (*error)->message);
    }
    g_clear_error(error);

    ICO_DBG("conf_check_gerror: Leave");
}

static int read_config(void)
{
    ICO_DBG("read_config: Enter");

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
        ICO_DBG("No config data [%s]", KEY_WAVFILE_PATH);
        conf_check_gerror(KEY_WAVFILE_PATH, &error);
        g_key_file_free(keyfile);
        return -1;
    }

    AudioConfig.server_ip = g_key_file_get_string(keyfile, GROUP_DATA,
                                                  KEY_SERVER_IP, &error);
    if ((error) || (strlen(AudioConfig.server_ip) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_SERVER_IP);
        conf_check_gerror(KEY_SERVER_IP, &error);
        AudioConfig.server_ip = NULL;
    }

    AudioConfig.device_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_DEVICE_NAME, &error);
    if ((error) || (strlen(AudioConfig.device_name) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_DEVICE_NAME);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.device_name = NULL;
    }

    AudioConfig.volume1 = g_key_file_get_integer(keyfile, GROUP_DATA,
                                                 KEY_VOLUME1, &error);
    if (error) {
        ICO_DBG("No config data [%s]", KEY_VOLUME1);
        conf_check_gerror(KEY_VOLUME1, &error);
        AudioConfig.volume1 = -1;
    }

    AudioConfig.volume2 = g_key_file_get_integer(keyfile, GROUP_DATA,
                                                 KEY_VOLUME2, &error);
    if (error) {
        ICO_DBG("No config data [%s]", KEY_VOLUME2);
        conf_check_gerror(KEY_VOLUME2, &error);
        AudioConfig.volume2 = -1;
    }

    AudioConfig.app_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                 KEY_APP_NAME, &error);
    if ((error) || (strlen(AudioConfig.app_name) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_APP_NAME);
        conf_check_gerror(KEY_APP_NAME, &error);
        AudioConfig.app_name = "TP_PulseAudio";
    }

    AudioConfig.stream_name = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_STREAM_NAME, &error);
    if ((error) || (strlen(AudioConfig.stream_name) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_STREAM_NAME);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.stream_name = "Pri0";
    }

    AudioConfig.repeat_flg = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_REPEAT_FLG, &error);
    if ((error) || (strlen(AudioConfig.repeat_flg) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_REPEAT_FLG);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.repeat_flg = "OFF";
    }

    AudioConfig.media_role = g_key_file_get_string(keyfile, GROUP_DATA,
                                                    KEY_MEDIA_ROLE, &error);
    if ((error) || (strlen(AudioConfig.media_role) <= 0)) {
        ICO_DBG("No config data [%s]", KEY_MEDIA_ROLE);
        conf_check_gerror(KEY_DEVICE_NAME, &error);
        AudioConfig.media_role = "none";
    }

    ICO_DBG("AudioConfig.wavfile_path = [%s]", AudioConfig.wavfile_path);
    ICO_DBG("AudioConfig.server_ip = [%s]", AudioConfig.server_ip);
    ICO_DBG("AudioConfig.device_name = [%s]", AudioConfig.device_name);
    ICO_DBG("AudioConfig.volume1 = [%d]", AudioConfig.volume1);
    ICO_DBG("AudioConfig.volume2 = [%d]", AudioConfig.volume2);
    ICO_DBG("AudioConfig.app_name = [%s]", AudioConfig.app_name);
    ICO_DBG("AudioConfig.stream_name = [%s]", AudioConfig.stream_name);
    ICO_DBG("AudioConfig.repeat_flg = [%s]", AudioConfig.repeat_flg);
    ICO_DBG("AudioConfig.media_role = [%s]", AudioConfig.media_role);
    ICO_DBG("read_config: Leave");
    return 0;
}

static void draw_text(Evas_Object *obj, struct object_text_val_t *text)
{
    ICO_DBG("draw_text: Enter");

//    evas_object_color_set(obj, text->color.r, text->color.g, text->color.b,
//                          text->color.a);
    evas_object_resize(obj, text->size.w * W_mag, text->size.h * H_mag);
    evas_object_move(obj, text->pos.x * W_mag, text->pos.y * H_mag);
    evas_object_show(obj);
    elm_object_text_set(obj, text->text);

    ICO_DBG("draw_text: Leave");
}

static void draw_figr(Evas_Object *obj, struct object_figure_val_t *figr)
{
    ICO_DBG("draw_figr: Enter");

//    evas_object_color_set(obj, figr->color.r, figr->color.g, figr->color.b,
//                          figr->color.a);
    evas_object_resize(obj, figr->size.w * W_mag, figr->size.h * H_mag);
    evas_object_move(obj, figr->pos.x * W_mag, figr->pos.y * H_mag);
    evas_object_show(obj);

    ICO_DBG("draw_figr: Leave");
}
#if 0
static void res_callback(ico_apf_resource_notify_info_t *info,
                         void *user_data)
{
    int ret;

    ICO_DBG
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
            ICO_DBG("##==> callback reply int_sound(%s,%d,1) = %d",
                      info->device, info->id, ret);
        }
        else {
            ret =
                ico_apf_resource_reply_sound_mode(info->device, info->id, 1);
            ICO_DBG("##==> callback reply sound(%s,%d,1) = %d",
                      info->device, info->id, ret);
        }
        break;
    default:
        /* NOP  */
        break;
    }
}
#endif
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

    if (Ad.snd_type) {
        evas_object_del(Ad.snd_type);
        Ad.snd_type = NULL;
    }

    for (i = 0; i < MAX_BUTTON_NUM; i++) {
        if (Ad.btn[i]) {
            evas_object_del(Ad.btn[i]);
            Ad.btn[i] = NULL;
        }

        if (Ad.btn_name[i]) {
            evas_object_del(Ad.btn_name[i]);
            Ad.btn_name[i] = NULL;
        }

        if (Ad.icon[i]) {
            evas_object_del(Ad.icon[i]);
            Ad.icon[i] = NULL;
        }
    }

    if (Ad.sts_txt) {
        evas_object_del(Ad.sts_txt);
        Ad.sts_txt = NULL;
    }

    if (Ad.app_name) {
        evas_object_del(Ad.app_name);
        Ad.app_name = NULL;
    }

    if (Ad.strm_name) {
        evas_object_del(Ad.strm_name);
        Ad.strm_name = NULL;
    }

    if (Ad.pid_txt) {
        evas_object_del(Ad.pid_txt);
        Ad.pid_txt = NULL;
    }

    ICO_DBG("LEAVE app_terminate");
    return;
}

static void _winCreate(void)
{
    ICO_DBG("ENTER _winCreate");

    char buf[256];
    char buf2[256];
    int i;

    memset(buf, 0x00, sizeof(buf));
    memset(buf2, 0x00, sizeof(buf));

    if (NULL == Ad.win) {
        ICO_DBG("Err Param NG");
        return;
    }

    /* Sound Type  */
    Ad.snd_type = elm_label_add(Ad.win);
    draw_text(Ad.snd_type, &SndTypeText);
    elm_object_text_set(Ad.snd_type, SsndType);

    /* Button */
    for (i = 0; i < MAX_BUTTON_NUM; i++) {
        /* Button Text */
        Ad.btn_name[i] = elm_label_add(Ad.win);
        draw_text(Ad.btn_name[i], &ButtonText[i]);

        /* Button Object */
        Ad.btn[i] = elm_button_add(Ad.win);
        Ad.icon[i] = elm_icon_add(Ad.win);
        elm_icon_standard_set(Ad.icon[i], ButtonFigr[i].icon_name);
        elm_object_part_content_set(Ad.btn[i], "icon", Ad.icon[i]);
        draw_figr(Ad.btn[i], &ButtonFigr[i]);
        evas_object_smart_callback_add(Ad.btn[i], "clicked",
                                       (Evas_Smart_Cb) (ButtonFigr[i].func),
                                       NULL);
    }

    /* Status Text */
    Ad.sts_txt = elm_label_add(Ad.win);
    draw_text(Ad.sts_txt, &StateInfoText);

    /* Application Name */
    Ad.app_name = elm_label_add(Ad.win);
    draw_text(Ad.app_name, &AppNameText);

    if (strlen(AudioConfig.app_name) > MAX_DRAW_LEM) {
        snprintf(buf2, MAX_DRAW_LEM, "%s", AudioConfig.app_name);
        snprintf(buf, sizeof(buf), "%s%s...", "App name :  ", buf2);
    }
    else {
        snprintf(buf, sizeof(buf), "%s%s", "App name :  ",
                 AudioConfig.app_name);
    }
    elm_object_text_set(Ad.app_name, buf);

    /* Stream Name */
    Ad.strm_name = elm_label_add(Ad.win);
    draw_text(Ad.strm_name, &StreamNameText);

    if (strlen(AudioConfig.stream_name) > MAX_DRAW_LEM) {
        snprintf(buf2, MAX_DRAW_LEM, "%s", AudioConfig.stream_name);
        snprintf(buf, sizeof(buf), "%s%s...", "Stream name :  ", buf2);
    }
    else {
        snprintf(buf, sizeof(buf), "%s%s", "Stream name :  ",
                 AudioConfig.stream_name);
    }
    elm_object_text_set(Ad.strm_name, buf);

    /* PID Text */
    Ad.pid_txt = elm_label_add(Ad.win);
    draw_text(Ad.pid_txt, &PidText);
    snprintf(buf, sizeof(buf), "%s%d", "PID :  ", getpid());
    elm_object_text_set(Ad.pid_txt, buf);

    ICO_DBG("LEAVE _winCreate");
    return;
}

static void _win_del(void *data, Evas_Object *obj, void *event_info)
{
    ICO_DBG("ENTER _win_del");

    elm_exit();

    ICO_DBG("LEAVE _win_del");
    return;
}

static Evas_Object *_create_win(const char *name)
{
    ICO_DBG("ENTER _create_win");

    Evas_Object *eo;
    eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
    if (eo) {
        elm_win_title_set(eo, name);
        evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
    }
    ICO_DBG("LEAVE _create_win");

    return eo;
}

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

    W_mag = Width / WIDTH;
    H_mag = Height / HEIGHT;

    ICO_DBG("Width =%f,Height=%f", Width, Height);
    ICO_DBG("W_mag =%f,H_mag=%f", W_mag, H_mag);

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
    (void)elm_config_font_overlay_set(TEXT_BUTTON, FONT_FAMILY, FONT_SIZE);
    (void)elm_config_font_overlay_set(TEXT_LABEL, FONT_FAMILY, FONT_SIZE);
    (void)elm_config_font_overlay_apply();

    _winCreate();

    ICO_DBG("LEAVE app_create");

    return TRUE;                /* EXIT_SUCCESS */
}

#if 0
static int get_sound(int argc, char **argv)
{
    ICO_DBG("ENTER get_sound");

    int getsound;
    bundle *b;
    const char *val;

    /* get resource control option  */
    b = bundle_import_from_argv(argc, argv);
    getsound = 0;
    SsndType[0] = 0;
    if (b != NULL) {
        val = bundle_get_val(b, "rightoption");
        if (val != NULL) {
            if (strcasecmp(val, "-basesound") == 0) {
                getsound = 1;   /* get base sound       */
                strcpy(SsndType, "BaseSound");
            }
            else if (strcasecmp(val, "-intsound") == 0) {
                getsound = 2;   /* get interrupt sound  */
                strcpy(SsndType, "IntSound");
            }
            else if (strncasecmp(val, "-int=", 5) == 0) {
                getsound = strtol(val + 5, (char **) 0, 0);
                getsound += 2;  /* get interrupt sound  */
                sprintf(SsndType, "IntSound.%d", getsound - 2);
            }
        }
    }

    if (getsound > 0) {
        /* initialize resource control for Ecore */
        if (ico_apf_ecore_init(NULL) != ICO_APF_E_NONE) {
            ICO_DBG("ico_apf_ecore_init() Error");
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
    ICO_DBG("LEAVE get_sound");
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
        ico_log_open("org.tizen.ico.app-soundsample");
    }

    ICO_DBG("ENTER main");

    if (pipe(Filedes) == -1) {
        ICO_DBG("Err pipe Filedes");
        return EXIT_FAILURE;
    }

    if (pipe(Filedes2) == -1) {
        ICO_DBG("Err pipe Filedes2");
        return EXIT_FAILURE;
    }

    if (read_config() == -1) {
        ICO_DBG("Err Config Read NG");
        return EXIT_FAILURE;
    }

    if (!ecore_evas_init()) {
        return EXIT_FAILURE;
    }

    /* get argument */
//    if (get_sound(argc, argv) != 0) {
//        ICO_DBG("Err get_sound");
//        return EXIT_FAILURE;
//    }

    /* File descriptor to monitor callback function entry */
    ecore_main_fd_handler_add(Filedes2[0], ECORE_FD_READ, stop_audio, NULL,
                              NULL, NULL);

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

    close(Filedes[0]);
    close(Filedes[1]);
    close(Filedes2[0]);
    close(Filedes2[1]);

    /* Process check */
    if ((ChPid > 0) && (kill(ChPid, 0) != EOF)) {
        kill(ChPid, SIGKILL);
        ICO_DBG("END Process ChPid = [%d]", ChPid);
    }
    ICO_DBG("main: Leave");

    return result;
}
