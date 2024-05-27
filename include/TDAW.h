#pragma once
//
//   ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
//   █▄▄ ▄▄█ ▄▄▀█ ▄▄▀█ ███ █      tiny C header-only cross-platform (linux centered)
//   ███ ███ ██ █ ▀▀ █ █ █ █      audio playback library
//   ███ ███ ▀▀ █ ██ █▄▀▄▀▄█      tailored for the demoscene.                by pipe
//   ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
//  ┌────────────────────────────────┬──────────────────────┐
//  │ source repository at           │ my github page       │
//  │ github.com/ppekko/tdaw         │ github.com/ppekko    │
//  └────────────────────────────────┴──────────────────────┘
//
//              licensing is located at the bottom of this header and in the
//              github repository.
//
//              insert the line
//                  #define TDAW_IMPLEMENTATION
//              before including this header in *one* C/C++ file to create the
//              implementation.
//
//              insert the line
//                  #define TDAW_USERDATA
//              before including this header in *one* C/C++ file to create the
//              implementation.
//
//              insert the line
//                  #define TDAW_BACKEND_PORTAUDIO
//              before including this header in *one* C/C++ file to use
//              PortAudio as a backend (better for compatibility with other platforms)
//
//              insert the line
//                  #define TDAW_BACKEND_ALSA
//              before including this header in *one* C/C++ file to use ALSA as
//              a backend (better for performance on linux)
//
//              insert the line
//                  #define TDAW_BACKEND_APLAY
//              before including this header in *one* C/C++ file to use ALSA as
//              a backend (better for size and (possibly) performance on linux)
//
//              insert the line
//                  #define TDAW_DEBUGTEXT
//              before including this header in *one* C/C++ file to print debug
//              text to console. 
//
//              insert the line
//                  #define TDAW_MULTITHREAD
//              before including this header in *one* C/C++ file to enable multithreaded
//              playback.
//

#ifdef TDAW_IMPLEMENTATION
#include <stdint.h>
#include <stdlib.h>

#ifdef TDAW_MULTITHREAD
#include <pthread.h>
#endif

typedef struct {
        float left;
        float right;
} TDAW_CHANNEL;

#ifdef TDAW_BACKEND_ALSA
#include <alsa/asoundlib.h>
typedef struct {
        snd_pcm_t *pcm_handle;
        snd_pcm_hw_params_t *hw_params;
        uint16_t sample_rate;
        uint16_t bufsz;
        float *buffer;
        float songtime;
        uint64_t samples_played;
#ifdef TDAW_MULTITHREAD
        pthread_t thread;
#endif
} TDAW_PIP;

#ifdef TDAW_MULTITHREAD
typedef struct {
        TDAW_PIP *pip;
#ifdef TDAW_USERDATA
        void* userdata;
#endif
        TDAW_CHANNEL (*render_func)(float, float
#ifdef TDAW_USERDATA
        ,void*);
#else
        );
#endif

} TDAW_RDA;
#endif

