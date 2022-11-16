#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define SYS_MBOX_NULL  (QueueHandle_t)0         //����ָ��ָ���
#define SYS_SEM_NULL   (SemaphoreHandle_t)0     //�ź���ָ��ָ���
#define SYS_MUTEX_NULL SYS_SEM_NULL             //������ָ��ָ���
#define SYS_DEFAULT_THREAD_STACK_DEPTH  configMINIMAL_STACK_SIZE

typedef SemaphoreHandle_t sys_sem_t;  //ָ��RTOS�ź���
typedef SemaphoreHandle_t sys_mutex_t;  //ָ��RTOS������
typedef QueueHandle_t sys_mbox_t;  //ָ��RTOS����
typedef TaskHandle_t sys_thread_t;   //ָ��RTOS����

typedef int sys_prot_t;




#endif

