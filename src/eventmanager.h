#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

class EventManager
{
public:
	
	typedef std::vector<EventFunctorBase<EventPack>*>	EventMessageSlot_Vector;
	typedef std::map<int, EventMessageSlot_Vector>		EventMessageSlot_Map; //<message_ID, EventPack_Vector>

    static EventManager* Get();
    static void Free();
	EventManager();
	~EventManager();

	// event sinks
	//void	ConnectEventSlot(int messageID, EventFunctorBase<EventPack>* functor);
	void 	ProcessEvent(int type, int messageID, void* callmember, EventPack* event);
	void	ConnectEventSlot(int messageID, void* callmember);
	void	DisconnectEventSlot(void* callmember);
	void	SendEventMessage(EventPack* event);
	
private:
	// event sinks
	EventMessageSlot_Map	m_EventMessageSlotMap;
};

#endif /// EVENT_MANAGER_H
