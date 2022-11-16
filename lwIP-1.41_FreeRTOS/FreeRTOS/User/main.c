/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/

/*
 *@Note
 task1 and task2 alternate printing
*/

#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Jubei_eth_driver.h"

/* Global define */
#define TASK1_TASK_PRIO     1
#define TASK1_STK_SIZE      512

/* Global Variable */
TaskHandle_t Task1Task_Handler;
TaskHandle_t Task2Task_Handler;

static void tcpecho_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 7);
    netconn_listen(conn);
    while (1) {

        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);
        /*printf("accepted new connection %p\n", newconn);*/
        /* Process the new connection. */
        if (err == ERR_OK) {
          struct netbuf *buf;
          void *data;
          u16_t len;

          while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
            /*printf("Recved\n");*/
            do {
                 netbuf_data(buf, &data, &len);
                 err = netconn_write(newconn, data, len, NETCONN_COPY);
    #if 1
                if (err != ERR_OK) {
                  printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
                }
    #endif
            } while (netbuf_next(buf) >= 0);
            netbuf_delete(buf);
          }
          /*printf("Got EOF, looping\n");*/
          /* Close connection and discard connection identifier. */
          netconn_close(newconn);
          netconn_delete(newconn);
        }
      }
}

/*********************************************************************
 * @fn      task1_task
 *
 * @brief   task1 program.
 *
 * @param  *pvParameters - Parameters point of task1
 *
 * @return  none
 */
void task1_task(void *pvParameters)
{
    LwIP_Init();
    sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO+1);
    vTaskDelete(Task1Task_Handler);
}

void task2_task(void *pvParameters)
{
while(1)
{
    printf("InTask2!\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("FreeRTOS Kernel Version:%s\r\n",tskKERNEL_VERSION_NUMBER);

    xTaskCreate((TaskFunction_t )task1_task,
                    (const char*    )"task1",
                    (uint16_t       )TASK1_STK_SIZE,
                    (void*          )NULL,
                    (UBaseType_t    )TASK1_TASK_PRIO,
                    (TaskHandle_t*  )&Task1Task_Handler);

    vTaskStartScheduler();
	while(1)
	{
	    printf("shouldn't run at here!!\n");
	}
}
