#include "with_soc.h"
#include "led.h"
#include "com.h"
#include "update_my_self.h"
#include "systick.h"
#include <string.h>
#include <stdio.h>


#define FCT_BUFF_SIZE 1024
static unsigned char fct_buff[FCT_BUFF_SIZE];

static unsigned int data_src_port, data_dest_port;

#define STR_CHECK !strcmp

enum {
    PORT_CAN0,
    PORT_CAN1,
    PORT_COM0,
    PORT_COM1,
};

#define UART_DATA_RECV_BUFF_SIZE 128

char usart0_data_recv[UART_DATA_RECV_BUFF_SIZE];
char uart3_data_recv[UART_DATA_RECV_BUFF_SIZE];

#define CAN_DATA_RECV_BUFF_SIZE 8

char can0_data_recv[CAN_DATA_RECV_BUFF_SIZE];
char can1_data_recv[CAN_DATA_RECV_BUFF_SIZE];

int need_reset = 0;

void gen_error_package(unsigned char *buf)
{
    if(!buf) return;
    buf[CMD_FILED_CMD] = CMD_ERROR;
    buf[CMD_FILED_LEN] = 0x2;
    buf[CMD_FILED_PAYLOAD] = 0xde;
    buf[CMD_FILED_PAYLOAD+1] = 0xad;
}

int check_cmd_is_good(unsigned char *buf)
{
    if(buf == NULL)
    {
        return -1;
    }

    if(buf[CMD_FILED_HEAD] != FRAME_HEAD)
    {
        return -1;
    }
    if(buf[3 + buf[CMD_FILED_LEN]  +0] != FRAME_END)
    {
        return -1;
    }
    return 0;
#if 0
    uint16_t  crc_value;
    crc_value = crc16(0, buf, 3 + buf[CMD_FILED_LEN] + 1);

    // CRC16,低位在前，高位在后
    if( ((crc_value >> 0) & 0xff)  ==  (buf[3 + buf[CMD_FILED_LEN] + 1 +0]) &&
        ((crc_value >> 8) & 0xff)  ==  (buf[3 + buf[CMD_FILED_LEN] + 1 +1])   )
    {
        return 0;
    }
    printf("Check ERROR\n");
    return -1;
#endif
}

// 3 + fct_buff[CMD_FILED_LEN] + 1
void add_package_tail_crc16_with_end(unsigned char *buf)
{
    //int len;
    //uint16_t  crc_value;
    if(buf == NULL)
        return;

    // 帧尾需要参与计算
    buf[3 + buf[CMD_FILED_LEN] + 1 +0 -1] = FRAME_END;
#if 0
    //len = 3 + buf[CMD_FILED_LEN] + 1;
    crc_value = crc16(0,buf,len);
   
    // CRC16,低位在前，高位在后
    buf[3 + buf[CMD_FILED_LEN] + 1 +1 -1] = (crc_value >> 0) & 0xff;
    buf[3 + buf[CMD_FILED_LEN] + 1 +2 -1] = (crc_value >> 8) & 0xff;
#endif
}

struct respond_type * handle_cmd(unsigned char *buf, uint32_t len)
{
    static struct respond_type respond;
    //char fhead;
    unsigned char cmd;
    unsigned char dataLen;
    unsigned char *payload;
    uint32_t tmp;
    int i;

    //memset(fct_buff, 0, FCT_BUFF_SIZE);
    if((!buf) || (len == 0) || (len > FCT_BUFF_SIZE))
        return NULL;

    for(i = 0; i < FCT_BUFF_SIZE; i++)
    {
        fct_buff[i] = 0;
    }
    //fhead = buf[CMD_FILED_HEAD];
    cmd = buf[CMD_FILED_CMD];
    dataLen = buf[CMD_FILED_LEN];
    payload = &buf[CMD_FILED_PAYLOAD];

    respond.respond_data = fct_buff;
    fct_buff[CMD_FILED_HEAD] = FRAME_HEAD;
    fct_buff[CMD_FILED_CMD] = cmd;
    fct_buff[CMD_FILED_LEN] = 0;

    if(check_cmd_is_good(buf) !=0)
    {
        goto error;
    }
#if 0
    // dump for debug
    for (i = 0; i < len; i++)
    {
        printf("%c/%x \n", buf[i], buf[i]);
    }
    delay_1ms(1000);
    printf("====\r\n");
#endif

    switch(cmd)
    {
        case CMD_RESET:
        fct_buff[CMD_FILED_LEN] = 1;
        fct_buff[CMD_FILED_PAYLOAD+0]   = 0x00;
        need_reset = 1;

        break;
        case CMD_ENTER_UPDATE_MODE: // 升级命令
        fct_buff[CMD_FILED_LEN] = 1;
        fct_buff[CMD_FILED_PAYLOAD+0] = 0x00;
        need_reset = 1;
        mark_to_update_mode(data_src_port);

        break;
        default:
        gen_error_package(fct_buff);
        break;
    }

    add_package_tail_crc16_with_end(fct_buff);
    // 帧头+特征码+长度  + 数据 + 帧尾 //+ CRC16
    respond.respond_len = 3 + fct_buff[CMD_FILED_LEN] + 1;
    return &respond;

error:
    gen_error_package(fct_buff);
    add_package_tail_crc16_with_end(fct_buff);
    return &respond;
}

void try_handle_uart_cmd(void)
{
    unsigned char *data = NULL;
    unsigned char * respond_data;
    struct respond_type *target_respond;

    int len, target_respond_len;
//wait_data0:
    data = com_try_recv(COM0, &len);
    if(data != NULL)
    {
        data_src_port = COM0;
        goto handle_cmd1;
    }
    data = com_try_recv(COM1, &len);
    if(data != NULL)
    {
        data_src_port = COM1;
        goto handle_cmd1;
    }
    
    return ;
handle_cmd1:
    target_respond = handle_cmd(data, len);
    if(target_respond->respond_len == 0)
    {
        return ;
    }

//send_data_to_port2:
    target_respond_len = target_respond->respond_len;
    data_dest_port = data_src_port;
    respond_data = target_respond->respond_data;

    if(data_dest_port  == COM0 || data_dest_port  == COM1)
    {
        com_send(data_dest_port, (uint8_t *)respond_data, target_respond_len);
    }
    if(need_reset)
    {
        NVIC_SystemReset();
    }
}
