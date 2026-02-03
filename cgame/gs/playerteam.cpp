#include "world.h"
#include "playerteam.h"
#include "player_imp.h"
#include <common/protocol.h>
#include <arandomgen.h>
#include "playertemplate.h"
#include "task/taskman.h"
#include <gsp_if.h>
#include "global_drop.h"

class std_team_control : public player_team::team_control
{
	private:
		//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			return ;
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			return ;
		}

		virtual void CliRejectInvite(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		virtual bool CliLeaveParty(player_team * pTeam) 
		{
			return false;
		}

		virtual void MsgJoinTeam(player_team *pTeam, const XID& leader,const player_team::member_entry*,unsigned int ,int pickup_flag,int64_t team_uid,const void * ins_key , unsigned int ik_size)
		{
			//���Ͳ��ܼ���������Ϣ
			gplayer_imp * pImp = GetData(pTeam).imp;
			pImp->SendTo<0>(GM_MSG_JOIN_TEAM_FAILED,leader,0);
		}

		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader)
		{
			return;
		}

		virtual void MsgMemberUpdateData(player_team *,const XID & member, const A3DVECTOR & pos,const team_mutable_prop & data)
		{
			ASSERT(false);
		}

		virtual void MsgLeaderUpdateData(player_team *,const XID & leader,const player_team::member_entry * list,unsigned int count)
		{
			ASSERT(false);
		}

		virtual	void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag)
		{
			ASSERT(false);
		}

		virtual void CliKickMember(player_team *pTeam, const XID & member)
		{
			ASSERT(false);
		}

		virtual void MsgMemberLeaveRequest(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		void MsgNotifyMemberLeave(player_team * pTeam, const XID & leader,const XID & member,int type) 
		{
			ASSERT(false);
		}

		virtual void MsgNewMember(player_team*,const XID& leader,const player_team::member_entry * list, unsigned int count)
		{
			ASSERT(false);
		}
		
		virtual void MsgLeaderCancelParty(player_team *,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		virtual bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount, bool is_silver)
		{
			ASSERT(false);
			return false;
		}
		
		virtual void LostConnection(player_team * pTeam)
		{
			// do nothing
		}
		
		virtual void Logout(player_team * pTeam)
		{
			// do nothing
		}

		virtual void CliAgreeApply(player_team *, int id, bool result)
		{
			//do nothing
		}

		virtual void MsgApplyPartyReply(player_team*, const XID & leader, int seq)
		{
			//do nothing
		}
		
		virtual void CliChangeLeader(player_team *, const XID & new_leader)
		{
			//do nothing 
		}

		virtual void MsgChangeToLeader(player_team*, const XID & leader)
		{
			//do nothing
		}
		
		virtual void MsgLeaderChanged(player_team*, const XID & new_leader)
		{
			//do nothing
		}

};

class team_control_normal : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			//���������ʱ������
			int t = g_timer.get_systime();
			gplayer_imp * pImp = GetData(pTeam).imp;
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			player_team::INVITE_MAP::iterator it = map.end();
			for(;it != map.begin(); )
			{
				--it;
				if(it->second.timestamp < t)
				{
					pImp->_runner->team_invite_timeout(it->second.id.id);
					map.erase(it);
				}
			}

			if(pTeam->_auto_team_info.composing_timeout > 0)
			{
				if(--pTeam->_auto_team_info.composing_timeout == 0) 
				{
					memset(&pTeam->_auto_team_info, 0, sizeof(pTeam->_auto_team_info));
				}
			}
		}

		//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() >= player_team::INVITE_MAP::CAPACITY)
			{
				//������ �������ڲ��������Ҫ��
				pImp->_runner->error_message(S2C::ERR_TEAM_CANNOT_INVITE);
				return ;
			}

			if(map.find(member.id) != map.end())
			{
				//������ �������ڲ��������Ҫ��
				pImp->_runner->error_message(S2C::ERR_TEAM_ALREADY_INVITE);
				return ;
			}

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = member;
			map[member.id] = it;

			//������Ϣ ������Ҽ������
			int pickup_flag = GetData(pTeam).pickup_flag;
			pImp->SendTo<0>(GM_MSG_TEAM_INVITE,member,GetData(pTeam).self_seq,&pickup_flag,sizeof(pickup_flag));
		}

		void MsgApplyPartyReply(player_team* pTeam, const XID & leader, int seq)
		{
			player_team::team_entry & data = GetData(pTeam);
			data.last_invite_time  = g_timer.get_systime() + 10;
			CliAgreeInvite(pTeam,leader,seq);
		}
		
		//ֻ��normal��������������������
		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			player_team::team_entry & data = GetData(pTeam);
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(data.last_invite_time < g_timer.get_systime())
			{
				//���볬ʱ��
				pImp->_runner->error_message(S2C::ERR_TEAM_INVITE_TIMEOUT);
				return;
			}

			//����ȴ�״̬
			data.invite_map.clear();
			ChangeState(pTeam,player_team::TEAM_WAIT);
			data.time_out = TEAM_WAIT_TIMEOUT;
			GetTeam(pTeam).leader = leader;
			GetTeam(pTeam).team_seq = seq;

			player_team::agree_invite_entry prop;
			gplayer * pPlayer = (gplayer*)pImp->_parent;
			prop.data.Init(pImp);
			prop.cs_index = pPlayer->cs_index;
			prop.cs_sid = pPlayer->cs_sid;
			prop.race = pPlayer->base_info.race;

			//������Ϣ��leader
			pImp->SendTo<0>(GM_MSG_TEAM_AGREE_INVITE,leader,seq,&prop,sizeof(prop));

		}

		virtual void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			player_team::team_entry & data = GetData(pTeam);
			if(data.last_invite_time < g_timer.get_systime())
			{
				//���볬ʱ�ˣ�ֱ�ӷ���
				return;
			}

			//���ﲻ����������Ǻ�ʱ�������Ϊͬʱ�����ж������

			//������Ϣ��leader
			gplayer_imp * pImp = data.imp;
			pImp->SendTo<0>(GM_MSG_TEAM_REJECT_INVITE,leader,0);
		}

		//��Ϣ
		void MsgAgreeInvite(player_team * pTeam,const XID & member,const A3DVECTOR & pos,const player_team::agree_invite_entry& prop,int seq)
		{
			//�ܵ�ĳ����ҷ�����ͬ��������Ϣ
			//��������Ƿ���invite�б���
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it || seq != data.self_seq)
			{
				//δ�ҵ����ʵ� ������ʱ
				return;
			}
			data.invite_map.erase(it);

			//��ɶ���, �������ģʽ
			//����Ա���뵽������
			data.min_timer_counter = 0;
			pTeam->BecomeLeader<0>(member,prop,pos);
			data.time_out = TEAM_LEADER_UPDATE_INTERVAL;
			pTeam->_team.team_seq = data.self_seq;
			pTeam->_team.pickup_flag = data.pickup_flag;
			pTeam->_team.team_uid = (int64_t)(world_manager::GetWorldIndex()) << 32 |(int64_t)(world_manager::GetWorldTeamUID());
				
			//������Ϣ���ö�Ա
			gplayer_imp * pImp = data.imp;
			pImp->_runner->join_team(pTeam->_team.leader,data.pickup_flag);
			int count = pTeam->_team.member_count;
			int pickup_flag = pTeam->_team.pickup_flag;
			unsigned int buf_size = sizeof(player_team::member_entry) * count + pImp->_cur_ins_key_list.size()*(sizeof(int)*3) + sizeof(int64_t);
			char * buf = (char*)malloc(buf_size);
			unsigned int buf_uid_header = sizeof(player_team::member_entry) * count;
			memcpy(buf,pTeam->_team.member_list,buf_uid_header);
			memcpy(buf+buf_uid_header,&(pTeam->_team.team_uid),sizeof(int64_t));
			unsigned int buf_header = buf_uid_header + sizeof(int64_t);
			bool bRst = pImp->GetInstanceKeyBuf(buf + buf_header, buf_size - buf_header);
			ASSERT(bRst);
			pImp->SendTo<0>(GM_MSG_JOIN_TEAM,member,(count & 0x7FFF) |(pickup_flag << 16),
					buf,buf_size);
			free(buf);

			//���͸������ݸ��ö�Ա ע����ܻ��������Ϣ֮ǰ��.....
			
			//����������صĵ���
			PlayerTaskInterface  task_if(pImp);
			OnTeamSetup(&task_if);

			//ˢ����ӱ�־�����ڷ���
			pImp->SendTeamVisibleStateToOther(member.id,prop.cs_index,prop.cs_sid);

			pImp->NotifyMasterInfoToPet();
				
			//���Ͷ�����Ϣ��gdelivery,�����˸���ӵĶ�Ա
			std::vector<int> members;
			members.push_back(pImp->_parent->ID.id);
			members.push_back(member.id);
			GMSV::SendPlayerTeamOp(0,pTeam->_team.team_uid,pImp->_parent->ID.id,members);	
		}

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//�ܵ�ĳ����ҷ����˲�ͬ��������Ϣ
			//��������Ƿ���invite�б���
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it)
			{
				//û���ҵ�,ֱ�ӷ���
				return;
			}
			data.invite_map.erase(it);

			//���;ܾ���Ϣ
			data.imp->_runner->player_reject_invite(member);
		}

		void MsgInvite(player_team *pTeam, const XID & leader,int seq,int pickup_flag)
		{
			//�յ���һ���˷�����������Ϣ
			player_team::team_entry & data = GetData(pTeam);

			data.last_invite_time = g_timer.get_systime() + TEAM_INVITE_TIMEOUT2;
			data.imp->_runner->leader_invite(leader,seq,pickup_flag);
		}
		
		void MsgApplyParty(player_team*, const XID & who)
		{
			//do nothing
		}
};

