#ifndef __ONLINEGAME_GS_MATTER_H__
#define __ONLINEGAME_GS_MATTER_H__

#include "object.h"
#include "actobject.h"
#include "vector.h"


class gmatter_imp : public gobject_imp
{
	
public:
DECLARE_SUBSTANCE(gmatter_imp);
public:
	bool _collision_actived;        //��ײ�Ƿ���Ч
	gmatter_imp():_collision_actived(false){}
	gmatter * GetParent() { return (gmatter*)_parent;}
	void ActiveCollision(bool active);
};

class gmatter_dispatcher: public dispatcher
{
public:
DECLARE_SUBSTANCE(gmatter_dispatcher);
public:
	virtual void begin_transfer() {ASSERT(false);}//��Ʒ���������ܵ��������Ա�����Ϊ��}
	virtual void end_transfer(){ASSERT(false);}
	virtual void enter_slice(slice * ,const A3DVECTOR &){ASSERT(false);}
	virtual void leave_slice(slice * ,const A3DVECTOR &){ASSERT(false);}
	virtual void get_base_info(){ASSERT(false);}
	virtual void leave_world(){ASSERT(false);}
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode){ ASSERT(false);}
	virtual void notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos){ASSERT(false);}

	virtual void enter_world();
	virtual void disappear();
	virtual void matter_pickup(int id);
	virtual void broadcast_mine_gatherd(int mid, int pid, int item_type);
};

class gmatter_controller : public controller
{
public:
	DECLARE_SUBSTANCE(gmatter_controller)
public:
	virtual int CommandHandler(int cmd_type,const void * buf, unsigned int size){ ASSERT(false); return -1;}
	virtual int MoveBetweenSlice(gobject *obj,slice * src, slice * dest) { ASSERT(false); return -1;}
	virtual void Release(bool free_parent);//������������free_parent��ֵ��һ�ɰ���true������
};

class gmatter_item_base_imp: public gmatter_imp
{
public:
	int _owner;			//0����every one
	int _team_owner;		//0����no team
	int _team_seq;			//�����seq��ֻ�е�_team_owner��0��ʱ�����Ч
	int _life;			//���� ��λ��
	int _owner_time;		//����Ȩά��ʱ�� ��λ��
	int _drop_user;			//˭�ӳ�����
	DECLARE_SUBSTANCE(gmatter_item_base_imp);
public:
	gmatter_item_base_imp(int life = 300,int belong_time = 30)
			:_owner(0),_team_owner(0),_life(life),_owner_time(belong_time),_drop_user(0)
	{}

	virtual ~gmatter_item_base_imp();

	virtual void Init(world * pPlane,gobject*parent);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);

public:
	void SetOwner(const XID & who, int team , int team_seq)
	{
		_owner = who.id;
		_team_owner = team;
		_team_seq = team_seq;
	}

	void SetDrop(int user)
	{
		_drop_user = user;
	}

	bool SpecUser()
	{
		return _owner && !_team_owner;
	}
	
	bool CheckPickup(const XID & who, int team_id, int team_seq)
	{
		return (_team_owner && team_id == _team_owner && team_seq == _team_seq) 
			|| (!_team_owner && !_owner)  
			|| (_owner && who.id == _owner);
	}

	template<int>
	void Pickup(const XID & who, int team_id, int team_seq, const A3DVECTOR &pos, const XID & bb, bool is_check)
	{
		if(is_check && !CheckPickup(who,team_id,team_seq))
		{
			MSG msg;
			BuildMessage(msg,GM_MSG_ERROR_MESSAGE,who,_parent->ID,_parent->pos,S2C::ERR_ITEM_CANT_PICKUP);
			_plane->PostLazyMessage(msg);
			return;
		}
		
		if(pos.squared_distance(_parent->pos) > (PICKUP_DISTANCE+0.1f)*(PICKUP_DISTANCE+0.1f))
		{
			//�Ƿ��д�����Ϣ��
			return ;
		}

		if(_owner)
		{
			//�������ض��û��ģ����������ԭ��������
			//�ӳ�������ƷҲ���ڴ���
			OnPickup(who,_team_owner,false);
		}
		else
		{
			//�������ض��û�����ӵ������������ ������ԭ��������
			OnPickup(bb,team_id,true);
		}

		//�Լ���ʧ
		_runner->matter_pickup(who.id);
		_commander->Release();
		return ;
	}

	virtual void OnPickup(const XID & id,int team_id,bool is_team) = 0;
	
	void SetLife(int new_life)
	{
		_life = new_life;
	}

	int GetLife()
	{
		return _life;
	}
};

