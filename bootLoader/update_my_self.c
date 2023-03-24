#include "update_my_self.h"
#include "com.h"
#include "crc16.h"
#include "fmc_operation.h"
#include "gd32f4xx.h"
#include "systick.h"

#define DEBUG(format...)

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

    fmc_write_8bit_data(ADDRESS_WORD_FOR_UPDATE_CHECK, sizeof(struct update_info), (int8_t *)&info);
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
    fmc_write_8bit_data(ADDRESS_WORD_FOR_UPDATE_CHECK, sizeof(struct update_info), (int8_t *)&info);
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

uint8_t xmodemReceive(void);

/*!
    \brief    goto update mode, read update data from port
    \param[in]  request_from : update request from
    \retval     none
*/
void enter_update_mode(int request_from)
{
    xmodemReceive();
}

typedef void(*pfunc)();

/*!
    \brief    goto App
    \param[in]  request_from : update request from, incase for boot failed
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


/*!

    \retval   return NEED_UPDATE or NOT_NEED_UPDATE
*/
/*!
    \brief    xmodem crc 校验
    \param[in]  buf: xmodem packet
    \param[in]  sz: packet length
    \retval     1 for OK, 0 for ERROR
*/
static int xmodem_check_crc(const unsigned char *buf, int sz)
{
    unsigned short crc_count = crc16_ccitt(buf, sz);
    unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];

    if (crc_count == tcrc)
        return 1;
    return 0;
}

enum update_status
{
    UPDATESTATUS_FAILED,
    UPDATESTATUS_SUCCESS
};



/*!
    \brief    向串口接收一个字符数据
    \param[in]  com: COM0, COM1
    \param[in]  timeout: 读取数据最大延时时间，单位为ms
    \retval     串口接收到的数据或者为0
*/
uint8_t port_inbyte(int com, uint32_t timeout)
{
    return com_try_recv_ch(com, NULL, timeout);
}

/**************************************************************************************************
** 函数名称 : port_outbyte
** 功能描述 : 向串口接收一个字符数据
** 入口参数 : <Time>[in] 读取数据最大延时时间
** 返 回 值 : 从串口接收到的数据或者为0
** 其他说明 : 无
***************************************************************************************************/

/*!
    \brief    向串口发出一个字符数据
    \param[in]  com: COM0, COM1
    \param[in]  ch: 发出的字符
*/
void port_outbyte(int com, unsigned char ch)
{
    unsigned char chs[1];
    chs[0] = ch;
    com_send(com, chs, 1);
}

// <SOH><包序号><包序号反码><文件数据(1024字节)><校验位>
#define PACKAGE_DATASIZE 1024

uint8_t xbuff[1029]; /* 3 head chars + 1024 buff +  2 crc */

#define     AUTO_BOOT_ADDR  USER_FLASH_BANK0_FIRST_PAGE_ADDRESS
//#define       AUTO_BOOT_ADDR  0X8006000  //24kb

int xmodemHeaderCheck(uint8_t *header, uint8_t packetNo)
{
    // <SOH><包序号><包序号反码><文件数据(1024字节)><校验位>
    if((header[1] == (uint8_t) ~header[2])
                && (packetNo == header[1])//包序号无误
                && (xmodem_check_crc(&header[3], PACKAGE_DATASIZE)))//CRC校验无误
    {
        return 0;
    }
    //DEBUG("Err in %dth but %d(%x, ~%x)\r\n", packetNo, xbuff[1], xbuff[1], (uint8_t)~xbuff[2]);
    return -1;
}

#define debug_printf printf
//#define debug_printf(format,...)


#define fmc_printf(format,...)