class team_control_wait : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			if(--(GetData(pTeam).time_out) <= 0)
			{
				//��ʱ��,�ص�normal״̬
				ChangeState(pTeam,player_team::TEAM_NORMAL);
				pTeam->_data.imp->_runner->error_message(S2C::ERR_TEAM_JOIN_FAILED);
			}
		}

		//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			ASSERT(false);
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & member)
		{
			ASSERT(false);
		}

		//��Ϣ

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//������,�Լ���Ҫ�����Ķ�����
			return ;
		}


		void MsgJoinTeam(player_team *pTeam, const XID & leader, const player_team::member_entry * list,unsigned int count, int pickup_flag,int64_t team_uid,const void * ins_key , unsigned int ik_size)
		{
			player_team::team_t & team =  GetTeam(pTeam);
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(!(team.leader == leader))
			{
				//��ƥ��Ķӳ�
				//���ش�����Ϣ
				pImp->SendTo<0>(GM_MSG_JOIN_TEAM_FAILED,leader,0);
				return ;
			}

			//�����������Ĳ���,���ڲ���֯�����б�,��Ϊ������û������
			team.member_count = 0;
			team.pickup_flag = pickup_flag;
			team.team_uid = team_uid;
			GetData(pTeam).min_timer_counter = 0;
			
			//�����Ա״̬ ,��������ṹ
			pTeam->BecomeMember(leader,list,count);
			pTeam->_data.time_out = TEAM_LEADER_TIMEOUT;
			__PRINTF("enter team: timeout%d\n",pTeam->_data.time_out);

			//�����Ͷӳ�һ����instance key
			pImp->SetInstanceKeyBuf(ins_key,ik_size);

			//֪ͨplayer�������״̬
			pImp->_runner->join_team(leader,pickup_flag);

			//�������е���ӿɼ����ݸ�����
			pImp->SendTeamDataToMembers();

			pImp->NotifyMasterInfoToPet();

		}

		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR &,const player_team::agree_invite_entry & prop,int seq)
		{
			//ʲô�����������Ե�
			//��Ϊֻ��leader����
		}

		void MsgApplyParty(player_team*, const XID & who)
		{
			//do nothing
		}
};

class team_control_member : public  std_team_control
{
	private:
		virtual void OnHeartbeat(player_team * pTeam) 
		{
			gplayer_imp * pImp = GetData(pTeam).imp;
			//������ leader��ʱ�䲻����notify�Ļ�,���г�ʱ����
			//�����ʱ���Կ�����30������
			if((--GetData(pTeam).time_out) <= 0)
			{
				__PRINTF("��Ա���ֶӳ���ʱ���Զ��뿪����\n");
				//��ʱ�ˣ�ִ���뿪�����߼�
				MsgNotifyMemberLeave(pTeam, pTeam->_team.leader,pImp->_parent->ID,0);
				return ;
			}
			if((GetData(pTeam).min_timer_counter++) > 30)
			{
				//ÿ30�������С����
				pTeam->CalcExactMaxLevel();
				GetData(pTeam).min_timer_counter = 0;

			}
		
			//���Ͷ������ݸ��ͻ���
			pTeam->SendTeamDataToClient<0>();

			//������Ҫ�Ƿ�Ҫ��������
			team_mutable_prop data(pImp);
			if(pImp->GetRefreshState())
			{
				pTeam->SendSelfDataToMember<0>(data);
			}
			else if(pTeam->_data.time_out & 0x01)	//ÿ���뷢��һ������
			{
				player_team::team_t & team =  GetTeam(pTeam);
				pImp->SendTo<0>(GM_MSG_MEMBER_NOTIFY_DATA,team.leader,0,&data,sizeof(data));
			}
		}


