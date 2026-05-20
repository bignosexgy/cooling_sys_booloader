#include "Int_bootloader.h"
#include "usart.h"


/// @brief  
/// @retval int

/****
 * (1)解决性能问题最简单直白的方法:加钱 加内存
 * (2)换一个高速稳定的协议USART协议高速不稳定降低波特率
 * (3)usart  降低波特率
 */



//接收程序的缓冲区
uint8_t uart_rec_buf[BOOTLOADER_UART_REC_BUFF_LEN] = {0};    
uint16_t uart_rec_len = 0;
uint16_t uart_rec_full_len = 0;
//记录写入程序地址的偏移量
uint32_t flash_write_offset = 0;
//记录当前一次接受数据的时间
uint32_t last_rec_time = 0;
//末尾是一个字节
uint8_t last_byte = 0;
uint8_t last_byte_flag = 0; //标记末尾字节是否有效

/// @brief    擦除flash    
/// @param  
static void Int_flash_erase(void)
{
  //遍历需要写入的地址 长度为当前接收的数据长度 如果全部内容都是0xff 则说明已经擦除过了
  uint8_t is_erase = 0;
  uint32_t page_addr = 0;
  for(uint16_t i=0;i<uart_rec_len;i++)
  {
    //uint32_t flash_addr = APP_START_ADDR + flash_write_offset + i;
    //uint8_t flash_data = *(uint8_t *)flash_addr;
    uint8_t data = *(volatile uint8_t *)(APP_START_ADDR + flash_write_offset + i);
    if(data != 0xff)
    {
      //printf("erase:%d,%d,%c",i,flash_write_offset,data);
      is_erase = 1;
      //记录记录当前页的起始地址
      page_addr = APP_START_ADDR + flash_write_offset + i - (APP_START_ADDR + flash_write_offset + i) % FLASH_PAGE_SIZE; //计算出所在页的起始地址
      break;
    }
  }
  if(is_erase)
  {
    //擦除flash
    FLASH_EraseInitTypeDef erase_init;
    //擦除单独页
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    //   擦除哪个bank  这里假设只有一个bank
    erase_init.Banks =  FLASH_BANK_1;
    //擦除页地址
    erase_init.PageAddress = page_addr;
    //擦除几页 
    erase_init.NbPages = 1; //一次擦除一页
    uint32_t page_error = 0;
    //f1ash擦除比较耗费性能
    if(HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
    {
      //擦除失败
      //printf("擦除flash失败\r\n");
      HAL_FLASH_Lock();
      return;
    }    
  }    
}

/// @brief 上次写完留有一个字节情况下的flash写入 
/// @param
static void Int_flash_write_with_last(void)
{
  for(uint16_t i=0; i<uart_rec_len; i += 2)
 {
    uint32_t flash_addr = APP_START_ADDR + flash_write_offset + i;
    uint16_t data16; 
    if(i == 0)   //第一个字节
    {
      //如果是第一个字节 则和上次留的字节组成一个16位数据写入                     
      data16 = last_byte | (uart_rec_buf[i] << 8); //高位是当前字节 低位是上次留的字节
    }
    else
    {
      //其他字节
      data16 = uart_rec_buf[i-1] | (uart_rec_buf[i] << 8); //高位是当前字节 低位是上次字节
    }
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_addr, data16);
 }


}


/// @brief 上次写完未留有字节情况下的flash写入
/// @param  
static void Int_flash_write_without_last(void)
{
  for(uint16_t i=0;i<uart_rec_len;i+=2)
  {          
    uint32_t flash_addr = APP_START_ADDR + flash_write_offset + i;
    uint16_t data16;      
    if(i + 1 < uart_rec_len)
    {
        data16 = uart_rec_buf[i] | (uart_rec_buf[i+1] << 8);      
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_addr, data16);
     }         
  }                              
}


/// @brief  flash半字写入
/// @param  
void Int_flash_write_halfword(void)
{
  //判断当前能够写入的内容是否为偶数。是偶数，整个数据都能写完
  if((uart_rec_len + last_byte_flag ) % 2 == 0)
  {
    //上次留有一个字节 
    if(last_byte_flag)   
    {
        Int_flash_write_with_last();
        flash_write_offset += uart_rec_len + 1; //因为第一个字节         
    }
    //上次没有留有一个字节,这次也没有剩余字节 
    else
    {
      Int_flash_write_without_last();
      flash_write_offset += uart_rec_len;
    }    
    last_byte_flag = 0; //清除末尾字节标记
  }
    //当前写入的内容是奇数，即剩下一个字节没写完
  else
  {     
    //上次留有一个字节，这次还会剩下一个字节
    if(last_byte_flag)
    {
      Int_flash_write_with_last();            
      //最后一个字节留到下次写入
      last_byte = uart_rec_buf[uart_rec_len - 1];       
      flash_write_offset += uart_rec_len; //因为最后一个字节留到下次写入 所以偏移量只加接收长度    
    }
    //上次未留有字节，这次还会剩下一个字节
    else
    {
      Int_flash_write_without_last();
      //最后一个字节留到下次写入         
      last_byte = uart_rec_buf[uart_rec_len - 1];   
      flash_write_offset += uart_rec_len-1; //因为最后一个字节留到下次写入 所以偏移量只加接收长度    
    }
    last_byte_flag = 1; //标记末尾字节有效
  }
}

