#include <stdio.h>

#define TDAW_IMPLEMENTATION
#define TDAW_NOTATION
#define TDAW_PESYNTH
#include "../../../include/TDAW.h"

#define bool int
#define true 1
#define false 0

#define PI 3.141592654
#define TAU 6.283185307

#include <math.h>

int step(float x, float edge){
    if(x>edge){return 1;}return 0;
}


float hihatc(float time, float decay){
    return (rand() * 0.000000001) * exp(decay * time);
}

float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

float sharpSaw( float _phase ) {
  return fmod( _phase, 1.0 ) * 2.0 - 1.0;
}

float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x*x*(3 - 2 * x);
}
float saturate(float i) {
return clamp(i,-1.,1.);
}
float saw( float _freq, float _phase, float _filt, float _q ) {
  if ( _phase < 0.0 ) { return 0.0; }
  float sum = 0.0;
  for ( int i = 1; i <= 32; i ++ ) {
    float cut = smoothstep( _filt * 1.2, _filt * 0.8, (float)i  * _freq );
    cut += smoothstep( _filt * 0.3, 0.0, abs( _filt - (float)i  * _freq ) ) * _q;
    sum += sin( (float)i  * _freq * _phase * TAU ) / (float)i  * cut;
  }
  return sum;
}
float kicwk( float _phase ) {
  if ( _phase < 0.0 ) { return (rand() * 0.000000001); }
  return (rand() * 0.000000001) * sin( _phase * 300.0 - exp( -_phase * 70.0 ) * 80.0 ) * exp( -_phase * 4.0 );
}

TDAW_CHANNEL test(float time, float samp)
{
  (void)samp;
  TDAW_CHANNEL out = {0,0};
  TDAW_CHANNEL baseline = {0,0};
  //     ITS MORBIN TIME
  float timeKeeper = time * 175.0 / 60.0;
  timeKeeper += 30;
  float kick = fmod( timeKeeper, 1. );
  float hihat = fmod(timeKeeper , 0.25);


  float kicka = 0;
  float hihata = 0;
  hihata = (hihatc(hihat, -20.0) * saturate(kick)) * 0.7;
  kicka = sin(exp( -kick * 24.0 ) * 99.0 - timeKeeper * 96.0) * 0.5;
  float br = kicwk(kicka) / 10.;
  //TDAW_addSelf(&out, TDAW_monoFloat(kicka));
  TDAW_addSelf(&out, TDAW_monoFloat(hihata));
  TDAW_addSelf(&out, TDAW_monoFloat(br));


  TDAW_hardLimiter(&baseline, 0.15);
  TDAW_addSelf(&out, baseline);
  TDAW_adjustVol(&out, 0.7);

  return out;
}


void _start()
{
    asm volatile("push %rax\n");

    TDAW_PASSDATA dat;
    dat.ptr = &test;
    srand((unsigned)0);
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