		//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			ASSERT(false);
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			ASSERT(false);
		}

		bool CliLeaveParty(player_team * pTeam)
		{
			gplayer_imp * pImp = GetData(pTeam).imp;
			player_team::team_t & team =  GetTeam(pTeam);
			//�����뿪������Ϣ �ӳ�ͬ����뿪����
			pImp->SendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,team.leader,0);
			return true;
		}

		//��Ϣ
		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR &,const player_team::agree_invite_entry & prop,int seq)
		{
			//��״̬�������˲���,������ʱ
			return;
		}

		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//������
			return ;
		}

		//��Ա�����Լ�������
		void MsgMemberUpdateData(player_team * pTeam,const XID & member, const A3DVECTOR &pos,const team_mutable_prop & data)
		{
			pTeam->UpdateMemberData(member,pos,data);
		}

		//�ӳ����´�ҵ�����
		void MsgLeaderUpdateData(player_team *pTeam,const XID & leader,const player_team::member_entry * list,unsigned int count)
		{
			pTeam->LeaderUpdateMembers(leader,list,count);
			//���¶ӳ���ʱ���
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;

			GetData(pTeam).imp->NotifyMasterInfoToPet();
		}


		//�յ�leader���������˼�����Ϣ
		void MsgNewMember(player_team* pTeam,const XID& leader,const player_team::member_entry * list, unsigned int count)
		{
			ASSERT(count >= 2);
			if(leader != pTeam->_team.leader)  return ;
			pTeam->LeaderUpdateMembers(leader,list,count);
			const player_team::member_entry & ent = list[count-1];
			pTeam->_data.imp->_runner->new_member(ent.id);
			//���¶ӳ���ʱ���
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;
			pTeam->_data.imp->SendTeamVisibleStateToOther(ent.id.id,ent.cs_index,ent.cs_sid);

			//�����ǰ��������״̬��֪ͨ�¼ӵĶ����Լ�appear
			gplayer* player = (gplayer*)pTeam->_data.imp->_parent;
			if(player->IsInvisible()) pTeam->_data.imp->SendAppearToTeamMember(ent.id.id,ent.cs_index,ent.cs_sid);
			
			return ;
		}

		void MsgNotifyMemberLeave(player_team * pTeam, const XID & leader,const XID & member,int type) 
		{
			//�յ�leader�������߳�ĳmember����Ϣ
			if(leader != pTeam->_team.leader)  return ;
			gplayer_imp * pImp = pTeam->_data.imp;
			if(member == pImp->_parent->ID)
			{
				//���Լ�,ֱ���뿪����
				pTeam->BecomeNormal<0>();
				pImp->_runner->leave_party(leader,type);

				//����Ҫ�ؽ����еĸ���key
				//��Ϊ������key��Ȼ����

				//��������ϵͳ�Ľӿ�
				PlayerTaskInterface task_if(pImp);
				OnTeamMemberLeave(&task_if);
				return ;
			}
			
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				//ɾ���ö�Ա
				pTeam->MemberRemoveMember(pEntry);
				pImp->_runner->member_leave(leader,member,type);
			}
			pImp->NotifyMasterInfoToPet();
		}

		//�յ�leader��������ȡ����ӵ���Ϣ
		virtual void MsgLeaderCancelParty(player_team *pTeam,const XID & leader,int seq)
		{
			if(leader != pTeam->_team.leader || seq != pTeam->_team.team_seq)  return;
			
			pTeam->BecomeNormal<0>();
			ChangeState(pTeam,player_team::TEAM_NORMAL);
			//	pTeam->_data.imp->_runner->leader_cancel_party(leader);
			pTeam->_data.imp->_runner->leave_party(leader,2);

			//��������ϵͳ�Ľӿ�
			PlayerTaskInterface task_if(pTeam->_data.imp);
			OnTeamMemberLeave(&task_if);

		}

		virtual void LostConnection(player_team * pTeam)
		{
			CliLeaveParty(pTeam);
		}

		virtual void Logout(player_team * pTeam)
		{
			LostConnection(pTeam);
		}

		void MsgApplyParty(player_team * pTeam, const XID & who)
		{
			//ת��������뵽�ӳ�����
			const XID & leader = pTeam->GetLeader();
			MSG msg;
			BuildMessage(msg, GM_MSG_TEAM_APPLY_PARTY, leader, who,A3DVECTOR(0,0,0)); 
			SendMessage<0>(pTeam->_data.imp->_plane,msg);

		}

		void MsgChangeToLeader(player_team* pTeam, const XID & leader)
		{
			if(leader != GetTeam(pTeam).leader)  return ;
			//��Ϣ�ķ����ߵ�ȷ��leader
			//����״̬���߼���ת�������Լ����leader��������ת����Ϣ�����еĶ��� 
			//���ҵ���player�Ķӳ�ת���߼�
			if(pTeam->FromMemberToLeader<0>(leader))
			{
				MSG msg;
				BuildMessage(msg, GM_MSG_TEAM_LEADER_CHANGED, XID(-1,-1),GetData(pTeam).imp->_parent->ID,A3DVECTOR(0,0,0));
				//���Ǹ������еĶ�Ա�б�
				pTeam->SendGroupMessage(msg);
				
				GetData(pTeam).time_out = 1;	//һ���Ӻ󼴽������е����ݸ���
				//���ͻ��˷�����Ϣ
				gplayer_imp * pImp = pTeam->_data.imp;
				pImp->_runner->change_team_leader(leader,pImp->_parent->ID);

				PlayerTaskInterface task_if(pImp);
				OnTeamTransferCaptain(&task_if);
				//֪ͨgdeliveryd�ӳ�����
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,2,pImp->_parent->ID.id);
			}
		}

		void MsgLeaderChanged(player_team* pTeam, const XID & new_leader)
		{
			if(!pTeam->IsMember(new_leader))
			{
				//���˲��ڶ�����
				return ;
			}
			
			//��ʱ��㲥һ����Ϣ֪ͨ��Χ����Ʒ�޸������Ȩ��
			//���߿������30������ɱ���Ĺ���Ƿ�õ�����Ӿ��飩�� ������ٽ�����Ʒ������Ϣ�㲥
			//��������Ʒ������Ȩ��������team/seq��Ϊ�ж���׼
			//���ڲ�������������ˣ�����Ҳ������Ҫ����
			
			XID old_leader = pTeam->_team.leader;
			pTeam->ChangeLeader<0>(new_leader);
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;

			//���ͻ��˷�����Ϣ
			gplayer_imp * pImp = pTeam->_data.imp;
			pImp->_runner->change_team_leader(old_leader,new_leader);
		}
		
		bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount, bool is_silver)
		{
			//��Ȼ���Ƕӳ���������Ȼ��Ҫ���з���
			if(amount <=0) return false;
			unsigned int count = pTeam->_team.member_count;
			if(count <= 1) return false;
			unsigned int acount = (unsigned int)((float)amount / (float)count);
			if(acount > MONEY_CAPACITY_BASE) acount = MONEY_CAPACITY_BASE;

			//��ʼ�ַ���Ǯ
			amount -= acount * count;
			int ex_id = -1;
			if(amount > 0)
			{
				ex_id = abase::Rand(0,count -1);
			}
			MSG msg;
			gplayer_imp *pImp = pTeam->_data.imp;
			BuildMessage(msg, is_silver ? GM_MSG_RECEIVE_MONEY2 : GM_MSG_RECEIVE_MONEY ,XID(-1,-1),pImp->_parent->ID,pos,acount);
			if(acount >0 )
			{
				pTeam->SendAllMessage(msg,ex_id);
			}

			if(amount > 0)
			{
				//������޷������Ǯ��ĳ����Ա
				msg.param += amount;
				pTeam->SendMemberMessage(ex_id,msg);
			}

			return true;
		}

};

