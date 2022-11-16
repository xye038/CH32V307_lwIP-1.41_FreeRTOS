#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define SYS_MBOX_NULL  (QueueHandle_t)0         //邮箱指针指向空
#define SYS_SEM_NULL   (SemaphoreHandle_t)0     //信号量指针指向空
#define SYS_MUTEX_NULL SYS_SEM_NULL             //互斥量指针指向空
#define SYS_DEFAULT_THREAD_STACK_DEPTH  configMINIMAL_STACK_SIZE

typedef SemaphoreHandle_t sys_sem_t;  //指向RTOS信号量
typedef SemaphoreHandle_t sys_mutex_t;  //指向RTOS互斥量
typedef QueueHandle_t sys_mbox_t;  //指向RTOS邮箱
typedef TaskHandle_t sys_thread_t;   //指向RTOS任务

typedef int sys_prot_t;




#endif

