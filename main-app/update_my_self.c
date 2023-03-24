#include "update_my_self.h"
#include "com.h"
#include "fmc_operation.h"
#include "gd32f4xx.h"
#include "systick.h"

/*!
    \brief    init for APP or Bootloader
    \retval     none
*/
void app_bootloader_init(enum WHO_AM_I who_am_i)
{
    systick_config();
    if(who_am_i == I_AM_APP)
    {
        //关闭全局中断
        nvic_irq_disable(EXTI0_IRQn);
        //重定向中断向量表
        nvic_vector_table_set(NVIC_VECTTAB_FLASH, APP_OFFSET);
        //开启全局中断
        __enable_irq();
    }else if(who_am_i == I_AM_BOOTLOADER)
    {
        //关闭全局中断
        nvic_irq_disable(EXTI0_IRQn);
        //重定向中断向量表（恢复原状）
        nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
        //开启全局中断
        __enable_irq();
    }else
    {
        return ;
    }
}

/*!
    \brief    let mcu update when bootloader at next boot time
    \param[in]  request_from : update request from
    \retval     none
*/
void mark_to_update_mode(int request_from)
{
    struct update_info info;
    info.need_update = UPDATE_MARK_FORCE;
    info.request_from = request_from;

    fmc_write_32bit_data(ADDRESS_WORD_FOR_UPDATE_CHECK, 2, (int32_t *)&info);
}

/*!
    \brief    let mcu normal boot when bootloader at next boot time
    \retval     none
*/
void unmark_to_update_mode(void)
{
    struct update_info info = {0};
    info.need_update = 0;
    info.request_from = 0;
    fmc_write_32bit_data(ADDRESS_WORD_FOR_UPDATE_CHECK, 2, (int32_t *)&info);
}

/*!
    \brief    check if need update
    \retval   return update_info
*/
struct update_info * update_check(void)
{
    static struct update_info info;

    fmc_read_8bit_data(ADDRESS_WORD_FOR_UPDATE_CHECK, sizeof(struct update_info), (int8_t *)&info);

    return &info;
}

typedef void(*pfunc)();

/*!
    \brief    goto App
    \retval     none
*/
void normal_boot_to_app(void)
{
    pfunc Jump_To_Application;
    uint32_t app_addr;
    //跳转到APP
    //检查APP地址首位是否为栈顶指针，如果为真则APP存在
    if (0x20000000 == ((*(__IO uint32_t*)USER_FLASH_BANK0_FIRST_PAGE_ADDRESS) & 0x2FFE0000))
    {
        delay_1ms(1);
        //关闭全局中断
        nvic_irq_disable(EXTI0_IRQn);
        //设置APP的跳转地址
        app_addr = *(__IO uint32_t*) (USER_FLASH_BANK0_FIRST_PAGE_ADDRESS + 4);
        Jump_To_Application = (pfunc)app_addr;
        //MSP设置为APP首地址保存的栈顶指针
        __set_MSP(*(__IO uint32_t*) USER_FLASH_BANK0_FIRST_PAGE_ADDRESS);
        //CPU跳转到APP
        Jump_To_Application();
    }
}
