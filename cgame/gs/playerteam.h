#ifndef __ONLINEGAME_GS_PLAYERTEAM_H__
#define __ONLINEGAME_GS_PLAYERTEAM_H__

#include <common/types.h>
#include <common/base_wrapper.h>
#include <glog.h>
#include "config.h"
#include "staticmap.h"
#include "property.h"

class gplayer_imp;
class world;
class player_team  // :: public abase::ASmallObject
{
public:
	struct invite_t
	{
		int timestamp;
		XID id;
	};
	typedef abase::static_map<int , invite_t, TEAM_MEMBER_CAPACITY - 1> INVITE_MAP;

	struct team_entry{
		int time_out;			//ĳ��״̬�ĳ�ʱʱ��
		int self_seq;			//��������ţ�ÿ���һ�ζ��飬��һ��һ,��ֵ������ϵͳʱ��)
		int pickup_flag;		//��ʼ��������ʱ��ȡ��Ʒ������
		int last_invite_time;		//���һ�α������ʱ���
		int race_count;			//������ְҵ��Ŀ
		int change_leader_timeout;	//����leader��timeout
		int cur_max_level;		//��ǰ�����е���󼶱�
		int cur_wallow_level;		//��ǰ�����е������Եȼ�
		int min_timer_counter;		//������С�����counter
		INVITE_MAP invite_map;		//������˵�����
		INVITE_MAP apply_map;		//����������
		gplayer_imp * imp;

	};

	struct agree_invite_entry
	{
		int race;
		int cs_index;
		int cs_sid;
		team_mutable_prop data;
	};

	struct member_entry
	{
		XID id;
		team_mutable_prop data;
		int race;			//ְҵ���Ա�
		int cs_index;			//��Ա��cs index
		int cs_sid;			//��Ա��sid
		A3DVECTOR pos;			//��Ա��λ��
		int timeout;			//��Ա��ʱ���
		bool is_changed;		//��һ���Ƿ����仯
	};

	struct team_t
	{
		enum
		{
			TEAM_NO_CHANGE,
			TEAM_PART_CHANGED,
			TEAM_ALL_CHANGED,
		};
		
		XID 		leader;		//�ӳ�
		int 		team_seq;	//�������
		unsigned int		member_count;
		int 		change_flag;
		int 		pickup_flag;		//��ʼ��������ʱ��ȡ��Ʒ������
		int64_t 	team_uid; //�����Ψһid�ţ������Ŷӳ��ı���ı䣬�ڴ�������ʱ���ɣ��ڶ����ɢʱʧЧ������gametalk
			
		member_entry 	member_list[TEAM_MEMBER_CAPACITY]; 
	};
	
	class team_control
	{
	protected:
		void ChangeState(player_team * pTeam, int state)
		{
			pTeam->_team_state = state;
		}
		
		team_entry & GetData(player_team * pTeam) { return pTeam->_data;}
		team_t & GetTeam(player_team * pTeam) { return pTeam->_team;}
		
		template <int foo>
		static inline void SendMessage(world *pPlane,const MSG & msg)
		{
			player_team::SendMessage<0>(pPlane,msg);
		}
		
