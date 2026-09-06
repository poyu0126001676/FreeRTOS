#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "stm32f4xx_hal.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define DWT_CTRL    (*(volatile uint32_t*)0xE0001000)
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
volatile BaseType_t status_button = 0;



/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* FreeRTOS handles */


/* Declare a variable of type xSemaphoreHandle.  This is used to reference the
semaphore that is used to synchronize both manager and employee task */
SemaphoreHandle_t xWork = NULL;

/* this is the queue which manager uses to put the work ticket id */
QueueHandle_t     xWorkQueue = NULL;


TaskHandle_t      xTaskHandleM = NULL;
TaskHandle_t      xTaskHandleE = NULL;

/* User buffer */
char usr_msg[256];

/* Private function prototypes -----------------------------------------------*/
static void prvSetupHardware(void);
static void prvSetupUart(void);
static void prvSetupGpio(void);
void printmsg(const char *msg);


static void vManagerTask(void *pvParameters);
static void vEmployeeTask(void *pvParameters);

void button_interrupt_handler(void);

/*----------------------------------------------------------------------------*/

int main(void)
{
    HAL_Init();

    prvSetupHardware();

    snprintf(usr_msg, sizeof(usr_msg), "Demo of Binary semaphore usage between 2 Tasks\r\n");
    printmsg(usr_msg);


    /*
     * SEGGER_SYSVIEW_Conf();
     * SEGGER_SYSVIEW_Start();
     */

    // Create Binary Semaphore
    xWork = xSemaphoreCreateBinary();

    xWorkQueue = xQueueCreate(1, sizeof(unsigned int));  // maximum is only 1 element (TicketID)

    if ((xWork != NULL) && (xWorkQueue != NULL))
    {
        // 建立 Manager 與 Employee 任務
        xTaskCreate(vManagerTask, "Manager", 256, NULL, 3, &xTaskHandleM);   // Manager 有較高的 priority
        xTaskCreate(vEmployeeTask, "Employee", 256, NULL, 1, &xTaskHandleE);

        vTaskStartScheduler();
    }

    // Fail message
    snprintf(usr_msg, sizeof(usr_msg), "Queue or semaphore create failed..\r\n");
    printmsg(usr_msg);

    while (1)
    {
    }
}

static void vManagerTask(void *pvParameters)
{
    unsigned int xWorkTicketId;
    BaseType_t xStatus;

    xSemaphoreGive(xWork);

    for (;;)
    {
        xWorkTicketId = (rand() & 0x1FF);

        xStatus = xQueueSend(xWorkQueue, &xWorkTicketId, portMAX_DELAY);

        if (xStatus != pdPASS)
        {
            snprintf(usr_msg, sizeof(usr_msg), "Could not send to the queue.\r\n");
            printmsg(usr_msg);
        }

        // Success
        else
        {
        	// 給出 semaphore
            xSemaphoreGive(xWork);
            taskYIELD();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


static void vEmployeeTask(void *pvParameters)
{
    unsigned int xWorkTicketId;
    BaseType_t xStatus;

    for (;;)
    {
        // 先等候 semaphore（blocking）— 使用 portMAX_DELAY 會永遠等待
        if (xSemaphoreTake(xWork, portMAX_DELAY) == pdTRUE)
        {
            // 從 queue 取得 ticket id
            xStatus = xQueueReceive(xWorkQueue, &xWorkTicketId, 0);

            if (xStatus == pdPASS)
            {
                snprintf(usr_msg, sizeof(usr_msg), "Employee task : Working on Ticket id : %u\r\n", xWorkTicketId);
                printmsg(usr_msg);

                TickType_t delayTicks = pdMS_TO_TICKS((xWorkTicketId % 200) + 1);
                vTaskDelay(delayTicks);
            }
            else
            {
                snprintf(usr_msg, sizeof(usr_msg), "Employee task : Queue is empty , nothing to do.\r\n");
                printmsg(usr_msg);
            }
        }
    }
}

/*---------------------- Hardware setup -------------------------------------*/
static void prvSetupHardware(void)
{
    prvSetupGpio();
    prvSetupUart();
}


static void prvSetupUart(void)
{
    /* 初始化 UART handle (USART2, PA2=TX, PA3=RX) */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    /* 開啟 UART 與 GPIO clock 並初始化 GPIO AF */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        /* 初始化錯誤處理 */
        Error_Handler();
    }
}

/* GPIO 設定（內建 LED 與 User button） */
void prvSetupGpio(void)
{
    /* Enable clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    GPIO_InitTypeDef gpio_init = {0};

    /* LED (PA5) */
    gpio_init.Pin = GPIO_PIN_5;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio_init);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    gpio_init.Pin = GPIO_PIN_13;
    gpio_init.Mode = GPIO_MODE_IT_FALLING;
    gpio_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &gpio_init);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}


void printmsg(const char *msg)
{
    if (msg == NULL) return;
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        unsigned int workId = 0xAB;

        if (xWorkQueue != NULL)
        {
            xQueueSendFromISR(xWorkQueue, &workId, &xHigherPriorityTaskWoken);
        }

        if (xWork != NULL)
        {
            xSemaphoreGiveFromISR(xWork, &xHigherPriorityTaskWoken);
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void button_interrupt_handler(void)
{

}


void Error_Handler(void)
{
    /* LED 快閃或停在此處 */
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(200);
    }
}


/* End of file */
