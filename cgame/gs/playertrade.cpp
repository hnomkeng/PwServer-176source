#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>

#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"
#include "actsession.h"
#include "playertemplate.h"
#include "serviceprovider.h"
#include <common/protocol_imp.h>
#include "userlogin.h"
#include "trade.h"
#include <factionlib.h>
#include "luamanager.h"

int 
gplayer_imp::GeneralTradeMessageHandler(world * pPlane, const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_PICKUP_MONEY:
		case GM_MSG_PICKUP_MONEY2:
		case GM_MSG_RECEIVE_MONEY:
		PickupMoneyInTrade(msg.param);
		break;
		case GM_MSG_PICKUP_ITEM:
		PickupItemInTrade(msg.pos,msg.content,msg.content_length,msg.param&0x80000000,msg.param*0x7FFFFFFF);
		break;


		case GM_MSG_SWITCH_GET:
		//������
		break;

		case GM_MSG_ENCHANT:
		case GM_MSG_ATTACK:
		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_HATE_YOU:
		case GM_MSG_HEARTBEAT:
		case GM_MSG_QUERY_OBJ_INFO00:
		case GM_MSG_ERROR_MESSAGE:
		case GM_MSG_GROUP_EXPERIENCE:
		case GM_MSG_EXPERIENCE:
		case GM_MSG_TEAM_EXPERIENCE:
//		case GM_MSG_GET_MEMBER_POS:
		case GM_MSG_TEAM_INVITE:
		case GM_MSG_TEAM_AGREE_INVITE:
		case GM_MSG_TEAM_REJECT_INVITE:
		case GM_MSG_JOIN_TEAM:
		case GM_MSG_LEADER_UPDATE_MEMBER:
		case GM_MSG_JOIN_TEAM_FAILED:
		case GM_MSG_MEMBER_NOTIFY_DATA:
		case GM_MSG_NEW_MEMBER:
		case GM_MSG_LEAVE_PARTY_REQUEST:
		case GM_MSG_LEADER_CANCEL_PARTY:
		case GM_MSG_MEMBER_NOT_IN_TEAM:
		case GM_MSG_LEADER_KICK_MEMBER:
		case GM_MSG_MEMBER_LEAVE:
		case GM_MSG_QUERY_PLAYER_EQUIPMENT:
		case GM_MSG_PICKUP_TEAM_MONEY:
		case GM_MSG_PICKUP_TEAM_MONEY2:
		case GM_MSG_NOTIFY_SELECT_TARGET:
		case GM_MSG_QUERY_SELECT_TARGET:

		case GM_MSG_NPC_BE_KILLED:
		case GM_MSG_PLAYER_TASK_TRANSFER:
		case GM_MSG_PLAYER_BECOME_PARIAH:
		case GM_MSG_PLAYER_BECOME_INVADER:
		case GM_MSG_SUBSCIBE_TARGET:
		case GM_MSG_UNSUBSCIBE_TARGET:
		case GM_MSG_SUBSCIBE_CONFIRM:
		case GM_MSG_SUBSCIBE_SUBTARGET:
		case GM_MSG_UNSUBSCIBE_SUBTARGET:
		case GM_MSG_SUBSCIBE_SUBTARGET_CONFIRM:
		case GM_MSG_NOTIFY_SELECT_SUBTARGET:
		case GM_MSG_HP_STEAL:
		case GM_MSG_TEAM_APPLY_PARTY:
		case GM_MSG_TEAM_APPLY_REPLY:
		case GM_MSG_QUERY_INFO_1:
		case GM_MSG_TEAM_CHANGE_TO_LEADER:
		case GM_MSG_TEAM_LEADER_CHANGED:

		case GM_MSG_GM_CHANGE_EXP:
		case GM_MSG_GM_OFFLINE:
		case GM_MSG_GM_MQUERY_MOVE_POS:
		case GM_MSG_GM_DEBUG_COMMAND:
		case GM_MSG_GM_QUERY_SPEC_ITEM:

		case GM_MSG_DBSAVE_ERROR:
		case GM_MSG_QUERY_EQUIP_DETAIL:
		case GM_MSG_ENABLE_PVP_DURATION:
		case GM_MSG_PLAYER_DUEL_REQUEST:
		case GM_MSG_PLAYER_DUEL_REPLY:
		case GM_MSG_PLAYER_DUEL_PREPARE:
		case GM_MSG_PLAYER_DUEL_START:
		case GM_MSG_PLAYER_DUEL_CANCEL:
		case GM_MSG_PLAYER_DUEL_STOP:
		case GM_MSG_PLAYER_BIND_REQUEST:
		case GM_MSG_PLAYER_BIND_INVITE:

		case GM_MSG_PLAYER_RECALL_PET:
		case GM_MSG_MOB_BE_TRAINED:
		case GM_MSG_PET_SET_COOLDOWN:
		case GM_MSG_PET_ANTI_CHEAT:
		case GM_MSG_PET_NOTIFY_DEATH:
		case GM_MSG_PET_NOTIFY_HP:
		case GM_MSG_PET_RELOCATE_POS:
		case GM_MSG_QUERY_PROPERTY:
		case GM_MSG_QUERY_PROPERTY_REPLY:
		case GM_MSG_PLANT_PET_NOTIFY_DEATH:
		case GM_MSG_PLANT_PET_NOTIFY_HP:
		case GM_MSG_PLANT_PET_NOTIFY_DISAPPEAR:
		case GM_MSG_CONGREGATE_REQUEST:
		case GM_MSG_REJECT_CONGREGATE:
		case GM_MSG_NPC_BE_KILLED_BY_OWNER:
		case GM_MSG_EXTERN_HEAL:
		case GM_MSG_QUERY_INVENTORY_DETAIL:
		case GM_MSG_PLAYER_KILLED_BY_PLAYER:

		//��Щ��Ϣ�Ǻ���ͨ��ʱ��ӵ��һ���Ĵ���
		return DispatchNormalMessage(pPlane,msg);
	}
	return 0;
}

