#ifndef __ONLINEGAME_GS_PLAYER_CLOCK_H__
#define __ONLINEGAME_GS_PLAYER_CLOCK_H__
#include "common/packetwrapper.h"
#include "vector.h"

class player_clock;
class gplayer_imp;
/**
 * clock_listener Ϊ������Ҫ���ƶ�ʱ��ص���Ļ���
   
   �����ฺ���Լ��趨��������
   GetPlayerClock().AddNotice(this,����,����) 
   �ɶ�����õ���ͬ�̶�
   ����Ĭ��-1Ϊ��
   OnClock ����ָ��ʱ�̺� ������30s  �ᷢ��OnClock�ĵ��ã����ж��˳��GPC_TYPE����ͬ���͵İ�AddNotice�Ⱥ�����
   OnPassClock ���ʱ��������ʱ����ָ���̶ȵĿ�Խ�������ߵĵ�һ��heartbeatʱ,�ᷢ��OnPassClock ���ã�˳��ͬ��
   ���ͣ�
   GPC_PER_HOUR_GLOBAL 	ÿСʱ���� 	ȫ��	����Ϊ�ƶ���ĳһСʱ(0-23) 
   GPC_PER_DAY_GLOBAL  	ÿ��0��  	ȫ��	����Ϊ�ܼ���ĳ��(0-6)
   GPC_PER_WEEK_GLOBAL 	ÿ��1    	ȫ��	����Ϊ�����˱��µĵڼ�����1(0-5)
   GPC_PER_MONTH_GLOBAL ÿ�µ�1��	ȫ��	����Ϊ�ڼ����·�(0-11)
   GPC_PER_HOUR_LOCAL 	ÿСʱ���� 	����	����Ϊ�ƶ���ĳһСʱ(0-23) 
   GPC_PER_DAY_LOCAL  	ÿ��0��  	����	����Ϊ�ܼ���ĳ��(0-6)
   GPC_PER_WEEK_LOCAL 	ÿ��1    	����	����Ϊ�����˱��µĵڼ�����1(0-5)
   GPC_PER_MONTH_LOCAL 	ÿ�µ�1��	����	����Ϊ�ڼ����·�(0-11)
 */
struct clock_listener
{
	// type ��Ӧ GPC_TYPE
	virtual void OnClock(gplayer_imp* player,int type) = 0;
	virtual void OnPassClock(gplayer_imp* player,int type,int lastupdate,int now) = 0;
};

class player_clock
{
public:
	enum GPC_TYPE
	{
		GPC_PER_HOUR_GLOBAL,	// ����ʱ��ΪÿСʱ  	����	ȫ��
		GPC_PER_DAY_GLOBAL,		// ����ʱ��Ϊÿ��  		0��		ȫ��
		GPC_PER_WEEK_GLOBAL,	// ����ʱ��Ϊÿ��1 		0��		ȫ��
		GPC_PER_MONTH_GLOBAL,	// ����ʱ��Ϊÿ�µ�1�� 	0��		ȫ��
		GPC_PER_HOUR_LOCAL,		// ����ʱ��ΪÿСʱ  	����	����	
		GPC_PER_DAY_LOCAL,		// ����ʱ��Ϊÿ��  		0��		����
		GPC_PER_WEEK_LOCAL,		// ����ʱ��Ϊÿ��1 		0��		����
		GPC_PER_MONTH_LOCAL,	// ����ʱ��Ϊÿ�µ�1�� 	0��		����

		GPC_TYPE_MAX	
	};

	enum
	{
		GPC_UNINIT,
		GPC_INIT,
		GPC_NORMAL,
	};

