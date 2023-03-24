/*!
    \file    main.c
    \brief   bootloader with IAP funciton : update or boot APP

    \version 2023-03-24 v1, By Schips
*/

#include "led.h"
#include "com.h"
#include "update_my_self.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

int main(void)
{
    struct update_info * pinfo;
    app_bootloader_init(I_AM_BOOTLOADER);
    led_config();
    com_init();
    delay_1ms(1);
    printf("Boot\r\n");
#if 1
    pinfo = update_check();
    //printf("is Need %x/%x\r\n", pinfo->need_update, UPDATE_MARK_FORCE);
    //printf("from %x\r\n", pinfo->request_from);
    if(pinfo->need_update == UPDATE_MARK_FORCE)
#else
    if(1) // DEBUG
#endif
    {
        //擦写APP
        enter_update_mode(pinfo->request_from);
    }
    else
    {
        printf("Goto APP\r\n");
        normal_boot_to_app();
    }
    enter_update_mode(pinfo->request_from);

    while(1)
    {
        printf("Wait while(1) at bootloader\r\n");
        delay_1ms(1000);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(COM0, (uint8_t)ch);
    while(RESET == usart_flag_get(COM0, USART_FLAG_TBE));
    return ch;
}