int 
gplayer_imp::WaitingTradeMessageHandler(world * pPlane ,const MSG & msg)
{
	//ֻ��������
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//�ȴ�ʱ�䳬ʱ�����Ͳ��ܽ�����Ϣ
			GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),_parent->ID.id,((gplayer*)_parent)->cs_sid,false);
			//�ص�����״̬
			FromTradeToNormal();
		}
		return DispatchNormalMessage(pPlane,msg);
		break;

		case GM_MSG_ENCHANT:
		if(((enchant_msg*)msg.content)->helpful)
		{
			return GeneralTradeMessageHandler(pPlane,msg);
		}

		//�к�������ֹ�˽��ײ���
		
		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_ATTACK:
		//�������ܽ�����Ϣ
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),_parent->ID.id,((gplayer*)_parent)->cs_sid,false);
		//�ص�����״̬
		FromTradeToNormal();
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingTradeCompleteHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//����Ҷ��� �Ҳ����д���
			GLog::log(LOG_ERR,"drop player for trade timeout roleid:%d",_parent->ID.id);
			FromTradeToNormal(-1);
			return 0;
		}
		return DispatchNormalMessage(pPlane,msg);
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingFactionTradeTradeHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//���ó�������
			FactionTradeTimeout();
			return 0;
		}
		return DispatchNormalMessage(pPlane,msg);
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::TradeMessageHandler(world * pPlane ,const MSG & msg)
{
	//ֻ��������
	switch(msg.message)
	{
		case GM_MSG_ENCHANT:
		if(((enchant_msg*)msg.content)->helpful)
		{
			return GeneralTradeMessageHandler(pPlane,msg);
		}
		//�к�������ֹ�˽��ײ���

		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_ATTACK:
		//�������ܽ�����Ϣ
		GMSV::DiscardTrade(_trade_obj->GetTradeID(),_parent->ID.id);
		//�ص��ȴ�������ɵ�״̬
		_player_state = PLAYER_WAIT_TRADE_COMPLETE;
		_trade_obj->SetTimeOut(10);

		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingTradeReadHandler(world * pPlane ,const MSG & msg)
{
	//ֻ��������
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//Ӧ������Ҷ��ߣ����Ҳ�����.....
			GLog::log(LOG_ERR,"drop player for trade timeout roleid:%d",_parent->ID.id);
			FromTradeToNormal(-1);
			return 0;
		}
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WatingFactionTradeReadHandler(world * pPlane ,const MSG & msg)
{
	//ֻ��������
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			GLog::log(LOG_ERR,"drop player for faction trade timeout roleid:%d",_parent->ID.id);

			faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
			if(pTrade)
			{
				user_save_data((gplayer*)_parent,NULL,2,pTrade->_put_mask);
				LuaManager * lua = LuaManager::GetInstance();
				lua->EventPlayerSave(_parent->ID.id);
			}
			else
			{
				ASSERT(false);
			}

			//Ӧ������Ҷ��ߣ����Ҳ�����.....
			FromFactionTradeToNormal(-1);
			return 0;
		}
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}


bool 
gplayer_imp::CanTrade(const XID & target)
{
	//����һ�ɽ�ֹ����
//	return false;
	if(_cheat_punish) return false;
	
	//������״̬���ܽ���
	if(_player_state != PLAYER_STATE_NORMAL) return false;
	ASSERT(_trade_obj == NULL);

	//���ײ��ܿ�ʼ��������
	//��ǰ���κ����͵�session ����ִ����
	if(_cur_session || _session_list.size()) return false;

	if(OI_TestSafeLock()) return false;

/*
	//����ս��״̬�������� 
	//�����᲻���ʲô����..
	//ս��״̬Ҳ���ܽ��� 
	if(IsCombatState()) return false;
	*/
	
	//����״̬���ܽ��� �������ж�
	//���������� ���벻�ܳ�����Χ���������ж�

	//����true
	return true;

}

void
gplayer_imp::StartTrade(int trade_id, const XID & target)
{
	//ս��״̬������������
	ASSERT(_player_state == PLAYER_STATE_NORMAL && !_cur_session);
	ASSERT(_trade_obj == NULL);

	//���뽻�׶��� ����ȴ�����״̬
	_trade_obj = new player_trade(trade_id);
	_player_state = PLAYER_WAITING_TRADE;
	_trade_obj->SetTimeOut(30);	//������ʮ����̳�ʱ����

	class WaitInfoWriteBack : public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		int _trade_id;
		unsigned int _counter;
		unsigned int _counter2; 
		unsigned int _counter3;
		int _mall_order_id;
		world *_plane;
	public:
		WaitInfoWriteBack(int trade_id, gplayer_imp * imp,world * pPlane):_trade_id(trade_id),_plane(pPlane)
		{
			gplayer * pPlayer = imp->GetParent();
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
			_counter = imp->_tb_change_counter;
			_counter2 = imp->_eq_change_counter;
			_counter3 = imp->_user_tb_change_counter;
			_mall_order_id = imp->_mall_order_id;
		}

		virtual void OnTimeOut()
		{
			CallWriteBack(false);
			delete this;
		}

		virtual void OnFailed()
		{
			CallWriteBack(false);
			delete this;
		}
		
		virtual void OnPutRole(int retcode, GDB::PutRoleResData *data = NULL)
		{
			ASSERT(retcode == 0);
			CallWriteBack(retcode == 0);
			delete this;
		}

		void CallWriteBack(bool success)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; //can't find
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;	// not match
			}
			ASSERT(pPlayer->imp);
			gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
			if(success)
			{
				if(_counter == pImp->_tb_change_counter )
				{
					pImp->TryClearTBChangeCounter();
				}

				if(_counter2 == pImp->_eq_change_counter )
				{
					pImp->_eq_change_counter = 0;
				}

				if(_counter3 == pImp->_user_tb_change_counter )
				{
					pImp->TryClearUserTBChangeCounter();
				}

				pImp->MallSaveDone(_mall_order_id);

				//�����̴�����0
				pImp->_db_save_error = 0;
			}
			pImp->WaitingTradeWriteBack(_trade_id,success);
		}


	};

	//���д��̲���
	user_save_data((gplayer*)_parent,new WaitInfoWriteBack(trade_id,this,_plane),0);
	LuaManager * lua = LuaManager::GetInstance();
	lua->EventPlayerSave(_parent->ID.id);
	return ;
}

void
gplayer_imp::TradeComplete(int trade_id, int reason,bool need_read)
{
	if(_player_state != PLAYER_WAITING_TRADE && 
			_player_state != PLAYER_TRADE &&
			_player_state != PLAYER_WAIT_TRADE_COMPLETE)
	{
		//δ�ں��ʵ�״̬��
		return;
	}

	class WaitInfoReadBack: public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		int _trade_id;
		world * _plane;
	public:
		WaitInfoReadBack(int trade_id, gplayer * pPlayer,world *pPlane):_trade_id(trade_id),_plane(pPlane)
		{
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
		}

		virtual void OnTimeOut()
		{
			CallReadBack(NULL);
			delete this;
		}

		virtual void OnFailed()
		{
			CallReadBack(NULL);
			delete this;
		}
		
		virtual void OnGetMoneyInventory(unsigned int money, const GDB::itemlist & list, int timestamp,int mask)
		{
			CallReadBack(&list, money);
			delete this;
		}

		void CallReadBack(const GDB::itemlist * pInv,int money = 0)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; 
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;
			}
			ASSERT(pPlayer->imp);
			((gplayer_imp*)pPlayer->imp)->WaitingTradeReadBack(_trade_id,pInv,money);
		}


	};

	if(_trade_obj->GetTradeID() != trade_id)
	{
		//����ID�����������֮
		GLog::log(GLOG_ERR,"�û�%d����ID%d��ɺ�ID������(ӦΪ%d)",_parent->ID.id, trade_id,_trade_obj->GetTradeID());
		FromTradeToNormal(-1);
		return;
	}

	if(need_read)
	{	
		//������̵ȴ�״̬
		_player_state = PLAYER_WAIT_TRADE_READ;
		//��������������ó�ʱ
		_trade_obj->SetTimeOut(45);

		GDB::get_money_inventory(_parent->ID.id,new WaitInfoReadBack(trade_id,(gplayer*)_parent,_plane));

	}
	else
	{
		//����Ҫ���¶��̣�ֱ�ӷ���normal״̬
		FromTradeToNormal();
	}
	return;
}

bool
gplayer_imp::StartFactionTrade(int trade_id,int get_mask, int put_mask ,bool no_response)
{
	if(_player_state != PLAYER_STATE_NORMAL || _cur_session) return false;
	ASSERT(_trade_obj == NULL);

	//���뽻�׶��� ����ȴ�����״̬
	_trade_obj = new faction_trade(trade_id, get_mask,put_mask);
	_player_state = PLAYER_WAITING_FACTION_TRADE;
	_trade_obj->SetTimeOut(45);	//������ʮ�볬ʱ����

	if(!no_response)
	{
		//���͸�������
		//no_response=falseֻ��gamed��gfactiond����FactionOPRequestʱ���֣���ȡsnycdataʱӦ����_db_timestamp
		int timestamp = OI_InceaseDBTimeStamp();
		GLog::log(GLOG_INFO,"�û�%d��ʼ���ɽ���,timestamp=%d,money=%d,silver=%d", _parent->ID.id, timestamp, GetGold(),GetSilver());
		GNET::syncdata_t data(GetAllMoney(),_basic.skill_point);
		GNET::SendFactionLockResponse(0,trade_id,_parent->ID.id,data);
	}
	((gplayer_dispatcher*)_runner)->mafia_trade_start();
	return true;
}
void 
gplayer_imp::WaitingTradeWriteBack(int trade_id,bool bSuccess)
{
	if(_player_state != PLAYER_WAITING_TRADE) return;
	if(_trade_obj->GetTradeID() != trade_id) return;

	gplayer * pPlayer = (gplayer*)_parent;
	if(bSuccess)
	{
		//���һ�²ֿ�ı��¼
		TryClearTBChangeCounter();

		//д�����ݳɹ�
		//����ͬ�⽻�׵�����
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),pPlayer->ID.id,pPlayer->cs_sid,true);

		//���뽻��״̬
		_player_state = PLAYER_TRADE;
	}
	else
	{
		//д������ʧ��
		//���Ͳ�ͬ�⽻�׵�����
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),pPlayer->ID.id,pPlayer->cs_sid,false);

		//�ص��ǽ���״̬
		FromTradeToNormal();
	}
}

void 
gplayer_imp::WaitingTradeReadBack(int trade_id,const GDB::itemlist * pInv,int money)
{
	if(_player_state != PLAYER_WAIT_TRADE_READ) return;
	if(_trade_obj->GetTradeID() != trade_id) return;
	if(pInv)
	{
		GLog::log(GLOG_INFO,"�û�%d���׳ɹ�������ID%d��Ǯ�ı�Ϊ%d",_parent->ID.id, trade_id,money - GetGold());
		//��ȡ���ݳɹ�
		//���������Լ�����Ʒ��
		_inventory.Clear();
		_inventory.InitFromDBData(*pInv);
		if(((unsigned int)money) > gplayer_money::MAX_GOLD) 
		{
			//
			GLog::log(GLOG_ERR,"�û�%d���׽�Ǯ�����������",_parent->ID.id);
			_money.SetGold(gplayer_money::MAX_GOLD);
		}
		_money.SetGold(money);
		

		//�ص�ͨ��״̬
		FromTradeToNormal();
	}
	else
	{
		GLog::log(GLOG_INFO,"�û�%d����ID%d��ɺ�ȡ����ʧ��",_parent->ID.id, trade_id);
		//��ȡ����ʧ�� ֱ�������Ǹ�������
		//���Ҳ�����д�̲���
		FromTradeToNormal(-1);
	}
}

void 
gplayer_imp::PutItemAfterTrade(item_list & backpack)
{
	XID self = _parent->ID;
	for(unsigned int i = 0; i < backpack.Size();i ++)
	{
		if(backpack[i].type == -1) break;
		item it;
		backpack.Remove(i,it);
		ASSERT(it.type != -1);
		unsigned int ocount = it.count;
		int expire_date = it.expire_date;
		int type = it.type;
		int rst = _inventory.Push(it);
		if(rst >= 0)
		{
			//����˶���
			((gplayer_dispatcher*)_runner)->receive_item_after_trade(type,expire_date,ocount-it.count,_inventory[rst].count,rst);
			LuaManager::GetInstance()->EventPlayerObtianItem(_parent->ID.id, rst, type, ocount-it.count, _inventory[rst].proc_type);
		}

		if(it.type != -1)
		{	
			//�޷������ˣ����ж����Ĳ���
			item_data data;
			item_list::ItemToData(it,data);
			DropItemFromData(_plane,_parent->pos,data,self,0,0,_parent->ID.id); 
			it.Release();
		}
	}
}

void 
gplayer_imp::DropAllAfterTrade(item_list & backpack,unsigned int money)
{
	XID self = _parent->ID;
	const A3DVECTOR & pos = _parent->pos;
	for(unsigned int i = 0; i < backpack.Size();i ++)
	{
		if(backpack[i].type == -1) break;
		item it;
		backpack.Remove(i,it);
		ASSERT(it.type != -1);
		item_data data;
		item_list::ItemToData(it,data);
		DropItemFromData(_plane,pos,data,self,0,0,_parent->ID.id); 
		it.Release();
	}
	if(money) DropMoneyItem(_plane,pos,money,self,0,0);
}

