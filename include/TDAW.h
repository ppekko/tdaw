#pragma once
//
//   ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
//   █▄▄ ▄▄█ ▄▄▀█ ▄▄▀█ ███ █      Tiny C Header-only cross-platform
//   ███ ███ ██ █ ▀▀ █ █ █ █      PortAudio wrapper, Sound and notation
//   management, ███ ███ ▀▀ █ ██ █▄▀▄▀▄█      tailored for the demoscene.
//   ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
//          (Tiny-DAW)                              By kbx
//          (github.com/kb-x/tdaw)
//
//
//              Licensing is located at the bottom of this header and in the
//              github repo
//
//
//              Insert the line
//                  #define TDAW_IMPLEMENTATION
//              before including this header in *one* C/C++ file to create the
//              implementation.
//
//              Insert the line
//                  #define TDAW_BACKEND_PORTAUDIO
//              before including this header in *one* C/C++ file to use
//              PortAudio as a backend
//
//              Insert the line
//                  #define TDAW_BACKEND_ALSA
//              before including this header in *one* C/C++ file to use ALSA as
//              a backend
//
//              Insert the line
//                  #define TDAW_USERDATA
//              before including this header in *one* C/C++ file to pass
//              userdata to your stream.
//
//              Insert the line
//                  #define TDAW_PESYNTH
//              before including this header in *one* C/C++ file to get exmaple
//              synthesisers.
//
//              Insert the line
//                  #define TDAW_DEBUGTEXT
//              before including this header in *one* C/C++ file to print debug
//              text to console.
//
//              Insert the line
//                  #define TDAW_DEBUGIMGUI
//              before including this header in *one* C++ file to create ImGui
//              windows for debugging.
//
//              Insert the line
//                  #define TDAW_UTILS
//              before including this header in *one* C/C++ file to access to
//              some utility functions.
//
//              Insert the line
//                  #define TDAW_PRERENDER [length in seconds]
//              before including this header in *one* C/C++ file to prerender a
//              song instead of generating it on the fly.
//              This feature is only available with TDAW_BACKEND_ALSA
//

//#define TDAW_BACKEND_ALSA
//#define TDAW_IMPLEMENTATION
//#define TDAW_PRERENDER 10
#pragma once

#pragma once

#ifdef TDAW_IMPLEMENTATION
// -----------------------
//      includes
// -----------------------

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef TDAW_BACKEND_PORTAUDIO 
#include "portaudio.h"
#endif

#ifdef TDAW_BACKEND_ALSA 
#include <alsa/asoundlib.h>
#include <pthread.h>
#endif

#ifdef TDAW_DEBUGIMGUI 
#include "imgui.h"
#include <math.h>
#endif

#ifdef TDAW_PESYNTH 
#include <math.h>
#define TDAW_UTILS
#endif

// -----------------------
//      containers
// -----------------------

//* An audio channel
typedef struct {
  float left;
  float right;
} TDAW_CHANNEL;

#ifdef TDAW_BACKEND_PORTAUDIO
//* A TDAW Instance
typedef struct {
  uint_fast16_t samplerate;
  uint_fast16_t fpb;
  PaStream* stream;
#ifdef TDAW_PRERENDER
  TDAW_CHANNEL* prerender;
  uint_fast32_t prerender_size;
#endif
} TDAW_PIP;

//* Data to be passed through to PortAudio
typedef struct {
  TDAW_CHANNEL(*ptr)
#ifdef TDAW_USERDATA
  (void* userData, float time, float samp);
#else
  (float time, float samp);
#endif
  float samplerate;
#ifdef TDAW_USERDATA
  void* userData;
#endif
  uint_fast64_t songd;
#ifdef TDAW_DEBUGIMGUI
  uint_fast16_t fpb;
  float values[2048];
#endif
} TDAW_PASSDATA;
#endif

#ifdef TDAW_BACKEND_ALSA
//* A TDAW Instance
typedef struct {
  uint_fast16_t samplerate;
  uint_fast16_t fpb;
  snd_pcm_t* handle;
  pthread_t thread;
} TDAW_PIP;

