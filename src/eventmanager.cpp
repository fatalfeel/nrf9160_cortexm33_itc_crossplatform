#if defined(__ZEPHYR__)
#include <zephyr.h>
#else
#include <pthread.h>
#endif

#include <stddef.h>
#include <vector>
#include <map>

#if !defined(__ZEPHYR__) //for x86
#include "memtracer.h"
#endif

#include "eventfunctor.h"
#include "eventpack.h"
#include "eventmanager.h"

#if defined(__ZEPHYR__)
K_MUTEX_DEFINE(s_evtmanager_mutex);
#else
pthread_mutex_t s_evtmanager_mutex;
#endif

enum {
EVENTMGR_CONNECT,
EVENTMGR_DISCONNECT,
EVENTMGR_SEND
};

static EventManager* evtmanager_instance = NULL;

EventManager* EventManager::Get()
{
  if (!evtmanager_instance)
    evtmanager_instance = new EventManager();

  return evtmanager_instance;
}

void EventManager::Free()
{
  if( evtmanager_instance )
  {
    delete evtmanager_instance;
    evtmanager_instance = NULL;
  }
}

EventManager::EventManager()
{
#if !defined(__ZEPHYR__)
  pthread_mutex_init(&s_evtmanager_mutex, NULL);
#endif
}

EventManager::~EventManager()
{
#if !defined(__ZEPHYR__)
  pthread_mutex_destroy(&s_evtmanager_mutex);
#endif
}

void EventManager::ProcessEvent(int type, int messageID, void* callmember, EventPack* event)
{
#if defined(__ZEPHYR__)
  k_mutex_lock(&s_evtmanager_mutex, K_FOREVER);
#else
  pthread_mutex_lock(&s_evtmanager_mutex);
#endif

  unsigned int                      i;
  unsigned int                      size;
  unsigned int                      shift_p;
  unsigned int                      remain_size_vit;
  unsigned int                      remain_size_mit;
  EventMessageSlot_Vector::iterator vit;
  EventMessageSlot_Map::iterator    mit;
  EventMessageSlot_Map::iterator    tmp_ptr;
  EventFunctor<EventPack>*          functor;

  switch(type)
  {
  	  case EVENTMGR_CONNECT:
  		  functor = new EventFunctor<EventPack>((EventFunctor<EventPack>::MemberPtr)callmember);
  		  m_EventMessageSlotMap[messageID].push_back(functor);
  		  break;

  	  case EVENTMGR_DISCONNECT:
  		  mit             = m_EventMessageSlotMap.begin();
  		  remain_size_mit = m_EventMessageSlotMap.size();
  		  while(remain_size_mit > 0)
  		  {
  		    shift_p         = 0;
  		    remain_size_vit = mit->second.size();

  		    while(remain_size_vit > 0)
  		    {
  		      if( mit->second[shift_p]->GetMember() == callmember )
  		      {
  		        vit = mit->second.begin() + shift_p;
  		        delete *vit; //delete vector
  		        mit->second.erase(vit);
  		      }
  		      else
  		      {
  		        shift_p++;
  		      }

  		      remain_size_vit--;
  		    }

  		    //empty slot remove
  		    if( mit->second.size() <= 0 )
  		    {
  		      tmp_ptr = mit; //content copy
  		      mit++;
  		      m_EventMessageSlotMap.erase(tmp_ptr);
  		    }
  		    else
  		    {
  		      mit++;
  		    }

  		    remain_size_mit--;
  		  }
  		  break;

  	  case EVENTMGR_SEND:
#if !defined(__ZEPHYR__) || defined(CONFIG_DEBUG)
  		  printf("Start Source=%d~~~\n", event->GetSourceId());
#endif
  		  mit = m_EventMessageSlotMap.find(event->GetMessageId());
  		  if( mit != m_EventMessageSlotMap.end() ) //prevent null map
  		  {
			  size  = mit->second.size();

			  for ( i=0; i<size; i++)
				  mit->second[i]->Call(event);
  		  }
#if !defined(__ZEPHYR__) || defined(CONFIG_DEBUG)
  		  printf("End   Source=%d###\n", event->GetSourceId());
#endif
  		  break;
  }


#if defined(__ZEPHYR__)
  k_mutex_unlock(&s_evtmanager_mutex);
#else
  pthread_mutex_unlock(&s_evtmanager_mutex);
#endif
}

void EventManager::ConnectEventSlot(int messageID, void* callmember)
{
	this->ProcessEvent(EVENTMGR_CONNECT, messageID, callmember, NULL);
}

void EventManager::DisconnectEventSlot(void* callmember)
{
	this->ProcessEvent(EVENTMGR_DISCONNECT, 0, callmember, NULL);
}

void EventManager::SendEventMessage(EventPack* event)
{
	this->ProcessEvent(EVENTMGR_SEND, 0, NULL, event);
}