	private:
		friend class player_team;

//����		
		virtual void OnHeartbeat(player_team * pTeam) = 0;

//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)  = 0;
		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)  = 0;
		virtual void CliRejectInvite(player_team * pTeam, const XID & leader) = 0;
		virtual bool CliLeaveParty(player_team * pTeam) = 0;
		virtual bool CliDismissParty(player_team * pTeam) { return true;}
		virtual void CliKickMember(player_team * pTeam,const XID & member) = 0;
		virtual void CliAgreeApply(player_team *, int id, bool result) = 0;
		virtual void CliChangeLeader(player_team *, const XID & new_leader) = 0;

//��Ϣ
		virtual void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag) = 0;
		virtual void MsgAgreeInvite(player_team *,const XID & member,const A3DVECTOR &pos,const agree_invite_entry&,int seq) = 0;
		virtual void MsgRejectInvite(player_team *,const XID & member) = 0;
		virtual void MsgJoinTeam(player_team *,const XID & leader,const member_entry*, unsigned int count,int pickup_flag,int64_t team_uid,const void * ins_key , unsigned int ik_size) = 0;
		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader) = 0;
		virtual void MsgMemberUpdateData(player_team *, const XID & member,const A3DVECTOR &,const team_mutable_prop &) = 0;
		virtual void MsgLeaderUpdateData(player_team *, const XID & leader,const member_entry *,unsigned int) = 0;
		virtual void MsgMemberLeaveRequest(player_team * pTeam, const XID & member) = 0;
		virtual void MsgNotifyMemberLeave(player_team*,const XID& leader,const XID& member,int type) = 0;
		virtual void MsgNewMember(player_team*,const XID& leader,const member_entry * list, unsigned int count) =0;
		virtual void MsgLeaderCancelParty(player_team *,const XID & leader,int seq) = 0;
		virtual void MsgApplyParty(player_team*, const XID & who) = 0;
		virtual void MsgApplyPartyReply(player_team*, const XID & leader, int seq) = 0;
		virtual void MsgChangeToLeader(player_team*, const XID & leader) = 0;
		virtual void MsgLeaderChanged(player_team*, const XID & new_leader) = 0;
		virtual bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount, bool is_silver) =0;
		virtual void LostConnection(player_team * pTeam) = 0;
		virtual void Logout(player_team * pTeam) = 0;

	};



	enum
	{
		TEAM_NORMAL,
		TEAM_WAIT,
		TEAM_MEMBER,
		TEAM_LEADER,
		TEAM_STATE_NUM,
	};

	struct auto_team_info_t
	{
		enum 
		{
			GOAL_TYPE_INVALID =0,
			GOAL_TYPE_TASK,
			GOAL_TYPE_ACTIVITY,
		};
		
		enum
		{
			TIMEOUT_COMPOSING = 30,
		};
		
		char composing_timeout; //��Ա��ƥ�䣬�������״̬�ĳ�ʱ
		XID candicate_leader;	
	};
	
protected:
	static team_control * _team_ctrl[TEAM_STATE_NUM];

	int 		_team_state;		//��ӵ�״̬
	auto_team_info_t _auto_team_info;  //�Զ����
	team_entry 	_data;	
	team_t		_team;
	friend class team_control;
	friend class team_control_normal;
	friend class team_control_wait;
	friend class team_control_member;
	friend class team_control_leader;

	member_entry * FindMember(const XID & id)
	{
		for(unsigned int i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == id) return _team.member_list + i;
		}
		return NULL;
	}


	template <int foo>
	inline static void SendMessage(world *pPlane,const MSG & msg)
	{
		pPlane->PostLazyMessage(msg);
	}
	

