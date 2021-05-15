#ifndef SDK_EVENT_H
#define SDK_EVENT_H

extern int EVT_HEARTBEAT_BROADCAST;
extern int EVT_HEARTBEAT_FEEDBACK;
extern int SRC_HEARTBEAT;
extern int SRC_TEST_UART00;
extern int SRC_TEST_UART01;
extern int SRC_TEST_UART02;

class EventPack
{
public:
  EventPack( int messageid );
  ~EventPack();
      
  void  SetMessageId( int id );
  int	GetMessageId();

  void	SetSourceId( int id );
  int	GetSourceId();

  void	SetText( char* text );
  char*	GetText();

/******************************/      
  int		m_messageid;
  int		m_sourceid;
  char		m_text[256];
};

#endif	/// #define CONNECT_EVENT_H
