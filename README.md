# TDAW

A tiny, header only, easy to use, minimal overhead, cross-platform audio wrapper, tailored for the demo scene.

This header enables you to do shader-like sound programming (similar to that of [ShaderToy](https://shadertoy.com "ShaderToy")) inside of C/C++ incredibly easily.

**Keep in mind that the demo projects sizes may vary on your machine, and additionally *if you incorporate them into your project, chances are it will be smaller due to compiler optimisations and better packing compression with larger files***

<ins>For actual sizes, refer to the table inside the demo folder's readme.</ins>


# Usage

Before including TDAW, you must first `#define TDAW_IMPLEMENTATION` in *one* C/C++ file.

You then must select a backend, the following are:

- PortAudio `#define TDAW_BACKEND_PORTAUDIO`
- ALSA `#define TDAW_BACKEND_ALSA` (linux only)
- aplay `#define TDAW_BACKEND_APLAY` (linux only)

There are also features you can activate by defining the following lines:

```c
#define TDAW_USERDATA // Allow user data to be passed to your stream
#define TDAW_DEBUGTEXT // Output debug text to console
#define TDAW_MULTITHREAD // Enables multithreaded playback
```
These are also detailed in the header itself.

## Creating a function to render

Your function must follow this format. `TDAW_CHANNEL` is a struct with 2 floats for both audio channels.
```c
TDAW_CHANNEL test(float time, float samp)
{
  TDAW_CHANNEL out;
  //code here
  return out;
}
```
If you have enabled `TDAW_USERDATA`, an extra `void* userdata` is required as an argument, allowing you to pass data in.

# Live rendering on a single threads

Initialise TDAW with a refernce to a TDAW_PIP and your "frames per buffer" size (FPB). If your audio lags, try increasing the FPB. Lower FPB results in faster output at the cost of potential missing buffers if your code fails to send out a buffer in time, and vice versa.

```c
TDAW_PIP tdaw;
TDAW_initTDAW(&tdaw, 1024);
```

Inside your application/demo loop, run the following

```c
TDAW_render(&tdaw, &function);
```

To terminate a TDAW instance run `TDAW_terminate()`.

# Live rendering with multithreading

Same as live rendering, except instead of running `TDAW_render()` in a loop, you run `TDAW_mt_render()` **outside of the loop**, otherwise you will be constantly spawning new threads. It holds the same parameters as `TDAW_render()`

# Dependencies

- PortAudio, ALSA or aplay depending on what backend you want to use
- For the demo, NASM and Python for vondehi and [Elfkickers for sstrip](https://www.muppetlabs.com/~breadbox/software/tiny/teensy.html)

# QNA
Sure, here is the text organized into a dropdown with blockquotes:

<details>
<summary>Why are there so many preprocessor definitions in `TDAW.h`, it's painful to read!! I hate you!!</summary>

> Packaging this all in one header file while trying to save on binary size will result at messy code at some point haha
</details>

<details>
<summary>Can I pass a single float[2] into TDAW_CHANNEL without having to apply left and right channels separately?</summary>

> Yes, just do the following pointer magic
>
> ```c
> TDAW_CHANNEL out;
> float (*ptr)[2] = (float (*)[2])&out;
> float data[2] = {0.2, 0.2};
> *ptr = &data;
> ```
</details>

<details>
<summary>What backend should I use?</summary>

> Portaudio if you want compatibility with other platforms, aplay if you want to save on size. ALSA is slightly worse in size but may offer better performance compared to aplay.
</details>

# Credits
- [pipe (creator)](https://github.com/ppekko)
- [PoroCYon for vondehi](VONDEHI-LICENSE) (original vondehi found [here](https://gitlab.com/PoroCYon/vondehi))
