#ifndef __LED_H__
#define __LED_H__


/* eval board low layer led */
#define LEDn                             1U

#define LED_RED_PIN                      GPIO_PIN_13
#define LED_RED_GPIO_PORT                GPIOC
#define LED_RED_GPIO_CLK                 RCU_GPIOC

void led_config(void);

/* exported types */
typedef enum 
{
    LED_RED = 0,
} led_typedef_enum;


/* configures led GPIO */
void board_led_init(led_typedef_enum lednum);
/* turn on selected led */
void board_led_on(led_typedef_enum lednum);
/* turn off selected led */
void board_led_off(led_typedef_enum lednum);
/* toggle the selected led */
void board_led_toggle(led_typedef_enum lednum);

#endif /*__LED_H__*/
