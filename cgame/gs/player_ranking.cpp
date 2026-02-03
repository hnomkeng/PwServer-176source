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
#include "player_ranking.h"

void 
gplayer_ranking::Init()
{
	memset(this,0x00,sizeof(gplayer_ranking));
}

void 
gplayer_ranking::SetPlayerRanking(short level, unsigned int exp, short count_kills, short count_deads)
{
	if(level <= 0) return;
	
	if(exp <= 0) exp = 1;
	if(count_kills <= 0) count_kills = 0;
	if(count_deads <= 0) count_deads = 0;

	player_ranking.level = level;
	player_ranking.exp = exp;
	player_ranking.count_kills = count_kills;
	player_ranking.count_deads = count_deads;
}

void 
gplayer_ranking::ResetKillCount()
{
    for (int i = 0; i < MAX_PLAYERS_TO_TRACK; ++i)
    {
        last_killed_ids[i] = -1;
        kill_count[i] = 0;
    }
    last_killed_index = 0;
}

bool 
gplayer_ranking::CheckForFreeKill(int killed_id)
{
    time_t current_time = time(nullptr);

    for (int i = 0; i < MAX_PLAYERS_TO_TRACK; ++i)
    {
        if (killed_id == last_killed_ids[i])
        {
            if (current_time - last_kill_time < RankingManager::GetInstance()->TimeIntervalForFreeKill())
            {
                kill_count[i]++;

                if (kill_count[i] >= RankingManager::GetInstance()->MaxKillsInInterval())
                {
                    ResetKillCount();
                    return true;
                }
            }
            else
            {
                ResetKillCount();
            }
        }
    }

    last_killed_index = (last_killed_index + 1) % MAX_PLAYERS_TO_TRACK;
    last_killed_ids[last_killed_index] = killed_id;
    kill_count[last_killed_index] = 1;
    last_kill_time = current_time;

    return false;
}