#ifndef  __INT_BOOTLOADER_H
#define  __INT_BOOTLOADER_H

#include "usart.h"
#include  "stdlib.h"
#include  "string.h"

#define BOOTLOADER_UART_REC_BUFF_LEN 512
#define STACK_ADDR   0x20000000

//程序写入的起始位置  => A区的起始位置  假设B区的大小32k（常用16K）---0X8000 A区(256-32)K--0X38000
#define APP_START_ADDR 0x08008000
#define APP_END_ADDR   0x08040000


/**
 *brief  串口接受 =>   准备接收A程序 
 * 
 * 
 */
void Int_bootloader_init(void);


/**
 * @brief  跳转到应用程序 A区
 * @note   需要在跳转之前做好一些准备工作 比如说关闭中断  关闭外设  反初始化外设  等等
 * @param  
 */
void Int_bootloader_jump_to_app(void);

#endif
