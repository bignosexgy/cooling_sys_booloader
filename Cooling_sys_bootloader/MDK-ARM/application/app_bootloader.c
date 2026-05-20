#include "app_bootloader.h"

uint8_t app_rec_start_buff[64] = {0};
uint16_t app_rec_start_len = 0;
//记录接受程序数据总长度
uint32_t app_rec_total_len = 0;
//记录上次接收的时间
extern uint32_t last_rec_time;
//记录实际接受数据的长度
extern uint16_t uart_rec_full_len;


//记录当前应用层的状态
Bootloader_status boot_status = BOOTLOADER_STATUS_INIT;

/**
 * @brief  初始化bootloader =》 打印日志
 * @note   需要在跳转到应用程序之前做好一些准备工作 比如
 */

 void App_bootloader_init(void)
 {
    printf("bootloader start\r\n");
    printf("wait user to send data\r\n");
    printf("send 'start:len' to start\r\n");
    boot_status = BOOTLOADER_STATUS_INIT;
 }    
   
        

 /**
  * @brief  等待用户传输确认
  */

 void App_bootloader_run(void)
 {
    //使用非中断方式接受，区分程序数据
   // 挂起等待接收一直等待接收到buff满 或者 收到idle空闲帧
   HAL_UARTEx_ReceiveToIdle(&huart4, app_rec_start_buff, 64, &app_rec_start_len, 0xffffff);
   if(app_rec_start_len > 0)
   {
      //判断数据中是否包含start:len
      char *start_str = strstr((char *)app_rec_start_buff, "start:");
      if(start_str != NULL)  
      {
            //获取长度
            app_rec_total_len = atoi((char *)start_str + 6);
            if(app_rec_total_len > 0)
            {
               //printf("start rec app, total len: %d\r\n", app_rec_total_len);
               printf("app len: %d\r\n", app_rec_total_len);
              boot_status = BOOTLOADER_STATUS_RUN;
            }
            else
            {
               printf("len error\r\n");
            }
            
      }
      else
      { 
         printf("data error\r\n"); 
      }
   }
 }


 /**
  * @brief  接收数据
  */

 void App_bootloader_rec_data(void)
 { 
   //接收完成之后 修改状态为check_data
   //第一种方法，软件方式 等待2s钟没有接收到数据了 就认为接收完成了
   //hal_gittick() - last_rec_time > 2000 && app_rec_total_len > 0
   if(((HAL_GetTick() - last_rec_time) > 2000) && (last_rec_time != 0))
   {
      printf("rec app data done, total len: %d\r\n", app_rec_total_len);
      boot_status = BOOTLOADER_STATUS_CHECK_DATA;
   }
   else ;
 }


 /**
  * @brief  传输完成，检验数据
  */
 void App_bootloader_check_data(void)
 {
      //这里简单的判断一下接收的数据长度和之前约定的长度是否一致
      if(uart_rec_full_len == app_rec_total_len)
      {
         printf("check data success\r\n");
         boot_status = BOOTLOADER_STATUS_JUMP_APP;
      }
      else
      {
         printf("check data error, rec len: %d\r\n", uart_rec_full_len);
      }
 }

 
 /**
  * @brief  跳转到应用程序
  */
 void App_bootloader_jump_app(void)
 {
   printf("jump to app\r\n"); 
   Int_bootloader_jump_to_app();
 }

 /**
 * @brief  在main的while(1)中调用，完成bootloader的工作流程
 */
 void App_bootloader_work(void)
 {
   switch (boot_status)
   {     
      case BOOTLOADER_STATUS_INIT:
         App_bootloader_run();
         //boot_status = BOOTLOADER_STATUS_RUN;
         break;    
      case BOOTLOADER_STATUS_RUN:
        //接受数据的准备工作  
        //确认要写入flash的页数，提前擦除flash  
         Int_bootloader_erase_flash(APP_START_ADDR, 50);  //假设最多写入30页 \100k
         printf("flash erase done\r\n");
         printf("redy to receive app data\r\n");
         boot_status = BOOTLOADER_STATUS_REC_DATA;
         Int_bootloader_receive_app();                    //接受数据
         break; 
      case BOOTLOADER_STATUS_REC_DATA:
         //等待接收完成
         App_bootloader_rec_data();
         //boot_status = BOOTLOADER_STATUS_CHECK_DATA;
          break;
      case BOOTLOADER_STATUS_CHECK_DATA:
         //检验数据
         App_bootloader_check_data();
         break;
      case BOOTLOADER_STATUS_JUMP_APP:
         //跳转到应用程序
         App_bootloader_jump_app();
         break;
      default:
         break;
   }


 }

/**
 * @brief  外部可调用，提前擦除flash空间
 * @param  page_addr: 页地址
 * @param  pages: 页数
 */
 void Int_bootloader_erase_flash(uint32_t page_addr, uint16_t pages)
 {
    //解锁flash
    HAL_FLASH_Unlock();
    //擦除flash
    FLASH_EraseInitTypeDef erase_init;
    //擦除单独页
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    //   擦除哪个bank  这里假设只有一个bank
    erase_init.Banks =  FLASH_BANK_1;
    //擦除页地址
    erase_init.PageAddress = page_addr;
    //擦除几页 
    erase_init.NbPages = pages; //一次擦除一页
    uint32_t page_error = 0;
    //f1ash擦除比较耗费性能
    if(HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
    {
      //擦除失败
      //printf("擦除flash失败\r\n");
      HAL_FLASH_Lock();
      return;
    }    
    //加锁flash
   HAL_FLASH_Lock();
 }