protected:

	void CalcRaceCount();
	void CalcMinLevel();
	void CalcExactMaxLevel();

	template <int foo>
	bool BecomeLeader(const XID & first_member, const agree_invite_entry & prop,const A3DVECTOR &pos)
	{
		if(_team_state != TEAM_NORMAL)
		{
			ASSERT(false);
			return false;
		}
		
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
		_team.leader = _data.imp->_parent->ID;
		_team.member_count = 1;
		_team.member_list[0].data = team_mutable_prop(_data.imp);
		_team.member_list[0].id = _team.leader;
		_team.member_list[0].is_changed = false;
		_team.member_list[0].timeout  = 0;
		_team.member_list[0].pos = _data.imp->_parent->pos;
		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		_team.member_list[0].race = pPlayer->base_info.race;
		_team.member_list[0].cs_index = pPlayer->cs_index;
		_team.member_list[0].cs_sid  = pPlayer->cs_sid;

		_data.change_leader_timeout = 0;
		_data.cur_max_level = _data.imp->_basic.level;
		_data.cur_wallow_level = _data.imp->_wallow_level;
		_team_state = TEAM_LEADER;
		LeaderAddMember<foo>(first_member,prop,pos);
		_team.change_flag = team_t::TEAM_ALL_CHANGED;
		pPlayer->object_state |= gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER;
		pPlayer->team_id = _team.leader.id;
		_data.imp->_runner->player_in_team(1);
		//ʹ���Լ��ĸ���key��Ϊ��Ӹ����б�
		_data.imp->_team_ins_key_list = _data.imp->_cur_ins_key_list;

		GLog::log(GLOG_INFO,"�û�%d�����˶���(%d,%d)", _team.leader.id, _team.leader.id,_team.team_seq);
		return true;
	}

	template <int foo>
	bool FromMemberToLeader(const XID & leader)
	{
		if(_team_state != TEAM_MEMBER)
		{
			ASSERT(false);
			return false;
		}
		XID self = _data.imp->_parent->ID;

		_data.change_leader_timeout = 0;
		for(unsigned int i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == self)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				ent.timeout = TEAM_MEMBER_TIMEOUT;
				_team.member_list[i] = ent;
				_team.leader = self;
				_team_state = TEAM_LEADER;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				CalcRaceCount();
				CalcMinLevel();
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->object_state |= gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER;
				pPlayer->team_id = _team.leader.id;
				//������б����Ǹ����б� , ������Щ�б��ͳ������ڱ������
				_data.imp->_cur_ins_key_list = _data.imp->_team_ins_key_list;
				_data.imp->_runner->player_in_team(1);

				GLog::log(GLOG_INFO,"�û�%d��Ϊ�ӳ�(%d,%d)", self.id, leader.id, _team.team_seq);
				return true;
			}
		}

		return false;
	}

	template <int foo>
	bool ChangeLeader(const XID & new_leader)
	{
		if(_team_state != TEAM_MEMBER)
		{
			ASSERT(false);
			return false;
		}
		bool bRst = false;
		for(unsigned int i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == new_leader)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				_team.member_list[i] = ent;
				_team.leader = new_leader;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->team_id = _team.leader.id;
				bRst = true;
			}
			_team.member_list[i].timeout =TEAM_MEMBER_TIMEOUT;
		}
		return bRst;
	}

	template <int foo>
	bool FromLeaderToMember(const XID & new_leader)
	{
		if(_team_state != TEAM_LEADER)
		{
			ASSERT(false);
			return false;
		}
		for(unsigned int i = 1; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == new_leader)
			{
				member_entry ent = _team.member_list[0];
				_team.member_list[0] = _team.member_list[i];
				_team.member_list[i] = ent;
				_team.leader = new_leader;
				_team_state = TEAM_MEMBER;
				_team.change_flag = team_t::TEAM_ALL_CHANGED;
				gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
				pPlayer->object_state &= ~(gactive_object::STATE_TEAM | gactive_object::STATE_TEAMLEADER);
				pPlayer->object_state |= gactive_object::STATE_TEAM;
				pPlayer->team_id = _team.leader.id;
				_data.imp->_runner->player_in_team(2);

				return true;
			}
		}
		return false;
	}

	template<typename __XID__>
	bool BecomeMember(const __XID__ & leader, const member_entry * list, unsigned int count)
	{
		if(_team_state != TEAM_WAIT)
		{
			ASSERT(false);
			return false;
		}
		ASSERT(leader == _team.leader);

		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
		_team.member_count = 0;
		_team_state = TEAM_MEMBER;
		_data.cur_max_level = _data.imp->_basic.level;
		_data.cur_wallow_level = _data.imp->_wallow_level;
		LeaderUpdateMembers(leader,list,count);

		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		pPlayer->object_state &= ~gactive_object::STATE_TEAMLEADER;
		pPlayer->object_state |= gactive_object::STATE_TEAM;
		pPlayer->team_id = _team.leader.id;
		_data.imp->_runner->player_in_team(2);
		GLog::log(GLOG_INFO,"�û�%d��Ϊ��Ա(%d,%d)",pPlayer->ID.id,leader.id,_team.team_seq);
		
		//�����ǰ��������״̬���ڽ�������֪ͨ�����Լ�appear
		if(pPlayer->IsInvisible()) _data.imp->SendAppearToTeam();
		
		return true;
	}

	template <int foo>
	void BecomeNormal()
	{
		gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
		GLog::log(GLOG_INFO,"�û�%d�������(%d,%d)",pPlayer->ID.id,_team.leader.id,_team.team_seq);
		
		//�����ǰ��������״̬�������ǰ֪ͨ�����Լ�disappear
		if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeam();
		
		_team_state = TEAM_NORMAL;
		_team.leader = XID(-1,-1);
		_team.member_count = 0;
		_team.team_seq = 0;
		_team.team_uid = 0;
		_data.time_out = 0;
		_data.invite_map.clear();
		_data.apply_map.clear();
		pPlayer->object_state &= ~(gactive_object::STATE_TEAMLEADER | gactive_object::STATE_TEAM);
		pPlayer->team_id = 0;
		_data.imp->_runner->player_in_team(0);
	}

	template <int foo>
	bool LeaderAddMember(const XID & member,const agree_invite_entry  &prop,const A3DVECTOR &pos)
	{
		if(_team_state != TEAM_LEADER) return false;
		if(_team.member_count < TEAM_MEMBER_CAPACITY)
		{
			//�����ǰ��������״̬֪ͨ�¼���Ķ�Ա�Լ�appear
			gplayer* player = (gplayer*)_data.imp->_parent;
			if(player->IsInvisible()) _data.imp->SendAppearToTeamMember(member.id,prop.cs_index,prop.cs_sid);
			
			int index = _team.member_count;
			_team.member_list[index].id = member;
			_team.member_list[index].is_changed = true;
			_team.member_list[index].timeout =TEAM_MEMBER_TIMEOUT;
			_team.member_list[index].data = prop.data;
			_team.member_list[index].pos = pos ;
			_team.member_list[index].race = prop.race ;
			_team.member_list[index].cs_index = prop.cs_index ;
			_team.member_list[index].cs_sid = prop.cs_sid ;
			_team.member_count ++;
			if(_data.cur_max_level < prop.data.level) _data.cur_max_level = prop.data.level;
			if(_data.cur_wallow_level < prop.data.wallow_level) _data.cur_wallow_level = prop.data.wallow_level;
			_team.change_flag = team_t::TEAM_PART_CHANGED;
			CalcRaceCount();
			CalcMinLevel();
			return true;
		}
		return false;
	}

	member_entry * UpdateMemberData(const XID & id, const A3DVECTOR &pos, const team_mutable_prop & data)
	{
		if(!IsInTeam()) return NULL;
		member_entry * pEntry = FindMember(id);
		if(pEntry)
		{
			if(pEntry->data != data)
			{
				pEntry->data = data;
				pEntry->is_changed = true;
				_team.change_flag = team_t::TEAM_PART_CHANGED;
			}
			//λ������ǿ�Ƹ���
			pEntry->pos = pos;
		}
		return pEntry;
	}

	bool LeaderUpdateMembers(const XID & source,const member_entry * list , unsigned int count)
	{
		if(source == _team.leader)
		{
			if(count > TEAM_MEMBER_CAPACITY)
			{
				return false;
			}
			for(unsigned int i = 0; i < count ; i++)
			{
				if(_team.member_count >= i
					|| _team.member_list[i].data != list[i].data
					|| !(_team.member_list[i].id == list[i].id))
				{
					//ֻ�е����ݸı���޸����ݣ�����Ϊ�˼��ٴ���ռ��
					_team.member_list[i] = list[i];
					_team.member_list[i].is_changed = true;
				}
				else
				{
					//ʲô�����޸� ����Ȼԭ��is_changedҲ����Ϊtrue
					//ֻ����λ����Ϣ
					_team.member_list[i].pos = list[i].pos;
				}
			}
			_team.member_count = count;
			_team.change_flag = team_t::TEAM_ALL_CHANGED;
			CalcRaceCount();
			CalcMinLevel();
			return true;
		}
		else
		{
			return false;
		}
	}

	void LeaderRemoveMember(member_entry * pEntry);
	void MemberRemoveMember(member_entry * pEntry);

	template <int foo>
	void SendSelfDataToMember(const team_mutable_prop & data)
	{
		if(!IsInTeam()) return;
		MSG msg;
		BuildMessage(msg,GM_MSG_MEMBER_NOTIFY_DATA,XID(-1,-1),_data.imp->_parent->ID,_data.imp->_parent->pos,0,0,&data,sizeof(data));
		SendGroupMessage(msg);
	}

	template < int foo>
	void SendTeamDataToClient()
	{
		switch(_team.change_flag)
		{
			case team_t::TEAM_NO_CHANGE:
			return;

			case team_t::TEAM_PART_CHANGED:
			{
				unsigned int member_count = _team.member_count;
				abase::vector<const member_entry *,abase::fast_alloc<> > list;
				list.reserve(member_count);
				for(unsigned int i = 0; i < member_count ; i++)
				{
					if(_team.member_list[i].is_changed)
					{
						list.push_back(_team.member_list + i);
						_team.member_list[i].is_changed = false;
					}
				}
				if(list.size())
				{
					_data.imp->SendTeamData(_team.leader,member_count,list.size(),list.begin());
				}
				_team.change_flag = team_t::TEAM_NO_CHANGE;
			}
			return;

			case team_t::TEAM_ALL_CHANGED:
			{
				unsigned int member_count = _team.member_count;
				if(member_count)
				{
					_data.imp->SendTeamData(_team.leader,member_count,_team.member_list);
				}

				for(unsigned int i = 0; i < member_count ; i++)
				{
					_team.member_list[i].is_changed = false;
				}
				_team.change_flag = team_t::TEAM_NO_CHANGE;
			}
			return;

			default:
			ASSERT(false);
			break;
		}
		
	}


