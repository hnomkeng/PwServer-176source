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
#include "luamanager.h"
#include <glog.h>
#include "player_storage_back.h"

void gplayer_storage_back::Init()
{
	_enabled_cash_reward = 0; 
	memset(_recipe_buy_info_map, 0, sizeof(_recipe_buy_info_map));  
	
	memset(_person_key_list, 0, sizeof(_person_key_list)); 
	memset(_event_task_complete, 0, sizeof(_event_task_complete)); 
	memset(_use_items, 0, sizeof(_use_items)); 
}
