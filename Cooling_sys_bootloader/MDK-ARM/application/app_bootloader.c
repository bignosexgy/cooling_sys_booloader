#include "app_bootloader.h"

uint8_t app_rec_start_buff[64] = {0};
uint16_t app_rec_start_len = 0;
uint32_t app_rec_total_len = 0;

/**
 * @brief  初始化bootloader =》 打印日志
 * @note   需要在跳转到应用程序之前做好一些准备工作 比如
 */

 void App_bootloader_init(void)
 {
    printf("bootloader start\r\n");
    printf("wait user to send data\r\n");
    printf("send 'start:len' to start\r\n");
    //使用非中断方式接受，区分程序数据
   while(1)
   {
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
                    break;
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
}
        

 /**
  * @brief  等待用户传输确认
  */

 void app_bootloader_run(void)
 {
    printf("bootloader run\r\n");
 }


 /**
  * @brief  接收数据
  */

 void app_bootloader_rec_data(void)
 {
    printf("bootloader rec data\r\n");
 }


 /**
  * @brief  传输完成，检验数据
  */
 void app_bootloader_check_data(void)
 {
    printf("bootloader check data\r\n");
 }

 
 /**
  * @brief  跳转到应用程序
  */
 void app_bootloader_jump_app(void)
 {
    printf("bootloader jump app\r\n");
 }