void 
gplayer_imp::FromTradeToNormal(int type)
{
	if(type < 0)
	{
		//����Ʒ���ڵ���
		DropAllAfterTrade(_trade_obj->GetBackpack(),_trade_obj->GetMoney());

		delete _trade_obj;
		_trade_obj = NULL;
		_player_state = PLAYER_STATE_NORMAL;

		//Ȼ��������߲����������д���
		_offline_type = PLAYER_OFF_LPG_DISCONNECT;
		Logout(-1);
		return;
	}
	//������ҵ�ǰ�İ���������
	PlayerGetInventoryDetail(0);
	_runner->get_player_money(GetGold(),GetSilver());

	//���Ƚ����۵���Ʒ��Ǯ�������
	unsigned int inc_money = _trade_obj->GetMoney();
	if(inc_money)
	{
		unsigned int tmp = GetGold();
		GainGoldWithDrop(inc_money,true);
		((gplayer_dispatcher*)_runner)->receive_money_after_trade(GetGold() - tmp);
	}
	PutItemAfterTrade(_trade_obj->GetBackpack());

	//Ȼ��ɾ�����󣬻ص�����״̬
	delete _trade_obj;
	_trade_obj = NULL;

	//Ȼ��ص�ͨ��״̬ ����֪ͨplayer���ڴ��ڷǽ���״̬
	//����״̬�Ѿ�����������
	_player_state = PLAYER_STATE_NORMAL; 
	_write_timer = 512;

	if(_parent->b_disconnect)
	{
		//����Ѿ����ߣ�ִ�ж����߼�  ���Ҵ���
		//��������߼�
		_player_state = PLAYER_DISCONNECT;
		_disconnect_timeout = LOGOUT_TIME_IN_NORMAL;
	}
}

void 
gplayer_imp::FromFactionTradeToNormal(int type)
{
	if(type)
	{
		//����Ʒ���ڵ���
		DropAllAfterTrade(_trade_obj->GetBackpack(),_trade_obj->GetMoney());

		delete _trade_obj;
		_trade_obj = NULL;
		_player_state = PLAYER_STATE_NORMAL;

		//Ȼ��������߲����������д���
		_offline_type = PLAYER_OFF_LPG_DISCONNECT;
		Logout(-1);
		return;
	}

	ASSERT(_player_state == PLAYER_WAITING_FACTION_TRADE || _player_state == PLAYER_WAIT_FACTION_TRADE_READ);
	//������ҵ�ǰ�İ���������
	//�ж��Ƿ�������ˢ��װ�� ��δ��
	PlayerGetInventory(0);
	_runner->get_player_money(GetGold(),GetSilver());

	//���Ƚ����۵���Ʒ��Ǯ�������
	unsigned int inc_money = _trade_obj->GetMoney();
	if(inc_money)
	{
		unsigned int tmp = GetGold();
		GainGoldWithDrop(inc_money,true);
		((gplayer_dispatcher*)_runner)->receive_money_after_trade(GetGold() - tmp);
	}
	PutItemAfterTrade(_trade_obj->GetBackpack());

	//���ɽ��������Ȳ����·�����Ʒ����
	PlayerGetInventory(0);

	//Ȼ��ɾ������
	delete _trade_obj;
	_trade_obj = NULL;

	//Ȼ��ص�ͨ��״̬ ����֪ͨplayer���ڴ��ڷǽ���״̬
	//����״̬�Ѿ�����������
	_player_state = PLAYER_STATE_NORMAL; 

	((gplayer_dispatcher*)_runner)->mafia_trade_end();
	if(_parent->b_disconnect)
	{
		//����Ѿ����ߣ�ִ�ж����߼�  ���Ҵ���
		Logout(GMSV::PLAYER_LOGOUT_FULL);
	}
}

