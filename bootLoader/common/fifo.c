#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fifo.h"

void pfifo_reset(Pfifo *p)
{
    p->in = p->out = 0;
}
uint pfifo_len(Pfifo *p)
{
    return p->in - p->out;
}

uint pfifo_get_ch(Pfifo *fifo, uchar *buffer)
{
    return pfifo_get(fifo, buffer, 1);
}

uint pfifo_get(Pfifo *fifo, uchar *buffer, uint len)
{
    uint l;

    len = min(len, fifo->in - fifo->out);

    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    fifo->out += len;
    return len;
}

uint pfifo_put(Pfifo *fifo, uchar *buffer, uint len)
{
    uint l;

    len = min(len, fifo->size - fifo->in + fifo->out);
    /* first put the data starting from fifo->in to buffer end */
    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    fifo->in += len;
    return len;
}

uint pfifo_put_ch(Pfifo *fifo, uchar ch)
{
    uchar buf[1];
    buf[0] = ch;
    return pfifo_put(fifo, buf, 1);
}

#if 0
Pfifo * pfifo_alloc(uint size)
{
    Pfifo *fifo = NULL;
    uchar *buffer;

    buffer = malloc(size);
    if (!buffer ) {
        printf("failed to malloc buffer!\n");
        goto err1;
    }

    fifo = malloc(sizeof(Pfifo));
    if (!fifo) {
        printf("failed to malloc Pfifo!\n");
        goto err2;
    }
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;

    return fifo;

err2:
    free(buffer);
err1:
    return NULL;
}

void pfifo_free(Pfifo *p)
{
    free(p->buffer);
    p->buffer = NULL;
    free(p);
    p = NULL;
}
#else

void pfifo_init(Pfifo * fifo, unsigned char *buff, int bfsize)
{
    if(!fifo || !buff)
    {
        return;
    }

    fifo->buffer = buff;
    fifo->size = bfsize;
    fifo->in = fifo->out = 0;

    memset(buff, 0, bfsize);

    return;
}
#endif


#if 0
int main(int argc, char *argv[])
{
    Pfifo fifo;
    uint8_t fifo_buf[1024];
    uchar buf[127];
    int i, ret;
    int ok;

    for (i = 0 ; i <127 ; i++ )
        buf[i] = i;

    pfifo_init(&fifo, fifo_buf , sizeof(fifo_buf));

    ret = pfifo_put(&fifo,buf, 127);
    if (ret < 127 ) {
        pfifo_put(&fifo, buf + ret, 127 - ret);
    }

    ok = 0;
    while (1 ) {
        ret = pfifo_get(&fifo, buf, 1);
        if (ret == 1 ) {
            ok++;
            printf("O");
        }
        if(ok == 127)
            break;
    }

    return 0;
}

#endif
