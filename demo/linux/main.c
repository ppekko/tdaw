#include <stdio.h>

#define TDAW_IMPLEMENTATION
#define TDAW_BACKEND_ALSA
#include "../../include/TDAW.h"

#include <math.h>


TDAW_CHANNEL test(float time, float samp)
{
  TDAW_CHANNEL out;
  (void)samp; //get rid of unused var warnings

  out.left = out.right = sin(6.2831*440.0*time)*exp(-3.0*time) * 0.5; // sine pluck

  return out;
}


void _start()
{
    asm volatile("push %rax\n");

    TDAW_PIP tdaw = TDAW_initTDAW(44100, 512);

    TDAW_PASSDATA dat;
    dat.ptr = &test;

    TDAW_openStream(&tdaw, &dat);

    while(1){}

    //TDAW_closeStream(&tdaw);

    //TDAW_terminate(&tdaw);
    
    //termination commands are not needed in this case as the program will never reach them 

    asm(
        "movl $1,%eax\n"
        "xor %ebx,%ebx\n"
        "int $128\n");
}
