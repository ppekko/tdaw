


# PLEASE READ ME

I have written a much more optimised version of the ALSA backend, which properly utilises multithreading and implements a circular buffer. Due to exams, I haven't found the time to implement inside of TDAW. I should be finished around July/August, so peek around then.

# TDAW

A tiny, header only, easy to use, cross-platform, portaudio/alsa wrapper, tailored for the demo scene.

This header enables you to do shader-like sound programming (similar to that of [ShaderToy](https://shadertoy.com "ShaderToy")) inside of C/C++ incredibly easy.

Currently the sine wave demo on Linux compiles to 1.6kb (Arch Linux, 1166 bytes, demo/linux compiled with gcc and packed with vondehi).


# Usage

Before including TDAW, you must first `#define TDAW_IMPLEMENTATION` in *one* C/C++ file.

You then must select a backend, the following are:

- PortAudio `#define TDAW_BACKEND_PORTAUDIO`
- ALSA `#define TDAW_BACKEND_ALSA`

There are various features you can activate by defining the following lines:

```c
#define TDAW_USERDATA // Allow user data to be passed to your stream
#define TDAW_UTILS // Access to some utility functions.
#define TDAW_PESYNTH // Access basic example synthesizers.
#define TDAW_DEBUGTEXT // Output debug text to console
#define TDAW_DEBUGIMGUI // Create ImGui Windows for debugging (C++ only)
#define TDAW_PRERENDER [length in seconds] // Allows for functions to prerender your music/sounds to save lots of CPU (currently only works with the ALSA backend)
#define TDAW_MULTITHREADING // Enables multithreaded playback. Causes slight performance decrease and no time variable to access. Single threading only avalible on alsa.
```

These are also detailed in the header itself.

**Note: Prerendering your audio could result in a massive performance increase if your computer struggles with playback.**

---

# Prerendering/Live rendering on a single threads

Initialise TDAW and open a stream for the sound to begin playing ().
Set a sample rate and a FPB. If your audio lags, try increasing the FPB:

```c
TDAW_PIP tdaw = TDAW_initTDAW(44100, 512); //sample rate, frames per buffer
```

If you plan to prerender, place this outside of your game/application loop.

```c
TDAW_PASSDATA dat;
dat.ptr = &function;
TDAW_prerender(&tdaw, &dat); //prerenders sounds
```

Inside your game/application loop, run the following if you are prerendering or live rendering

```c
TDAW_render(&tdaw, &function); //live rendering
TDAW_playPrerender(&dat); //prender playback
```

To terminate a TDAW instance run `TDAW_terminate()`.

# Prerendering/Live rendering with multithreading

Create a `TDAW_PASSDATA` instance like so:

```c
TDAW_PASSDATA data;
```

From here you can put the pointer to your music code like so:

```c
data.ptr = &music;
```

Functions passed through `data.ptr` must return `TDAW_CHANNEL` (which is 2 floats making up the left and right audio channel).
They must also take `float time` and `float samp` (`void* userData` too if you have `TDAW_USERDATA` defined!)

And any userdata can be passed through `data.userData` (make sure `TDAW_USERDATA` is defined!)

Next you must initialise TDAW and open a stream for the sound to begin playing ().
Set a sample rate and a FPB. If your audio lags, try increasing the FPB:

```c
TDAW_PIP tdaw = TDAW_initTDAW(44100, 512); //sample rate, frames per buffer
TDAW_openStream(&tdaw, &dat);
```

If you are planning to prerender your audio, TDAW_openStream() is not needed, instead do the following:

```c
TDAW_prerender(&tdaw, &dat); //prerenders sounds
TDAW_playPrerender(&dat);    //plays sound
```

To close a stream, run `TDAW_closeStream()` and to terminate a TDAW instance run `TDAW_terminate()`.

---

Take a look at the demo folder for a basic completed project containing a 'plucked' sine wave.

The example folder contains some more projects.

Documentation will come soon. 

# Dependencies

- PortAudio or ALSA depending on what backend you want to use
- NASM and Python for vondehi

# Future

- PortAudio prerendering support
- A Windows build

# Credits

- [PoroCYon for vondehi](VONDEHI-LICENSE)