	static tm dbgtime;
	static const int GPC_HB_IDLE = 30;  		// 30s
	static const int GPC_DAY_SEC = 86400;
	static const int GPC_INTERVAL[GPC_TYPE_MAX];

public:
	player_clock() : _idle_time(0),_state(GPC_UNINIT)
	{	
		_update_time.insert(_update_time.begin(),GPC_TYPE_MAX,player_clock::clock_update());
		_notice_list.insert(_notice_list.begin(),GPC_TYPE_MAX,player_clock::NOTICE_NODE_LIST());
	}		
	~player_clock() { _update_time.clear(); _notice_list.clear(); }

	bool Save(archive & ar)
	{
		ar << _idle_time;
		ar << _state;
		
		unsigned int size = _update_time.size();
		ar << size;
		if( !_update_time.empty() )
		{			
			for(unsigned int n = 0; n < _update_time.size(); ++n)	
			{
				ar << _update_time[n].lasttime;
				ar << _update_time[n].nexttime;
			}
		}
		return true;	
	}
	bool Load(archive & ar)
	{
		try
		{
			ar >> _idle_time;
			ar >> _state;
			int upsize;
			ar >> upsize;
			if(upsize && upsize <= GPC_TYPE_MAX)
			{
				for(int n = 0; n < upsize; ++n)	
				{
					ar >> _update_time[n].lasttime; 
					ar >> _update_time[n].nexttime;
				}
			}
			else
			{
				_update_time.clear();
				return false;
			}
		}
		catch(...)
		{
			_idle_time = 0;
			_update_time.clear();
			_state = GPC_UNINIT;
			return false;
		}
		return true;
	}
	void Swap(player_clock& rhs)
	{
		abase::swap(_idle_time, rhs._idle_time);
		abase::swap(_state, rhs._state);
		_update_time.swap(rhs._update_time);	
	}

public:
	void InitCheck(gplayer_imp* player,int now,bool incentral);
	void CheckTime(gplayer_imp* player,int now,bool incentral);
	void OnHeartbeat(gplayer_imp* player,int now,bool incentral);

	void SaveToDB(archive & ar);
	void InitFromDB(archive & ar,int roleid);

	void AddNotice(clock_listener* listener,int type,int cond = -1) // ���ж��ظ�����
	{
		if(type < GPC_TYPE_MAX && listener) 	_notice_list[type].push_back(clock_node(listener,cond));
	}

	// for debug
	void Reset(gplayer_imp* player,int type) ;
public:	
	static int GetWeekNum(int nowtime);
	static int GetMonthDiff(int oldtime, int nowtime);
	static int GetMonthDayDiff(int year ,int mon);
	static int GetNextUpdatetime(int type,int nowtime);
	static int GetPassPeriod(int type, int oldtime, int nowtime);
	static bool CheckPrviateCond(int type, int cond, int stubtime, int nowtime, int passperiod, bool nopass);

private:
	struct clock_node
	{
		clock_node(clock_listener* l, int cond) : listener(l), condition(cond) {}
		clock_listener* listener;
		int	condition;
		void Init(gplayer_imp* player,int type, int old, int stub, int now, int passperiod)
		{
			if(listener && (-1 == condition || CheckPrviateCond(type,condition,stub,now,passperiod,false))) 
				listener->OnPassClock(player, type, old, now);
		}
		void Run(gplayer_imp* player,int type,int now)
		{
			if(listener && (-1 == condition || CheckPrviateCond(type,condition,now,now,0,true))) 
				listener->OnClock(player, type);
		}
	};
	
	typedef abase::vector<clock_node , abase::fast_alloc<> > NOTICE_NODE_LIST;
	typedef abase::vector<NOTICE_NODE_LIST, abase::fast_alloc<> > NOTICE_LIST;
	NOTICE_LIST _notice_list;  // ����Ҫ����

	struct clock_update
	{
		clock_update() : lasttime(0), nexttime(0) {}
		int lasttime;
		int nexttime;
	};

	typedef abase::vector<clock_update, abase::fast_alloc<> > UPDATE_LIST;
	UPDATE_LIST	_update_time;

	int _idle_time;
	int _state;
};

#endif
