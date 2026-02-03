#ifndef __ONLINEGAME_GS_PLAYER_STORAGE_BACK_H__
#define __ONLINEGAME_GS_PLAYER_STORAGE_BACK_H__

#include <uchar.h>
#include <db_if.h>
#include <gsp_if.h>
#include "luamanager.h"
#include <openssl/md5.h>
#include <vector>
#include <glog.h>

class gplayer_storage_back
{
public:
	enum 
	{
		MAX_VALUE = 256,		
		ENABLED_LOG = 1,
	};
public:
		
	struct RECIPE_BUY_INFO
	{
		int recipe_id;
		int role_limit_personal_value;
		int role_limit_type;
	}; 

	struct PERSON_KEY
	{
		int key_id;
		int key_value;
	};

	struct POINTS_EVENT
	{
		int task_id;
		int points;
	};

	struct USE_ITEMS
	{
		int task_id;
		int count;
	};

	int _enabled_cash_reward;
	RECIPE_BUY_INFO _recipe_buy_info_map[MAX_VALUE];

	PERSON_KEY _person_key_list[MAX_VALUE/2];
	POINTS_EVENT _event_task_complete[MAX_VALUE/10];
	USE_ITEMS _use_items[MAX_VALUE/10];
		
public:
	
	void Init();
	inline void SetEnabledCashReward(int enabled_cash_reward) { _enabled_cash_reward = enabled_cash_reward; }	
	inline bool GetEnabledCashReward() { return (char)_enabled_cash_reward; }

	inline void SetRecipeBuyInfo(int recipe_id, int role_limit_personal_value, int role_limit_type)
	{
		if (recipe_id <= 0)
			return;

		for (int i = 0; i < MAX_VALUE; i++)
		{
			if (_recipe_buy_info_map[i].recipe_id == 0)
			{
				_recipe_buy_info_map[i].recipe_id = recipe_id;
				_recipe_buy_info_map[i].role_limit_personal_value = role_limit_personal_value;
				_recipe_buy_info_map[i].role_limit_type = role_limit_type;
				break;
			}
		}		
	}

	inline RECIPE_BUY_INFO* GetRecipeBuyInfo(int recipe_id)
	{
		for (int i = 0; i < MAX_VALUE; i++)
		{
			if (_recipe_buy_info_map[i].recipe_id == recipe_id)
			{
				return &_recipe_buy_info_map[i];
			}
		}
		return NULL;
	}
	
	inline bool UpdateOrAddKeyValue(int key_id, int key_value)
	{
		for (int i = 0; i < MAX_VALUE/2; i++)
		{
			if (_person_key_list[i].key_id == key_id)
			{
				_person_key_list[i].key_value = key_value;
				return true;
			}
		}

		for (int i = 0; i < MAX_VALUE/2; i++)
		{
			if (_person_key_list[i].key_id == 0)
			{
				_person_key_list[i].key_id = key_id;
				_person_key_list[i].key_value = key_value;
				return true;
			}
		}

		if(ENABLED_LOG)
			GLog::log(GLOG_ERR, "UpdateOrAddKeyValue failed. key_id: %d, key_value: %d", key_id, key_value);
		return false;
	}

	inline bool RemoveKeyValue(int key_id)
	{
		for (int i = 0; i < MAX_VALUE/2; i++)
		{
			if (_person_key_list[i].key_id == key_id)
			{
				_person_key_list[i].key_id = 0;
				_person_key_list[i].key_value = 0;
				return true;
			}
		}

		if(ENABLED_LOG)
			GLog::log(GLOG_ERR, "RemoveKeyValue failed. key_id: %d", key_id);
		return false;
	}
		
	inline int GetKeyValue(int key_id)
	{
		for (int i = 0; i < MAX_VALUE/2; i++)
		{
			if (_person_key_list[i].key_id == key_id)
			{
				return _person_key_list[i].key_value;
			}
		}
		return 0;
	}

	inline void ClearKeyValue()
	{
		memset(_person_key_list, 0, sizeof(_person_key_list));
	}

	inline PERSON_KEY* GetKeyValueList(int pos)
	{
		return &_person_key_list[pos];
	}

		inline int GetPlayerEventTask(int task_id)
	{
		for (int i = 0; i < 25; i++)
		{
			if (_event_task_complete[i].task_id == task_id)
				return _event_task_complete[i].points;
		}
		return 0;
	}

	inline void UpdatePlayerEventTask(int task_id, int points)
	{
		for (int i = 0; i < 25; i++)
		{
			if (_event_task_complete[i].task_id == task_id)
			{				
				_event_task_complete[i].points = points;
				return;
			}
		}

		int count = 0;
		for (int i = 0; i < 25; i++)
		{
			if (_event_task_complete[i].task_id == 0)
			{
				count++;
				_event_task_complete[i].task_id = task_id;
				_event_task_complete[i].points = points;
				return;
			}
		}

		if (count == 0)
		{
			memset(&_event_task_complete,0,sizeof(_event_task_complete));

			_event_task_complete[0].task_id = task_id;
			_event_task_complete[0].points = points;		
		}	
	}

	inline void RemovePlayerEventTask(int task_id)
	{
		for (int i = 0; i < 25; i++)
		{
			if (_event_task_complete[i].task_id == task_id)
			{
				_event_task_complete[i].task_id = 0;
				_event_task_complete[i].points = 0;
				return;
			}
		}
	}

	inline int GetPlayerUseItems(int task_id)
	{
		for (int i = 0; i < 25; i++)
		{
			if (_use_items[i].task_id == task_id)
				return _use_items[i].count;
		}
		return 0;
	}

	inline void UpdatePlayerUseItems(int task_id, int count)
	{
		for (int i = 0; i < 25; i++)
		{
			if (_use_items[i].task_id == task_id)
			{				
				_use_items[i].count = count;
				return;
			}
		}

		int count1 = 0;
		for (int i = 0; i < 25; i++)
		{
			if (_use_items[i].task_id == 0)
			{
				count1++;
				_use_items[i].task_id = task_id;
				_use_items[i].count = count;
				return;
			}			
		}

		if (count1 == 0)
		{
			memset(&_use_items,0,sizeof(_use_items));

			_use_items[0].task_id = task_id;
			_use_items[0].count = count;		
		}	
	}

};


#endif