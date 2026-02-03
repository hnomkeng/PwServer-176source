#include <common/utf.h>
#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include <db_if.h>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "usermsg.h"
#include "public_quest.h"
#include "luamanager.h"
#include "association_manager.h"

AssociationConfig* AssociationConfig::instance = 0;

AssociationConfig::AssociationConfig()
{
	memset(this,0x00,sizeof(*this));
}

AssociationConfig::~AssociationConfig()
{
	;
}

void AssociationConfig::Init()
{
	memset(this,0x00,sizeof(*this));

	int camp_level[] = { IDX_ATK_CAMP_LEVEL_CONFIG, IDX_DEF_CAMP_LEVEL_CONFIG, 0};

	for(unsigned int i = 0; i < 2; i++)
	{
		DATA_TYPE dt;
		CAMP_LEVEL_CONFIG *config = (CAMP_LEVEL_CONFIG*)world_manager::GetDataMan().get_data_ptr(camp_level[i],ID_SPACE_CONFIG,dt);
		if (config && dt == DT_CAMP_LEVEL_CONFIG)
		{
			for(unsigned int j = 0; j < 16; j++)
			{
				if(i == 0)
				{
					atk_camp_level_config[j].require_scor = config->level[j].require_scor;
					atk_camp_level_config[j].award_item_id = config->level[j].award_item_id;
				}
				else
				{
					def_camp_level_config[j].require_scor = config->level[j].require_scor;
					def_camp_level_config[j].award_item_id = config->level[j].award_item_id;
				}
			}
		}
		else
		{
			printf("CAMP_LEVEL_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}

	int camp_tech_tree[] = { IDX_ATK_BATTLE_TECH_TREE_CONFIG, IDX_DEF_BATTLE_TECH_TREE_CONFIG, 0};

	for(unsigned int i = 0; i < 2; i++)
	{
		DATA_TYPE dt;
		CAMP_BATTLE_TECH_TREE_CONFIG *config = (CAMP_BATTLE_TECH_TREE_CONFIG*)world_manager::GetDataMan().get_data_ptr(camp_tech_tree[i],ID_SPACE_CONFIG,dt);
		if (config && dt == DT_CAMP_BATTLE_TECH_TREE_CONFIG)
		{
			for(unsigned int j = 0; j < 16; j++)
			{
				if(i == 0)
				{
					atk_battle_tech_tree_config[j].parent_node = config->node[j].parent_node;
					atk_battle_tech_tree_config[j].common_value1 = config->node[j].common_value1;
					atk_battle_tech_tree_config[j].common_value2 = config->node[j].common_value2;
				}
				else
				{
					def_battle_tech_tree_config[j].parent_node = config->node[j].parent_node;
					def_battle_tech_tree_config[j].common_value1 = config->node[j].common_value1;
					def_battle_tech_tree_config[j].common_value2 = config->node[j].common_value2;
				}
			}
		}
		else
		{
			printf("CAMP_BATTLE_TECH_TREE_CONFIG INIT FAILED!!! \n");
			ASSERT(false);
			return;
		}
	}

}

