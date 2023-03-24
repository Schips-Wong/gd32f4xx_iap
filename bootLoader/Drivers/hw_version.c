
#include "hw_version.h"
#include "gd32f450i_eval.h"

/* private variables */
#define HW_PIN_COUNTS 2
static uint32_t HW_PIN_PORT[HW_PIN_COUNTS] = {GPIOC, GPIOC};
static uint32_t HW_PIN_PIN[HW_PIN_COUNTS] = {GPIO_PIN_2, GPIO_PIN_3};
static rcu_periph_enum HW_PIN_CLK[HW_PIN_COUNTS] = {RCU_GPIOC};

void _hw_version_pin_init(void);

/*!
    \brief      get hw version
    \param[in]  none
    \retval     hw status
*/
unsigned char get_hw_version(void)
{
	int i;
    unsigned char hw_version = 0;
    _hw_version_pin_init();
	for(i = 0; i < HW_PIN_COUNTS; i++)
	{
		if(gpio_input_bit_get(HW_PIN_PORT[i], HW_PIN_PIN[i]))
		{
			hw_version = hw_version | (0x1 << i);
		}
	}
    return hw_version;
}


void _hw_version_pin_init(void)
{
	int i;
	static int once = 0;
	if(once == 0)
	{
		once = 1;
		for(i = 0; i < HW_PIN_COUNTS; i++)
		{
		/* enable the key clock */
		rcu_periph_clock_enable(HW_PIN_CLK[i]);

		/* configure button pin as input */
		gpio_mode_set(HW_PIN_PORT[i], GPIO_MODE_INPUT, GPIO_PUPD_NONE, HW_PIN_PIN[i]);
		}
	}
}
