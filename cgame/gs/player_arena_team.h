#ifndef __ONLINEGAME_GS_PLAYER_ARENA_TEAM_H__
#define __ONLINEGAME_GS_PLAYER_ARENA_TEAM_H__

#include <uchar.h>
#include <db_if.h>
#include <gsp_if.h>
#include "luamanager.h"
#include <openssl/md5.h>


class gplayer_arena_team
{
public:
	enum 
	{
		MAX_TEAM_COUNT = 512,
		REWARD_DEFAULT_ID = 53423,
		REWARD_WIN_COUNT = 270,
		REWARD_LOSE_COUNT = 135,
		REWARD_WEEK_COUNT = 5000,
		REWARD_DEFAULT_SEASON = 1,
	};

public:
	
	struct ARENA_TEAM_STORAGE
    {
        struct MEMBER_INFO
        {
            int player_id;
            int is_leader;
            int cls;
            int patent_level;
            int score;
            char player_name[16];
        };

        int team_id;
        int leader_id;
        int team_score;
        int team_win;
        int team_battle;
        int team_battle_week;
        char team_name[16];

        MEMBER_INFO members[3];

        int today_count_battle;
		int today_count_max_battle;
    };

	struct ARENA_REWARD
	{
		int reward_id;
		int reward_date;
		int reward_count;
		int reward_season;
	};

	ARENA_TEAM_STORAGE _my_arena_team;
	ARENA_REWARD _my_arena_reward_week;
	ARENA_REWARD _my_arena_reward_season;

	
public:
	
	void Init();
	inline void SetTeamInfo(gplayer_arena_team::ARENA_TEAM_STORAGE team)
	{
		_my_arena_team = team;
	}

	inline void SetRewardWeek(gplayer_arena_team::ARENA_REWARD reward)
	{
		_my_arena_reward_week = reward;
	}

	inline void SetRewardSeason(gplayer_arena_team::ARENA_REWARD reward)
	{
		_my_arena_reward_season = reward;
	}

	inline void GetTeamInfo(gplayer_arena_team::ARENA_TEAM_STORAGE &team)
	{
		team = _my_arena_team;
	}
	inline void GetRewardWeek(gplayer_arena_team::ARENA_REWARD &reward)
	{
		reward = _my_arena_reward_week;
	}

	inline void GetRewardSeason(gplayer_arena_team::ARENA_REWARD &reward)
	{
		reward = _my_arena_reward_season;
	}
};


#endif