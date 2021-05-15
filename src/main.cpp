/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(__ZEPHYR__)
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/uart.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>
//#include <data/json.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h> //for memset
#include <math.h>

#include <string>
#include <list>
#include <vector>
#include <map>

#if !defined(__ZEPHYR__) //for x86
#include "memtracer.h"
#endif

#include "global.h"
#include "eventfunctor.h"
#include "eventpack.h"
#include "eventmanager.h"
#include "watchdog.h"
#include "heartbeat.h"
#include "test_uart00.h"
#include "test_uart01.h"
#include "test_uart02.h"

#define THREAD_STACKSIZE  4096

#if defined(__ZEPHYR__)
K_THREAD_STACK_DEFINE(stack_application_area, THREAD_STACKSIZE);
static struct k_work_q    s_application_work_q;
#endif

//~/ncs/zephyr/include/data/json.h
/*static void test_json_obj_arr_encoding()
{
  typedef struct _test_struct_t
  {
    //char  some_string[64]; //will crash in json lib
    char* some_string;
    int   some_int;
    bool  some_bool;
  }test_struct_t;

  test_struct_t ts = 
  {
    //.some_string  = {0},
    .some_string  = "zephyr 123",
    .some_int     = 42,
    .some_bool    = true,
  };

  struct json_obj_descr test_descr[] = 
  {
    JSON_OBJ_DESCR_PRIM(test_struct_t, some_string,  JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(test_struct_t, some_int,     JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(test_struct_t, some_bool,    JSON_TOK_TRUE),
  };

  char buffer[256] = {0};
  
  json_obj_encode_buf(test_descr, ARRAY_SIZE(test_descr), &ts, buffer, sizeof(buffer));
  
  printk("%s\n", buffer);
}

static void test_json_obj_arr_decoding()
{
  struct  test_struct 
  {
    char*   some_string;
    int     some_int;
    bool    some_bool;
  };

  struct test_struct ts = 
  {
    .some_string  = "zephyr 000",
    .some_int     = 0,
    .some_bool    = false,
  };

  struct json_obj_descr test_descr[] = 
  {
    JSON_OBJ_DESCR_PRIM(struct test_struct, some_string,  JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct test_struct, some_int,     JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct test_struct, some_bool,    JSON_TOK_TRUE),
  };

  char encoded[] =  "{\"some_string\":\"zephyr 123\","
                    "\"some_int\":42,\"some_bool\":true,"
                    "}";

  json_obj_parse(encoded, sizeof(encoded) - 1, test_descr, ARRAY_SIZE(test_descr), &ts);

  printk("%s : %d : %d\n", ts.some_string, ts.some_int, ts.some_bool);
}*/


//"memory" = memory barrier, save to register not memory, in -Og -O1 -O2 -O3 of gcc
int test_asm00()
{
  int ret;
  uint32_t arm_reg_input[4] = {0x01};

  arm_reg_input[0] += 1;
  __asm__ volatile("" ::: "memory");
  arm_reg_input[1] += 1;
 
  ret = arm_reg_input[0]+arm_reg_input[1];
  
  return ret;
}

int test_asm01()
{
  int ret;
  uint32_t arm_reg_input[4] = {0x01};
  
  __asm__ volatile("" ::: "memory");

  arm_reg_input[0] += 1;
  arm_reg_input[1] += 1;

  ret = arm_reg_input[0]+arm_reg_input[1];

  return ret;
}

//check https://docs.zephyrproject.org/latest/reference/kernel/threads/index.html?highlight=k_thread_create#c.k_thread_create
#if defined(__ZEPHYR__)
void main(void)
#else
int main(int argc, char* argv[])
#endif
{
  printf("main start\n");

#if defined(__ZEPHYR__)
  k_work_q_start( &s_application_work_q, 
                  stack_application_area,  
                  K_THREAD_STACK_SIZEOF(stack_application_area),
                  CONFIG_SYSTEM_WORKQUEUE_PRIORITY);
  
  watchdog_init_and_start(&s_application_work_q);//Hank request work

  uint32_t output           = 0x01;
  uint32_t arm_reg_input[4] = {0};
  arm_reg_input[0]          = 0x02;
  arm_reg_input[1]          = 0x03;
  arm_reg_input[2]          = 0x04;
  arm_reg_input[3]          = 0x05;
  
  __asm__ volatile 
  ( 
    "add r0, r1 \n\t"
    "add r0, r2 \n\t"
    "add r0, r3 \n\t"
    "mov %0, r0 \n\t"
    : "=r"(output)
    : "r"(arm_reg_input[0]), "r"(arm_reg_input[1]), "r"(arm_reg_input[2]), "r"(arm_reg_input[3])
  );
#endif

  test_asm00();
  test_asm01();

  HeartBeat_Init();
  Test_Uart00_Init();
  Test_Uart01_Init();
  Test_Uart02_Init();

  for(int i=0; i<30; i++)
  {
    printf("main loop: %d\n", i);
    HeartBeat_BroadCast();

#if defined(__ZEPHYR__)
    k_usleep(SLEEP_HEARTBEAT);
#else
    usleep(SLEEP_HEARTBEAT);
#endif

    HeartBeat_LiveCheck();

#if defined(__ZEPHYR__)
    k_usleep(SLEEP_HEARTBEAT);
#else
    usleep(SLEEP_HEARTBEAT);
#endif
  }

  Test_Uart00_Destory(); //for test seq
  Test_Uart01_Destory();
  Test_Uart02_Destory();
  HeartBeat_Destory();


  EventManager::Free();

  //////////////////enable this can see memory leak, in debug mode/////////////////////
  //int* a = new int[1];
  //int* a = (int*)malloc(sizeof(int));
#if !defined(__ZEPHYR__) //for x86
  dumpAlloc();
#endif

  printf("main end\n");
}
