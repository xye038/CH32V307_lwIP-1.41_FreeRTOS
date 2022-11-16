#include "lwip/sys.h"

#define SYS_THREAD_MAX 4    //允许协议栈创建的最大任务数
u32_t lwip_sys_now;

u32_t sys_now(void)
{
  lwip_sys_now = xTaskGetTickCount();
  return lwip_sys_now;
}

int sys_sem_valid(sys_sem_t *sem)     //判断一个信号量是否有效
{
  return (*sem != SYS_SEM_NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem)  //将一个信号量置为无效
{
  *sem = SYS_SEM_NULL;
}

int sys_mbox_valid(sys_mbox_t *mbox)    //判断一个邮箱是否有效
{
  return (*mbox != SYS_MBOX_NULL);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)  //将一个邮箱置为无效
{
  *mbox = SYS_MBOX_NULL;
}

void sys_init(void)   //初始化操作系统模拟层
{
    printf("[sys_arch] init ok\r\n");      //打印初始化成功信息
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)   //新建一个信号量
{
  /* 创建 sem */
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

void sys_sem_free(sys_sem_t *sem)   //删除一个信号量
{
#if SYS_STATS
   --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
  /* 删除 sem */
  vSemaphoreDelete(*sem);
  *sem = SYS_SEM_NULL;
}

void sys_sem_signal(sys_sem_t *sem)  //释放一个信号量
{
  if(xSemaphoreGive( *sem ) != pdTRUE)
    printf("[sys_arch]:sem signal fail!\n");
}

/*
 如果timeout参数不为零，则返回值为
 等待信号量所花费的毫秒数。如果
 信号量未在指定时间内发出信号，返回值为
 SYS_ARCH_TIMEOUT。如果线程不必等待信号量
 该函数返回零。 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;

  //看看信号量是否有效
  if(*sem == SYS_SEM_NULL)
    return SYS_ARCH_TIMEOUT;

  //首先获取开始等待信号量的时钟节拍
  start_tick = xTaskGetTickCount();

  //timeout != 0，需要将ms换成系统的时钟节拍
  if(timeout != 0)
  {
    //将ms转换成时钟节拍
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  else
    wait_tick = portMAX_DELAY;  //一直阻塞

  //等待成功，计算等待的时间，否则就表示等待超时
  if(xSemaphoreTake(*sem, wait_tick) == pdTRUE)
    return ((xTaskGetTickCount()-start_tick)*portTICK_RATE_MS);
  else
    return SYS_ARCH_TIMEOUT;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)   //新建一个邮箱
{
    /* 创建Test_Queue */
  *mbox = xQueueCreate((UBaseType_t ) size,/* 消息队列的长度 */
                       (UBaseType_t ) sizeof(void *));/* 消息的大小 */
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

void sys_mbox_free(sys_mbox_t *mbox)  //删除一个邮箱
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

void sys_mbox_post(sys_mbox_t *q, void *msg)  //向邮箱投递消息，阻塞
{
  while(xQueueSend( *q, /* 消息队列的句柄 */
                    &msg,/* 发送的消息内容 */
                    portMAX_DELAY) != pdTRUE); /* 等待时间 */
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)  //尝试向邮箱投递消息，不阻塞
{
  if(xQueueSend(*q,&msg,0) == pdPASS)
    return ERR_OK;
  else
    return ERR_MEM;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)   //从邮箱获取消息，阻塞
{
  void *dummyptr;
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;

  if ( msg == NULL )  //看看存储消息的地方是否有效
        msg = &dummyptr;

  //首先获取开始等待信号量的时钟节拍
  start_tick = sys_now();

  //timeout != 0，需要将ms换成系统的时钟节拍
  if(timeout != 0)
  {
    //将ms转换成时钟节拍
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  //一直阻塞
  else
    wait_tick = portMAX_DELAY;

  //等待成功，计算等待的时间，否则就表示等待超时
  if(xQueueReceive(*q,&(*msg), wait_tick) == pdTRUE)
    return ((sys_now() - start_tick)*portTICK_PERIOD_MS);
  else
  {
    *msg = NULL;
    return SYS_ARCH_TIMEOUT;
  }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)  //尝试从邮箱中取消息，不阻塞
{
    void *dummyptr;
    if ( msg == NULL )
        msg = &dummyptr;

  //等待成功，计算等待的时间
  if(xQueueReceive(*q,&(*msg), 0) == pdTRUE)
    return ERR_OK;
  else
    return SYS_MBOX_EMPTY;
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)  //创建新任务
{
  sys_thread_t handle = NULL;
  BaseType_t xReturn = pdPASS;
  /* 创建MidPriority_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )function,  /* 任务入口函数 */
                        (const char*    )name,/* 任务名字 */
                        (uint16_t       )stacksize,  /* 任务栈大小 */
                        (void*          )arg,/* 任务入口函数参数 */
                        (UBaseType_t    )prio, /* 任务的优先级 */
                        (TaskHandle_t*  )&handle);/* 任务控制块指针 */
  if(xReturn != pdPASS)
  {
    printf("[sys_arch]:create task fail!err:%#lx\n",xReturn);
    return NULL;
  }
  return handle;
}

sys_prot_t sys_arch_protect(void)   //进入临界区保护
{
    vPortEnterCritical();
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval)   //退出临界区保护
{
    ( void ) pval;
    vPortExitCritical();
}