public:
	void SendGroupMessage(const MSG & msg);
	void SendAllMessage(const MSG & msg, int ex_id);
	void SendAllMessage(const MSG & msg);
	void SendMemberMessage(int idx, MSG & msg);
	void SendMessage(const MSG & msg, float range);		//������Ϣ������
	void SendAllMessage(const MSG & msg, float range);	//������Ϣ������ �����Լ�
	template <int>
	void SendGroupData(const void * buf, unsigned int size)
	{
		unsigned int count = _team.member_count;
		int self = _data.imp->_parent->ID.id;
		for(unsigned int i = 0;i < count; i ++)
		{
			const member_entry &  ent = _team.member_list[i];
			if(self == ent.id.id) continue;
			send_ls_msg(ent.cs_index, ent.id.id, ent.cs_sid,buf,size);
		}
	}

	player_team()
	{
		memset(&_team,0,sizeof(_team));
		memset(&_data,0,sizeof(_data));
		_team_state = TEAM_NORMAL;
		_data.time_out = 0;
		_data.last_invite_time = 0;
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
	}

	void Init(gplayer_imp * pPlayer);
	
	bool Save(archive & ar);
	bool Load(archive & ar);
	void Swap(player_team & rhs);
	
	const member_entry &GetMember(unsigned int index)  const
	{ 
		ASSERT(index < _team.member_count);
		return _team.member_list[index];
	}

	int GetMemberNum() const 
	{ 
		if(_team_state != TEAM_MEMBER && _team_state != TEAM_LEADER) return 0;
		return _team.member_count;
	}
	bool IsInTeam() const { return _team_state == TEAM_MEMBER || _team_state == TEAM_LEADER; } 
	bool IsLeader() const { return _team_state == TEAM_LEADER; } 
	bool IsAutoComposingTeam() { return _auto_team_info.composing_timeout > 0; }
	const XID & GetLeader() const  {ASSERT(IsInTeam()); return _team.leader;}
	bool IsMember(const XID & member) const
	{
		ASSERT(IsInTeam());
		for(unsigned int i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				return true;
			}
		}
		return false;
	}

	int GetMemberList(XID * list) 
	{ 
		if(_team_state != TEAM_MEMBER && _team_state != TEAM_LEADER) return 0;
		for(unsigned int i = 0; i < _team.member_count; i ++)
		{
			list[i] = _team.member_list[i].id;
		}
		return _team.member_count;
	}

	int GetMember(const XID & member, A3DVECTOR & pos, int & level,int & tag, int & plane_index)
	{
		if(!IsInTeam()) return -1;
		for(int i = 0; i < (int)_team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				pos = _team.member_list[i].pos;
				level = _team.member_list[i].data.level;
				tag = _team.member_list[i].data.world_tag;
				plane_index = _team.member_list[i].data.plane_index;
				return i;
			}
		}
		return -1;
	}
	
	bool GetMemberPos(const XID & member,A3DVECTOR &pos,int & tag, int& plane_index)
	{
		ASSERT(IsInTeam());
		for(unsigned int i = 0; i < _team.member_count; i ++)
		{
			if(_team.member_list[i].id == member)
			{
				pos = _team.member_list[i].pos;
				tag = _team.member_list[i].data.world_tag;
				plane_index = _team.member_list[i].data.plane_index;
				return true;
			}
		}
		return false;
	}
	
	int GetMemberCountInSpecWorld(int world_tag)
	{
		if(!IsInTeam()) return 0;
		int count = 0;
		for(unsigned int i = 0; i < _team.member_count; i ++)
			if(_team.member_list[i].data.world_tag == world_tag)
				++ count;
		return count;
	}
	
	void GetTeamID(int & id, int & seq)
	{
		if(IsInTeam())
		{
			id = _team.leader.id;
			seq = _team.team_seq;
		}
	}

	int GetEffLevel()
	{
		if(IsInTeam())
		{
			return _data.cur_max_level;
		}
		return 0;
	}

	int GetWallowLevel();
	int GetTeamSeq()
	{
		ASSERT(IsInTeam());
		return _team.team_seq;
	}

	int GetTeamID()
	{
		if(IsInTeam()) 
			return _team.leader.id;
		else
			return -1;
	}
	
	int64_t GetTeamUID()
	{
		if(IsInTeam())
			return _team.team_uid;
		return 0;
	}

	void OnHeartbeat()
	{
		_team_ctrl[_team_state]->OnHeartbeat(this);
	}

	
	