int TDAW_initTDAW(TDAW_PIP *pip, uint16_t bufsz)
{
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: initializing...\n");
#endif
        pip->bufsz = bufsz;
        pip->buffer = (float *)malloc(pip->bufsz * 2 * sizeof(float));
        int rc;
        rc = snd_pcm_open(&pip->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
#ifdef TDAW_DEBUGTEXT
        if(rc < 0) {
                printf("tdaw: unable to open PCM device: %s\n", snd_strerror(rc));
                return -1;
        }
#endif
        snd_pcm_hw_params_alloca(&pip->hw_params);
        snd_pcm_hw_params_any(pip->pcm_handle, pip->hw_params);
        snd_pcm_hw_params_set_access(pip->pcm_handle, pip->hw_params,
                                SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pip->pcm_handle, pip->hw_params,
                                SND_PCM_FORMAT_FLOAT_LE);
        snd_pcm_hw_params_set_rate(pip->pcm_handle, pip->hw_params, 44100, 0);
        snd_pcm_hw_params_set_channels(pip->pcm_handle, pip->hw_params, 2);
        snd_pcm_hw_params_set_buffer_size(pip->pcm_handle, pip->hw_params,
                                        pip->bufsz);
        snd_pcm_hw_params_set_period_size(pip->pcm_handle, pip->hw_params,
                                        pip->bufsz, 0);
        rc = snd_pcm_hw_params(pip->pcm_handle, pip->hw_params);
#ifdef TDAW_DEBUGTEXT
        if(rc < 0) {
                printf("tdaw: unable to set hardware parameters: %s\n", snd_strerror(rc));
                return -1;
        }
#endif
        pip->sample_rate = 44100;
        pip->songtime = 0.0;
        pip->samples_played = 0;
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: done\n");
#endif
        return 0;
}

#ifdef TDAW_USERDATA
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                                                                void* user_data)
#else
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        float sample_rate = (float)pip->sample_rate;
        uint64_t frames_left = pip->bufsz;
        while(frames_left > 0) {
#ifdef TDAW_USERDATA
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate, user_data);
#else
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate);
#endif
                pip->buffer[(pip->bufsz - frames_left) * 2] = channel.left;
                pip->buffer[(pip->bufsz - frames_left) * 2 + 1] = channel.right;
                pip->songtime += 1.0 / sample_rate;
                frames_left--;
        }
        uint64_t frames_written =
        snd_pcm_writei(pip->pcm_handle, pip->buffer, pip->bufsz);
        if(frames_written == -EPIPE) {
                snd_pcm_prepare(pip->pcm_handle);
        }
#ifdef TDAW_DEBUGTEXT
        else if(frames_written < 0) {
                printf("tdaw: error writing to PCM device: %s\n",
                       snd_strerror(frames_written));
        } else if(frames_written < pip->bufsz) {
                printf("tdaw: short write (expected %d, wrote %d)\n", pip->bufsz, 
                                                                      frames_written);
        }
#endif

        uint64_t frames_avail = snd_pcm_avail_update(pip->pcm_handle);
        if(frames_avail < 0) {
#ifdef TDAW_DEBUGTEXT
                printf("tdaw: error getting available frames: %s\n", 
                snd_strerror(frames_avail));
#endif
                snd_pcm_prepare(pip->pcm_handle);
        }

        pip->samples_played = frames_avail;
}

void TDAW_terminate(TDAW_PIP *pip)
{
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: shutting down...\n");
#endif
#ifdef TDAW_MULTITHREAD
        if(pip->thread) {
                pthread_cancel(pip->thread);
        }
#endif
        snd_pcm_drain(pip->pcm_handle);
        snd_pcm_close(pip->pcm_handle);
}

#ifdef TDAW_MULTITHREAD
void *_TDAW_render_loop(void *arg)
{
        TDAW_RDA *render_args = (TDAW_RDA *)arg;
        while(1) {
#ifdef TDAW_USERDATA
                TDAW_render(render_args->pip, render_args->render_func,
                            render_args->userdata);
#else
                TDAW_render(render_args->pip, render_args->render_func);
#endif
        }
        pthread_exit(NULL);
}

#ifdef TDAW_USERDATA
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                    void* user_data)
#else
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        TDAW_RDA *render_args = (TDAW_RDA *)malloc(sizeof(TDAW_RDA));
        render_args->pip = pip;
        render_args->render_func = render_func;
#ifdef TDAW_USERDATA
        render_args->userdata = user_data;
#endif
        pthread_create(&pip->thread, NULL, _TDAW_render_loop, (void *)render_args);
}
#endif // end multithread
#endif // end alsa backend

#ifdef TDAW_BACKEND_PORTAUDIO
#include <portaudio.h>

typedef struct {
        PaStream *stream;
        uint16_t sample_rate;
        uint16_t bufsz;
        float *buffer;
        float songtime;
        uint64_t samples_played;
#ifdef TDAW_MULTITHREAD
        pthread_t thread;
#endif
} TDAW_PIP;

