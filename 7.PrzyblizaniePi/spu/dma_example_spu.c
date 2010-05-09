#include <stdlib.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <dma_example.h>

/* allocating the control_block structure on the stack.  We align it by  */
control_block_t control_block __attribute__ ((aligned (128)));

int main(unsigned long long speid __attribute__ ((unused)),
         unsigned long long argp, 
         unsigned long long envp __attribute__ ((unused)))
{
  unsigned int tag;
  int i, num_chunks;

  /* First, we reserve a MFC tag for use */
  tag = mfc_tag_reserve();
  if (tag == MFC_TAG_INVALID)
  {
    printf ("SPU ERROR, unable to reserve tag\n");
    return 1;
  }

  /* DMA the control block information from system memory */
  mfc_get (&control_block, argp, sizeof (control_block_t), tag, 0, 0);
  mfc_write_tag_mask (1 << tag);
  mfc_read_tag_status_all ();

  double pi=0.0;
  double h = (double)(1.0/(control_block.N*control_block.N));

  for(i=control_block.Nstart;i<control_block.Nend;i++) {
    double x;
    double y;
    x=(1.0+(0.5+i)*(0.5+i)*h);
    y=1.0/x;
    pi=pi+y;
  }

  pi*=(double)(4.0/control_block.N);

  control_block.pi = pi;

  mfc_put(&control_block, argp, sizeof(control_block), tag,0,0);
  mfc_write_tag_mask(1<<tag);
  mfc_read_tag_status_all();

  return 0;
}
