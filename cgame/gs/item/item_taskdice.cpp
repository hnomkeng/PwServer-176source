#include "../clstab.h"
#include "../actobject.h"
#include "../world.h"
#include "../item_list.h"
#include "item_taskdice.h"
#include "../task/taskman.h"
#include "../player_imp.h"
#include "../template/itemdataman.h"
#include "../luamanager.h"

DEFINE_SUBSTANCE(item_taskdice,item_body,CLS_ITEM_TASKDICE)

int
item_taskdice::OnUse(item::LOCATION l,gactive_imp * obj,unsigned int count)
{
	ASSERT(obj->GetRunTimeClass()->IsDerivedFrom(CLASSINFO(gplayer_imp)));
	__PRINTF("ʹ�������񴥷���Ʒ�� ��ͼ��������%d\n",_ess.task_id);

	if(obj->IsCombatState())
	{
		DATA_TYPE dt;
		struct TASKDICE_ESSENCE* ess = (struct TASKDICE_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
		if(ess == NULL || dt != DT_TASKDICE_ESSENCE)
		{
			ASSERT(false);
			return -1;
		}
		if(ess->no_use_in_combat)
		{
			obj->_runner->error_message(S2C::ERR_INVALID_OPERATION_IN_COMBAT);
			return -1;
		}
	}

	enum
	{
		PASS_TICKET_1 = 110138,
		PASS_TICKET_2 = 110139,
		PASS_TICKET_3 = 110140,
		ADD_TICKET_COUNT = 30,
	    SECOND_TIME = 86400,
        BASIC_DAYS = 30,
    };

	enum 
	{
		GET100_GOLDS2 = 110157,
		GET200_GOLDS2 = 110158,
		GET500_GOLDS2 = 110159,
		GET1000_GOLDS2 = 110160,
	};

	bool notify = false;
	bool notify2 = false;

	if(_tid == PASS_TICKET_1)
	{		
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
    		int now = g_timer.get_systime();

			int ticket_count = pImp->GetLua()->GetBattlePass(0)->ticket_type;
			pImp->GetLua()->SetBattlePass(0, ticket_count + ADD_TICKET_COUNT, now + (SECOND_TIME * BASIC_DAYS));

			notify = true;
		}
	}
	else if(_tid == PASS_TICKET_2)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			int now = g_timer.get_systime();

			int ticket_count = pImp->GetLua()->GetBattlePass(1)->ticket_type;
			pImp->GetLua()->SetBattlePass(1, ticket_count + ADD_TICKET_COUNT, now + (SECOND_TIME * BASIC_DAYS));

			notify = true;
		}
	}
	else if(_tid == PASS_TICKET_3)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			int now = g_timer.get_systime();

			int ticket_count = pImp->GetLua()->GetBattlePass(2)->ticket_type;
			pImp->GetLua()->SetBattlePass(2, ticket_count + ADD_TICKET_COUNT, now + (SECOND_TIME * BASIC_DAYS));

			notify = true;
		}
	}

	if(notify)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;
		if(pImp)
		{
			pImp->PassBattleProtocol();

			LuaManager * lua = LuaManager::GetInstance();

			char MsgStr[256];
			if (EmulateSettings::GetInstance()->GetMsgLanguage() == 1) // PT-BR
				snprintf(MsgStr, sizeof(MsgStr), "^ffff80O seu passe de batalha foi aberto com sucesso! Você recebeu 30 Tickets do passe de batalha.\n");
			else // US-UK
				snprintf(MsgStr, sizeof(MsgStr), "^ffff80Your battle pass has been successfully opened! You received 30 Battle Pass Tickets.\n");

			lua->game__ChatMsg(0, pImp->_parent->ID.id, 0, MsgStr, -1);
		}
		return 1;
	}

	// Troca de Golds
	if(_tid == GET100_GOLDS2)
	{		
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			pImp->AddMallCash((100 * 1) * 100);
			pImp->_runner->player_cash(pImp->GetMallCash());

			notify2 = true;
		}
	}
	else if(_tid == GET200_GOLDS2)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			pImp->AddMallCash((200 * 1) * 100);
			pImp->_runner->player_cash(pImp->GetMallCash());

			notify2 = true;
		}
	}
	else if(_tid == GET500_GOLDS2)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			pImp->AddMallCash((500 * 1) * 100);
			pImp->_runner->player_cash(pImp->GetMallCash());

			notify2 = true;
		}
	}
	else if(_tid == GET1000_GOLDS2)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;

		if(pImp)
		{
			pImp->AddMallCash((1000 * 1) * 100);
			pImp->_runner->player_cash(pImp->GetMallCash());

			notify2 = true;
		}
	}

	if(notify2)
	{
		gplayer_imp* pImp = (gplayer_imp*)obj;
		if(pImp)
		{
			LuaManager * lua = LuaManager::GetInstance();

			char MsgStr[256];
			if (EmulateSettings::GetInstance()->GetMsgLanguage() == 1) // PT-BR
				snprintf(MsgStr, sizeof(MsgStr), "^ffff80Seus golds foram trocados com sucesso, abra sua loja para verificar.\n");
			else // US-UK
				snprintf(MsgStr, sizeof(MsgStr), "^ffff80Your golds have been successfully exchanged, open your boutique to check.\n");

			lua->game__ChatMsg(0, pImp->_parent->ID.id, 0, MsgStr, -1);
		}
		return 1;
	}

	
	PlayerTaskInterface  task_if((gplayer_imp*)obj);
	if(OnTaskCheckDeliver(&task_if,_ess.task_id,0))
	{
		__PRINTF("��������ɹ�\n");
		return 1;
	}
	obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
	return -1;
}

int item_taskdice::generate_task_id()
{
	DATA_TYPE datatype;
	TASKDICE_ESSENCE * ess = (TASKDICE_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, datatype);
	if(ess == NULL || datatype != DT_TASKDICE_ESSENCE)	return 0;
	unsigned int dice_index = element_data::RandSelect(&(ess->task_lists[0].probability), sizeof(int)+sizeof(float), sizeof(ess->task_lists)/sizeof(ess->task_lists[0]), element_data::NORMAL(0),element_data::LOWER_TREND);		//�׶�����Ŀ
	return ess->task_lists[dice_index].id;
}

