#if defined(__ZEPHYR__)
#include <zephyr.h>
#include <drivers/uart.h>
#else
#include <pthread.h>
#endif
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <map>

#if !defined(__ZEPHYR__) //for x86
#include "memtracer.h"
#endif

#include "global.h"
#include "eventfunctor.h"
#include "eventpack.h"
#include "eventmanager.h"
#include "heartbeat.h"
#include "test_uart01.h"

#define THREAD_STACKSIZE  4096
#define UART_BUFSIZE      2048

enum
{
  UART_READ,
  UART_WRITE
};

enum
{
  FEEDBACK_READ,
  FEEDBACK_WRITE
};

#if defined(__ZEPHYR__)
K_THREAD_STACK_DEFINE(s_test_uart01_stack, THREAD_STACKSIZE);
K_MUTEX_DEFINE(s_test_uart01_fifo_mutex);
static struct k_thread	s_kthread_test_uart01   = {0};
#else
pthread_mutex_t 		s_test_uart01_fifo_mutex;
static pthread_t        s_kthread_test_uart01	= 0;
#endif


static volatile int     s_test_uart01_work      = 0;
static volatile int     s_test_uart01_loop      = 0;

static volatile bool    s_test_uart01_heartbeat = false;
static volatile int     s_test_uart01_shift     = 0;
static char*            s_test_uart01_buf       = NULL;

static void Test_Uart01_OnHeartBeat(EventPack* event)
{
  s_test_uart01_heartbeat = true;
}

static void Test_Uart01_HeartBeat_Reponse()
{
  EventPack* event = new EventPack(EVT_HEARTBEAT_FEEDBACK);

  event->SetSourceId(SRC_TEST_UART01);
  EventManager::Get()->SendEventMessage(event);

  delete event;
}

static bool Test_Uart01_Fifo_Process(int type, char* read_str, char write_char)
{
  int ret = false;

#if defined(__ZEPHYR__)
  k_mutex_lock(&s_test_uart01_fifo_mutex, K_FOREVER);
#else
  pthread_mutex_lock(&s_test_uart01_fifo_mutex);
#endif
  
  switch(type)
  {
    case UART_READ:
              if( s_test_uart01_shift > 0 )
              {
                if( s_test_uart01_buf[s_test_uart01_shift-1] == '\n'
                    ||
                    s_test_uart01_buf[s_test_uart01_shift-1] == '\r')
                {
                  strcpy(read_str, s_test_uart01_buf);
                  
                  s_test_uart01_shift = 0;
                  memset(s_test_uart01_buf, 0, UART_BUFSIZE);
                
                  ret = true;
                }
              }
              break;
    
    case UART_WRITE:
              //buf 0~2047, data 0~2046, 2047 string end always 0x01
              if( s_test_uart01_shift < UART_BUFSIZE-1 )
              {
                s_test_uart01_buf[s_test_uart01_shift] = write_char;
                s_test_uart01_shift++;
              }
              break;
  }

#if defined(__ZEPHYR__)
  k_mutex_unlock(&s_test_uart01_fifo_mutex);
#else
  pthread_mutex_unlock(&s_test_uart01_fifo_mutex);
#endif

  return ret;
}

#if defined(__ZEPHYR__)
static void uart_cb1(const struct device* dev, void* user_data)
{
  char c;
  
  uart_irq_update(dev);
  if (uart_irq_rx_ready(dev)) 
  {
    while (uart_fifo_read(dev, (uint8_t*)&c, 1)) 
    {
      Test_Uart01_Fifo_Process(UART_WRITE, NULL, c);
    }
  }
}

static void Test_Uart01_Poll_Out(const struct device* uart_dev, char* data, uint32_t len)
{
    char      c;
    uint32_t  i;

    for (i = 0; i<len; i++) 
    {
      uart_poll_out(uart_dev, data[i]);

      if( data[i] == 0x0a )
      {
        c = 0x0d; //return
        uart_poll_out(uart_dev, c);
      }
    }
}
#endif

#if defined(__ZEPHYR__)
static void Test_Uart01_Thread(void* p0, void* p1, void* p2)
#else
static void* Test_Uart01_Thread(void* pparam)
#endif
{
  s_test_uart01_loop = 1;

  char* local_buf             = (char*)calloc(UART_BUFSIZE, sizeof(char));
#if defined(__ZEPHYR__)
  const struct device* uart1  = device_get_binding("UART_1");

  uart_irq_callback_set(uart1, uart_cb1);
  uart_irq_rx_enable(uart1);
#endif

  while(s_test_uart01_work)
  {
    if( Test_Uart01_Fifo_Process(UART_READ, local_buf, 0) )
    {
      //do something here
      // ...
      memset(local_buf, 0, UART_BUFSIZE);
    }
    
#if defined(__ZEPHYR__)
    //Test_Uart01_Poll_Out(uart1, "test uart01", strlen("test uart01"));
#endif

    if( s_test_uart01_heartbeat )
    {
      s_test_uart01_heartbeat = 0;
      Test_Uart01_HeartBeat_Reponse();
    }

#if defined(__ZEPHYR__)
    k_usleep(SLEEP_LOOP);
#else
    usleep(SLEEP_LOOP);
#endif
  }

#if defined(__ZEPHYR__)
  uart_irq_rx_disable(uart1);
  uart_irq_callback_set(uart1, NULL);
#endif

  free(local_buf);

  s_test_uart01_loop = 0;

#if !defined(__ZEPHYR__)
  return NULL;
#endif
}

void Test_Uart01_Init()
{
  //EventManager::Get()->ConnectEventSlot(EVT_HEARTBEAT_BROADCAST, new EventFunctor<EventPack>(&Test_Uart01_OnHeartBeat));
  EventManager::Get()->ConnectEventSlot(EVT_HEARTBEAT_BROADCAST, (void*)&Test_Uart01_OnHeartBeat);

  s_test_uart01_buf = (char*)calloc(UART_BUFSIZE, sizeof(char));
  
  s_test_uart01_work  = 1;
#if defined(__ZEPHYR__)
  k_thread_create(&s_kthread_test_uart01, //local kthread_softuart will crash in k_sleep
                  s_test_uart01_stack,
                  THREAD_STACKSIZE,
                  Test_Uart01_Thread,
                  NULL, 
                  NULL, 
                  NULL,
                  0, 
                  K_INHERIT_PERMS, 
                  K_NO_WAIT);
#else
  pthread_attr_t	pattr;
  pthread_attr_init(&pattr);
  pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);
  pthread_create(&s_kthread_test_uart01, &pattr, Test_Uart01_Thread, NULL);
  pthread_attr_destroy(&pattr);
#endif
}

void Test_Uart01_Destory()
{
  s_test_uart01_work = 0; //need before k_sem_reset
  while( s_test_uart01_loop )
  {
#if defined(__ZEPHYR__)
    k_usleep(SLEEP_LOOP); //6 second
#else
    usleep(SLEEP_LOOP);
#endif
  }

  free(s_test_uart01_buf);

  EventManager::Get()->DisconnectEventSlot((void*)&Test_Uart01_OnHeartBeat);
}
