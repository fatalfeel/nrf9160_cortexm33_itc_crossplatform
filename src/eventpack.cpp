#include <stdio.h>

#if !defined(__ZEPHYR__) //for x86
#include "memtracer.h"
#endif

#include "eventfunctor.h"
#include "eventpack.h"

int EVT_HEARTBEAT_BROADCAST = 0x0000;
int EVT_HEARTBEAT_FEEDBACK  = 0x0001;
int SRC_HEARTBEAT           = 0x0002;
int SRC_TEST_UART00         = 0x0003;
int SRC_TEST_UART01         = 0x0004;
int SRC_TEST_UART02         = 0x0005;

EventPack::EventPack( int messageid )
{
  this->SetMessageId(messageid);
}

EventPack::~EventPack()
{
}

void EventPack::SetMessageId( int id ) 
{ 
  m_messageid = id; 
}

int EventPack::GetMessageId() 
{ 
  return m_messageid; 
}

void EventPack::SetSourceId( int id ) 
{ 
  m_sourceid = id;
}
	
int EventPack::GetSourceId() 
{ 
  return m_sourceid; 
}

void EventPack::SetText( char* text ) 
{ 
  sprintf(m_text, "%s", text);
}
	
char* EventPack::GetText() 
{ 
  return m_text; 
}
