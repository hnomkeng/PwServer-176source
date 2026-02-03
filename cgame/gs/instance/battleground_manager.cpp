#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <threadpool.h>
#include <conf.h>
#include <io/pollio.h>
#include <io/passiveio.h>
#include <gsp_if.h>
#include <db_if.h>
#include <amemory.h>
#include <meminfo.h>
#include <strtok.h>
#include <glog.h>

#include "../template/itemdataman.h"
#include "../template/npcgendata.h"
#include "../world.h"
#include "../player_imp.h"
#include "../npc.h"
#include "../matter.h"
#include "../playertemplate.h"
#include "instance_config.h"
#include "battleground_manager.h"
#include "../pathfinding/pathfinding.h"
#include "../template/globaldataman.h"
#include "battleground_ctrl.h"
#include "../obj_interface.h"
#include <factionlib.h>
#include "../aei_filter.h"

world * 
battleground_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);
	
	pPlane->SetWorldCtrl(new battleground_ctrl());
	return pPlane;
}

world_message_handler * 
battleground_world_manager::CreateMessageHandler()
{
	return new battleground_world_message_handler(this);
}

bool 
battleground_world_manager::InitNetClient(const char * gmconf)
{
	return instance_world_manager::InitNetClient(gmconf);
}

void 
battleground_world_manager::OnDeliveryConnected()
{
	GNET::SendBattleServerRegister(0, GetWorldIndex(),GetWorldTag());
	return ;
}

void
battleground_world_manager::Heartbeat()
{
	_msg_queue.OnTimer(0,100);
	world_manager::Heartbeat();
	unsigned int ins_count = _max_active_index;
	for(unsigned int i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		//int result = pPlane->w_battle_result;
		pPlane->RunTick();
		//if(pPlane->w_battle_result != result)
		//{
			//ս�������¼����� ׼���߳�������ң�����������ҵĴ�������
			//�߳���ҵĲ���������Լ������
		//}
	}

	mutex_spinlock(&_heartbeat_lock);
	
	if((++_heartbeat_counter) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//ÿ10�����һ��
		//������г�ʱʱ��Ĵ���
		for(unsigned int i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//������
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//���ڵȴ��ϳ�״̬
				if(pPlane->w_player_count)
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if(pPlane->w_destroy_timestamp <= g_timer.get_systime())
					{
						//û����ұ�����20������Ӧ�ý����world�ع鵽������
						FreeWorld(pPlane,i);
					}
				}
			}
			else
			{
				if(!pPlane->w_player_count)
				{
					pPlane->w_obsolete = 1;
				}
			}
			
		}
		_heartbeat_counter = 0;

		//������ȴ�б��Ĵ��� ��Զ������������
		RegroupCoolDownWorld();
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//�������ص��������㣬��������´�������
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);
}

bool 
battleground_world_manager::CreateBattleGround(const battle_ground_param & param)
{
	//����ȡ�û��ߴ���һ������ 
	spin_autolock keeper(_key_lock);
	//�������ȫ�̱��ּ�����ȷ��״̬��ȷ�����ڴ��ֲ�������������ȫ�̼���Ӧ�ò������̫��ĳ�ͻ 

	//��ʼ��������,��������ķ��䷽ʽҪ������ͬ�Ŷ�(���߿�������NPC����������,�ٸ��ݲ�ͬ������������Ƿ���ĳЩNPC��ʧ��) 
	instance_hash_key hkey;
	hkey.key1 = param.battle_id;
	hkey.key2 = 0;
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);

	if(pPlane == NULL)
	{
		return false;
	}
	
	//����Ϣ����ʽ֪ͨ����������, �ô�������������ȷ��������߼���ĳ��NPC
	battleground_ctrl * pCtrl = dynamic_cast<battleground_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		//�����ڲ���ctrl���ǺϷ���
		ASSERT(false);
		return false;
	}

	
	pCtrl->_data.battle_id = param.battle_id;
	pCtrl->_data.faction_attacker = param.attacker;
	pCtrl->_data.faction_defender = param.defender;
	pCtrl->_data.attacker_count = 0;
	pCtrl->_data.defender_count = 0; 
	pCtrl->_data.player_count_limit = _player_count_limit;
	pCtrl->_data.end_timestamp = param.end_timestamp;
	pCtrl->_defence_init_data = _defence_data;
	pCtrl->_offense_init_data = _offense_data;
	pCtrl->_battle_result = 0;
	pCtrl->_win_condition = _win_condition;

	pCtrl->_offense_award = 5;			// �����������ȼ�����
	int attacker_city = GMSV::GetMafiaCityCount(param.attacker); 
	int defender_city = GMSV::GetMafiaCityCount(param.defender); 

	if(defender_city > attacker_city)
		pCtrl->_offense_award += defender_city - attacker_city;	

	pPlane->w_destroy_timestamp = param.end_timestamp + 300;
	__PRINTF("create battle %d , attacker %d, defender %d\n",param.battle_id, param.attacker, param.defender);
	__PRINTF("%p world %d ϵͳʱ��%d\n",pPlane,pPlane->w_destroy_timestamp, g_timer.get_systime());

	return true;
}