//* Data to be passed through to ALSA
typedef struct {
#ifdef TDAW_USERDATA
  TDAW_CHANNEL(*ptr)
  (void* userData, float time, float samp);
#else
  TDAW_CHANNEL(*ptr)
  (float time, float samp);
#endif
#ifdef TDAW_USERDATA
  void* userData;
#endif
#ifdef TDAW_DEBUGIMGUI
  uint_fast16_t fpb;
  float values[2048];
#endif
#ifdef TDAW_PRERENDER
  TDAW_CHANNEL* prerender;
  uint_fast32_t prerender_size;
#endif
  float samplerate;
  uint_fast64_t songd;  // current buffer position
  TDAW_PIP* pip;        // pointer to TDAW_PIP
} TDAW_PASSDATA;

#endif

// -------------------------
//     util functions
// -------------------------

#ifdef TDAW_UTILS
//* Custs off TDAW_CHANNEL at a given peak.
void TDAW_hardLimiter(TDAW_CHANNEL* channel, float peak) {
  if (channel->left > peak) {
    channel->left = peak;
  }
  if (channel->right > peak) {
    channel->right = peak;
  }
  if (channel->left < -peak) {
    channel->left = -peak;
  }
  if (channel->right < -peak) {
    channel->right = -peak;
  }
}

//* Adds 2 channels together
void TDAW_addSelf(TDAW_CHANNEL* channel, TDAW_CHANNEL channel2) {
  channel->left += channel2.left;
  channel->right += channel2.right;
}

//* Places input in both channels
TDAW_CHANNEL TDAW_monoFloat(float i) {
#ifdef __cplusplus  // avoids warning from C++ which forbids compound literals.
  TDAW_CHANNEL o = {i, i};
  return o;
#else
  return (TDAW_CHANNEL){i, i};
#endif
}
//* Adjust volume for any TDAW_CHANNEL
void TDAW_adjustVol(TDAW_CHANNEL* d, float i) {
  d->left *= i;
  d->right *= i;
}

#endif

// --------------------------
//           synths
// --------------------------

#ifdef TDAW_PESYNTH

//* Sine wave synth
TDAW_CHANNEL TDAW_SYNTH_sine(float time, float freq) {
  return TDAW_monoFloat(sin(6.28318530718 * freq * time) * 0.1);
}

//* Plucked sine wave synth
TDAW_CHANNEL TDAW_SYNTH_sinePluck(float time, float freq, float decay,
                                  int vol) {
  return TDAW_monoFloat((sin(6.28318530718 * freq * time) * exp(decay * time)) *
                        (vol * 0.1));
}

#endif

// -------------------------
//   imgui debug functions
// -------------------------

#ifdef TDAW_DEBUGIMGUI

//* Plot graph for ImGui with sliders for scale
void TDAW_imguiPlot(TDAW_PASSDATA* data) {
  static bool animate = true;
  static int values_offset = 0;
  static float br = 0.5;
  static double refresh_time = 0.0;
  if (!animate || refresh_time == 0.0) refresh_time = ImGui::GetTime();
  while (refresh_time <
         ImGui::GetTime())  // Create data at fixed 60 Hz rate for the demo
  {
    static float phase = 0.0 f;
    data->values[values_offset] = cosf(phase);
    values_offset = (values_offset + 1) % 2048;
    phase += 0.10 f * values_offset;
    refresh_time += 1.0 f / 60.0 f;
  }
  ImGui::SliderFloat("waveform vis scale y ", &br, 1.0 f, 0.1 f, "scale: %.3f");
  ImGui::PlotLines("Calculated\nWaveform\nData", data->values, data->fpb,
                   values_offset, "left channel", -br, br, ImVec2(0, 80.0 f));
}
#endif

// -------------------------
//     general functions
// -------------------------

#ifdef TDAW_BACKEND_PORTAUDIO  //! PORTAUDIO BACKEND
//* Create a TDAW Instance
TDAW_PIP TDAW_initTDAW(uint_fast16_t samplerate, uint_fast16_t fpb) {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Init called with %i as samplerate and %i as buffersize\n",
         (int)samplerate, (int)fpb);
  fflush(NULL);
#endif
  TDAW_PIP base;
  base.fpb = fpb;
  base.samplerate = samplerate;
  Pa_Initialize();
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: TDAW Initiated\n");
  fflush(NULL);
#endif
  return base;
}

