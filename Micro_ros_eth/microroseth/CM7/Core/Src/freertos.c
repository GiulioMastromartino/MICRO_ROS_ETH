/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  ******************************************************************************
  */
/* USER CODE END Header */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>
#include <microros_transports.h>
#include "lwip.h"
#include "lwip/etharp.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4000 * 4,  // 14KB minimum, 4000 * 4 (16KB) safer
  .priority = (osPriority_t) osPriorityNormal,
};

void StartDefaultTask(void *argument);
extern void MX_LWIP_Init(void);

void MX_FREERTOS_Init(void) {
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
}

void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  MX_LWIP_Init();
  extern struct netif gnetif;  // Reference to network interface in lwip.c

  uint32_t wait_start = HAL_GetTick();

  while ((HAL_GetTick() - wait_start) < 30000) {
      if (netif_is_up(&gnetif) && netif_is_link_up(&gnetif)) {
          // Network ready!
          break;
      }

      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
      osDelay(100);
  }

  if (!netif_is_link_up(&gnetif)) {
      // Network failed - error handling
      while(1) {
          HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
          osDelay(200);
      }
  }
  // ============================================
    // Wait for network to fully initialize
    // ============================================
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  // Green on = waiting
    osDelay(5000);  // ← 5 SECOND DELAY!
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    // Force ARP initialization
    ip_addr_t gateway_ip;
    IP4_ADDR(&gateway_ip, 192, 168, 0, 8);  // Your computer
    etharp_request(&gnetif, &gateway_ip);

    osDelay(2000);  // Wait for ARP response
    // ============================================
    //

  rmw_uros_set_custom_transport(
    false,
    NULL,
    cubemx_transport_open,
    cubemx_transport_close,
    cubemx_transport_write,
    cubemx_transport_read
  );

  rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
  freeRTOS_allocator.allocate = microros_allocate;
  freeRTOS_allocator.deallocate = microros_deallocate;
  freeRTOS_allocator.reallocate = microros_reallocate;
  freeRTOS_allocator.zero_allocate = microros_zero_allocate;

  if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
      while(1) { HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); osDelay(100); }
  }

  rcl_publisher_t publisher;
  std_msgs__msg__Int32 msg;
  rclc_support_t support;
  rcl_allocator_t allocator;
  rcl_node_t node;

  allocator = rcl_get_default_allocator();

  rcl_ret_t ret = rclc_support_init(&support, 0, NULL, &allocator);

  if (ret != RCL_RET_OK) {
    while(1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
        osDelay(100);
    }
  }

  rclc_node_init_default(&node, "stm32_eth_node", "", &support);
  rclc_publisher_init_default(&publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "eth_publisher");

  msg.data = 0;

  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

    ret = rcl_publish(&publisher, &msg, NULL);

    if (ret != RCL_RET_OK) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    }

    msg.data++;
    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */
