#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "codex_manager.h"
#include "gsp_if.h"

CodexConfig* CodexConfig::instance = NULL;

void CodexConfig::Init()
{
	memset(this,0x00,sizeof(CodexConfig));
	
	// Recompensa Ano do Tigre (Roupa)
	for (int i = 0; i < COUNT_TYPE_FASHION; i++)
	{
		DATA_TYPE dt;
		ILLUSTRATED_FASHION_SERIES_CONFIG *it = (ILLUSTRATED_FASHION_SERIES_CONFIG *)world_manager::GetDataMan().get_data_ptr(IDX_REWARD_SINGLE_FASHION_BEGIN+i, ID_SPACE_CONFIG, dt);
		if (it && dt == DT_ILLUSTRATED_FASHION_SERIES_CONFIG)
		{
			reward_fashion[i].award_item_count = it->award_item_count;
			reward_fashion[i].award_require_count = it->award_require_count;
			reward_fashion[i].award_item_id = it->award_item_id;
		}
		else
		{
			printf("ILLUSTRATED_FASHION_SERIES_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}

	// Recompensa Ano do Tigre (Voo)
	for (int i = 0; i < COUNT_TYPE_FLY; i++)
	{
		DATA_TYPE dt2;
		ILLUSTRATED_WING_SERIES_CONFIG *it = (ILLUSTRATED_WING_SERIES_CONFIG *)world_manager::GetDataMan().get_data_ptr(IDX_REWARD_SINGLE_FLY_BEGIN+i, ID_SPACE_CONFIG, dt2);
		if (it && dt2 == DT_ILLUSTRATED_WING_SERIES_CONFIG)
		{
			reward_fly[i].award_item_count = it->award_item_count;
			reward_fly[i].award_require_count = it->award_require_count;
			reward_fly[i].award_item_id = it->award_item_id;
		}
		else
		{
			printf("ILLUSTRATED_WING_SERIES_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}

	// Recompensa Ano do Tigre (Mount)
	for (int i = 0; i < COUNT_TYPE_MOUNT; i++)
	{
		DATA_TYPE dt3;
		ILLUSTRATED_PET_SERIES_CONFIG *it = (ILLUSTRATED_PET_SERIES_CONFIG *)world_manager::GetDataMan().get_data_ptr(IDX_REWARD_SINGLE_MOUNT_BEGIN+i, ID_SPACE_CONFIG, dt3);
		if (it && dt3 == DT_ILLUSTRATED_PET_SERIES_CONFIG)
		{
			reward_mount[i].award_item_count = it->award_item_count;
			reward_mount[i].award_require_count = it->award_require_count;
			reward_mount[i].award_item_id = it->award_item_id;
		}
		else
		{
			printf("ILLUSTRATED_PET_SERIES_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}

	// Recompensa de Títulos

	for (int i = 0; i < COUNT_MAX_TYPE; i++)
	{
		DATA_TYPE dt4;
		ILLUSTRATED_REWARD_CONFIG *it = (ILLUSTRATED_REWARD_CONFIG *)world_manager::GetDataMan().get_data_ptr(IDX_REWARD_TITLE_BEGIN+i, ID_SPACE_CONFIG, dt4);
		if (it && dt4 == DT_ILLUSTRATED_REWARD_CONFIG)
		{
			reward_title[i].type = it->type;
			reward_title[i].step = it->step;

			for (int j = 0; j < COUNT_MAX_TITLE_REWARD; j++)
			{
				reward_title[i].award_item_id[j] = it->reward[j].id;		
			}

		}
		else
		{
			printf("ILLUSTRATED_REWARD_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}


	// Validar ID de Roupas

	const int idx_fashions[12] = { 6719, 6724, 6725, 6726, 6727, 6728, 6729, 6730, 6731, 6732, 6733, 6734 };
	const int idx_pets[6] = { 6753, 6754, 6755, 6756, 6863, 6864 };
 	const int idx_flys[24] = { 6742, 6757, 6758, 6759, 6760, 6761, 6762, 6763, 6764, 6765, 6766, 6767, 6865, 6866, 6867, 6868, 6869, 6870, 7879, 7880, 7881, 7882, 7883, 7884 };

	for (int i = 0; i < 12; i++)
	{
		DATA_TYPE dt5;
		ILLUSTRATED_FASHION_SERIES_CONFIG *it5 = (ILLUSTRATED_FASHION_SERIES_CONFIG *)world_manager::GetDataMan().get_data_ptr(idx_fashions[i], ID_SPACE_CONFIG, dt5);
		if (it5 && dt5 == DT_ILLUSTRATED_FASHION_SERIES_CONFIG)
		{
			for (unsigned int j = 0; j < 255; j++)
			{
				if (it5->list[j] > 0)
				{
					DATA_TYPE dt6;
					FASHION_SUITE_ESSENCE *it6 = (FASHION_SUITE_ESSENCE *)world_manager::GetDataMan().get_data_ptr(it5->list[j], ID_SPACE_ESSENCE, dt6);
					if (it6 && dt6 == DT_FASHION_SUITE_ESSENCE)
					{
						for (int k = 0; k < 6; k++)
						{
							if (it6->list[k] > 0)
							{
								AddFashionId(it6->list[k]);
							}
						}
					}
					else
					{
						printf("FASHION_SUITE_ESSENCE INIT FAILED!!! \n");
						ASSERT(false);
						return;
					}
				}
			}
		}
		else 
		{
			printf("ILLUSTRATED_FASHION_SERIES_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;		
		}
	}
}

