#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "ranking_manager.h"
#include "gsp_if.h"

#include <glog.h>

RankingManager* RankingManager::instance = NULL;

void RankingManager::Init()
{	
	memset(this,0x00,sizeof(RankingManager));	

	const char * conf_file = "ranking.conf";
	Conf *rankingconf = Conf::GetInstance(conf_file);	

	//[CONFIG]
	log_enable = atoi(rankingconf->find("CONFIG","log_enable").c_str());
	elos_count = atoi(rankingconf->find("CONFIG","elos_count").c_str());
	ranking_day_reward = atoi(rankingconf->find("CONFIG","ranking_day_reward").c_str());
	cooldown_killed_sec = atoi(rankingconf->find("CONFIG","cooldown_killed_sec").c_str());

    //[FILTERS]

	cooldown_killed_sec = atoi(rankingconf->find("FILTERS","cooldown_killed_sec").c_str());
    cooldown_enable = atoi(rankingconf->find("FILTERS","enable_cooldown").c_str());
    hwid_enable = atoi(rankingconf->find("FILTERS","enable_hwid").c_str());
    enable_free_kill = atoi(rankingconf->find("FILTERS","enable_free_kill").c_str());
    max_kills_in_interval = atoi(rankingconf->find("FILTERS","max_kills_in_interval").c_str());
    time_interval_for_free_kill = atoi(rankingconf->find("FILTERS","time_interval_for_free_kill").c_str());
    
	//[ELO_%d]
    for (int i = 0; i < elos_count && i < MAX_RANKING_TYPE; i++)
    {
        char section[20];
        snprintf(section, sizeof(section), "ELO_%d", i + 1);

        elos[i].elo_name = strdup(rankingconf->find(section, "elo_name").c_str());
        elos[i].elo_required_level = atoi(rankingconf->find(section, "elo_required_level").c_str());
        elos[i].elo_required_level_2 = atoi(rankingconf->find(section, "elo_required_level_2").c_str());
        elos[i].elo_exp_min = atoi(rankingconf->find(section, "elo_exp_min").c_str());
        elos[i].elo_exp_max = atoi(rankingconf->find(section, "elo_exp_max").c_str());
        elos[i].elo_item_reward_id = atoi(rankingconf->find(section, "elo_item_reward_id").c_str());
        elos[i].elo_item_reward_count = atoi(rankingconf->find(section, "elo_item_reward_count").c_str());
        elos[i].elo_killed_exp_lose = atoi(rankingconf->find(section, "elo_killed_exp_lose").c_str());
        elos[i].elo_killer_exp_gain = atoi(rankingconf->find(section, "elo_killer_exp_gain").c_str());
    }   

}