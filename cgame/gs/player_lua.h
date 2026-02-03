#ifndef __ONLINEGAME_GS_PLAYER_LUA_H__
#define __ONLINEGAME_GS_PLAYER_LUA_H__

#include <uchar.h>
#include <db_if.h>
#include <gsp_if.h>

#include "player_lottery.h"
#include "player_glyph.h"
#include "player_lib.h"
#include "luamanager.h"

#include <openssl/md5.h>
#include "item/item_addon.h"
#include "item/set_addon.h"
#include "petman.h"

class gplayer_lua
{
public:
	enum 
	{
		MAX_STORAGE_COUNT = 32,
	};

public:
	struct POINTS_EVENT
	{
		short task_id;
		short points;
	};

	struct ARMOR_NEW_SAVE_POS
	{
		int armor_id;
		short armor_pos;
		short inject_pos;
	};

	struct BATTLE_PASS
	{
		int ticket_type;
		int expire_time;
	};

	struct BATTLE_PASS_REWARD
	{
		int item_id;
		int item_count;
	};	

#pragma pack(push, 4)
	struct LUA_ROLE
	{
		float storage[MAX_STORAGE_COUNT];

		short _storage_1; // Submundo Frígido
		short _storage_2; // Submundo Frígido

		unsigned int _storage_reward; // 0~6

		short _anecdote_task_id[71];
		POINTS_EVENT _event_task_complete[10];

		int _imperial_enabled;
		int _imperial_expire_time;

		int _portatil_picture_storage[6];
		int _portatil_picture_active;

		ARMOR_NEW_SAVE_POS _new_armor_spirit[32];		
		ARMOR_NEW_SAVE_POS _new_armor_cristal[32];		

		int _reset_tower_day;

		char _merge_zenith;

		int _reward_lasted_time;
		int _reward_lasted_gold;
		int _reward_next_time;		
		int _reward_next_gold;	

		bool _clear_reward_gold;	
		bool _reset_celestial_memorial;

		BATTLE_PASS _battle_pass[3];
		BATTLE_PASS_REWARD _battle_pass_reward[30];
		int _battle_pass_tag;

		char _child_reset_day;

		int _lasted_queue_time;
		int _next_queue_time;

		bool _reset_lottery;
	};

	struct LUA_USER
	{
		float storage[MAX_STORAGE_COUNT];
	};
#pragma pack(pop)
	int roleid;
	LUA_ROLE _lua_role;
	LUA_USER _lua_user;

	
	
public:
	
	void Init();
	void EnterRole(int rid);
	void LoadRole(int rid, void * iData, unsigned int iSize);
	void SaveRole();

	inline double GetLuaStorage(int pos) {	return _lua_role.storage[pos]; }
	inline void SetLuaStorage(int pos, double value) { _lua_role.storage[pos] = value; }
	inline double GetLuaUserStorage(int pos) {	return _lua_user.storage[pos]; }
	inline void SetLuaUserStorage(int pos, double value) { _lua_user.storage[pos] = value; }

	// Submundo Frígido
	inline void SetLuaStorage1(short value) { _lua_role._storage_1 = value; }
	inline void SetLuaStorage2(short value) { _lua_role._storage_2 = value; }
	inline short GetLuaStorage1() { return _lua_role._storage_1; }
	inline short GetLuaStorage2() { return _lua_role._storage_2; }

	// 0~6
	inline void SetLuaStorageReward(unsigned int value) { _lua_role._storage_reward = value; }
	inline unsigned int GetLuaStorageReward() { return _lua_role._storage_reward; }


	// Anedota
	inline void SetAnecdoteTaskid(short uTaskid)
	{
		for( unsigned int i = 0; i < 71; i++)
		{
			if(_lua_role._anecdote_task_id[i] == 0)
			{
				_lua_role._anecdote_task_id[i] = uTaskid;
				break;
			}
		}		
	}

	inline unsigned short GetAnecdoteTaskid(int pos)
	{
		return _lua_role._anecdote_task_id[pos];
	}

	inline bool CheckAnecdoteTaskid(short uTaskid)
	{
		for( unsigned int i = 0; i < 71; i++)
		{
			if(uTaskid == _lua_role._anecdote_task_id[i]) return true;
		}
		return false; // Caso não exista
	}

	
	inline LUA_ROLE * GetRole() { return &_lua_role; }
	inline LUA_USER * GetUser() { return &_lua_user; }	


	// Imperial
	inline void SetImperialEnabled(int value) { _lua_role._imperial_enabled = value; }
	inline int GetImperialEnabled() { return _lua_role._imperial_enabled; }

	inline void SetImperialExpireTime(int value) { _lua_role._imperial_expire_time = value; }
	inline int GetImperialExpireTime() { return _lua_role._imperial_expire_time; }
	