void 
gplayer_imp::FactionTradeTimeout()
{
	ASSERT(_player_state == PLAYER_WAITING_FACTION_TRADE);
	class WaitInfoReadBack: public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		world * _plane;
	public:
		WaitInfoReadBack(gplayer * pPlayer,world *pPlane):_plane(pPlane)
		{
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
		}

		virtual void OnTimeOut()
		{
			//CallReadBack(NULL);
			//���ﲻ���ڵ���rpc�ˣ�������
			//����ʹ��trade_obj�ĳ�ʱ
			delete this;
		}

		virtual void OnFailed()
		{
			CallReadBack(NULL);
			delete this;
		}
		
		virtual void OnGetMoneyInventory(unsigned int money, const GDB::itemlist & list,int timestamp, int mask)
		{
			CallReadBack(&list, money, timestamp, mask);
			delete this;
		}

		void CallReadBack(const GDB::itemlist * pInv,int money = 0, int timestamp = 0, int mask = 0)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; 
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;	
			}
			ASSERT(pPlayer->imp);
			gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
			pImp->WaitingFactionTradeReadBack(pInv, money, timestamp, mask);
		}

	};

	//������̵ȴ�״̬
	_player_state = PLAYER_WAIT_FACTION_TRADE_READ;
	//��������������ó�ʱ
	_trade_obj->SetTimeOut(45);

	faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
	if(pTrade)
	{
		GDB::get_money_inventory(_parent->ID.id,new WaitInfoReadBack((gplayer*)_parent,_plane),pTrade->_get_mask);
	}
	else
	{
		ASSERT(false);
		//Ӧ������Ҷ��ߣ����Ҳ�����.....
		FromFactionTradeToNormal(-1);
	}
	return;
}

void 
gplayer_imp::WaitingFactionTradeReadBack(const GDB::itemlist * pInv,int money, int timestamp, int mask)
{
	if(_player_state != PLAYER_WAIT_FACTION_TRADE_READ) return;
	if(pInv)
	{
		if(((int)(timestamp -_db_timestamp)) >= 0)
		{       
			//����Ǯ��
			if(((unsigned int)money) > gplayer_money::MAX_GOLD) 
			{       
				//      
				GLog::log(GLOG_ERR,"�û�%d�ⲿ���׳�ʱ�󣬽�Ǯ�����������",_parent->ID.id);
				_money.SetGold(gplayer_money::MAX_GOLD);
			}       
			_money.SetGold(money);
			GLog::log(GLOG_INFO,"�û�%d�ⲿ���׳�ʱ�󣬽�Ǯ�ı� %d" ,_parent->ID.id,money - GetGold());

			//��ȡ���ݳɹ�
			//���������Լ�����Ʒ��
			_inventory.Clear();
			_inventory.InitFromDBData(*pInv);
			PlayerGetInventoryDetail(0);
			_db_timestamp = timestamp;			
		}       
		else    
		{       
			GLog::log(GLOG_ERR,"�û�%d�ⲿ���׽�������ʱ����С%d(%d)",_parent->ID.id,timestamp,_db_timestamp);
			//��������ʱ�����С����ʹ�� �������Լ�������
		}               

		//�ص�ͨ��״̬
		FromFactionTradeToNormal();
	}
	else
	{
		//��ȡ����ʧ�� 
		GLog::log(GLOG_ERR,"�û�%d�ⲿ������ɺ�ȡ����ʧ��",_parent->ID.id);

		//����һ�ηǰ�������
		faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
		if(pTrade)
		{       
			user_save_data((gplayer*)_parent,NULL,2,pTrade->_put_mask);
			LuaManager * lua = LuaManager::GetInstance();
			lua->EventPlayerSave(_parent->ID.id);
		}
		else
		{
			ASSERT(false);
		}

		//���Ҳ�����д�̲���
		FromFactionTradeToNormal(-1);
	}
}

