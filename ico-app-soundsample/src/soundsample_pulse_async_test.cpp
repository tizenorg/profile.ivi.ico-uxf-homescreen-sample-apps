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

/**
 *  TP using PulseAudio with wavfile
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>
#include <pulse/error.h>

#include "soundsample_yswavfile.h"

//#include "app_log.h"
#include "ico_apf_log.h"
#include "soundsample.h"

#define ICO_PA_STREAM_PAUSE  (1)
#define ICO_PA_STREAM_RESUME (0)

extern "C"
{
    int pulse_main(struct audio_config_t *audio_config, int filedes);
}

static int ExitFlg;
static int ReadFiledes;
#define REPEAT_FLG 1            /* ON:0 ,OFF:1 */

static YSRESULT YsPulseAudioWaitForConnectionEstablished(pa_context *
                                                         paContext,
                                                         pa_mainloop *
                                                         paMainLoop,
                                                         time_t timeOut)
{
    time_t timeLimit = time(NULL) + timeOut;
    while (timeLimit >= time(NULL)) {
        pa_mainloop_iterate(paMainLoop, 0, NULL);
        if (PA_CONTEXT_READY == pa_context_get_state(paContext)) {
            return YSOK;
        }
    }
    return YSERR;
}

static void event_input_callback(pa_mainloop_api *a, pa_io_event *e, int fd,
                                 pa_io_event_flags_t f, void *userdata)
{

    uim_debug("event_input_callback: Enter");

    char buff[255];
    memset(buff, 0x00, sizeof(buff));

    /* From a pipe to reading */
    read(ReadFiledes, buff, sizeof(buff));
    uim_debug("buff :%s", buff);

    switch (atoi(buff)) {
    case STOP_REQ:
        uim_debug("Stop directions reception");
        ExitFlg = 1;
        break;

    case PAUSE_REQ:
        uim_debug("Pause directions reception");

        if (0 == pa_stream_is_corked((pa_stream *) userdata)) {
            pa_stream_cork((pa_stream *) userdata, ICO_PA_STREAM_PAUSE, NULL,
                           NULL);
            uim_debug("pa_stream_cork(PAUSE)");
            sleep(2);
            uim_debug("sleep end");
        }
        else {
            pa_stream_cork((pa_stream *) userdata, ICO_PA_STREAM_RESUME, NULL,
                           NULL);
            uim_debug("pa_stream_cork(RESUME)");
            sleep(2);
            uim_debug("sleep end");
        }
        break;

    default:
        uim_debug("Err Reception range outside");
        break;
    }

    uim_debug("event_input_callback: Leave");
}

