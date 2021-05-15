#ifndef EVENT_FUNCTOR_H
#define EVENT_FUNCTOR_H

template<class ParamInfo> 
class EventFunctorBase
{
public:
	virtual ~EventFunctorBase()	{ }
	virtual void*	GetMember()				= 0;
	virtual void	Call(ParamInfo* event)	= 0;
};

template<class ParamInfo> 
class EventFunctor : public EventFunctorBase<ParamInfo>
{
public:
	typedef void (*MemberPtr)(ParamInfo*); //ParamInfo is func's paramater

	EventFunctor(MemberPtr callmember)
	{
		m_pCallMember = callmember;
	}

	virtual ~EventFunctor()	{ }

	virtual void* GetMember() 
	{ 
		return (void*)m_pCallMember;
	}

	virtual void Call(ParamInfo* event)
	{ 
		(*m_pCallMember)(event);
	}

private:
	MemberPtr	m_pCallMember;
};

#endif // I_FUNCTOR_H
