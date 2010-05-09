#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common.h"

int main(uint64_t spid __attribute__ ((unused)), uint64_t argp,
uint64_t envp __attribute__((unused)))
{

 uint32_t tag;
 if((tag=mfc_tag_reserve())==MFC_TAG_INVALID)
 {
   printf("SPE: ERROR - can't reserve a tag ID\n"); return 1;
 }

 printf("adres struktury common %llu\n", argp);

 COMMON p;
 mfc_get(&p, argp, sizeof(p), tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 float tab[8192] __attribute__(( aligned(128) ));

 mfc_get(tab, p.tab, sizeof(tab)/2, tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 mfc_get(&tab[8192/2], p.tab+((8192/2)*sizeof(float)), sizeof(tab)/2, tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 int Nv = 2048;

 vector float *vin1 = (vector float*)(tab);
 vector float  vin2 = (vector float){10.0,10.0,10.0,10.0};

 int i;
 for (i=0; i<Nv; ++i)
 {
  vin1[i] = vin1[i] + vin2;
 }

 mfc_put(tab, p.tab, sizeof(tab)/2, tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 mfc_put(&tab[8192/2], p.tab+((8192/2)*sizeof(float)), sizeof(tab)/2, tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 int SYNC = 2;
 mfc_put(&SYNC, p.sync, sizeof(SYNC), tag,0,0);
 mfc_write_tag_mask(1<<tag);
 mfc_read_tag_status_all();

 return 0;
}
