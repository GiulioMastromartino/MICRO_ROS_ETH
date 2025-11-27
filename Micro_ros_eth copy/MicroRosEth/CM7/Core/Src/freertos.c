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

// Standard micro-ROS utils header for Ethernet
#include <microros_transports.h>
#include "lwip.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
// Allocator wrappers
void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);
/* USER CODE END PTD */

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 3000 * 4, // Large stack for LwIP + ROS
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

  // 1. Initialize LwIP (if not done in main)
  MX_LWIP_Init();

  // Wait for Link Up (Simple delay, better to check netif flags)
  osDelay(3000);

  // 2. Configure Transport to Agent
  // Set this to the IP of your Raspberry Pi
  rmw_uros_set_custom_transport(
    false,
    NULL,
    platformio_transport_open,
    platformio_transport_close,
    platformio_transport_write,
    platformio_transport_read
  );

  // 3. Set Allocators
  rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
  freeRTOS_allocator.allocate = microros_allocate;
  freeRTOS_allocator.deallocate = microros_deallocate;
  freeRTOS_allocator.reallocate = microros_reallocate;
  freeRTOS_allocator.zero_allocate = microros_zero_allocate;

  if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
      while(1) { HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); osDelay(100); } // Error
  }

  // 4. Init Node
  rcl_publisher_t publisher;
  std_msgs__msg__Int32 msg;
  rclc_support_t support;
  rcl_allocator_t allocator;
  rcl_node_t node;

  allocator = rcl_get_default_allocator();

  // Init support - This tries to contact the Agent via UDP
  rcl_ret_t ret = rclc_support_init(&support, 0, NULL, &allocator);

  if (ret != RCL_RET_OK) {
    // Connection Failed - Rapid Red Blink
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
    // Blink Green to show "Trying to Publish"
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

    ret = rcl_publish(&publisher, &msg, NULL);

    if (ret != RCL_RET_OK) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // Red ON = Publish Error
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // Red OFF = Success
    }

    msg.data++;
    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
// FreeRTOS Heap Wrappers
void * microros_allocate(size_t size, void * state) { (void) state; return pvPortMalloc(size); }
void microros_deallocate(void * pointer, void * state) { (void) state; if (pointer) vPortFree(pointer); }
void * microros_reallocate(void * pointer, size_t size, void * state) {
  (void) state;
  if (!pointer) return pvPortMalloc(size);
  if (size == 0) { vPortFree(pointer); return NULL; }
  void * new_ptr = pvPortMalloc(size);
  if (new_ptr) { memcpy(new_ptr, pointer, size); vPortFree(pointer); }
  return new_ptr;
}
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state) {
  (void) state;
  size_t total = number_of_elements * size_of_element;
  void * ptr = pvPortMalloc(total);
  if (ptr) memset(ptr, 0, total);
  return ptr;
}
/* USER CODE END Application */