class team_control_leader: public std_team_control
{
	private:

		virtual void OnHeartbeat(player_team * pTeam) 
		{
			int ts = g_timer.get_systime();
			gplayer_imp * pImp = GetData(pTeam).imp;
			XID self = pImp->_parent->ID;

			//���������ʱ������
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			player_team::INVITE_MAP::iterator it = map.end();
			for(;it != map.begin(); )
			{
				--it;
				if(it->second.timestamp < ts)
				{
					pImp->_runner->team_invite_timeout(it->second.id.id);
					map.erase(it);
				}
			}

			//���������ʱ������
			player_team::INVITE_MAP & apply_map = GetData(pTeam).apply_map;
			it = apply_map.end();
			for(;it != apply_map.begin(); )
			{
				--it;
				if(it->second.timestamp < ts)
				{
					apply_map.erase(it);
				}
			}
			
			//�����ʱ�ĸı�ӳ�����
			if(GetData(pTeam).change_leader_timeout)
			{
				if( (-- GetData(pTeam).change_leader_timeout) <= 0)
				{
					pImp->RebuildInstanceKey();
					__leave_party(pTeam);
					return;
				}
			}

			//������ member ��ʱ�䲻����notify�Ļ�,���г�ʱ����
			//�����ʱ���Կ�����10������(���߸��ӳ�)
			if(GetData(pTeam).time_out & 0x01)
			{
				MSG msg;
				MSG * pMsg = NULL;
				for(unsigned int i = 0 ; i < pTeam->_team.member_count; )
				{
					player_team::member_entry & member = pTeam->_team.member_list[i];
					if((member.timeout -= 2 ) <= 0 && member.id != self)
					{	
						//��ʱ,ɾ��֮
						if(!pMsg)
						{
							BuildMessage(msg,GM_MSG_MEMBER_LEAVE,XID(-1,-1),
									self,pImp->_parent->pos,member.id.id);
							pMsg = &msg;
						}
						SendGroupMessage(pTeam,*pMsg);
						pImp->_runner->member_leave(self,member.id,0);
						__PRINTF("�ӳ����ֶ�Ա%d��ʱ,���¶����б�\n",member.id.id);
						
						int member_id = member.id.id;
						//ȥ����Ա		
						pTeam->LeaderRemoveMember(&member);
						if(pTeam->_team.member_count > 1)
						{
							//��������������һ������������__leave_party�߼�
							//֪ͨ����ӿ�
							PlayerTaskInterface task_if(pImp);
							OnTeamDismissMember(&task_if,member_id);
						}


					}
					else
					{
						i ++;
					}
				}
			}

			//���ֻʣ���Լ������˳�����
			if(pTeam->_team.member_count <= 1)
			{
				__leave_party(pTeam);
				return;
			}

			//���ŷ��Ͷ������ݸ��ͻ���
			pTeam->SendTeamDataToClient<0>();

			//ÿ30�������С����
			if((GetData(pTeam).min_timer_counter++) > 30)
			{
				pTeam->CalcExactMaxLevel();
				GetData(pTeam).min_timer_counter = 0;

			}

			//�����Լ���λ�� 
			pTeam->_team.member_list[0].pos = pImp->_parent->pos;

			//�Ƿ�Ҫ��������
			if((--GetData(pTeam).time_out) <= 0)
			{
				//��������member���ݵ������������
				GetData(pTeam).time_out = TEAM_LEADER_UPDATE_INTERVAL;

				//���Ͷ�����Ϣ�����ж�Ա
				int count = pTeam->_team.member_count;
				MSG msg;
				BuildMessage(msg,GM_MSG_LEADER_UPDATE_MEMBER,XID(-1,-1),
						self,pImp->_parent->pos,
						count,0,pTeam->_team.member_list,
						sizeof(player_team::member_entry)*count);
				SendGroupMessage(pTeam,msg);
				pTeam->CalcMinLevel();

				//����ȫ�����±�־��ʹ���Լ�Ҳ�����յ���������
				pTeam->_team.change_flag = player_team::team_t::TEAM_ALL_CHANGED;
			}
			else
			if(pImp->GetRefreshState())
			{
				team_mutable_prop data(pImp);
				//�����Լ���Ѫֵ�Ȳ���
				pTeam->_team.member_list[0].pos = pImp->_parent->pos;
				pTeam->_team.member_list[0].data = data;
				pTeam->SendSelfDataToMember<0>(data);
			}
		}

		//�ͻ��˲���
		virtual void CliInviteOther(player_team * pTeam,const XID & member)
		{
			player_team::INVITE_MAP & map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() + GetTeam(pTeam).member_count >= TEAM_MEMBER_CAPACITY)
			{
				//������ �������ڲ��������Ҫ��
				pImp->_runner->error_message(S2C::ERR_TEAM_CANNOT_INVITE);
				return ;
			}

			if(map.find(member.id) != map.end())
			{
				//���͹�������
				pImp->_runner->error_message(S2C::ERR_TEAM_ALREADY_INVITE);
				return ;
			}

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = member;
			map[member.id] = it;

			//������Ϣ ������Ҽ������
			int pickup_flag = GetTeam(pTeam).pickup_flag;	//�ӵ�ǰ�Ķ�����ȡ��
			pImp->SendTo<0>(GM_MSG_TEAM_INVITE,member,GetTeam(pTeam).team_seq,&pickup_flag, sizeof(pickup_flag));
		}

		virtual void CliAgreeInvite(player_team * pTeam,const XID & leader,int seq)
		{
			ASSERT(false);
		}

		void CliRejectInvite(player_team * pTeam, const XID & leader)
		{
			ASSERT(false);
		}


		inline 	void __leave_party(player_team * pTeam)
		{
			//֪ͨ����ӿ� ,���������������ɢ֮ǰ��� ���������ǲ��Ǻ���?
			PlayerTaskInterface task_if(pTeam->_data.imp);
			OnTeamDisband(&task_if);
			
			if(GetData(pTeam).change_leader_timeout > 0)
			{
				//������ڵȴ����Ķӳ�����ȴ�ڣ����ؽ�����key
				pTeam->_data.imp->RebuildInstanceKey();
			}

			pTeam->_data.imp->_runner->leave_party(pTeam->_team.leader,0);
			//֪ͨgdelivery�����ɢ
			std::vector<int> tmp;
			GMSV::SendPlayerTeamOp(1,GetTeam(pTeam).team_uid,GetTeam(pTeam).leader.id,tmp);
			//ת������״̬
			GetData(pTeam).self_seq ++;
			pTeam->BecomeNormal<0>();
			
		}

		bool CliDismissParty(player_team * pTeam)
		{
			//�����ͻ��˷������뿪���������
			//������Ҫ���ǽ�ɢ����
			gplayer_imp * pImp = GetData(pTeam).imp;
			//���������ɢ��Ϣ
			MSG msg;
			BuildMessage(msg,GM_MSG_LEADER_CANCEL_PARTY,XID(-1,-1),
					pImp->_parent->ID,pImp->_parent->pos,
					pTeam->_team.team_seq);
			SendGroupMessage(pTeam,msg);
			__leave_party(pTeam);
			return true;
		}

		bool CliLeaveParty(player_team * pTeam)
		{
			if(pTeam->_team.member_count > 2)
			{
				XID new_leader = pTeam->_team.member_list[1].id;
				CliChangeLeader(pTeam, new_leader);
				pTeam->_data.imp->LazySendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,new_leader,0,17);

				//��������key
				pTeam->_data.imp->RebuildInstanceKey();
			}
			else
			{
				CliDismissParty(pTeam);
			}
			return true;
		}

		void CliKickMember(player_team *pTeam, const XID & member)
		{
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				//���ö�Ա�����߳�����Ϣ���뿪�ķ�ʽҲ����
				MSG msg;
				gplayer_imp *pImp = pTeam->_data.imp;
				XID self = pImp->_parent->ID;
				BuildMessage(msg,GM_MSG_LEADER_KICK_MEMBER,XID(-1,-1),
						self,pImp->_parent->pos,member.id);
				SendGroupMessage(pTeam,msg);

				//ɾ���ö�Ա
				pTeam->LeaderRemoveMember(pEntry);
				pImp->_runner->member_leave(self,member,1);

				//����ʣ���˼�����Ա 
				//�����һ������ô��ɢ����
				if(pTeam->_team.member_count <= 1)
				{
					//�ߵ�
					__leave_party(pTeam);
					return ;
				}

				//֪ͨ����ӿ�
				PlayerTaskInterface task_if(pImp);
				OnTeamDismissMember(&task_if,member.id);
				
				//֪ͨgdelivery��Ա���
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,1,member.id);
			}
		}

