#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H 

#include "Int_bootloader.h"

/**
 * @brief  初始化bootloader =》 打印日志
 * @note   需要在跳转到应用程序之前做好一些准备工作 比如
 */

 void App_bootloader_init(void);


 /**
  * @brief  等待用户传输确认
  */

 void app_bootloader_run(void);


 /**
  * @brief  接收数据
  */

 void app_bootloader_rec_data(void);


 /**
  * @brief  传输完成，检验数据
  */
 void app_bootloader_check_data(void);

 
 /**
  * @brief  跳转到应用程序
  */
 void app_bootloader_jump_app(void);

 #endif // __APP_BOOTLOADER_H