/**/
void 
battleground_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, unsigned int auth_size, bool isshielduser, char flag)
{
	//ս�����޷�ֱ�ӵ�¼��
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void 
battleground_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aebf_filter(pImp,FILTER_CHECK_INSTANCE_KEY,ikey->target.key_level3, 
			((battleground_ctrl*)pImp->_plane->w_ctrl)->GetAtkDefAward(pImp->OI_GetMafiaID())));
}

void 
battleground_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	//����Ӧ���ö�̬��savepoint ��������ʱ��Ҫָ����Щ����
	pImp->GetLastInstanceSourcePos(world_tag,pos);
	if(world_tag != 1)
	{
		//������ĺã� ����Ҳû�취�� ��Ȼ��������ô�죿
		world_tag = 1;
		pos = A3DVECTOR(320,0,3200);
	}
}


world * 
battleground_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int )
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(ikey);
	world_index = -1;
	if(pTmp)
	{
		//�������������� 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);

		//����Ҫ��������Ƿ���������  ����������¼,��ֱ�ӷ���NULL
		//$$$$$$ 
		
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

int
battleground_world_manager::CheckPlayerSwitchRequest(const XID & who, const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	//����Ƿ�ȥ��ȷ�ĸ��� ���ɱ�����ȷ ���� �������ս��ID
	if(ikey->target.key_level4 == 0 || ikey->target.key_level3 == 0)
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	int faction = ikey->target.key_level3;
	
//�����ɸ������͹���
//���ȼ��Key�Ƿ����
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	int rst = 0;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		mutex_spinunlock(&_key_lock);
		return S2C::ERR_BATTLEFIELD_IS_CLOSED;
	}
	pPlane = _cur_planes[*pTmp];
	if(pPlane)
	{
		if(!(ikey->special_mask & IKSM_GM) && pPlane->w_player_count >= _player_limit_per_instance) 
		{
			//��������������
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			//�������Ƿ����Ҫ��
			battleground_ctrl * pCtrl = (battleground_ctrl*)pPlane->w_ctrl;

			//��������Ƿ��Ѿ�����
			if(pCtrl->_data.faction_attacker == faction)
			{
				if(pCtrl->_data.attacker_count >= pCtrl->_data.player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else if(pCtrl->_data.faction_defender == faction)
			{
				if(pCtrl->_data.defender_count >= pCtrl->_data.player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else 
			{
				rst = S2C::ERR_FACTION_IS_NOT_MATCH;
			}

			if(!rst)
			{
				//��������Ƿ��Ѿ������ر�
				if(pCtrl->_data.end_timestamp <= g_timer.get_systime())
				{
					rst = S2C::ERR_BATTLEFIELD_IS_CLOSED;
				}
				else
				if(pPlane->w_battle_result)
				{
					rst = S2C::ERR_BATTLEFIELD_IS_FINISHED;
				}
			}
		}
	}
	else
	{
		rst = S2C::ERR_CANNOT_ENTER_INSTANCE;
	}

	//�����ҵ������� ״̬�����������Ƿ�ƥ��
	mutex_spinunlock(&_key_lock);
	return rst;
}

bool 
battleground_world_manager::GetTown(int faction, A3DVECTOR &pos, int & tag)
{
	int list[64];
	int counter = 0;
	for(unsigned int i = 0; i < _town_list.size() && counter < 64; i ++)
	{
		if(_town_list[i].faction & faction)
		{
			list[counter] = i;
			counter ++;
		}
	}
	if(counter > 0)
	{		
		return true;
	}
	return false;
}

bool 
battleground_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	int worldTag = GetWorldTag();
	if (worldTag == 230 || worldTag == 231 || worldTag == 232 ||
		worldTag == 233 || worldTag == 234 || worldTag == 235)
	{
		if (pImp->GetFaction() & (FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND))
		{
			pos = A3DVECTOR(-322.993000, 202.005000, -306.902000);
		}
		else if (pImp->GetFaction() & (FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND))
		{
			pos = A3DVECTOR(336.403000, 202.005000, 342.290000);
		}
	}

	return GetTown(pImp->GetFaction(),pos,tag);
}

void 
battleground_world_manager::RecordTownPos(const A3DVECTOR &pos,int faction)
{
	ASSERT(faction & ( FACTION_BATTLEOFFENSE | FACTION_BATTLEDEFENCE | FACTION_OFFENSE_FRIEND | FACTION_DEFENCE_FRIEND));
	town_entry ent = {faction,pos};
	_town_list.push_back(ent);
}

void 
battleground_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos,int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;

	battleground_ctrl * pCtrl = (battleground_ctrl*)(pPlane->w_ctrl);

	int faction = 0;
	int id = pPlayer->id_mafia;
	if(id)  
	{       
		if(id == pCtrl->_data.faction_attacker)
		{
			faction = FACTION_OFFENSE_FRIEND | FACTION_BATTLEOFFENSE;
		}
		else if (id == pCtrl->_data.faction_defender)
		{
			faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
		}
	}

	if(faction)
	{
		int tag;
		if(GetTown(faction,pPlayer->pos,tag)) return;
	}

	instance_world_manager::SetIncomingPlayerPos(pPlayer, origin_pos, special_mask);
}

void 
battleground_world_manager::RecordMob(int type, int tid, const A3DVECTOR &pos,int faction,int count)
{
	map_data * pData = NULL;
	if(faction & (FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND))
	{
		//���� 
		pData = &_offense_data;
	}
	else
	{
		//�ǹ�������Ϊ�ط�
		pData = &_defence_data;
	}
	
	switch(type)
	{
		case 0:
			pData->key_building +=  count;
			break;
			
		case 2:
			pData->war_mobs += count;
			break;
		
		case 3:
			pData->mobs += count;
			break;

		default:
			world_manager::RecordMob(type,tid,pos,faction,count);
		return;
	}
}

int 
battleground_world_manager::OnMobDeath(world * pPlane, int faction,  int tid)
{
	npc_template * pTemplate = npc_stubs_manager::Get(tid);
	if(!pTemplate) return 0;
	
	battleground_ctrl * pCtrl = (battleground_ctrl*)(pPlane->w_ctrl);
	if(pTemplate->role_in_war == 1)
	{
		//�ؼ�����
		pCtrl->DestroyKeyBuilding(faction);
	}
	if(pTemplate->role_in_war == 0)
	{
		//��ͨ����
		pCtrl->DestroyMobs(faction);
	}
	return 1;
}

void 
battleground_world_manager::PreInit(const char * servername)
{
	std::string section = "Instance_";
	section += servername;
	Conf *conf = Conf::GetInstance();
	std::string dest = conf->find(section,"battle_destionation");
	const char * str = dest.c_str();
	if(strcmp(str,"destroy_key_building") == 0)
	{
		_win_condition = 0;
		__PRINTINFO("ʤ���������ݻ��������Ľ���\n");
	}
	else if(strcmp(str,"kill_all_mobs") == 0)
	{
		_win_condition = 1;
		__PRINTINFO("ʤ��������ɱ�����еж�����\n");
	}
	else
	{
		_win_condition = 0;
		__PRINTINFO("ʤ������δ�ҵ���ʹ��Ĭ��ʤ���������ݻ��������Ľ�����\n");
	}
	
	_player_count_limit = atoi(conf->find(section,"battle_player_count").c_str());
	if(_player_count_limit <= 0) 
	{
		_player_count_limit = 10;
		__PRINTINFO("ս��ÿ����������Ϊ����ȷ��ʹ��Ĭ��ֵ%d\n",10);
	}
	else
	{
		__PRINTINFO("ս��ÿ����������Ϊ:%d\n",_player_count_limit);
	}
}

void 
battleground_world_manager::FinalInit(const char * servername)
{
	_npc_idle_heartbeat = 1;
	if(!city_region::QueryTransportExist(GetWorldTag()))
	{
		__PRINTINFO("ս���ڲ��ܴ��������͵�\n");
		exit(-1);
	}
}