//��Ϣ
		void MsgAgreeInvite(player_team * pTeam, const XID & member,const A3DVECTOR & member_pos,const player_team::agree_invite_entry & prop,int seq)
		{
			//�ܵ�ĳ����ҷ�����ͬ��������Ϣ
			//��������Ƿ���invite�б���
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if( pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY || data.invite_map.end() == it || seq != GetTeam(pTeam).team_seq)
			{
				//δ�ҵ����ʵ� ������ʱ
				return;
			}
			data.invite_map.erase(it);

			//����Ա���뵽������
			pTeam->LeaderAddMember<0>(member,prop,member_pos);


			player_team::member_entry * list = pTeam->_team.member_list;
			int count = pTeam->_team.member_count;
			//���Ͷ�����Ϣ�����ж�Ա
			gplayer_imp * pImp = data.imp;
			const A3DVECTOR & pos = pImp->_parent->pos;
			XID self= pImp->_parent->ID;
			MSG msg;
			int pickup_flag = pTeam->_team.pickup_flag;

			unsigned int buf_size = sizeof(player_team::member_entry) * count + pImp->_cur_ins_key_list.size()*(sizeof(int)*3) + sizeof(int64_t);
			char * buf = (char*)malloc(buf_size);
			unsigned int buf_uid_header = sizeof(player_team::member_entry) * count;
			memcpy(buf,list,buf_uid_header);
			memcpy(buf+buf_uid_header,&(pTeam->_team.team_uid),sizeof(int64_t));
			unsigned int buf_header = buf_uid_header + sizeof(int64_t);
			bool bRst = pImp->GetInstanceKeyBuf(buf + buf_header, buf_size - buf_header);
			ASSERT(bRst);
			BuildMessage(msg,GM_MSG_JOIN_TEAM,member,self,pos,(count & 0x7FFF) |(pickup_flag << 16),0,
					buf,buf_size);
			SendMessage<0>(pImp->_plane,msg);
			free(buf);

			msg.content_length = buf_uid_header;
			msg.content = list;

			//���һ����Ա�϶����¶�Ա
			ASSERT(list[count-1].id == member);
			//������������Ա���������˽������Ϣ 
			msg.param = count;
			msg.message = GM_MSG_NEW_MEMBER;
			for(int i = 0; i < count - 1 ; i ++)
			{
				msg.target = list[i].id;
				SendMessage<0>(pImp->_plane,msg);
			}

			//�������ø���ʱ�䣨��Ϊ�Ѿ��������˷����˸�����Ϣ��
			data.time_out = TEAM_LEADER_UPDATE_INTERVAL;
			pTeam->_data.imp->_runner->new_member(list[count-1].id);

			pImp->SendTeamVisibleStateToOther(member.id,prop.cs_index,prop.cs_sid);

			//֪ͨ����ϵͳ
			PlayerTaskInterface task_if(pImp);
			Task_OnTeamAddMember(&task_if,count - 1);
			
			//֪ͨgdeliveryd
			GMSV::SendPlayerTeamMemberOp(pTeam->_team.team_uid,0,member.id);	
		}


		virtual void MsgJoinTeamFailed(player_team *pTeam, const XID & leader)
		{
			//��Ա�������ʧ��
			//���ö�Աɾ������֪ͨ������Ա
			return;
		}
		
		void MsgRejectInvite(player_team *pTeam,const XID & member)
		{
			//�ܵ�ĳ����ҷ����˲�ͬ��������Ϣ
			//��������Ƿ���invite�б���
			player_team::team_entry & data = GetData(pTeam);
			player_team::INVITE_MAP::iterator it = data.invite_map.find(member.id);
			if(data.invite_map.end() == it)
			{
				//û���ҵ�,ֱ�ӷ���
				return;
			}
			data.invite_map.erase(it);

			//���;ܾ���Ϣ
			data.imp->_runner->player_reject_invite(member);
		}

		void SendGroupMessage(player_team *pTeam,MSG & msg)
		{
			player_team::member_entry * list = pTeam->_team.member_list;
			int count = pTeam->_team.member_count;
			world *pPlane = pTeam->_data.imp->_plane;
			for(int i = 0; i < count; i++)
			{
				if(list[i].id == msg.source) continue;

				msg.target = list[i].id;
				pPlane->PostLazyMessage(msg);
			}
		}


		void MsgMemberLeaveRequest(player_team * pTeam, const XID & member)
		{
			player_team::member_entry * pEntry = pTeam->FindMember(member);
			if(pEntry)
			{
				gplayer_imp *pImp = pTeam->_data.imp;
				//֪ͨ���ж�Ա�ö�Ա�뿪���뿪��Ҳʹ�������Ϣ���뿪����
				MSG msg;
				BuildMessage(msg,GM_MSG_MEMBER_LEAVE,XID(-1,-1),
						pImp->_parent->ID,pImp->_parent->pos,member.id);
				SendGroupMessage(pTeam,msg);

				pImp->_runner->member_leave(pImp->_parent->ID,member,0);

				// ɾ���ö�Ա
				pTeam->LeaderRemoveMember(pEntry);

				//����ʣ���˼�����Ա 
				//�����һ������ô��ɢ����
				if(pTeam->_team.member_count <= 1)
				{
					__leave_party(pTeam);
					return ;
				}

				//֪ͨ����ӿ�
				PlayerTaskInterface task_if(pImp);
				OnTeamDismissMember(&task_if,member.id);
				//֪ͨgdeliveryd
				GMSV::SendPlayerTeamMemberOp(GetTeam(pTeam).team_uid,1,member.id);
			}
		}

		void MsgMemberUpdateData(player_team *pTeam, const XID & member,const A3DVECTOR & pos, const team_mutable_prop & prop)
		{
			//��Ա����������
			player_team::member_entry * pEntry = pTeam->UpdateMemberData(member,pos,prop);
			if(pEntry)
			{
				//���¶�Ա��ʱ���
				pEntry->timeout = TEAM_MEMBER_TIMEOUT;
			}
			else
			{
				gplayer_imp * pImp = pTeam->_data.imp;
				//����һ��������Ϣ
				pImp->SendTo<0>(GM_MSG_MEMBER_NOT_IN_TEAM,member,0);
			}
		}

		void MsgLeaderUpdateData(player_team *, const XID & leader, const player_team::member_entry *, unsigned int count)
		{
			ASSERT(false);
			return ;
		}

		bool PickupTeamMoney(player_team * pTeam, const A3DVECTOR & pos, int amount, bool is_silver)
		{
			if(amount <=0) return false;
			unsigned int count = pTeam->_team.member_count;
			if(count <= 1) return false;
			unsigned int acount = (unsigned int)((float)amount / (float)count);
			if(acount > MONEY_CAPACITY_BASE) acount = MONEY_CAPACITY_BASE;

			//��ʼ�ַ���Ǯ
			amount -= acount * count;
			int ex_id = -1;
			if(amount > 0)
			{
				ex_id = abase::Rand(0,count -1);
			}
			MSG msg;
			gplayer_imp *pImp = pTeam->_data.imp;
			BuildMessage(msg, is_silver ? GM_MSG_RECEIVE_MONEY2 : GM_MSG_RECEIVE_MONEY ,XID(-1,-1),pImp->_parent->ID,pos,acount);
			if(acount >0 )
			{
				pTeam->SendAllMessage(msg,ex_id);
			}

			if(amount > 0)
			{
				//������޷������Ǯ��ĳ����Ա
				msg.param += amount;
				pTeam->SendMemberMessage(ex_id,msg);
			}

			return true;
		}

		void LostConnection(player_team * pTeam)
		{
			//������,��Ҫ���ӳ������ڶ�λ
			if(pTeam->_team.member_count >= 2)
			{
				XID new_leader = pTeam->_team.member_list[1].id;
				CliChangeLeader(pTeam, new_leader);
				pTeam->_data.imp->LazySendTo<0>(GM_MSG_LEAVE_PARTY_REQUEST,new_leader,0,37);
			}
			//��������key
			pTeam->_data.imp->RebuildInstanceKey();
		}

		void Logout(player_team * pTeam)
		{
			//�ǳ���������ҵ�instance_key,�϶������Լ���
			CliLeaveParty(pTeam);
		}
		
		void MsgApplyParty(player_team* pTeam, const XID & who)
		{
			if(pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY) 
			{
				//�����Ƿ񷢳��޷����������
				return;
			}

			//�Լ���������
			player_team::INVITE_MAP & map = GetData(pTeam).apply_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.size() >= player_team::INVITE_MAP::CAPACITY) return;

			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = who;
			map[who.id] = it;

			pImp->_runner->send_party_apply(who.id);
		}

		void CliAgreeApply(player_team *pTeam, int id, bool result)
		{
			//�Լ���������
			player_team::INVITE_MAP & map = GetData(pTeam).apply_map;
			player_team::INVITE_MAP & invite_map = GetData(pTeam).invite_map;
			gplayer_imp * pImp = GetData(pTeam).imp;
			if(map.find(id) == map.end())
			{
				//δ�ҵ������� 
				return;
			}

			if(!result) 
			{
				//���;ܾ����ݸ�������
				pImp->SendTo<0>(GM_MSG_ERROR_MESSAGE,XID(GM_TYPE_PLAYER,id),S2C::ERR_TEAM_REFUSE_APPLY);
				map.erase(id);
				return;
			}

			if(pTeam->_team.member_count >= TEAM_MEMBER_CAPACITY 
					|| invite_map.size() >= player_team::INVITE_MAP::CAPACITY)
			{
				//�����Ƿ񷢳��޷����������
				return;
			}

			//����ͬ���������Ϣ�����
			//�������б������һ��
			//������������
			player_team::invite_t it;
			it.timestamp = g_timer.get_systime() + TEAM_INVITE_TIMEOUT;
			it.id = XID(GM_TYPE_PLAYER,id);
			invite_map[id] = it;
			pImp->SendTo<0>(GM_MSG_TEAM_APPLY_REPLY,XID(GM_TYPE_PLAYER,id),pTeam->_team.team_seq);
		}

		virtual void CliChangeLeader(player_team * pTeam, const XID & new_leader)
		{
			//����Ŀ���Ƿ��Ա
			if(GetData(pTeam).change_leader_timeout) return;
			if(new_leader == pTeam->_team.leader) return;
			if(GetData(pTeam).change_leader_timeout) return;
			if(pTeam->IsMember(new_leader))
			{
				gplayer_imp * pImp = GetData(pTeam).imp;
				pImp->SendTo<0>(GM_MSG_TEAM_CHANGE_TO_LEADER,new_leader,0);
				pTeam->_data.change_leader_timeout = 25;
			}
		}

		void MsgLeaderChanged(player_team* pTeam, const XID & new_leader)
		{
			if(!pTeam->IsMember(new_leader))
			{
				//���˲��ڶ�����
				return ;
			}
			GetData(pTeam).change_leader_timeout = 0;
			pTeam->FromLeaderToMember<0>(new_leader);
			GetData(pTeam).time_out = TEAM_LEADER_TIMEOUT;
			//��������ԭʼkey ���⸱������
			pTeam->_data.imp->RebuildInstanceKey();


			//���ͻ��˷�����Ϣ
			gplayer_imp * pImp = pTeam->_data.imp;
			pImp->_runner->change_team_leader(pImp->_parent->ID,new_leader);
		}


};

