#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H 

#include "Int_bootloader.h"

typedef enum
{
    BOOTLOADER_STATUS_INIT,
    BOOTLOADER_STATUS_RUN,
    BOOTLOADER_STATUS_REC_DATA,
    BOOTLOADER_STATUS_CHECK_DATA,
    BOOTLOADER_STATUS_JUMP_APP
} Bootloader_status;

/**
 * @brief  初始化bootloader =》 打印日志
 * @note   需要在跳转到应用程序之前做好一些准备工作 比如
 */
 void App_bootloader_init(void); 

 /**
  * @brief  等待用户传输确认
  */
 void App_bootloader_run(void);

 /**
  * @brief  接收数据
  */
 void App_bootloader_rec_data(void);

 /**
  * @brief  传输完成，检验数据
  */
 void App_bootloader_check_data(void);
 
 /**
  * @brief  跳转到应用程序
  */
 void App_bootloader_jump_app(void);

/**
 * @brief  擦除flash
 * @param  page_addr: 页地址
 * @param  pages: 页数
 */
 void Int_bootloader_erase_flash(uint32_t page_addr, uint16_t pages);
 

/**
 * @brief  在main的while(1)中调用，完成bootloader的工作流程
 */
 void App_bootloader_work(void);

 #endif // __APP_BOOTLOADER_H