/*!
    \brief      write 8 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x082FFFFF)
    \param[in]  length: data length
    \param[in]  data_8: data pointer
    \param[out] none
    \retval     none
*/
void flash_write_8bit(uint32_t address_old, uint32_t address_new, uint16_t length, int8_t* data_8)
{
    uint32_t address = address_new;
    fmc_sector_info_struct start_sector_info;
    fmc_sector_info_struct end_sector_info;
    uint32_t sector_num,i;

    fmc_printf("\r\nFMC half_word program operation:\n");
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* get the information of the start and end sectors */
    start_sector_info = fmc_sector_info_get(address);
    end_sector_info = fmc_sector_info_get(address + 2*length);
    /* erase sector */
    // 之前的fmc_write_8bit_data函数擦除数据的时候是一次性擦除需要用到的所有块，
    // 而后续写入数据的时候会把刚刚写进去的数据又擦掉了，所以会导致固件写不全
    // 这里添加了一个判断，只有在刚写起始地址的时候，才做擦除。
    if(address_old == address_new) {
        for(i = start_sector_info.sector_name; i <= end_sector_info.sector_name; i++){
            sector_num = sector_name_to_number(i);
            if(FMC_READY != fmc_sector_erase(sector_num)){
                while(1);
            }
        }
    }

    /* write data_8 to the corresponding address */
    for(i = 0; i < length; i++){
        if(FMC_READY == fmc_byte_program(address, data_8[i])){
            address++;
        }else{
            while(1);
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
    fmc_printf("\r\nWrite complete!\n");
    fmc_printf("\r\n");
}

uint8_t xmodemReceive(void)
{
    int flag = 0;
    uint16_t packetno = 1;
    uint32_t i;
    uint8_t chTemp = 0;//接收字符缓存
    uint32_t app_addr = AUTO_BOOT_ADDR;
    uint32_t com_from = COM0;
    int ret;

     com_reset_recv(com_from);
    //debug_printf("\n\r->");
    //debug_printf("    'B' : Cancel Loading \n\r");
    //向上位机请求发送文件，要求BIN文件
    for(i = 0; i < 5000; i++)
    {
        chTemp = port_inbyte(com_from, 500);
        if(chTemp > 0)
        {
            flag = 1;
            break;
        }
        else
        {
            port_outbyte(com_from, 'C');
        }
    }
    if(flag == 0)
    {
        debug_printf("\n\r->Transmit Time Out!!");
        debug_printf("\n\r->");
        return TIME_OUT;
    }

    while(1)
    {
        //文件传输超时
        if( chTemp == 0 )
        {
            debug_printf("\n\r->Transmit Time Out!!");
            //debug_printf("\n\r->");
            return TIME_OUT;
        }
        //用户取消文件传输
        else if((chTemp == 'B')||(chTemp=='b'))
        {
            debug_printf("\n\r->USER CANCEL!!");
            return USER_CANCELED;
        }

        if(chTemp == EOT) //文件发送结束标志
        {
            port_outbyte(com_from, ACK);
            debug_printf("\n\r->EOT");
            break;
        }

        if(chTemp == STX) //接收到有效数据帧头
        {

            xbuff[0] = chTemp;
            for(i = 0; i < sizeof(xbuff) ; i++)//接收一帧数据
            {
                xbuff[i+1] = port_inbyte(com_from, 2);
            }

            ret = xmodemHeaderCheck(xbuff, packetno);
            if(ret != 0)
            {
                //要求重发
                port_outbyte(com_from, NAK);
                goto next;
            }

            // 数据正常
            /* Update MCU firmware */
            flash_write_8bit(AUTO_BOOT_ADDR, app_addr, 1024, (int8_t *)(xbuff+3));

            app_addr += 1024;

            packetno++;
            if(packetno >= 255)
            {
                packetno = 1;
            }
        }
        port_outbyte(com_from, ACK); // 返回正常应答
next:
        delay_1ms(10);
        do
        {
            chTemp = port_inbyte(com_from, 10);//读取下一帧数据的帧头
        }while(chTemp == 0);
    }
    delay_1ms(50);
    unmark_to_update_mode();
#if 0
    for(i = 0; i< 5; i++)
    {
        delay_1ms(1000);
        printf("%d/%d\r\n", i, 5);
    }
    printf("GOTO APP!!!\r\n");
#endif
    normal_boot_to_app();
    return SUCCESSFULL;
}