/*
 *		Ǯ��һ�������matter,��������Ʒ
 */
class gmatter_money_imp : public gmatter_item_base_imp
{
public:
	unsigned int _money;
	DECLARE_SUBSTANCE(gmatter_money_imp);
public:
	gmatter_money_imp():_money(0){}
	gmatter_money_imp(unsigned int money):_money(money){}
	virtual void Init(world * pPlane,gobject*parent)
	{
		gmatter_item_base_imp::Init(pPlane,parent);
		gmatter * pMatter = (gmatter *) parent;
		pMatter->matter_type = MONEY_MATTER_ID;		//������Ǯ����Ʒ���:)
	}
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual void OnPickup(const XID & who, int team_id,bool is_team);
};

class mine_spawner;
class gmatter_mine_imp : public gmatter_imp
{
public:
	DECLARE_SUBSTANCE(gmatter_mine_imp);
	int _produce_id;	//���ֵ����Ϊ0
	int _produce_amount;
	int _produce_tool;
	int _produce_task_id;	//�����0 ��ʾ��������س�������  ���id�ǲ�����id
	int _task_id;		//�����0 ��ʾ��������ؿ���      ���id�����������id
	int _get_task;      // new	
	//int _lock_id;		//ȥ���ˣ��ĳ���_gather_players���vector���洢���Բɼ������id
	mine_spawner  * _spawner;
	bool _eliminate_tool;      //�Ƿ����Ĳɼ�����
	bool _gather_no_disappear; //�ɼ���ɺ��Ƿ���ʧ
	bool _can_be_interrupted;  //�ɼ����Ա������ж�
	bool _broadcast_aggro;
	//bool _lock;		//ȥ���ˣ���_gather_players��Ϊ�յ�ʱ��͵��ڴ�������״̬
	char _lock_time_out;
	unsigned short _gather_time_min;
	unsigned short _gather_time_max;
	int _level;
	int _exp;
	int _sp;
	int _self_faction;	
	int _ask_help_faction;
	float _aggro_range;
	int _aggro_count;
	char _mine_type;	//��������(0��ͳ���� 1Ԫ���)
	int _owner;	//�����ߵ�id,�����Ϊ0�Ļ�ֻ�и��������ܲɼ�
	int _gather_player_max;	//��ͬʱ�ɼ����������
	float _gather_distance_max;	//�ܲɼ�����Զ����
	int _produce_life;	//������Ʒ������(0Ϊû������)
	int _mine_life;		//���ﱾ��������
	float _gather_success_prob;	//�ɹ��ɼ�����Ʒ�ĸ��ʣ����㲻�ɹ�Ҳ�ɼ�������
	bool _broadcast_on_gain;	//����ɼ��ɹ��Ļ��㲥����Χ���
	
	typedef abase::vector<XID> GATHER_PLAYERS_VEC;
	GATHER_PLAYERS_VEC _gather_players;	//��ʼ�ɼ��ÿ��������б�

	struct 
	{
		int mob_id;
		int num;
		float radius;
		int remain_time;
	}produce_monster[4];

	enum
	{
		MINE_TYPE_NORMAL,	//��ͨ��
		MINE_TYPE_SOUL,		//Ԫ���
	};

public:
	gmatter_mine_imp():_produce_id(0),_produce_amount(0),_produce_tool(0),_produce_task_id(0),_task_id(0), _get_task(0),			   
			   /*_lock_id(0),*/_spawner(0),_gather_no_disappear(false),_can_be_interrupted(true),
			   /*_lock(false),*/_lock_time_out(0), _level(0),_exp(0),_sp(0)
			   {
				   _eliminate_tool = false;
			   	_broadcast_aggro = false;
				_self_faction = 0;
				_ask_help_faction = 0;
				_aggro_count = 0;
				_mine_type = 0;
				_owner = 0;
				_gather_player_max = 0;
				_gather_distance_max = 0.f;
				_produce_life = 0;
				_mine_life = 0;
				_gather_success_prob = 0.f;
				_broadcast_on_gain = false;
				memset(produce_monster,0,sizeof(produce_monster));
			   }
			   
