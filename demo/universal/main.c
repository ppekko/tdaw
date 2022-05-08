#include <stdio.h>

#define TDAW_IMPLEMENTATION
#include "../../include/TDAW.h"

#include <math.h>


TDAW_CHANNEL test(float time, float samp)
{
  TDAW_CHANNEL out = {0,0};
  (void)samp; //get rid of unused var warnings

  TDAW_addSelf(&out, TDAW_monoFloat(sin(6.2831*440.0*time)*exp(-3.0*time) * 0.5)); // sine pluck

  return out;
}


void _start()
{
    asm volatile("push %rax\n");

    TDAW_PASSDATA dat;
    dat.ptr = &test;

    TDAW_PIP tdaw =TDAW_initTDAW(44100, 1400);

    TDAW_openStream(&tdaw, &dat);

    while(1){}

    TDAW_closeStream(&tdaw);

    TDAW_terminate(&tdaw);

    asm(
        "movl $1,%eax\n"
        "xor %ebx,%ebx\n"
        "int $128\n");
}