int pulse_main(struct audio_config_t *audio_config, int filedes)
{
    char *server_ip = NULL;
    char *device_name = NULL;
    int volume_set = -1;
    int volume_set2 = -1;
    char *app_name = NULL;
    char *stream_name = NULL;

    pa_cvolume *cvolume = NULL;

    char cm[PA_CHANNEL_MAP_SNPRINT_MAX];
    pa_channel_map cmap;

    YsWavFile wavFile;

    ReadFiledes = filedes;
    ExitFlg = 0;

    if (audio_config == NULL) {
        uim_debug("Param Err");
        return -1;
    }

    if (YSOK != wavFile.LoadWav(audio_config->wavfile_path)) {
//        fprintf(stderr, "Cannot open wave file.\n");
        uim_debug("Cannot open wave file.");
        return -1;
    }

    if (audio_config->volume1 != -1) {
        cvolume = (pa_cvolume *) malloc(sizeof(pa_cvolume));
    }
    else {
        cvolume = NULL;
    }

    server_ip = audio_config->server_ip;
    device_name = audio_config->device_name;
    volume_set = audio_config->volume1;
    volume_set2 = audio_config->volume2;
    app_name = audio_config->app_name;
    stream_name = audio_config->stream_name;

    uim_debug("Before Resampling:");
    uim_debug("Bit per sample: %d", wavFile.BitPerSample());
    uim_debug("Stereo: %d", wavFile.Stereo());
    uim_debug("Playback Rate: %d", wavFile.PlayBackRate());
    uim_debug("Signed: %d", wavFile.IsSigned());

    wavFile.ConvertTo16Bit();
    wavFile.ConvertToSigned();
//    wavFile.ConvertToStereo();
    wavFile.Resample(44100);
//    wavFile.ConvertToMono();

    uim_debug("After Resampling:");
    uim_debug("Bit per sample: %d", wavFile.BitPerSample());
    uim_debug("Stereo: %d", wavFile.Stereo());
    uim_debug("Playback Rate: %d", wavFile.PlayBackRate());
    uim_debug("Signed: %d", wavFile.IsSigned());

    pa_mainloop *paMainLoop = pa_mainloop_new();
    if (NULL == paMainLoop) {
//        fprintf(stderr,"Cannot create main loop.\n");
        uim_debug("Cannot create main loop.");
        return -1;
    }
    uim_debug("pa_mainloop_new()");

    pa_context *paContext = pa_context_new(pa_mainloop_get_api(paMainLoop),
                                           app_name);
    if (NULL == paContext) {
//        fprintf(stderr,"Cannot create context.\n");
        uim_debug("Cannot create context.");
        return -1;
    }
    uim_debug("pa_context_new()");

    // pa_context_set_state_callback(paContext,YsPulseAudioConnectionCallBack,NULL);
    pa_context_connect(paContext, server_ip, (pa_context_flags_t) 0, NULL);
    uim_debug("pa_context_connect()");

    YsPulseAudioWaitForConnectionEstablished(paContext, paMainLoop, 5);

    pa_sample_format_t format = PA_SAMPLE_U8;
    switch (wavFile.BitPerSample()) {
    case 8:
        if (YSTRUE == wavFile.IsSigned()) {
            wavFile.ConvertToUnsigned();
        }
        format = PA_SAMPLE_U8;
        break;
    case 16:
        if (YSTRUE != wavFile.IsSigned()) {
            wavFile.ConvertToSigned();
        }
        format = PA_SAMPLE_S16LE;
        break;
    }
    const int rate = wavFile.PlayBackRate();
    const int nChannel = (YSTRUE == wavFile.Stereo()? 2 : 1);

    const pa_sample_spec ss = {
        format,
        rate,
        nChannel
    };

    pa_channel_map_init_auto(&cmap, nChannel, PA_CHANNEL_MAP_ALSA);
    uim_debug("pa_channel_map_init_auto()");
    uim_debug("map: <%s>", pa_channel_map_snprint(cm, sizeof(cm), &cmap));

    if (NULL != cvolume) {
//        int i = 0;
        unsigned int i = 0;
        for (i = 0; i < PA_CHANNELS_MAX; i++) {
            if (1 == i) {
                if (-1 != volume_set2) {
                    cvolume->values[i] = volume_set2;
                }
            }
            else {
                cvolume->values[i] = volume_set;
            }
            uim_debug("volume[%d]_%d", i, cvolume->values[i]);
        }
        cvolume->channels = nChannel;
    }

    unsigned int playBackPtr = 0;
    YSBOOL checkForUnderflow = YSTRUE;
//    time_t t0 = time(NULL);
    time_t prevT = time(NULL) - 1;

    uim_debug("enter mainloop");
    pa_stream *paStream;
    int loopcnt;
    static pa_io_event *stdio_event = NULL;
    static pa_mainloop_api *mainloop_api = pa_mainloop_get_api(paMainLoop);

    paStream = pa_stream_new(paContext, stream_name, &ss, &cmap);
    if (NULL != paStream) {
        uim_debug("Stream created!  Getting there!");
    }
    else {
        uim_debug("Stream created : NG");
        ExitFlg = 1;
    }

    uim_debug("pa_stream_new()");

    if (!(stdio_event = mainloop_api->io_new(mainloop_api, ReadFiledes,
                                             PA_IO_EVENT_INPUT,
                                             event_input_callback,
                                             paStream))) {
        uim_debug("io_new() failed()");
        ExitFlg = 1;
    }

    pa_stream_connect_playback(paStream, device_name, NULL,
                               (pa_stream_flags_t) 0, cvolume, NULL);
    uim_debug("pa_stream_connect_playback()");

    int st = -1;
    for (loopcnt = 0; ExitFlg != 1; loopcnt++) {
        if (st != pa_stream_get_state(paStream)) {
            st = pa_stream_get_state(paStream);

            uim_debug("TURN(%6d)_", loopcnt);

            switch (pa_stream_get_state(paStream)) {
            case PA_STREAM_UNCONNECTED:
                uim_debug("PA_STREAM_UNCONNECTED");
                break;
            case PA_STREAM_CREATING:
                uim_debug("PA_STREAM_CREATING");
                break;
            case PA_STREAM_READY:
                uim_debug("PA_STREAM_READY");
                break;
            case PA_STREAM_FAILED:
                uim_debug("PA_STREAM_FAILED");
                break;
            case PA_STREAM_TERMINATED:
                uim_debug("PA_STREAM_TERMINATED");
                break;
            }
        }

        if (time(NULL) != prevT) {
/*          uim_debug("Ping..."); */
            prevT = time(NULL);
        }

        if (PA_STREAM_READY == pa_stream_get_state(paStream)) {
            const size_t writableSize = pa_stream_writable_size(paStream);
            const size_t sizeRemain = wavFile.SizeInByte() - playBackPtr;
            const size_t writeSize =
                (sizeRemain < writableSize ? sizeRemain : writableSize);

            if (0 < writeSize) {
                pa_stream_write(paStream, wavFile.DataPointer() + playBackPtr,
                                writeSize, NULL, 0, PA_SEEK_RELATIVE);
                playBackPtr += writeSize;
                uim_debug("pa_stream_write()_%d", (int) writeSize);
            }
        }

        if ((wavFile.SizeInByte() <= playBackPtr) &&
            (0 <= pa_stream_get_underflow_index(paStream)) &&
            (YSTRUE == checkForUnderflow)) {
            uim_debug
                ("Underflow detected. (Probably the playback is done.)");
            playBackPtr = 0;
#if REPEAT_FLG
            break;
#endif
        }
        pa_mainloop_iterate(paMainLoop, 0, NULL);
    }

    uim_debug("STREAM is END.");

    pa_stream_disconnect(paStream);
    uim_debug("pa_stream_disconnect()");

    pa_stream_unref(paStream);
    uim_debug("pa_stream_unref()");
//  sleep(1);

    uim_debug("leave mainloop");

    if (NULL != cvolume) {
        free(cvolume);
    }

    pa_context_disconnect(paContext);
    uim_debug("pa_context_disconnect()");

    pa_context_unref(paContext);
    uim_debug("pa_context_unref()");

    pa_mainloop_free(paMainLoop);
    uim_debug("pa_mainloop_free()");

    uim_debug("The End");

    return 0;
}