#ifdef TDAW_MULTITHREAD
typedef struct {
        TDAW_PIP *pip;
#ifdef TDAW_USERDATA
        void* userdata;
#endif
        TDAW_CHANNEL (*render_func)(float, float
#ifdef TDAW_USERDATA
        ,void*);
#else
        );
#endif

} TDAW_RDA;
#endif

int TDAW_initTDAW(TDAW_PIP *pip, uint16_t bufsz)
{
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: initializing...\n");
#endif
        pip->bufsz = bufsz;
        pip->buffer = (float *)malloc(pip->bufsz * 2 * sizeof(float));
        int rc;
        rc = Pa_Initialize();
#ifdef TDAW_DEBUGTEXT
        if(rc != paNoError) {
                printf("tdaw: unable to initialize PortAudio: %s\n", Pa_GetErrorText(rc));
                return -1;
        }
#endif
        rc = Pa_OpenDefaultStream(&pip->stream, 0, 2, paFloat32, 44100, pip->bufsz, 
                                                                        NULL, NULL);
#ifdef TDAW_DEBUGTEXT
        if(rc != paNoError) {
                printf("tdaw: unable to open default stream: %s\n", Pa_GetErrorText(rc));
                return -1;
        }
#endif
        rc = Pa_StartStream(pip->stream);
#ifdef TDAW_DEBUGTEXT
        if(rc != paNoError) {
                printf("tdaw: unable to start stream: %s\n", Pa_GetErrorText(rc));
                return -1;
        }
#endif
        pip->sample_rate = 44100;
        pip->songtime = 0.0;
        pip->samples_played = 0;
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: done\n");
#endif
        return 0;
}

#ifdef TDAW_USERDATA
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                                                                void* user_data)
#else
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        float sample_rate = (float)pip->sample_rate;
        uint64_t frames_left = pip->bufsz;
        while(frames_left > 0) {
#ifdef TDAW_USERDATA
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate, user_data);
#else
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate);
#endif
                pip->buffer[(pip->bufsz - frames_left) * 2] = channel.left;
                pip->buffer[(pip->bufsz - frames_left) * 2 + 1] = channel.right;
                pip->songtime += 1.0 / sample_rate;
                frames_left--;
        }
        uint64_t frames_written =
        Pa_WriteStream(pip->stream, pip->buffer, pip->bufsz);
#ifdef TDAW_DEBUGTEXT
        if(frames_written < 0) {
                printf("tdaw: error writing to stream: %s\n",
                           Pa_GetErrorText(frames_written));
        }else if(frames_written < pip->bufsz) {
                printf("tdaw: short write (expected %d, wrote %d)\n", pip->bufsz,
                                                                      frames_written);
        }
#endif

        pip->samples_played += frames_written;
}

void TDAW_terminate(TDAW_PIP *pip) {
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: shutting down...\n");
#endif
#ifdef TDAW_MULTITHREAD
        if(pip->thread) {
                pthread_cancel(pip->thread);
        }
#endif
        Pa_StopStream(pip->stream);
        Pa_CloseStream(pip->stream);
        Pa_Terminate();
}

#ifdef TDAW_MULTITHREAD
void *_TDAW_render_loop(void *arg) {
        TDAW_RDA *render_args = (TDAW_RDA *)arg;
        while(1) {
#ifdef TDAW_USERDATA
                TDAW_render(render_args->pip, render_args->render_func,
                                                render_args->userdata);
#else
                TDAW_render(render_args->pip, render_args->render_func);
#endif
        }
        pthread_exit(NULL);
}

#ifdef TDAW_USERDATA
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                                                                    void* user_data)
#else
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        TDAW_RDA *render_args = (TDAW_RDA *)malloc(sizeof(TDAW_RDA));
        render_args->pip = pip;
        render_args->render_func = render_func;
#ifdef TDAW_USERDATA
        render_args->userdata = user_data;
