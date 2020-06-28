/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "lwipopts.h"
#include "lwip/opt.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif
#include "ethernetif.h"
#include "gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t DHCP_state;
/* USER CODE END Variables */
osThreadId StartupTaskHandle;
osThreadId HeartBeat_TaskHandle;
osThreadId UdpServer_TaskHandle;
osThreadId DHCP_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void Task_Startup(void const * argument);
void Task_HeartBeat(void const * argument);
void Task_UdpServer(void const * argument);
void TASK_DHCP(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
  
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )  
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}                   
/* USER CODE END GET_TIMER_TASK_MEMORY */

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
  /* definition and creation of StartupTask */
  osThreadDef(StartupTask, Task_Startup, osPriorityNormal, 0, 2048);
  StartupTaskHandle = osThreadCreate(osThread(StartupTask), NULL);

  /* definition and creation of HeartBeat_Task */
  osThreadDef(HeartBeat_Task, Task_HeartBeat, osPriorityNormal, 0, 2048);
  HeartBeat_TaskHandle = osThreadCreate(osThread(HeartBeat_Task), NULL);

  /* definition and creation of UdpServer_Task */
  osThreadDef(UdpServer_Task, Task_UdpServer, osPriorityBelowNormal, 0, 2048);
  UdpServer_TaskHandle = osThreadCreate(osThread(UdpServer_Task), NULL);

  /* definition and creation of DHCP_Task */
  osThreadDef(DHCP_Task, TASK_DHCP, osPriorityNormal, 0, 1024);
  DHCP_TaskHandle = osThreadCreate(osThread(DHCP_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Task_Startup */
/**
  * @brief  Function implementing the StartupTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_Task_Startup */
void Task_Startup(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN Task_Startup */



  /* Infinite loop */
  for(;;)
  {

	  // Terminate the current thread
 	osThreadTerminate(NULL);
  }
  /* USER CODE END Task_Startup */
}

/* USER CODE BEGIN Header_Task_HeartBeat */
/**
* @brief Function implementing the HeartBeat_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_HeartBeat */
void Task_HeartBeat(void const * argument)
{
  /* USER CODE BEGIN Task_HeartBeat */
  /* Infinite loop */
  for(;;)
  {


	  HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    osDelay(100);
  }
  /* USER CODE END Task_HeartBeat */
}

/* USER CODE BEGIN Header_Task_UdpServer */
/**
* @brief Function implementing the UdpServer_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_UdpServer */
void Task_UdpServer(void const * argument)
{
  /* USER CODE BEGIN Task_UdpServer */


#if 0
	const char* message = "Hello UDP message!\n\r";
	ip_addr_t PC_IPADDR;
	IP_ADDR4(&PC_IPADDR, 10, 10, 0, 1);

	struct udp_pcb* my_udp = udp_new();
	udp_connect(my_udp, &PC_IPADDR, 55151);
	struct pbuf* udp_buffer = NULL;
#endif
	/* Infinite loop */
	for (;;) {
	  osDelay(1000);
#if 0
	  udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);
	  if (udp_buffer != NULL) {
	    memcpy(udp_buffer->payload, message, strlen(message));
	    udp_send(my_udp, udp_buffer);
	    pbuf_free(udp_buffer);

	  }
#endif
	}
  /* USER CODE END Task_UdpServer */
}

/* USER CODE BEGIN Header_TASK_DHCP */
/**
* @brief Function implementing the DHCP_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TASK_DHCP */
void TASK_DHCP(void const * argument)
{
  /* USER CODE BEGIN TASK_DHCP */

#if LWIP_DHCP
	  struct netif *netif = (struct netif *) argument;
	  ip_addr_t ipaddr;
	  ip_addr_t netmask;
	  ip_addr_t gw;
	  struct dhcp *dhcp;
#endif
	  for (;;)
	  {

#if LWIP_DHCP
	    switch (DHCP_state)
	    {
	    case DHCP_START:
	      {
	        ip_addr_set_zero_ip4(&netif->ip_addr);
	        ip_addr_set_zero_ip4(&netif->netmask);
	        ip_addr_set_zero_ip4(&netif->gw);
	        DHCP_state = DHCP_WAIT_ADDRESS;

	        dhcp_start(netif);
	      }
	      break;
	    case DHCP_WAIT_ADDRESS:
	      {
	        if (dhcp_supplied_address(netif))
	        {
	          DHCP_state = DHCP_ADDRESS_ASSIGNED;

	        }
	        else
	        {
	          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

	          /* DHCP timeout */
	          if (dhcp->tries > MAX_DHCP_TRIES)
	          {
	            DHCP_state = DHCP_TIMEOUT;

	            /* Stop DHCP */
	            dhcp_stop(netif);

	            /* Static address used */
	            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
	            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
	            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
	          }
	        }
	      }
	      break;
	  case DHCP_LINK_DOWN:
	    {
	      /* Stop DHCP */
	      dhcp_stop(netif);
	      DHCP_state = DHCP_OFF;

	    }
	    break;
	    default: break;
	    }
#endif
	    /* wait 500 ms */
	    osDelay(500);
	  }
  /* USER CODE END TASK_DHCP */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
