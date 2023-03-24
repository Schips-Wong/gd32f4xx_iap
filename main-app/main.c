/*!
    \file    main.c
    \brief   Main APP

    \version 2023-03-24, V1.0.0, Main APP for GD32F4xx, By Schip
*/

#include "led.h"
#include "com.h"
#include "delay.h"
#include "with_soc.h"
#include "update_my_self.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f450i_eval.h"

/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    app_bootloader_init(I_AM_APP);

    led_config();
    delay_timer_init();
    com_init();

    board_led_on(LED_RED);
    printf("ALL Config Done\r\n");
    while(1) {
        try_handle_uart_cmd();
        //board_led_toggle(LED_RED);
        //delay_1ms(1000);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(COM0, (uint8_t)ch);
    while(RESET == usart_flag_get(COM0, USART_FLAG_TBE));
    return ch;
}
