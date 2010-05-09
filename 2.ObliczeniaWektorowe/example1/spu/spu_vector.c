#include <spu_mfcio.h>
#include <stdio.h>

void sum_sek(float* a,float* b,float* c, int size)
{
  int i;
  for (i=0; i<size; i++)
    c[i] = a[i] + b[i];
}

void sum_wek(float* a,float* b,float* c, int size)
{
  int i, Nv;
  Nv = size >> 2;

  vector float *vin1 = (vector float*)(a);
  vector float *vin2 = (vector float*)(b);
  vector float *vout = (vector float*)(c);

  for (i=0; i<Nv; ++i)
  {
    vout[i] = vin1[i] + vin2[i];
  }
}

int main
(
  uint64_t spid __attribute__ ((unused)),
  uint64_t argp __attribute__ ((unused)),
  uint64_t envp __attribute__((unused))
)
{
  float a0[16];
  float b0[16];
  float c0[16];
  float a1[22];
  float b1[22];
  float c1[22];
  float a2[2211];
  float b2[2211];
  float c2[2211];

  memset(a0, 1, 16);
  memset(b0, 2, 16);
  memset(c0, 3, 16);
  memset(a0, 4, 22);
  memset(b0, 5, 22);
  memset(c0, 6, 22);
  memset(a0, 7, 2211);
  memset(b0, 8, 2211);
  memset(c0, 9, 2211);
  
  uint time1, time2;
  spu_write_decrementer(0xffffff);
  
  sum_wek(a0,b0,c0,16);
  sum_wek(a1,b1,c1,22);
  sum_wek(a2,b2,c2,2211);
  time1 = 0xffffff - spu_read_decrementer();
  
  sum_sek(a0,b0,c0,16);
  sum_sek(a1,b1,c1,22);
  sum_sek(a2,b2,c2,2211);
  time2 = 0xffffff - spu_read_decrementer() - time1;
  
  double t1 = time1/79800000.0; //odczytujemy poleceniem: cat /proc/cpuinfo
  double t2 = time2/79800000.0; //wartosc timebase
  printf("Czas wykonania funkcji wektorowej: %.9f\n", t1);
  printf("Czas wykonania funkcji sekwencyjnej: %.9f\n", t2);

  return 0;
}