player_team::team_control * player_team::_team_ctrl[player_team::TEAM_STATE_NUM] =
{
	new team_control_normal,
	new team_control_wait,
	new team_control_member,
	new team_control_leader
};

void
player_team::Init(gplayer_imp * pPlayer)
{
	_data.imp = pPlayer;
	_data.self_seq = g_timer.get_systime();
}



XID 
player_team::GetLuckyBoy(XID  self, const A3DVECTOR & pos)
{
	ASSERT(IsInTeam() && IsRandomPickup());
	int who[TEAM_MEMBER_CAPACITY];	
	int cand_count = 0;
	ASSERT(IsInTeam());
	unsigned int count = _team.member_count;
	world::object_info info;
	world *pPlane = _data.imp->_plane;
	int tag = world_manager::GetWorldTag();
	for(unsigned int i = 0; i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		if(self != member && 
				(_team.member_list[i].data.world_tag != tag || 
				!pPlane->QueryObject(member,info) || 
				 info.pos.squared_distance(pos) >= TEAM_ITEM_DISTANCE*TEAM_ITEM_DISTANCE))
		{
			continue;
		}

		if(_team.member_list[i].data.hp <= 0) continue;

		who[cand_count] = i;
		cand_count ++;
	}
	if(cand_count <=1) return self;
	int max = 0;
	int index = 0;
	for(int i = 0; i < cand_count; i ++)
	{
		int val = abase::Rand(0,100);
		if(val >= max)
		{
			max = val;
			index = i;
		}
	}
	//int index = abase::Rand(0,cand_count - 1);
	return _team.member_list[who[index]].id;
}

