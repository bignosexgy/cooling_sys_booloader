/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "Int_bootloader.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//#define REC_BUFF_LEN 512
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
 //uint8_t receive_buff[REC_BUFF_LEN] =  {0}; 
 uint16_t receive_len = 0;
 extern uint16_t uart_rec_full_len;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask_led */
osThreadId_t myTask_ledHandle;
const osThreadAttr_t myTask_led_attributes = {
  .name = "myTask_led",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTask_uart_rx */
osThreadId_t myTask_uart_rxHandle;
const osThreadAttr_t myTask_uart_rx_attributes = {
  .name = "myTask_uart_rx",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTask_uart_tx */
osThreadId_t myTask_uart_txHandle;
const osThreadAttr_t myTask_uart_tx_attributes = {
  .name = "myTask_uart_tx",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void led(void *argument);
void uart_rx(void *argument);
void uart_tx(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask_led */
  myTask_ledHandle = osThreadNew(led, NULL, &myTask_led_attributes);

  /* creation of myTask_uart_rx */
  myTask_uart_rxHandle = osThreadNew(uart_rx, NULL, &myTask_uart_rx_attributes);

  /* creation of myTask_uart_tx */
  myTask_uart_txHandle = osThreadNew(uart_tx, NULL, &myTask_uart_tx_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_led */
/**
* @brief Function implementing the myTask_led thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_led */
void led(void *argument)
{
  /* USER CODE BEGIN led */
  //NonBlockingDelay_t ledDelay;
	//nonBlockingDelay_Init(&ledDelay, 500); // 500ms延时	
  /* Infinite loop */
  for(;;)
  {
    //osDelay(1);
    HAL_GPIO_TogglePin(SYS_RUN_GPIO_Port, SYS_RUN_Pin);                            	//  系统运行指示灯		
		HAL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);	                                    //  喂狗
		osDelay(500);
	}			
  //}
  /* USER CODE END led */
}

/* USER CODE BEGIN Header_uart_rx */
/**
* @brief Function implementing the myTask_uart_rx thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_uart_rx */
void uart_rx(void *argument)
{
  /* USER CODE BEGIN uart_rx */
 
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
    //HAL_UART_Receive(&huart4, receive_buff, REC_BUFF_LEN , 0xffffff );
    //HAL_UARTEx_ReceiveToIdle(&huart4,receive_buff,REC_BUFF_LEN,&receive_len,0xffffff);
    //if(receive_len > 0)
    //{
      //HAL_UART_Transmit(&huart4, receive_buff, strlen((char *)receive_buff), 1000);
      //memset(receive_buff, 0, REC_BUFF_LEN);   
      //receive_len = 0;  
    //}
  }
  /* USER CODE END uart_rx */
}

/* USER CODE BEGIN Header_uart_tx */
/**
* @brief Function implementing the myTask_uart_tx thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_uart_tx */
void uart_tx(void *argument)
{
  /* USER CODE BEGIN uart_tx */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);  
    //osDelay(3000);
    //printf("buff: %d", uart_rec_full_len);   ///
    
  }
  /* USER CODE END uart_tx */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

