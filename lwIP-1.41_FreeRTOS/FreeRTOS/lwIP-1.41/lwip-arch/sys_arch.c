#include "lwip/sys.h"

#define SYS_THREAD_MAX 4    //����Э��ջ���������������
u32_t lwip_sys_now;

u32_t sys_now(void)
{
  lwip_sys_now = xTaskGetTickCount();
  return lwip_sys_now;
}

int sys_sem_valid(sys_sem_t *sem)     //�ж�һ���ź����Ƿ���Ч
{
  return (*sem != SYS_SEM_NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem)  //��һ���ź�����Ϊ��Ч
{
  *sem = SYS_SEM_NULL;
}

int sys_mbox_valid(sys_mbox_t *mbox)    //�ж�һ�������Ƿ���Ч
{
  return (*mbox != SYS_MBOX_NULL);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)  //��һ��������Ϊ��Ч
{
  *mbox = SYS_MBOX_NULL;
}

void sys_init(void)   //��ʼ������ϵͳģ���
{
    printf("[sys_arch] init ok\r\n");      //��ӡ��ʼ���ɹ���Ϣ
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)   //�½�һ���ź���
{
  /* ���� sem */
  if(count <= 1)
  {
    *sem = xSemaphoreCreateBinary();
    if(count == 1)
    {
      sys_sem_signal(sem);
    }
  }
  else
    *sem = xSemaphoreCreateCounting(count,count);

#if SYS_STATS
    ++lwip_stats.sys.sem.used;
    if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
        lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
    }
#endif /* SYS_STATS */

  if(*sem != SYS_SEM_NULL)
    return ERR_OK;
  else
  {
#if SYS_STATS
    ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
    printf("[sys_arch]:new sem fail!\n");
    return ERR_MEM;
  }
}

void sys_sem_free(sys_sem_t *sem)   //ɾ��һ���ź���
{
#if SYS_STATS
   --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
  /* ɾ�� sem */
  vSemaphoreDelete(*sem);
  *sem = SYS_SEM_NULL;
}

void sys_sem_signal(sys_sem_t *sem)  //�ͷ�һ���ź���
{
  if(xSemaphoreGive( *sem ) != pdTRUE)
    printf("[sys_arch]:sem signal fail!\n");
}

/*
 ���timeout������Ϊ�㣬�򷵻�ֵΪ
 �ȴ��ź��������ѵĺ����������
 �ź���δ��ָ��ʱ���ڷ����źţ�����ֵΪ
 SYS_ARCH_TIMEOUT������̲߳��صȴ��ź���
 �ú��������㡣 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;

  //�����ź����Ƿ���Ч
  if(*sem == SYS_SEM_NULL)
    return SYS_ARCH_TIMEOUT;

  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = xTaskGetTickCount();

  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  else
    wait_tick = portMAX_DELAY;  //һֱ����

  //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
  if(xSemaphoreTake(*sem, wait_tick) == pdTRUE)
    return ((xTaskGetTickCount()-start_tick)*portTICK_RATE_MS);
  else
    return SYS_ARCH_TIMEOUT;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)   //�½�һ������
{
    /* ����Test_Queue */
  *mbox = xQueueCreate((UBaseType_t ) size,/* ��Ϣ���еĳ��� */
                       (UBaseType_t ) sizeof(void *));/* ��Ϣ�Ĵ�С */
#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
      if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
         lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
      }
#endif /* SYS_STATS */
    if(NULL == *mbox)
    return ERR_MEM;

  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)  //ɾ��һ������
{
  if( uxQueueMessagesWaiting( *mbox ) )
    {
        /* Line for breakpoint.  Should never break here! */
        portNOP();
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */

        // TODO notify the user of failure.
    }

  vQueueDelete(*mbox);

#if SYS_STATS
     --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

void sys_mbox_post(sys_mbox_t *q, void *msg)  //������Ͷ����Ϣ������
{
  while(xQueueSend( *q, /* ��Ϣ���еľ�� */
                    &msg,/* ���͵���Ϣ���� */
                    portMAX_DELAY) != pdTRUE); /* �ȴ�ʱ�� */
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)  //����������Ͷ����Ϣ��������
{
  if(xQueueSend(*q,&msg,0) == pdPASS)
    return ERR_OK;
  else
    return ERR_MEM;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)   //�������ȡ��Ϣ������
{
  void *dummyptr;
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;

  if ( msg == NULL )  //�����洢��Ϣ�ĵط��Ƿ���Ч
        msg = &dummyptr;

  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = sys_now();

  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  //һֱ����
  else
    wait_tick = portMAX_DELAY;

  //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
  if(xQueueReceive(*q,&(*msg), wait_tick) == pdTRUE)
    return ((sys_now() - start_tick)*portTICK_PERIOD_MS);
  else
  {
    *msg = NULL;
    return SYS_ARCH_TIMEOUT;
  }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)  //���Դ�������ȡ��Ϣ��������
{
    void *dummyptr;
    if ( msg == NULL )
        msg = &dummyptr;

  //�ȴ��ɹ�������ȴ���ʱ��
  if(xQueueReceive(*q,&(*msg), 0) == pdTRUE)
    return ERR_OK;
  else
    return SYS_MBOX_EMPTY;
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)  //����������
{
  sys_thread_t handle = NULL;
  BaseType_t xReturn = pdPASS;
  /* ����MidPriority_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )function,  /* ������ں��� */
                        (const char*    )name,/* �������� */
                        (uint16_t       )stacksize,  /* ����ջ��С */
                        (void*          )arg,/* ������ں������� */
                        (UBaseType_t    )prio, /* ��������ȼ� */
                        (TaskHandle_t*  )&handle);/* ������ƿ�ָ�� */
  if(xReturn != pdPASS)
  {
    printf("[sys_arch]:create task fail!err:%#lx\n",xReturn);
    return NULL;
  }
  return handle;
}

sys_prot_t sys_arch_protect(void)   //�����ٽ�������
{
    vPortEnterCritical();
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval)   //�˳��ٽ�������
{
    ( void ) pval;
    vPortExitCritical();
}