void 
player_team::NotifyTeamPickup(const A3DVECTOR & pos, int type, int count)
{	
	//������ô��֪ͨ��....������������еĶ��ѵ�cs_index��sid�Ļ�������ֱ�ӷ������ݣ�Ҳδ������
	//����Ч�ʿ��ܱȽϸ�   �����������Ļ�Ҳ���Կ������ַ�ʽ
	//����������Ϣϵͳ�����У����Ҫ����cs_index��sid������Ҫ���ϴ�䶯

	MSG msg;
	BuildMessage(msg,GM_MSG_TEAM_PICKUP,XID(-1,-1),_data.imp->_parent->ID,pos,type,0,&count,sizeof(count));
	SendGroupMessage(msg);
}

void 
player_team::TeamChat(char channel, char emote_id, const void * buf, unsigned int len,int srcid, const void * aux_data, unsigned int dsize)
{	
	GMSV::chat_msg msg;
	int self = srcid;
	msg.speaker = self;
	msg.msg = buf;
	msg.size = len;
	msg.data = aux_data;
	msg.dsize = dsize;
	msg.channel = channel;
	msg.emote_id = emote_id;
	msg.speaker_level = 0;
	unsigned int count = _team.member_count;
	for(unsigned int i = 0;i < count; i ++)
	{
		const member_entry & member = _team.member_list[i];
		GMSV::SendChatMsg(member.cs_index, member.id.id, member.cs_sid,msg);
	}
}

void 
player_team::SendGroupMessage(const MSG & msg)
{
	unsigned int count = _team.member_count;
	unsigned int index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(unsigned int i = 0;i < count; i ++)
	{
		if(_team.member_list[i].id == msg.source) continue;
		list[index++] = _team.member_list[i].id;
	}
	if(index) _data.imp->_plane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg, int ex_id)
{
	unsigned int count = _team.member_count;
	unsigned int index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(unsigned int i = 0;i < count; i ++)
	{
		if(i == (unsigned int) ex_id) continue;
		list[index++] = _team.member_list[i].id;
	}
	if(index) _data.imp->_plane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg)
{
	unsigned int count = _team.member_count;
	XID  list[TEAM_MEMBER_CAPACITY];
	for(unsigned int i = 0;i < count; i ++)
	{
		list[i] = _team.member_list[i].id;
	}
	_data.imp->_plane->SendMessage(list,list + count, msg);
}

void 
player_team::SendMemberMessage(int idx, MSG & msg)
{
	ASSERT((unsigned int)idx <_team.member_count);
	msg.target = _team.member_list[idx].id;
	_data.imp->_plane->PostLazyMessage(msg);
}
	
void 
player_team::SendMessage(const MSG & msg, float range)
{
	unsigned int count = _team.member_count;
	unsigned int index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	ASSERT(msg.source == _data.imp->_parent->ID);
	range *= range;
	world *pPlane = _data.imp->_plane;
	for(unsigned int i = 0;i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		world::object_info info;
		if(msg.source == member  || 
				!pPlane->QueryObject(member,info) || 
				info.pos.squared_distance(msg.pos) >= range)
		{
			continue;
		}
		list[index++] = member;
	}
	if(index) pPlane->SendMessage(list,list + index, msg);
}

void 
player_team::SendAllMessage(const MSG & msg, float range)
{
	unsigned int count = _team.member_count;
	unsigned int index = 0;
	XID  list[TEAM_MEMBER_CAPACITY];
	ASSERT(msg.source == _data.imp->_parent->ID);
	range *= range;
	world *pPlane = _data.imp->_plane;
	for(unsigned int i = 0;i < count; i ++)
	{
		const XID & member =_team.member_list[i].id;
		world::object_info info;
		if(msg.source != member  &&
				(!pPlane->QueryObject(member,info) || 
				 info.pos.squared_distance(msg.pos) >= range))
		{
			continue;
		}
		list[index++] = member;
	}
	if(index) pPlane->SendMessage(list,list + index, msg);
}


bool player_team::Save(archive & ar)
{
	ar << _team_state << _data.time_out << _data.self_seq << _data.pickup_flag << _data.cur_max_level << _data.cur_wallow_level;
	ar << _data.invite_map.size();
	INVITE_MAP::iterator it = _data.invite_map.begin();
	int t = g_timer.get_systime();
	for(; it != _data.invite_map.end(); it ++)
	{
		ar << it->first << (int)(it->second.timestamp - t) << it->second.id;	
	}

	ar << _team.leader << _team.team_seq << _team.member_count << _team.change_flag << _team.pickup_flag << _team.team_uid;

	for(unsigned int i = 0; i < _team.member_count; i ++)
	{
		member_entry & ent = _team.member_list[i];
		ar << ent.id << ent.data << ent.race << ent.cs_index << ent.cs_sid  << ent.pos << ent.timeout << ent.is_changed;
	}
	return true;
}

void 
player_team::Swap(player_team & rhs)
{
#define Set(var,cls) var = cls.var
	Set(_team_state , rhs);
	Set(_data.time_out,rhs);
	Set(_data.self_seq,rhs);
	Set(_data.pickup_flag,rhs);
	Set(_data.cur_max_level,rhs);
	Set(_data.cur_wallow_level,rhs);
	INVITE_MAP::iterator it = rhs._data.invite_map.begin();
	_data.invite_map.clear();
	for(; it != rhs._data.invite_map.end(); it ++)
	{
		_data.invite_map[it->first] = it->second;
	}

	Set(_team.leader ,rhs);
	Set(_team.team_seq ,rhs);
	Set(_team.member_count ,rhs);
	Set(_team.change_flag ,rhs);
	Set(_team.pickup_flag ,rhs);
	Set(_team.team_uid,rhs);

	for(unsigned int i = 0; i < _team.member_count; i ++)
	{
		_team.member_list[i] = rhs._team.member_list[i];
	}
#undef Set
}

