#if defined(__ZEPHYR__)
#include <zephyr.h>
#include <drivers/uart.h>
#include <power/reboot.h>
#endif

#include <stdint.h>
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

std::map<int, bool> s_heartbeat_live_map;

void HeartBeat_BroadCast()
{
  printf("BroadCast\n");
  
  EventPack* event = new EventPack(EVT_HEARTBEAT_BROADCAST);

  event->SetSourceId(SRC_HEARTBEAT);
  EventManager::Get()->SendEventMessage(event);

  delete event;
}

static void HeartBeat_FeedBack(EventPack* event)
{
  int srcid = event->GetSourceId();
  s_heartbeat_live_map[srcid] = true;
}

int HeartBeat_LiveCheck()
{
  uint32_t  total_live;
  char      buf[64];
  
  total_live = 0;
  for (auto& mit : s_heartbeat_live_map)
  {
    sprintf(buf,"id=%d, live=%d\n", mit.first, mit.second);
    printf("%s", buf);
    
    if( mit.second )
      total_live++;

    mit.second = false;
  }

  if( total_live < s_heartbeat_live_map.size() )
  {
#if defined(__ZEPHYR__)
    sys_reboot(0);
#else
    exit(0);
#endif
  }
  
    
  return total_live;
}

void HeartBeat_Init()
{
  s_heartbeat_live_map.insert({SRC_TEST_UART00, false });
  s_heartbeat_live_map.insert({SRC_TEST_UART01, false });
  s_heartbeat_live_map.insert({SRC_TEST_UART02, false });

  //EventManager::Get()->ConnectEventSlot(EVT_HEARTBEAT_FEEDBACK, new EventFunctor<EventPack>(&HeartBeat_FeedBack));
  EventManager::Get()->ConnectEventSlot(EVT_HEARTBEAT_FEEDBACK, (void*)&HeartBeat_FeedBack);
}

void HeartBeat_Destory()
{
  EventManager::Get()->DisconnectEventSlot((void*)&HeartBeat_FeedBack);

  s_heartbeat_live_map.clear();
}
