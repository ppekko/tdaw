#include <stdio.h>

#define TDAW_IMPLEMENTATION
#define TDAW_NOTATION
#define TDAW_PESYNTH
#include "../../../include/TDAW.h"

#include <math.h>

int step(float x, float edge){
    if(x>edge){return 1;}return 0;
}

TDAW_CHANNEL test(float time, float samp)
{
  TDAW_CHANNEL out = {0,0};
  (void)samp;

  TDAW_addSelf(&out, TDAW_SYNTH_sinePluck(time, NOTE_C, -10, 9));

  for(int i = 1; i <= 7; i++){
    float note;
    switch(i){
      case 1: note = NOTE_D; break;
      case 2: note = NOTE_E; break;
      case 3: note = NOTE_F; break;
      case 4: note = NOTE_G; break;
      case 5: note = NOTE_A; break;
      case 6: note = NOTE_B; break;
      case 7: note = NOTE_C * 2.; break;
    }
    TDAW_addSelf(&out, TDAW_SYNTH_sinePluck(step(time, i) * (time - i), note, -10, 9));
  }

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
