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

#ifndef _SAMPLE_H_
#define _SAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    struct audio_config_t
    {
        char *wavfile_path;
        char *server_ip;
        char *device_name;
        int volume1;
        int volume2;
        char *app_name;
        char *stream_name;
        char *repeat_flg;
        char *media_role;
    };

/* STATE */
#define STATE_START  0
#define STATE_STOP   1
#define STATE_PAUSE  2

/* EVENT */
#define START_REQ    0
#define STOP_REQ     1
#define PAUSE_REQ    2

/* PULSE DEFAULT VAL */
#define APP_NAME    "TP_PulseAudio"
#define STREAM_NAME "Pri0"

#ifdef __cplusplus
};
#endif

#endif /* _SAMPLE_H_ */

/**
 * End of File. (soundsample.h)
 */
