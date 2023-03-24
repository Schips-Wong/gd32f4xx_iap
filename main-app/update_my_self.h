#ifndef __UPDATE_H__
#define __UPDATE_H__

#include <stdint.h>

#define ADDRESS_WORD_FOR_UPDATE_CHECK          ((uint32_t)0x08028000)
#define UPDATE_MARK_FORCE                      0xd06f00d  // dog food


enum WHO_AM_I
{
    I_AM_APP,
    I_AM_BOOTLOADER,
    I_AM_NOTHING
};

struct update_info
{
    int need_update;  // 等于 UPDATE_MARK_FORCE 时，升级
    int request_from; // 标记这个请求是来自于哪里的，是COM0还是COM1
    //int sw_version; // 标记软件版本，用于判断版本号是否满足要求（不使用此策略，客户可能要求在新版上升级旧版本）
};

#define USER_FLASH_BANK0_FIRST_PAGE_ADDRESS     0x08010000
#define APP_OFFSET                              0x00010000// (FLASH_BASE - USER_FLASH_BANK0_FIRST_PAGE_ADDRESS) // 0x10000
#define NOT_NEED_UPDATE                         0
#define NEED_UPDATE                             1


#if 0
//0x8000000~0x8001FFF is reserved for bootloader.
#define FLASH_APP1_ADDR ADDR_FMC_SECTOR_8			//Address of the first app. 
//0x8002000~0x80087FF is reserved for app1
#define FLASH_APP2_ADDR ADDR_FMC_SECTOR_34		//Address of the second app. 
//0x8008800~0x800EFFF is reserved for storage.
#define FLASH_DATA_ADDR   ADDR_FMC_SECTOR_60  //user storage
#define VERSION_ID_ADDR 	ADDR_FMC_SECTOR_61	//version id
//0x800F000~0x800FFFF is reserved for user data.
#define APP_PAGE_SIZE 26
#define USR_PAGE_SIZE 4
#endif

/*!
    \brief    goto update mode, read update data from port
    \retval     none
*/
void enter_update_mode(void);

/*!
    \brief    init for APP or Bootloader
    \retval     none
*/
void app_bootloader_init(enum WHO_AM_I who_am_i);

/*!
    \brief    let mcu update when bootloader at next boot time
    \param[in]  request_from : update request from
    \retval     none
*/
void mark_to_update_mode(int request_from);

/*!
    \brief    goto App
    \retval     none
*/
void normal_boot_to_app(void);

/*!
    \brief    check if need update
    \retval   return update_info
*/
struct update_info * update_check(void);

#endif /*__UPDATE_H__*/
