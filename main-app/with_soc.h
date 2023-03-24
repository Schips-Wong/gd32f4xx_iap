#ifndef __WITH_SOC_H__
#define __WITH_SOC_H__

#include <stdint.h>
#include "gd32f4xx.h"

#define FRAME_HEAD 0x01
#define FRAME_END 0x05

enum {
    CMD_SET_TEST_LED = 0x81,
    CMD_RESET = 0x82,
    CMD_ENTER_UPDATE_MODE = 0x88,
    CMD_ERROR = 0xfe
};

enum {
    CMD_FILED_HEAD,
    CMD_FILED_CMD,
    CMD_FILED_LEN,
    CMD_FILED_PAYLOAD
};


struct respond_type
{
    int respond_to_uart;
    unsigned char * respond_data;
    int respond_len;
};

void try_handle_uart_cmd(void);

#endif /*__WITH_SOC_H__*/
