
#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "questionmanager.h"
#include "gsp_if.h"

QuestionManager* QuestionManager::instance = NULL;

void QuestionManager::Init()
{
	memset(this,0x00,sizeof(QuestionManager));
	
	DATA_TYPE dt;
	QUESTION_TIP_CONFIG *it = (QUESTION_TIP_CONFIG*)world_manager::GetDataMan().get_data_ptr(GROUP_IDX0,ID_SPACE_CONFIG,dt);
	if (it && dt == DT_QUESTION_TIP_CONFIG)
	{
		for (int i = 0; i < QUESTION_MAX_COUNT; i++)
		{
			list[i].question = it->questions[i].question_tip;
			list[i].answer_a = it->questions[i].answer_a;
			list[i].answer_b = it->questions[i].answer_b;
			list[i].answer_c = it->questions[i].answer_c;
			list[i].answer_d = it->questions[i].answer_d;
		}
	}
	else
	{
		printf("QUESTION_TIP_CONFIG INIT FAILED!!! \n");
		//ASSERT(false);
		return;
	}	

	DATA_TYPE dt2;
	QUESTION_AWARD_CONFIG *it2 = (QUESTION_AWARD_CONFIG*)world_manager::GetDataMan().get_data_ptr(GROUP_REWARD_IDX0,ID_SPACE_CONFIG,dt2);
	if (it2 && dt2 == DT_QUESTION_AWARD_CONFIG)
	{
		for (int i = 0; i < QUESTION_MAX_REWARD; i++)
		{
			award[i].exp = it2->award[i].exp;
			award[i].sp = it2->award[i].sp;
			award[i].money = it2->award[i].silver_money;
		}
	}
}