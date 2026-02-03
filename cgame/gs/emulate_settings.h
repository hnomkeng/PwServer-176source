#ifndef __GNET_EMULATE_SETTINGS_H
#define __GNET_EMULATE_SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>

class EmulateSettings
{
public:
	enum
	{	
		MAX_RATE_VALUE = 999999,
		MAX_EMULATE_VERSION = 176,
		MAX_ENABLED_TRANSLATE = 1,
		MAX_ENABLED_NW = 1,
		MAX_LEVEL = 105,
		MAX_LEVEL_2 = 41,
		MAX_REALM_LEVEL = 120,
		MAX_ENABLED_REFINE = 1,
		MAX_REFINE_LVL = 12,
		MAX_ENABLED_CODEX = 1,
		MAX_ENABLED_CAMP_TOKEN = 1,
		MAX_ENABLED_CHILD = 1,
		MAX_ENABLED_RANKING = 1,
		MAX_ENABLED_NEWHISTORY = 1,
		MAX_ENABLED_NEW_DUNGEONS = 1,
		MAX_BOSS_RUSH_MUTIPLE = 100,
		MAX_ENABLED_REWARD_GOLD = 1,
		MAX_ENABLED_MEMORIAL_RESET = 1,
		MAX_ENABLED_BLATTE_PASS = 1,
		MAX_MSG_LANGUAGE = 2,
		MAX_ENABLED_ARENA = 1,
		MAX_ENABLED_KILLED_GLOBAL_MESSAGE = 1,
		MAX_ENABLED_EVENT_FORBID = 1,
		TODAY_COUNT_MAX_BATTLE = 100,
		MAX_TASK_COUNT = 100,
		MAX_CHILD_AWAKENING_DAYS = 30,
	};

	static EmulateSettings * instance;

private:

	struct RATES
	{
		int	mob_exp;	
		int	mob_sp;
		int	mob_money;
		int	mob_drop;
				
		int	task_exp;
		int	task_sp;
		int	task_money;
		
		int realm_exp;

		int pet_exp;
		int genie_exp;
	};

	struct NW
	{
		int enabled_team;
		int min_level_required_nw;
		int min_level2_required_nw;
		int min_realm_level_required_nw;
	};

private:
	// Config do emulador
	int emulate_version;
	int enabled_translation;

	RATES rate_config;
	NW nw_config;

	int enabled_refine_msg;
	int refine_msg_lvl_min;
	int msg_language;

	// Configurações padrão
	int enabled_codex;

	// Configurações personalizadas
	int enabled_camp_token;
	int enabled_child;
	int child_awakening_days;
	int enabled_ranking;
	int enabled_newhistory;
	int enabled_new_dungeons;
	int boss_rush_mutiple;

	int enabled_reward_gold;
	int enabled_memorial_reset;
	int enabled_blatte_pass;
	int enabled_arena;
	int killed_global_message;
	int enabled_event_forbid;

	// New Arena
	int totay_count_max_battle;

	// Config de Missões
	int task_default_count;
	int task_expand_count;
	int task_hidden_count;
	int task_title_count;


public:
	void Init();
	void SetRatesConfig();
	RATES * GetRatesConfig() { return &rate_config; };
	inline int GetEmulateVersion() { return emulate_version; }
	inline bool GetEnabledTranslation() { return (char)enabled_translation; }
	NW * GetNwConfig() { return &nw_config; };

	inline bool GetEnabledRefineMsg() { return (char)enabled_refine_msg; }
	inline int GetRefineMsgLvLMin() { return refine_msg_lvl_min; }
	inline int GetMsgLanguage() { return msg_language; }

	inline bool GetEnabledCodex() { return (char)enabled_codex; }
	inline bool GetEnabledCampToken() { return (char)enabled_camp_token; }
	
	inline bool GetEnabledChild() { return (char)enabled_child; }
	inline int GetChildAwakeningDays() { return child_awakening_days; }
	
	inline bool GetEnabledRanking() { return (char)enabled_ranking; }
	inline bool GetEnabledNewHistory() { return (char)enabled_newhistory; }
	inline bool GetEnabledNewDungeons() { return (char)enabled_new_dungeons; }
	inline int GetBossRushMutiple() { return boss_rush_mutiple; }
	inline bool GetEnabledRewardGold() { return (char)enabled_reward_gold; }
	inline bool GetEnabledMemorialReset() { return (char)enabled_memorial_reset; }
	inline bool GetEnabledBlattePass() { return (char)enabled_blatte_pass; }
	inline bool GetEnabledArena() { return (char)enabled_arena; }
	inline bool GetEnabledKilledMsg() { return (char)killed_global_message; }
	inline bool GetEnabledEventForbid() { return (char)enabled_event_forbid; }
	inline int GetTodayCountMaxBattle() { return totay_count_max_battle; }

	inline int GetTaskDefaultCount() { return task_default_count; }
	inline int GetTaskExpandCount() { return task_expand_count; }
	inline int GetTaskHiddenCount() { return task_hidden_count; }
	inline int GetTaskTitleCount() { return task_title_count; }


EmulateSettings()
{

}

~EmulateSettings()
{
	
}

static EmulateSettings * GetInstance()
{
	if (!instance)
	{
		instance = new EmulateSettings();
	}
	return instance;
}

};


#endif