/**
 * @brief 串口开启中断接收之后 触发空闲帧时使用的回调函数
 * @param huart
 * @param Size
 * 
 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if(huart->Instance == UART4)  
  {
    //接受到数据，记录stm32当前的系统时间,mS
    last_rec_time = HAL_GetTick();
    //保存接收数据的长度
    uart_rec_len = Size;
    uart_rec_full_len += uart_rec_len;
    //打印日志
    //printf("接收到数据长度：%d\r\n", uart_rec_len); 

    //printf   出问题了，程序宕机
    //printf("buff: %d", uart_rec_full_len);   ///
    //fputc('\n', stdout); // 输出换行符
    //将接收的数据写入到flash中
    //解锁flash    
    HAL_FLASH_Unlock();
    //判断写入的地址是否为新的一页

    //擦除flash
    //擦除一页的时间大约是20-30ms  这里可以优化一下 先判断需要写入的地址是否已经擦除过了 如果没有擦除过才去擦除
    Int_flash_erase();

    //写入flash
    //flash写入16位   40us*256=10ms  这里也可以优化一下 如果上次写入留有一个字节 则这次写入的第一个字节和上次留的字节组成一个16位数据一起写入 这样就能减少一次flash写入的时间 
    Int_flash_write_halfword();


    // //使用16位写入
    // for(uint16_t i=0;i<uart_rec_len;i+=2)
    // {
    //   //uint16_t data = uart_rec_buf[i] | (uart_rec_buf[i+1] << 8);
    //   uint32_t flash_addr = APP_START_ADDR + flash_write_offset + i;
    //   uint16_t data16;
    //   if(i+1 < uart_rec_len)
    //   {
    //       data16 = uart_rec_buf[i] | uart_rec_buf[i+1] << 8;               
    //   }
    //   else
    //   {
    //       //如果是最后一个字节 则高位补0xff
    //       data16 = uart_rec_buf[i] | 0xff00; //如果是最后一个字节 则高位补0xff
    //   }
    //   HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_addr, data16);
    // }
    // //记录偏移量
    // flash_write_offset += uart_rec_len; 



    //枷锁
    HAL_FLASH_Lock();

    //清空
    memset(uart_rec_buf, 0, BOOTLOADER_UART_REC_BUFF_LEN); 
    //比如说把接收到的数据放到一个队列里 让其他线程去处理
    //或者直接在这里处理 接收完成之后的事情
     //清空掉初始化串口使用之前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart4);
    __HAL_UART_CLEAR_IDLEFLAG(&huart4);
    //中断接收
    HAL_UARTEx_ReceiveToIdle_IT(&huart4, uart_rec_buf, BOOTLOADER_UART_REC_BUFF_LEN);
  }
}



/**
 *brief  串口接受 =>   准备接收A程序 
 * 
 */
void Int_bootloader_receive_app(void)
{
    //清空掉初始化串口使用之前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart4);
    __HAL_UART_CLEAR_IDLEFLAG(&huart4);
    //中断接收
    HAL_UARTEx_ReceiveToIdle_IT(&huart4, uart_rec_buf, BOOTLOADER_UART_REC_BUFF_LEN);

}

/**
 * @brief  跳转到应用程序 A区
 * @note   需要在跳转之前做好一些准备工作 比如说关闭中断  关闭外设  反初始化外设  等等
 * @param  
 */
void Int_bootloader_jump_to_app(void)
{
 
  typedef void(*pFunc)(void);
  
  //跳转到应用程序
  //获取应用程序的复位地址和入口地址
  //uint32_t app_reset_addr = *(volatile uint32_t *)APP_START_ADDR; //复位地址
  //uint32_t app_entry_addr = *(volatile uint32_t *)(APP_START_ADDR + 4); //入口地址

  //设置主堆栈指针
  //__set_MSP(app_reset_addr);

  //跳转到应用程序的入口地址
  //typedef void (*app_entry_t)(void);
  //app_entry_t app_entry = (app_entry_t)app_entry_addr;
  //app_entry();


  //获取栈顶地址的值
  uint32_t app_stack_ptr = *(volatile uint32_t *)(APP_START_ADDR);
  //获取复位中断地址的值
  uint32_t app_reset_handler = *(volatile uint32_t *)(APP_START_ADDR + 4);

  //校验栈顶地址的值
  if((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
  {
    printf("stack addr error\r\n");
    return;
  }
  else
  {
    printf("stack addr: 0x%08X\r\n", app_stack_ptr);
  }
 //校验复位中断地址的值
  if((app_reset_handler < APP_START_ADDR) || (app_reset_handler > APP_END_ADDR))
  {
    printf("reset handler addr error\r\n");
    return;
  }
  else
  {
    printf("reset handler addr: 0x%08X\r\n", app_reset_handler);
  }
  //注销bootloader程序
  
  //关闭中断
  __disable_irq();

  // 4. 停止 FreeRTOS (如果还在运行)
  vTaskEndScheduler();

  //一下几条命令注销内核  
  NVIC_DisableIRQ (EXTI9_5_IRQn);       //注销外设  
  NVIC_DisableIRQ(UART4_IRQn);          //注销串口4
  SysTick->CTRL = 0;                    //注销系统滴答定时器
  SysTick->VAL = 0;                     //清空系统滴答定时器的计数值
  SysTick->LOAD = 0;                    //清空系统滴答定时器的重装载值
  
  // 6. 清除所有 NVIC 中断挂起标志 (防止旧中断干扰)
  for(int i=0; i<8; i++) {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
  }
  
  //关闭中断
  //__disable_irq();
  //注销hal库设置

  //HAL_DeInit();
 

  //重定向中断向量表
  SCB->VTOR = APP_START_ADDR;

   //设置堆栈指针
  __set_MSP(app_stack_ptr);

  //跳转到A程序复位中断
  pFunc jump_to_app = (pFunc)app_reset_handler; 
  jump_to_app();

  //return 0;
}