void 
gplayer_imp::FactionTradeComplete(int trade_id,const GNET::syncdata_t & data)
{
	if(_player_state != PLAYER_WAITING_FACTION_TRADE)
	{
		//δ�ں��ʵ�״̬��
		return;
	}

	GLog::log(GLOG_INFO,"�û�%d���ɲ�����ɣ���Ǯ�ı�Ϊ%d",_parent->ID.id, data.money - GetGold());

	//��������
	//printf("PLAYER_TRADE_SYNC: func = FactionTradeComplete , data.money = %d\n", data.money);
	
	ASSERT(_trade_obj != NULL);
	//ASSERT(data.money >=0 && data.money <= (_money_capacity*2));
	if(trade_id != _trade_obj->GetTradeID())
	{
		return ;
	}

	//���ͳɹ�����
	//֪ͨ�ͻ�����ɴ˲��������Խ��в���
	if (!SyncMoney(data.money))
	{
		return;
	}
	//unsigned int req_money = (_player_money[0] + _player_money[1]) - data.money;
	//if (req_money == 0)
	//{
	//	return;
	//}
	
	//CheckSilver(req_money);
	//_player_money[1] -= req_money;
	//printf("PLAYER_TRADE_SYNC: func = FactionTradeComplete , money_gold = %d , money_silver = %d , req_money = %d \n", _player_money[0], _player_money[1], req_money);
	
	_basic.skill_point += data.sp;
	if(data.sp)
	{
		GLog::log(GLOG_INFO,"�û�%d���а��ɲ���(tid:%d)��sp�ı�%d",_parent->ID.id,trade_id,data.sp);
	}
	
	SetRefreshState();
	//GSMV_SEND FACTIONTRADECOMPELETE
	//��ý��д��̲���
	
	FromFactionTradeToNormal();
	return;
}

void 
gplayer_imp::SyncTradeComplete(int trade_id,unsigned int money, const GDB::itemlist & item_change,bool writetrashbox, bool cash_change)
{
	if(_player_state != PLAYER_WAITING_FACTION_TRADE)
	{
		//δ�ں��ʵ�״̬��
		return;
	}
	//��������
	ASSERT(_trade_obj != NULL);
	ASSERT(money >=0 && money <= gplayer_money::MAX_GOLD);
	
	GLog::log(GLOG_INFO,"�û�%d�ⲿ������ɣ���Ǯ�ı�Ϊ%d",_parent->ID.id, money - GetGold());


	if(trade_id != _trade_obj->GetTradeID()) return ;

	//���ͳɹ�����
	//֪ͨ�ͻ�����ɴ˲��������Խ��в���
	
	_money.SetGold(money);
	//printf("PLAYER_TRADE_SYNC: func = SyncTradeComplete , money_gold = %d , money_silver = %d \n", _player_money[0], _player_money[1]);

	//���а��������޸�
	for(unsigned int i = 0; i < item_change.count; i ++)
	{
		GDB::itemdata * pData = item_change.list + i;
		if(pData->count)
		{
			item & it = _inventory[pData->index];
			//���²���
			if(pData->id != (unsigned int)it.type) 
			{
				_inventory.Remove(pData->index);
				item it2;
				if(MakeItemEntry(it2,*pData))
				{
					bool bRst = _inventory.Put(pData->index,it2);
					ASSERT(bRst);
				}
				else
				{
					GLog::log(LOG_ERR, "SyncTradeCompleteʱ������Ʒʧ��%d",it2.type);
					it2.Clear();
				}
			}
			else
			{
				if((unsigned int)pData->count > it.count)
				{
					_inventory.IncAmount(pData->index, pData->count - it.count);
				}
				else
				{
					_inventory.DecAmount(pData->index, it.count - pData->count);
				}
			}
		}
		else
		{
			//ɾ������
			_inventory.Remove(pData->index);
		}
	}
	
/*	if(writetrashbox) $$$$$$$$$$$ ����һ�ɲ�����ֿ���̱�־
	{
		TryClearTBChangeCounter();
	}
	*/
	
	PlayerGetInventoryDetail(0);


	if(cash_change)
	{
		MallSaveDone(_mall_order_id);
	}

	FromFactionTradeToNormal();
	return;
}

