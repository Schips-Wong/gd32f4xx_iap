#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>
#define uchar uint8_t
#define  uint uint32_t


struct _pfifo
{
    uchar *buffer;
    uint size;
    uint in;
    uint out;
};
typedef struct _pfifo Pfifo;

#define min(a,b) ((a) < (b) ? (a):(b))

void pfifo_reset(Pfifo *p);
uint pfifo_len(Pfifo *p);
uint pfifo_get(Pfifo *fifo, uchar *buffer, uint len);
uint pfifo_get_ch(Pfifo *fifo, uchar *buffer);

uint pfifo_put(Pfifo *fifo, uchar *buffer, uint len);
uint pfifo_put_ch(Pfifo *fifo, uchar ch);

#if 0
/* size must be 2^n */
Pfifo * pfifo_alloc(uint size);
void pfifo_free(Pfifo *p);
#else
/* size must be 2^n */
void pfifo_init(Pfifo * fifo, unsigned char *buff, int bfsize);
#endif

#endif /*__FIFO_H__*/