//* Callback for PortAudio
static int __tdaw_tdc(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags, void* userData) {
  TDAW_PASSDATA* data = (TDAW_PASSDATA*)userData;
  float* out = (float*)outputBuffer;
  (void)timeInfo;
  (void)statusFlags;
  (void)inputBuffer;
  static uint_fast64_t fpbRes;  // keeps track of how big the current buffer is
  // before it is sent to ALSA
  if (data->songd < framesPerBuffer) {
    fpbRes = framesPerBuffer;
  } else {
    fpbRes = framesPerBuffer * ((data->songd / framesPerBuffer) + 1);
  }

  for (data->songd = data->songd; data->songd <= fpbRes; data->songd++) {
#ifdef TDAW_USERDATA
    TDAW_CHANNEL oub = data->ptr(
        data->userData, (data->songd / data->samplerate), data->samplerate);
#else
    TDAW_CHANNEL oub =
        data->ptr((data->songd / data->samplerate), data->samplerate);
#endif
    *out++ = oub.left;
    *out++ = oub.right;
#ifdef TDAW_DEBUGIMGUI
    data->values[abs((int)(data->songd - fpbRes))] = oub.left;
#endif
  }
  return paContinue;
}

//* Opens a stream
void TDAW_openStream(TDAW_PIP* tdp, TDAW_PASSDATA* data) {
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf(
      "TDAW-DEBUG: Open Stream called with %p as callback function and %p "
      "as user data\n",
      (void*)&data->ptr, (void*)&data->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Open Stream called with %p as callback function\n",
         (void*)&data->ptr);
  fflush(NULL);
#endif
#endif
  data->songd = 0;
  data->samplerate = tdp->samplerate;
#ifdef TDAW_DEBUGIMGUI
  data->fpb = tdp->fpb;
#endif

  PaStreamParameters outputParameters;
  outputParameters.device = Pa_GetDefaultOutputDevice();
  outputParameters.channelCount = 2;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  outputParameters.sampleFormat = paFloat32;
  Pa_OpenStream(&tdp->stream, NULL, &outputParameters, (float)tdp->samplerate,
                tdp->fpb, paClipOff, __tdaw_tdc, data);
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf("TDAW-DEBUG: Stream with callback and userdata as %p and %p opened\n",
         (void*)&data->ptr, (void*)&data->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Stream with callback as %p opened\n", (void*)&data->ptr);
  fflush(NULL);
#endif
#endif
  Pa_StartStream(tdp->stream);
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf("TDAW-DEBUG: Stream with callback and userdata as %p and %p started\n",
         (void*)&data->ptr, (void*)&data->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Stream with callback and userdata as %p started\n",
         (void*)&data->ptr);
  fflush(NULL);
#endif
#endif
}

//* Closes a stream
void TDAW_closeStream(TDAW_PIP* tdp) {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Close stream called with %p as pip\n", (void*)tdp);
  fflush(NULL);
#endif
  Pa_StopStream(tdp->stream);
  Pa_CloseStream(tdp->stream);
#ifdef TDAW_DEBUGIMGUI
  printf("TDAW-DEBUG: Stream with %p as pip closed\n", (void*)tdp);
  fflush(NULL);
#endif
}

//* Terminates TDAW
void TDAW_terminate() {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: TDAW terminated.\n");
  fflush(NULL);
#endif
  Pa_Terminate();
}
#endif

#ifdef TDAW_BACKEND_ALSA  //! ALSA BACKEND

//* Create a TDAW Instance
TDAW_PIP TDAW_initTDAW(uint_fast16_t samplerate, uint_fast16_t fpb) {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Init called with %i as samplerate and %i as buffersize\n",
         (int)samplerate, (int)fpb);
  fflush(NULL);
#endif
  TDAW_PIP tda;
  tda.samplerate = samplerate;
  tda.fpb = fpb;
  snd_pcm_open(&tda.handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

  snd_pcm_set_params(tda.handle, SND_PCM_FORMAT_FLOAT,SND_PCM_ACCESS_RW_INTERLEAVED, 2, tda.samplerate, 1, 500000);
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: TDAW Initiated\n");
  fflush(NULL);
#endif
  return tda;
}

#ifdef TDAW_PRERENDER
void __tdaw_tdc(TDAW_PASSDATA* p) {
  for (p->songd = 0; p->songd < p->prerender_size; p->songd++) {
    snd_pcm_writei(p->pip->handle, &p->prerender[p->songd], 1.);
  }
}
#else
//* Opens a stream
void __tdaw_tdc(TDAW_PASSDATA* p) {
  u_int64_t fpbRes = 0;  // keeps track of how big the current buffer is before
  // it is sent to ALSA
  while (1) {
    if (p->songd < p->pip->fpb) {
      fpbRes = p->pip->fpb;
    } else {
      fpbRes = p->pip->fpb * ((p->songd / p->pip->fpb) + 1);
    }
    for (p->songd = p->songd; p->songd <= fpbRes; p->songd++) {
      TDAW_CHANNEL out = p->ptr((float)p->songd / p->samplerate, p->samplerate);
      snd_pcm_writei(p->pip->handle, &out, 1.);
#ifdef TDAW_DEBUGIMGUI
      p->values[abs((int)(p->songd - fpbRes))] = out.left;
#endif
      while (snd_pcm_state(p->pip->handle) == SND_PCM_STATE_RUNNING){
		if (snd_pcm_avail_update(p->pip->handle) > 0){ break;}
      snd_pcm_wait(p->pip->handle, 10000);
      }
    }
  }
}
#endif

#ifdef TDAW_PRERENDER
//* Prerenders audio to a buffer
void TDAW_prerender(TDAW_PIP* tdp, TDAW_PASSDATA* data) {
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf("TDAW-DEBUG: Prerender called with %p as callback function and %p as user data\n", (void*)&data->ptr, (void*)&data->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Prerender called with %p as callback function\n", (void*)&data->ptr);
  fflush(NULL);
#endif
#endif
  data->prerender_size = TDAW_PRERENDER * tdp->fpb * 45;
  data->prerender =(TDAW_CHANNEL*)malloc(data->prerender_size * sizeof(TDAW_CHANNEL));
  data->pip = tdp;

  for (data->songd = 0; data->songd < data->prerender_size; data->songd++) {

#ifdef TDAW_USERDATA
    data->prerender[data->songd] = data->ptr((float)data->songd / tdp->samplerate, tdp->samplerate, data->userData);
#else
    data->prerender[data->songd] = data->ptr((float)data->songd / (int)tdp->samplerate, data->samplerate);
#endif

  }
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Prerendered %i stereo frames\n", (int)data->prerender_size);
  fflush(NULL);
#endif
}

//* Plays a prerendered buffer in TDAW_PASSDATA
void TDAW_playPrerender(TDAW_PASSDATA* data) {
  pthread_create(&data->pip->thread, NULL, (void*)&__tdaw_tdc, data);
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Thread with %p as userdata started\n", (void*)&data->ptr);
  fflush(NULL);
#endif
}
#endif

#ifndef TDAW_PRERENDER
//* Opens a stream
void TDAW_openStream(TDAW_PIP* pip, TDAW_PASSDATA* passdata) {
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf("TDAW-DEBUG: Open Stream called with %p as callback function and %p as user data\n", (void*)&passdata->ptr, (void*)&passdata->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Open Stream called with %p as callback function\n", (void*)&passdata->ptr);
  fflush(NULL);
#endif
#endif
  passdata->pip = pip;
  passdata->samplerate = pip->samplerate;
  passdata->songd = 0;
#ifdef TDAW_DEBUGIMGUI
  data->fpb = tdp->fpb;
#endif
  pthread_create(&pip->thread, NULL, (void*)__tdaw_tdc, passdata);
#ifdef TDAW_DEBUGTEXT
#ifdef TDAW_USERDATA
  printf("TDAW-DEBUG: Stream with callback and userdata as %p and %p opened\n", (void*)&passdata->ptr, (void*)&passdata->userData);
  fflush(NULL);
#else
  printf("TDAW-DEBUG: Stream with callback as %p opened\n", (void*)&passdata->ptr);
  fflush(NULL);
#endif
#endif
}
#endif
void TDAW_closeStream(TDAW_PIP* pip) {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Close stream called with %p as pip\n", (void*)pip);
  fflush(NULL);
#endif
  pthread_cancel(pip->thread);
  snd_pcm_close(pip->handle);
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: Stream with %p as pip closed\n", (void*)pip);
  fflush(NULL);
#endif
}

void TDAW_terminate(TDAW_PIP* pip) {
#ifdef TDAW_DEBUGTEXT
  printf("TDAW-DEBUG: TDAW terminated.\n");
  fflush(NULL);
#endif
  snd_pcm_close(pip->handle);
}

#endif

#endif

//
// >The MIT License (MIT)
//
// >Copyright (c) 2022 kbx
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
