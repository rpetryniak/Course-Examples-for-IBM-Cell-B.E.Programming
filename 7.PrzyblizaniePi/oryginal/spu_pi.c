#include <stdio.h>
#include <malloc.h>
#include "globals.h"
//#include <libmisc.h>
#include <spu_mfcio.h>
#include <simdmath.h>

volatile context ctx;

int main(unsigned long long id,unsigned long long parm)
{
  int i;
  unsigned int tag_id=0;
  float pi;
  float h;

  spu_writech(MFC_WrTagMask,-1);

  spu_mfcdma32((void*)(&ctx),(unsigned int)parm, sizeof(context), tag_id, MFC_GET_CMD);
  (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

  pi=0.0;

  h = (float)(1.0/(ctx.N*ctx.N));

  for(i=ctx.Nstart;i<ctx.Nend;i++) {
    float x;
    float y;
    x=(1.0+(0.5+i)*(0.5+i)*h);
    y=1.0/x;
    pi=pi+y;
  }

  pi*=(float)(4.0/ctx.N);

  int tag_putback=0;

  spu_mfcdma32((void*)(&pi),(unsigned int)(ctx.pi),4,tag_putback,MFC_PUT_CMD);
  (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

  return 0;
}