	/*176+*/
	inline void SetPortatilPictureStorage(int pos, int value) { _lua_role._portatil_picture_storage[pos] = value; }
	inline int GetPortatilPictureStorage(int pos) { return _lua_role._portatil_picture_storage[pos]; }

	inline void SetPortatilPictureActive(int value) { _lua_role._portatil_picture_active = value; }
	inline int GetPortatilPictureActive() { return _lua_role._portatil_picture_active; }

	inline ARMOR_NEW_SAVE_POS * GetNewArmorSpirit(int pos) { return &_lua_role._new_armor_spirit[pos]; }
	inline ARMOR_NEW_SAVE_POS * GetNewArmorCrystal(int pos) { return &_lua_role._new_armor_cristal[pos]; }

	inline void SetNewArmorSpirit(int pos, int armor_id, short armor_pos, short inject_pos)
	{
		_lua_role._new_armor_spirit[pos].armor_id = armor_id;
		_lua_role._new_armor_spirit[pos].armor_pos = armor_pos;
		_lua_role._new_armor_spirit[pos].inject_pos = inject_pos;
	}

	inline void SetNewArmorCrystal(int pos, int armor_id, short armor_pos, short inject_pos)
	{
		_lua_role._new_armor_cristal[pos].armor_id = armor_id;
		_lua_role._new_armor_cristal[pos].armor_pos = armor_pos;
		_lua_role._new_armor_cristal[pos].inject_pos = inject_pos;
	}		

	inline int GetResetTowerDay() { return _lua_role._reset_tower_day; }
	inline void SetResetTowerDay(int value) { _lua_role._reset_tower_day = value; }

	inline bool GetMergeZenith() { return _lua_role._merge_zenith; }
	inline void SetMergeZenith(bool b) { _lua_role._merge_zenith = b; }

	inline int GetRewardLastedTime() { return _lua_role._reward_lasted_time; }
	inline void SetRewardLastedTime(int value) { _lua_role._reward_lasted_time = value; }

	inline int GetRewardLastedGold() { return _lua_role._reward_lasted_gold; }
	inline void SetRewardLastedGold(int value) { _lua_role._reward_lasted_gold = value; }

	inline int GetRewardNextTime() { return _lua_role._reward_next_time; }
	inline void SetRewardNextTime(int value) { _lua_role._reward_next_time = value; }

	inline int GetRewardNextGold() { return _lua_role._reward_next_gold; }
	inline void SetRewardNextGold(int value) { _lua_role._reward_next_gold = value; }

	inline bool GetClearRewardGold() { return _lua_role._clear_reward_gold; }
	inline void SetClearRewardGold(bool b) { _lua_role._clear_reward_gold = b; }

	inline bool GetResetCelestialMemorial() { return _lua_role._reset_celestial_memorial; }
	inline void SetResetCelestialMemorial(bool b) { _lua_role._reset_celestial_memorial = b; }

	inline BATTLE_PASS * GetBattlePass(int pos) { return &_lua_role._battle_pass[pos]; }

	inline void SetBattlePass(int pos, int ticket_type, int expire_time)
	{
		_lua_role._battle_pass[pos].ticket_type = ticket_type;
		_lua_role._battle_pass[pos].expire_time = expire_time;
	}

	inline BATTLE_PASS_REWARD * GetBattlePassReward(int pos) { return &_lua_role._battle_pass_reward[pos]; }

	inline void SetBattlePassReward(int pos, int item_id, int item_count)
	{
		_lua_role._battle_pass_reward[pos].item_id = item_id;
		_lua_role._battle_pass_reward[pos].item_count = item_count;
	}

	inline int GetBattlePassTag() { return _lua_role._battle_pass_tag; }
	inline void SetBattlePassTag(int value) { _lua_role._battle_pass_tag = value; }

	inline char GetChildResetDay() { return _lua_role._child_reset_day; }
	inline void SetChildResetDay(char value) { _lua_role._child_reset_day = value; }

	inline int GetLastedQueueTime() { return _lua_role._lasted_queue_time; }
	inline void SetLastedQueueTime(int value) { _lua_role._lasted_queue_time = value; }

	inline int GetNextQueueTime() { return _lua_role._next_queue_time; }
	inline void SetNextQueueTime(int value) { _lua_role._next_queue_time = value; }

	inline bool GetResetLottery() { return _lua_role._reset_lottery; }
	inline void SetResetLottery(bool b) { _lua_role._reset_lottery = b; }

	inline bool GetReset2Lottery() { _lua_role._merge_zenith; }
	inline void SetReset2Lottery(bool b) { _lua_role._merge_zenith = b; }

};







#endif