bool player_team::Load(archive & ar)
{
	ar >> _team_state >> _data.time_out >> _data.self_seq >> _data.pickup_flag >> _data.cur_max_level >> _data.cur_wallow_level;
	unsigned int size;
	ar >> size;
	_data.invite_map.clear();
	int t = g_timer.get_systime();
	for(unsigned int i = 0; i < size; i ++)
	{
		int id;
		invite_t inv;
		ar >> id >> (int&)(inv.timestamp) >> inv.id;
		inv.timestamp += t;
		_data.invite_map[id] = inv;
	}

	ar >> _team.leader >> _team.team_seq >> _team.member_count >> _team.change_flag >> _team.pickup_flag >> _team.team_uid;

	for(unsigned int i = 0; i < _team.member_count; i ++)
	{
		member_entry & ent = _team.member_list[i];
		ar >> ent.id >> ent.data >> ent.race >> ent.cs_index >> ent.cs_sid  >> ent.pos >> ent.timeout >> ent.is_changed;
	}
	return true;
}

void 
player_team::DispatchExp(const A3DVECTOR &pos,int * list ,unsigned int count, int exp,int sp,int level,int total_level, int max_level ,int min_level,int npcid,float r)
{
	ASSERT(IsLeader());

	//������С������мӳ�
	float exp_adj = 0,sp_adj = 0;
	player_template::GetExpPunishment(max_level - level,&exp_adj,&sp_adj);
	//�ṩ����ӳ�
	//�������20��û�мӳ�
	if(max_level - min_level < 20)
	{
		//����������ڱ����Ļ���ֵ�Ͻ��е��� 
		player_template::SetTeamBonus(count,_data.race_count,&exp_adj,&sp_adj);
	}
	exp = (int)(exp * exp_adj + 0.5f);
	sp = (int)(sp * sp_adj + 0.5f);        

	//���о������
	MSG msg;
	msg_grp_exp_t mexp;
	mexp.level = level;
	mexp.rand = r;
	BuildMessage(msg,GM_MSG_TEAM_EXPERIENCE,XID(-1,-1),_team.leader,pos,npcid,0,&mexp,sizeof(mexp));

	//total_level�����Ѿ������˲���С��20��������
	float factor = 1/(float)total_level;

	int effect_player = world_manager::GetInstance()->GetEffectPlayerPerInstance();
	for(unsigned int i = 0;i < count; i ++)
	{
		if(effect_player && (int)i >= effect_player) break;
		int index = list[i];
		int level = _team.member_list[index].data.level;
		if(level < 20) level = 20;	
		float factor2 = level * factor;
		mexp.exp = (int)(exp * factor2 + 0.5f);
		mexp.sp = (int)(sp * factor2 + 0.5f);
		msg.target = _team.member_list[index].id;
		SendMessage<0>(_data.imp->_plane,msg);
	}
}

void player_team::CalcRaceCount()
{
	_data.race_count = 0;
	if(!IsInTeam()) return;
	unsigned int member_count = _team.member_count;
	int race_mask = 0;
	for(unsigned int i = 0; i < member_count ; i++)
	{
		int race = 1 << (_team.member_list[i].race & 0x1F);
		if((race_mask & race) == 0)
		{
			race_mask |= race;
			_data.race_count ++;
		}
	}
}

void player_team::CalcMinLevel()
{
	if(!IsInTeam()) return;
	CalcExactMaxLevel();
}

void 
player_team::CalcExactMaxLevel()
{
	int self = _data.imp->_parent->ID.id;
	A3DVECTOR pos = _data.imp->_parent->pos;
	unsigned int count = _team.member_count;
	world::object_info info;
	int tag = world_manager::GetWorldTag();

	int max_level = 0;
	int wallow_level = 0;
	for(unsigned int i = 0; i < count; i ++)
	{
		const member_entry & member = _team.member_list[i];
		if(self == member.id.id)
		{
			int level = _data.imp->_basic.level;
			if(level > max_level)  max_level = level;

			int wallow = _data.imp->_wallow_level;
			if(wallow > wallow_level) wallow_level = wallow;
			continue;
		}
		if(member.data.world_tag != tag)
		{
			continue;
		}
		if(member.pos.squared_distance(pos) >= TEAM_ITEM_DISTANCE*TEAM_ITEM_DISTANCE)
		{
			continue;
		}
		int level = member.data.level;
		if(level > max_level)  max_level = level;

		int wallow = member.data.wallow_level;
		if(wallow > wallow_level) wallow_level = wallow;
	}
	_data.cur_max_level = max_level;
	_data.cur_wallow_level = wallow_level;
}

void 
player_team::LeaderRemoveMember(member_entry * pEntry)
{
	//�����ǰ��������״̬��֪ͨ��ӵĶ����Լ�disappear
	gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
	if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeamMember(pEntry->id.id,pEntry->cs_index,pEntry->cs_sid);
	
	unsigned int offset = (_team.member_list + _team.member_count) - pEntry;
	memmove(pEntry,pEntry + 1, offset * sizeof(member_entry));
	_team.member_count --;
	CalcRaceCount();
	CalcMinLevel();
	_data.imp->NotifyMasterInfoToPet();
}

void 
player_team::MemberRemoveMember(member_entry * pEntry)
{
	//�����ǰ��������״̬��֪ͨ��ӵĶ����Լ�disappear
	gplayer * pPlayer = (gplayer*)(_data.imp->_parent);
	if(pPlayer->IsInvisible()) _data.imp->SendDisappearToTeamMember(pEntry->id.id,pEntry->cs_index,pEntry->cs_sid);
	
	unsigned int offset = (_team.member_list + _team.member_count) - pEntry;
	memmove(pEntry,pEntry + 1, offset * sizeof(member_entry));
	_team.member_count --;
}

int 
player_team::GetWallowLevel()
{
	if(IsInTeam())
	{
		return _data.cur_wallow_level;
	}
	return _data.imp->_wallow_level;

}

int player_team::OnAutoTeamPlayerReady(int leader_id)
{
	if(_team_state != TEAM_NORMAL) return -1;

	_data.invite_map.clear();
	_data.apply_map.clear();
	_auto_team_info.candicate_leader = XID(GM_TYPE_PLAYER, leader_id);
	_auto_team_info.composing_timeout = auto_team_info_t::TIMEOUT_COMPOSING;
	
	return 0;
}

void player_team::OnAutoTeamComposeFailed(int leader_id)
{	
	if(leader_id == _auto_team_info.candicate_leader.id)
	{
		memset(&_auto_team_info, 0, sizeof(_auto_team_info));
	}
}

void player_team::OnAutoTeamComposeStart(int member_list[], unsigned int cnt)
{
	gplayer_imp* pImp = _data.imp;
	if(pImp->_parent->ID != _auto_team_info.candicate_leader || _auto_team_info.composing_timeout <= 0)
	{
		__PRINTF("�Զ���ӽ������б������˲���ȷ�����%d���ӳ�Ӧ��%d, �Զ���ӳ�ʱ����ʱ%d\n", pImp->_parent->ID.id, _auto_team_info.candicate_leader.id, _auto_team_info.composing_timeout);
		return;
	}
	if(_team_state != TEAM_NORMAL) return;	

	for(unsigned int i = 0; i < cnt; ++i) 
	{
		if(member_list[i] == pImp->_parent->ID.id) continue;
		
		XID member(GM_TYPE_PLAYER, member_list[i]);
		_team_ctrl[_team_state]->CliInviteOther(this,member);
	}
}