	void SetParam(int id,int amount,unsigned short gather_time_min,unsigned short gather_time_max,int tool,
			int level, int exp,int sp,int gather_player_max,float distance,int produce_life,
			float success_prob,bool broadcast_on_gain,char mine_type)
	{
		_produce_id = id;
		_produce_amount = amount;
		_gather_time_min = gather_time_min;
		_gather_time_max = gather_time_max;
		_produce_tool = tool;
		_level = level;
		_exp = exp;
		_sp = sp;
		_mine_type = mine_type;
		_gather_player_max = gather_player_max;
		_gather_distance_max = distance;
		_produce_life = produce_life;
		_gather_success_prob = success_prob;
		_broadcast_on_gain = broadcast_on_gain;

		gmatter * pMatter = (gmatter *) _parent;
		if (MINE_TYPE_SOUL == _mine_type)
			pMatter->matter_state = gmatter::STATE_MASK_SOUL_MINE;
		else
			pMatter->matter_state = gmatter::STATE_MASK_NORMAL_MINE;
	}

	void SetTaskParam(int task_in, int task_out, int get_task, bool no_interrupted, bool gather_no_disappear, bool eliminate_tool, int self_faction , int ask_help_faction ,float aggro_range ,int aggro_count)
	{
		_task_id = task_in;
		_produce_task_id = task_out;
		_get_task = get_task;
		_can_be_interrupted = !no_interrupted;
		_gather_no_disappear = gather_no_disappear;
		_eliminate_tool = eliminate_tool;
		_broadcast_aggro = (ask_help_faction != 0);
		_self_faction = self_faction;
		_ask_help_faction = ask_help_faction;
		_aggro_range = aggro_range;
		_aggro_count = aggro_count;
	}
	void SetMonsterParam(void * buf, unsigned int count);
	
	virtual int MessageHandler(world * pPlane ,const MSG & msg);

	template <int> void SendErrMessage(const XID & who, int message)
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_ERROR_MESSAGE,who,_parent->ID,_parent->pos,message);
		_plane->PostLazyMessage(msg);
	}

	void BeMined();
	virtual void Reborn();

	inline void SetOwner(const XID & who) {_owner = who.id;}
	inline void SetLife(int new_life) {_mine_life=new_life;}
};

class gmatter_dyn_imp : public gmatter_imp
{
	public:
		DECLARE_SUBSTANCE(gmatter_dyn_imp);
		mine_spawner  * _spawner;
	public:
		gmatter_dyn_imp(): _spawner(0)
		{}

		virtual void Init(world * pPlane,gobject*parent)
		{
			gmatter_imp::Init(pPlane,parent);
			gmatter * pMatter = (gmatter *) parent;
			pMatter->matter_state = gmatter::STATE_MASK_DYN_OBJECT;
		}


		virtual int MessageHandler(world * pPlane ,const MSG & msg);
		virtual void Reborn();
};

class gmatter_money2_imp : public gmatter_item_base_imp
{
public:
	unsigned int _money;
	DECLARE_SUBSTANCE(gmatter_money2_imp);
public:
	gmatter_money2_imp():_money(0){}
	gmatter_money2_imp(unsigned int money):_money(money){}
	virtual void Init(world * pPlane,gobject*parent)
	{
		gmatter_item_base_imp::Init(pPlane,parent);
		gmatter * pMatter = (gmatter *) parent;
		pMatter->matter_type = MONEY_MATTER_ID2;		//������Ǯ����Ʒ���:)
	}
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual void OnPickup(const XID & who, int team_id,bool is_team);
};

void DropMoneyItem(world * pPlane, const A3DVECTOR & pos, unsigned int amount,const XID &owner,int owner_team,int seq,int drop_id = 0);
void DropMoney2Item(world * pPlane, const A3DVECTOR & pos, unsigned int amount,const XID &owner,int owner_team,int seq,int drop_id = 0);

#endif

