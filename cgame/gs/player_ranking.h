#ifndef __PLAYER_RANKING_H__
#define __PLAYER_RANKING_H__

#include "ranking_manager.h"

class gplayer_ranking
{
public:
	enum 
	{
		MAX_PLAYERS_TO_TRACK = 3,		
	};
public:	
	struct RANKING
	{
		short level;
		unsigned int exp;
		short count_kills;
		short count_deads;
		char lock_unlock;
	};
private:	
	RANKING player_ranking;
	time_t last_kill_time;
    int last_killed_ids[MAX_PLAYERS_TO_TRACK];
    int kill_count[MAX_PLAYERS_TO_TRACK];
    int last_killed_index;
	
public:	
	void Init();
	inline RANKING * GetPlayerRanking() { return &player_ranking; };
	void SetPlayerRanking(short level, unsigned int exp, short count_kills, short count_deads);
	inline void LockUnlockRanking(bool b) { player_ranking.lock_unlock = b; };
	bool CheckForFreeKill(int killer_id);
	void ResetKillCount();
};


#endif