//��ҵĲ����ͽ�����Ϣ

	//�ͻ���Ҫ�󷢳�����				�ͻ���
	bool CliInviteOther(const XID & member)
	{
		if(!(_team_state == TEAM_LEADER) && !(_team_state == TEAM_NORMAL))
		{	
			return false;
		}

		if(IsAutoComposingTeam()) return false; //���ڵȴ��Զ����״̬����ң��������������˼���
		
		_team_ctrl[_team_state]->CliInviteOther(this,member);
		return true;
	}

	//�ͻ��˵���ͬ�����				�ͻ���
	bool CliAgreeInvite(const XID & leader,int seq)	
	{
		if(_team_state != TEAM_NORMAL)
		{
			return false;
		}

		_team_ctrl[_team_state]->CliAgreeInvite(this, leader,seq);
		return true;
	}

	//�ͻ��˵���˾ܾ�����				�ͻ���
	void CliRejectInvite(const XID & leader)
	{
		if(_team_state == TEAM_NORMAL)
		{
			_team_ctrl[_team_state]->CliRejectInvite(this,leader);
		}
	}

	//�ͻ��˷������뿪����				�ͻ���
	bool CliLeaveParty()
	{
		if(!(_team_state == TEAM_LEADER) && !(_team_state == TEAM_MEMBER))
		{
			return false;
		}

		return _team_ctrl[_team_state]->CliLeaveParty(this);
	}

	//�ͻ��˷����˽⿪����                          �ͻ���
	bool CliDismissParty()
	{
		if(_team_state != TEAM_LEADER)
		{
			return false;
		}

		return _team_ctrl[_team_state]->CliDismissParty(this);
	}       

	//�ͻ���Ҫ���߳�һ���û�			�ͻ���
	bool CliKickMember(const XID & member)	
	{
		if(_team_state != TEAM_LEADER) return false;
		_team_ctrl[TEAM_LEADER]->CliKickMember(this,member);
		return false;
	}

	//����һ��һ����Ա���ö�Աͬ�� ���룩	��Ϣ
	void MsgAgreeInvite(const XID & member,const A3DVECTOR &pos,const agree_invite_entry& prop,int seq)
	{
		_team_ctrl[_team_state]->MsgAgreeInvite(this,member,pos,prop,seq);
	}

	//�����뷽�����˾ܾ��������Ϣ			��Ϣ
	void MsgRejectInvite(const XID & member)
	{
		_team_ctrl[_team_state]->MsgRejectInvite(this,member);
	}

	//leader������invite��Ϣ			��Ϣ
	bool MsgInvite(const XID & leader,int seq,int pickup_flag)
	{
		if(_team_state == TEAM_NORMAL)
		{
			if(IsAutoComposingTeam()) //���ڵȴ��Զ���ӵ�״̬
			{
				if(leader != _auto_team_info.candicate_leader) return false;
				_data.last_invite_time  = g_timer.get_systime() + 10;
				_team_ctrl[TEAM_NORMAL]->CliAgreeInvite(this, leader, seq);	
				return true;
			}
			else 
			{
				_team_ctrl[TEAM_NORMAL]->MsgInvite(this,leader,seq,pickup_flag);
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	//leader������ͬ��������Ϣ  ��Ϣ
	void MsgJoinTeam(const XID & leader,const member_entry *list,unsigned int count, int pickup_flag,int64_t team_uid,const void * ins_key , unsigned int ik_size)
	{
		_team_ctrl[_team_state]->MsgJoinTeam(this,leader,list,count,pickup_flag,team_uid,ins_key,ik_size);
	}
	
	//��Ա�������޷��������Ϣ			��Ϣ
	void MsgJoinTeamFailed(const XID & member)
	{
		if(_team_state == TEAM_LEADER)
		{
			_team_ctrl[_team_state]->MsgJoinTeamFailed(this,member);
		}
	}

	//��Ա�����˸����������ݵ���Ϣ			��Ϣ
	void MsgMemberUpdateData(const XID & member , const A3DVECTOR &pos, const team_mutable_prop & data)
	{
		if(IsInTeam())
		{
			_team_ctrl[_team_state]->MsgMemberUpdateData(this,member,pos,data);
		}
	}

	//leader�����˸������г�Ա���ݵ���Ϣ		��Ϣ
	void MsgLeaderUpdateData(const XID & leader, const member_entry * list, unsigned int count)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[_team_state]->MsgLeaderUpdateData(this,leader,list,count);
		}
	}

	//leader�յ��˳�ԱҪ���뿪����Ϣ		��Ϣ
	void MsgMemberLeaveRequest(const XID & member)
	{
		if(_team_state == TEAM_LEADER)
		{
			_team_ctrl[TEAM_LEADER]->MsgMemberLeaveRequest(this, member);
		}
	}

	//leader �߳�������Ա				��Ϣ 
	void MsgLeaderKickMember(const XID & leader, const XID & member)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNotifyMemberLeave(this,leader,member,1);
		}
	}

	//leader  ֪ͨ��֪������Ա���뿪��Ϣ
	void MsgNotifyMemberLeave(const XID & leader,const XID & member,int type = 0)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNotifyMemberLeave(this,leader,member,type);
		}
	}


	//leader ֪ͨ��������Ա�������
	void MsgNewMember(const XID & leader, const member_entry * list, unsigned int count)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgNewMember(this,leader,list,count);
		}
	}

	//leader������ȡ����ӵ���Ϣ
	void MsgLeaderCancelParty(const XID & leader,int seq)
	{
		if(_team_state == TEAM_MEMBER)
		{
			_team_ctrl[TEAM_MEMBER]->MsgLeaderCancelParty(this,leader,seq);
		}
	}

	//��ҽ������߲���
	void PlayerLostConnection()
	{
		return _team_ctrl[_team_state]->LostConnection(this);
	}

	void PlayerLogout()
	{
		return _team_ctrl[_team_state]->Logout(this);
	}

	//��������������������Ϣ
	void ApplyParty(const XID & who)
	{
		if(IsAutoComposingTeam()) return; //�����Զ����״̬����������������
		return _team_ctrl[_team_state]->MsgApplyParty(this,who);
	}

	void AgreeApply(int id, bool result)
	{
		return _team_ctrl[_team_state]->CliAgreeApply(this,id,result);
	
	}
	void ApplyPartyReply(const XID & leader,int seq)
	{
		_team_ctrl[_team_state]->MsgApplyPartyReply(this,leader,seq);
	}

	void CliChangeLeader(const XID & new_leader)
	{
		_team_ctrl[_team_state]->CliChangeLeader(this,new_leader);
	}

	void ChangeToLeader(const XID & leader)
	{
		_team_ctrl[_team_state]->MsgChangeToLeader(this,leader);
	}

	void LeaderChanged(const XID & newleader)
	{
		_team_ctrl[_team_state]->MsgLeaderChanged(this,newleader);
	}

	//void DispatchExp(const A3DVECTOR & pos, int exp,int sp,int level);
	void DispatchExp(const A3DVECTOR &pos,int * list ,unsigned int count, int exp,int sp,int level,int total_level, int max_level ,int min_level, int npcid,float r);
	bool PickupTeamMoney(const A3DVECTOR & pos,int amount, bool is_silver)
	{
		if(!IsInTeam() || amount <=0) return false;
		return _team_ctrl[TEAM_LEADER]->PickupTeamMoney(this, pos,amount,is_silver);
	}

	bool IsRandomPickup()
	{
		ASSERT(IsInTeam());
		return _team.pickup_flag == 0;
	}
	
	void SetPickupFlag(int pickup_flag)
	{
		_data.pickup_flag = pickup_flag;
	}

	XID GetLuckyBoy(XID  self, const A3DVECTOR & pos);
	void NotifyTeamPickup(const A3DVECTOR & pos, int type, int count);
	void TeamChat(char channel, char emote_id, const void * buf, unsigned int len, int srcid, const void * aux_data, unsigned int dsize);
	void SendTeamMessage(MSG & msg, float range, bool exclude_self);

	//-------------AUTO TEAM---------------------
	int OnAutoTeamPlayerReady(int leader_id);
	void OnAutoTeamComposeFailed(int leader_id);
	void OnAutoTeamComposeStart(int member_list[], unsigned int cnt);
};

#endif

