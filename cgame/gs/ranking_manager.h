#ifndef __GNET_RANKING_MANAGER_H
#define __GNET_RANKING_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>

class RankingManager
{
public:
	enum
	{	
		MAX_RANKING_TYPE = 9,
		MAX_MAPS_ENABLED = 20,
	};

	static RankingManager * instance;

private:

	struct ELOS_CONFIG
	{
		const char * elo_name;
		int elo_required_level;
		int elo_required_level_2;
		int elo_exp_min;
		int elo_exp_max;
		int elo_item_reward_id;
		int elo_item_reward_count;
		int elo_killed_exp_lose;
		int elo_killer_exp_gain;
	};

private:

	int log_enable;
	int elos_count;
	int ranking_day_reward;
	int cooldown_killed_sec;

	int cooldown_enable;
	int hwid_enable;

	ELOS_CONFIG elos[MAX_RANKING_TYPE];
	int maps_enabled[MAX_MAPS_ENABLED];

	int enable_free_kill;
	int max_kills_in_interval;
	int time_interval_for_free_kill;

public:
	void Init();

	inline bool GetLogEnable () { return log_enable; }
	inline int ElosCount () { return elos_count; }
	inline int RankingDayReward () { return ranking_day_reward; }
	inline int CooldownKilledSec () { return cooldown_killed_sec; }

	inline int CooldownEnable () { return cooldown_enable; }
	inline int HwidEnable () { return hwid_enable; }

	ELOS_CONFIG * GetElos(int pos) { return &elos[pos]; };

	inline int EnableFreeKill () { return enable_free_kill; }
	inline int MaxKillsInInterval () { return max_kills_in_interval; }
	inline int TimeIntervalForFreeKill () { return time_interval_for_free_kill; }
	
RankingManager()
{

}

~RankingManager()
{
	
}

static RankingManager * GetInstance()
{
	if (!instance)
	{
		instance = new RankingManager();
	}
	return instance;
}

};


#endif