#endif
        pthread_create(&pip->thread, NULL, _TDAW_render_loop, (void *)render_args);
}
#endif // end multithread
#endif // end portaudio backend

#ifdef TDAW_BACKEND_APLAY
#include <stdio.h>
typedef struct {
        FILE *aplay_handle;
        uint16_t sample_rate;
        uint16_t bufsz;
        float *buffer;
        float songtime;
        uint64_t samples_played;
#ifdef TDAW_MULTITHREAD
        pthread_t thread;
#endif
} TDAW_PIP;

#ifdef TDAW_MULTITHREAD
typedef struct {
        TDAW_PIP *pip;
#ifdef TDAW_USERDATA
        void* userdata;
#endif
        TDAW_CHANNEL (*render_func)(float, float
#ifdef TDAW_USERDATA
        ,void*);
#else
        );
#endif

} TDAW_RDA;
#endif

int TDAW_initTDAW(TDAW_PIP *pip, uint16_t bufsz)
{
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: initializing...\n");
#endif
        pip->bufsz = bufsz;
        pip->buffer = (float *)malloc(pip->bufsz * 2 * sizeof(float));
        pip->aplay_handle = popen("aplay -f FLOAT_LE -r 44100 -c 2", "w");
#ifdef TDAW_DEBUGTEXT
        if(!pip->aplay_handle) {
                printf("tdaw: unable to open aplay\n");
                return -1;
        }
#endif
        pip->sample_rate = 44100;
        pip->songtime = 0.0;
        pip->samples_played = 0;
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: done\n");
#endif
        return 0;
}

#ifdef TDAW_USERDATA
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                                                                void* user_data)
#else
void TDAW_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        float sample_rate = (float)pip->sample_rate;
        uint64_t frames_left = pip->bufsz;
        while(frames_left > 0) {
#ifdef TDAW_USERDATA
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate, user_data);
#else
                TDAW_CHANNEL channel = render_func(pip->songtime, sample_rate);
#endif
                pip->buffer[(pip->bufsz - frames_left) * 2] = channel.left;
                pip->buffer[(pip->bufsz - frames_left) * 2 + 1] = channel.right;
                pip->songtime += 1.0 / sample_rate;
                frames_left--;
        }
        fwrite(pip->buffer, sizeof(float), pip->bufsz * 2, pip->aplay_handle);
}

void TDAW_terminate(TDAW_PIP *pip) {
#ifdef TDAW_DEBUGTEXT
        printf("tdaw: shutting down...\n");
#endif
#ifdef TDAW_MULTITHREAD
        if(pip->thread) {
                pthread_cancel(pip->thread);
        }
#endif
        pclose(pip->aplay_handle);
}

#ifdef TDAW_MULTITHREAD
void *_TDAW_render_loop(void *arg) {
        TDAW_RDA *render_args = (TDAW_RDA *)arg;
        while(1) {
#ifdef TDAW_USERDATA
                TDAW_render(render_args->pip, render_args->render_func,
                                                render_args->userdata);
#else
                TDAW_render(render_args->pip, render_args->render_func);
#endif
        }
        pthread_exit(NULL);
}

#ifdef TDAW_USERDATA
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float, void*),
                                                                    void* user_data)
#else
void TDAW_mt_render(TDAW_PIP *pip, TDAW_CHANNEL (*render_func)(float, float))
#endif
{
        TDAW_RDA *render_args = (TDAW_RDA *)malloc(sizeof(TDAW_RDA));
        render_args->pip = pip;
        render_args->render_func = render_func;
#ifdef TDAW_USERDATA
        render_args->userdata = user_data;
#endif
        pthread_create(&pip->thread, NULL, _TDAW_render_loop, (void *)render_args);
}
#endif // end multithread
#endif // end aplay backend

#endif // end 


//
// >The MIT License (MIT)
//
// >Copyright (c) 2024 pipe
//
// >Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// >The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// >THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
