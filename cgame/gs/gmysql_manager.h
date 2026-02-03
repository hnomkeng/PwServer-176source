#ifndef __GNET_GMYSQL_MANAGER_H
#define __GNET_GMYSQL_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <string>
#include <future>
#include <utility>

using namespace GNET;

class GMysqlManager
{
private:
	enum : size_t
	{
        //TODO
	};

	string host;
	string user;
	string password;
	string db;
	unsigned int port;

	bool enabled_log;
	bool enabled_gmysql;

public:
    enum OperationType { ADD = 1, REMOVE = 2 };

#pragma pack(push, 1)
	struct DB_RANKING
    {
        int player_id;
        int player_cls;
        int player_gender;
        int player_exp_elo;
        int player_kill_elo;
        int player_death_elo;
        int role_patent_kills;
        int role_patent_deaths;
        int player_lvl_elo;
        float kda;
        short name_len;
        char player_name[16];
    };
#pragma pack(pop)

    struct DB_ARENA_TEAM
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

    struct DB_TEAM_INFO
    {
        int leader_id;
        int team_win;
        int team_score;
        char leader_name[16];
        char team_name[16];
    };

    struct DB_ARENA_PLAYER
    {
        int player_id;
        int team_id;
        int cls;
        int score;
        int win_count;
        int team_score;
        int week_battle_count;
        int invite_time;
        int last_visite;
        int battle_count;
        char name[64];
    };

public:
	void Init();
	bool Connect();
	void Disconnect();
    bool GetEnabledLog() { return enabled_log; }

	// Patentes
    std::future<void> AddOrUpdatePatentePlayer(int player_id, const std::string &player_name, int player_cls, int player_gender, int player_lvl_elo, int player_exp_elo, int player_kill_elo, int player_death_elo, float kda);
    std::future<void> GetRankingData(int type, int& count_ranking, std::vector<GMysqlManager::DB_RANKING>& rankingData);

    // Gold por Hora
    std::future<void> TruncateCashRewardTable();
    std::future<void> AddPlayerHwid(int player_id, unsigned long long player_hwid);
    std::future<void> RemovePlayerHwid(int player_id);
    std::future<std::pair<bool, int>> CheckPlayerHwid(unsigned long long player_hwid);

    // Bloqueador Evento PvP
    std::future<void> TruncateeEventForbidTable();
    std::future<void> AddPlayerEventForbid(int player_id, unsigned long long player_hwid, int event_type /* NW = 1, ARTES = 2, ARENA = 3*/);
    std::future<void> RemovePlayerEventForbid(int player_id);
    std::future<std::pair<bool, int>> CheckPlayerEventForbid(unsigned long long player_hwid);

    // New Arena
    std::future<std::pair<bool, int>> CheckPlayerArenaTeam(int player_id);
    std::future<std::pair<bool, int>> CheckNameArenaTeam(const char name[], size_t name_len);
    std::future<void> CreateArenaTeam(int team_id, int leader_id, int cls, int patent, const char team_name[], size_t team_name_len);
    std::future<std::pair<bool, int>> AddRemoveMember(int player_id, int member_id, const char* member_name, int member_cls, int member_patent, OperationType op);
    std::future<void> GetArenaTeamInfoByLeaderOrMember(int leader_or_member_id, DB_ARENA_TEAM& arenaTeam);    
    std::future<void> SetArenaTeamInfo(const GMysqlManager::DB_ARENA_TEAM& arenaTeam);
    std::future<std::pair<bool, int>> CheckPlayerNewInviter(int player_id, int cls);
    std::future<int> GetTeamLeader(int player_id);
    std::future<bool> RemoveArenaTeam(int team_id);
    std::future<bool> KickArenaMember(int leader_id, int pos);
    std::future<void> GetArenaRankingData(int &teamCount, std::vector<GMysqlManager::DB_TEAM_INFO> &teamData);
    std::future<void> ResetDailyArenaPoints();
    std::future<void> ResetWeeklyArenaPoints();
    std::future<void> AddOrUpdateArenaPlayer(const GMysqlManager::DB_ARENA_PLAYER &player);
    std::future<bool> CheckAndResetDailyArenaPoints(int player_id, int tm_day); 
    std::future<bool> CheckAndResetWeekArenaPoints(int player_id, int tm_day); 
    
	static GMysqlManager * GetInstance()
	{
		if (!instance)
		instance = new GMysqlManager();
		return instance;
	}
	static GMysqlManager * instance;
};


#endif
