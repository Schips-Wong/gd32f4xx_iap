
#include "led.h"
#include "gd32f450i_eval.h"

/* private variables */
static uint32_t GPIO_PORT[LEDn] = {LED_RED_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn] = {LED_RED_PIN};
static rcu_periph_enum GPIO_CLK[LEDn] = {LED_RED_GPIO_CLK};

/*!
    \brief      configure the leds
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    board_led_init(LED_RED);
}




/*!
    \brief    configure led GPIO
    \param[in]  lednum: specify the Led to be configured
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void  board_led_init (led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    /* configure led GPIO port */ 
    gpio_mode_set(GPIO_PORT[lednum], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN[lednum]);
    gpio_output_options_set(GPIO_PORT[lednum], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);

    board_led_off(lednum);
}

/*!
    \brief    turn on selected led
    \param[in]  lednum: specify the Led to be turned on
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void board_led_on(led_typedef_enum lednum)
{
    // 拉低才是亮
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    turn off selected led
    \param[in]  lednum: specify the Led to be turned off
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void board_led_off(led_typedef_enum lednum)
{
    // 拉高反而是灭
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    toggle selected led
    \param[in]  lednum: specify the Led to be toggled
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void board_led_toggle(led_typedef_enum lednum)
{
    GPIO_TG(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}
