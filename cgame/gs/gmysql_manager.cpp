#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "gmysql_manager.h"
#include "gsp_if.h"
#include <vector>
#include <glog.h>
#include "player_imp.h"
#include "emulate_settings.h"
#include <mysql++.h>

using namespace GNET;

GMysqlManager* GMysqlManager::instance = NULL;
std::mutex mysql_mutex;
mysqlpp::Connection conn(false);

class MySQLConnectionGuard 
{
public:
    explicit MySQLConnectionGuard(mysqlpp::Connection& conn) : conn_(conn) 
    {
        if (!conn_.connected()) 
        {
            throw std::runtime_error("Connection not established");
        }
    }

    ~MySQLConnectionGuard() 
    {
        if (conn_.connected()) 
        {
            conn_.disconnect();            
        }
    }

private:
    mysqlpp::Connection& conn_;
};

void 
GMysqlManager::Init()
{
	memset(this,0x00,sizeof(GMysqlManager));	

	const char * conf_file = "gmysql.conf";
	Conf *gmysqlconf = Conf::GetInstance(conf_file);	

	enabled_gmysql = atoi(gmysqlconf->find("GMysqlClient","enabled_mysql").c_str());
	enabled_log = atoi(gmysqlconf->find("GMysqlClient","enabled_log").c_str());

	host = gmysqlconf->find("GMysqlClient","address").c_str();
	user = gmysqlconf->find("GMysqlClient","user").c_str();
	password = gmysqlconf->find("GMysqlClient","passwd").c_str();
	db = gmysqlconf->find("GMysqlClient","name").c_str();
	port = atoi(gmysqlconf->find("GMysqlClient","port").c_str());	
   
}

bool GMysqlManager::Connect() 
{
    if (!enabled_gmysql) 
    {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::Connect() GMysqlManager is not enabled");
        }
        return false;
    }

    try {
        if (!conn.connected()) 
        {
            if (!conn.connect(db.c_str(), host.c_str(), user.c_str(), password.c_str(), port)) 
            {
                if (enabled_log) 
                {
                    GLog::log(GLOG_ERR, "GMysqlManager::Connect() Failed to connect to MySQL");
                }
                return false;
            }
        }

        if (enabled_log) 
        {
            GLog::log(GLOG_INFO, "GMysqlManager::Connect() Connected to MySQL");
        }
        return true;

    } catch (const mysqlpp::Exception& e) 
    {
        if (enabled_log) 
        {
            GLog::log(GLOG_ERR, "GMysqlManager::Connect() MySQL connection exception: %s", e.what());
        }
        return false;
    }
}

std::future<void> GMysqlManager::AddOrUpdatePatentePlayer(
    int player_id,
    const std::string &player_name,
    int player_cls,
    int player_gender,
    int player_lvl_elo,
    int player_exp_elo,
    int player_kill_elo,
    int player_death_elo,
    float kda)
{
    if (!enabled_gmysql) {
        if (enabled_log)
            GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdatePatentePlayer GMysqlManager is not enabled");
        return std::async([]() {});
    }

    return std::async(std::launch::async, 
    [this, player_id, player_name, player_cls, player_gender, player_lvl_elo, player_exp_elo, player_kill_elo, player_death_elo, kda]() 
    {
        try {
           
           std::lock_guard<std::mutex> lock(mysql_mutex);

            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdatePatentePlayer Connection failed");
                }
                return;
            }

            mysqlpp::Query checkQuery = conn.query("SELECT * FROM patente_players WHERE player_id = %d");
            checkQuery.parse();
            auto result = checkQuery.store(player_id);

            if (result.num_rows() > 0) { 
                mysqlpp::Query updateQuery = conn.query();
                updateQuery << "UPDATE patente_players SET "
                            << "player_name = '" << player_name << "', "
                            << "player_cls = " << player_cls << ", "
                            << "player_gender = " << player_gender << ", "
                            << "player_lvl_elo = " << player_lvl_elo << ", "
                            << "player_exp_elo = " << player_exp_elo << ", "
                            << "player_kill_elo = " << player_kill_elo << ", "
                            << "player_death_elo = " << player_death_elo << ", "
                            << "kda = " << kda
                            << " WHERE player_id = " << player_id;
                updateQuery.execute();

                if(enabled_log)
                {
                    GLog::log(GLOG_INFO, "GMysqlManager::AddOrUpdatePatentePlayer Player updated successfully");
                }
            } else { 
                mysqlpp::Query insertQuery = conn.query();
                insertQuery << "INSERT INTO patente_players (player_id, player_name, player_cls, player_gender, player_lvl_elo, player_exp_elo, player_kill_elo, player_death_elo, kda) VALUES ("
                            << player_id << ", '"
                            << player_name << "', "
                            << player_cls << ", "
                            << player_gender << ", "
                            << player_lvl_elo << ", "
                            << player_exp_elo << ", "
                            << player_kill_elo << ", "
                            << player_death_elo << ", "
                            << kda << ")";
                insertQuery.execute();

                if(enabled_log)
                {
                    GLog::log(GLOG_INFO, "GMysqlManager::AddOrUpdatePatentePlayer Player added successfully");
                }
            }
        } catch (const mysqlpp::BadQuery& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdatePatentePlayer Error adding or updating player: %s", e.what());
            }
        } catch (...) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdatePatentePlayer Unexpected exception when adding or updating player");
            }
        }
    });
}


std::future<void> GMysqlManager::GetRankingData(
    int type,
    int& count_ranking,
    std::vector<GMysqlManager::DB_RANKING>& rankingData)
{
    if (!enabled_gmysql) {
        if (enabled_log)
            GLog::log(GLOG_ERR, "GMysqlManager::GetRankingData GMysqlManager is not enabled");
        return std::async([]() {});
    }

    return std::async(std::launch::async, [this, type, &count_ranking, &rankingData]() 
    {
        try {

            std::lock_guard<std::mutex> lock(mysql_mutex);

            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::GetRankingData Connection failed");
                }
                return;
            }

            mysqlpp::Query selectQuery = conn.query();
            
            if (type == 0) {
                selectQuery << "SELECT * FROM patente_players ORDER BY kda DESC";
            } else if (type > 0 && type < 10) {
                selectQuery << "SELECT * FROM patente_players WHERE player_lvl_elo = " << type << " ORDER BY kda DESC";
            } else {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "Invalid ranking type: %d", type);
                return;
            }

            auto result = selectQuery.store();

            if (result.empty()) {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "GMysqlManager::GetRankingData No ranking data found");
               return;
            }

            for (const auto& row : result) {
                GMysqlManager::DB_RANKING playerData;
                playerData.player_id = row["player_id"];
                playerData.player_cls = row["player_cls"];
                playerData.player_gender = row["player_gender"];
                playerData.player_exp_elo = row["player_exp_elo"];
                playerData.player_kill_elo = row["player_kill_elo"];
                playerData.player_death_elo = row["player_death_elo"];
                playerData.player_lvl_elo = row["player_lvl_elo"];
                playerData.kda = row["kda"];
                strncpy(playerData.player_name, row["player_name"].c_str(), sizeof(playerData.player_name) - 1);
                playerData.player_name[sizeof(playerData.player_name) - 1] = '\0';

                rankingData.push_back(playerData);
            }

            count_ranking = rankingData.size();

            if (enabled_log)
            {
                GLog::log(GLOG_INFO, "GMysqlManager::GetRankingData Ranking data retrieved successfully");
            }
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log)
                GLog::log(GLOG_ERR, "GMysqlManager::GetRankingData Failed to get ranking data: %s", e.what());
        }
    });
}

std::future<void> GMysqlManager::TruncateCashRewardTable() 
{
    if (!enabled_gmysql) {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::TruncateCashRewardTable GMysqlManager is not enabled");
        }
        return std::async([]() {});
    }

    return std::async(std::launch::async, [this]() 
    {
        try {

            std::lock_guard<std::mutex> lock(mysql_mutex);

            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::TruncateCashRewardTable Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query("TRUNCATE TABLE cash_reward");

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::TruncateCashRewardTable Successfully truncated cash_reward table");
            }
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::TruncateCashRewardTable Error executing truncate query for cash_reward table: %s", e.what());
            }
        }
    });
}


std::future<void> GMysqlManager::AddPlayerHwid(int player_id, unsigned long long player_hwid) 
{
    if (!enabled_gmysql) {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid GMysqlManager is not enabled");
        }
        return std::async([]() {});
    }

    return std::async(std::launch::async, [this, player_id, player_hwid]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);

            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();

            query << "INSERT INTO cash_reward (player_id, player_hwid) VALUES ("
                  << player_id << ", " << player_hwid << ")";

            query.execute(); 

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::AddPlayerHwid Successfully inserted player HWID");
            }
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid Error executing insert query for adding player HWID: %s", e.what());
            }
        }
    });
}

std::future<void> GMysqlManager::RemovePlayerHwid(int player_id) 
{
    if (!enabled_gmysql) {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid GMysqlManager is not enabled");
        }
        return std::async([]() {});
    }

    return std::async(std::launch::async, [this, player_id]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();

            query << "DELETE FROM cash_reward WHERE player_id = " << player_id;

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::AddPlayerHwid Successfully deleted player HWID");
            }
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerHwid Error executing delete query to remove player HWID: %s", e.what());
            }
        }
    });
}

std::future<std::pair<bool, int>> GMysqlManager::CheckPlayerHwid(unsigned long long player_hwid) 
{
    if (!enabled_gmysql) {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerHwid GMysqlManager is not enabled");
        }
        return std::async([]() { return std::make_pair(false, -1); });
    }

    return std::async(std::launch::async, [this, player_hwid]() 
    {
        bool playerFound = false;
        int player_id = -1;

        try {

            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerHwid Connection failed");
                }
                return std::make_pair(false, -1);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT player_id FROM cash_reward WHERE player_hwid = '" << player_hwid << "'";

            mysqlpp::StoreQueryResult result = query.store();

            if (result && !result.empty()) {
                playerFound = true;
                player_id = result[0]["player_id"];
            }

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::CheckPlayerHwid Player HWID checked successfully");
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerHwid Error executing query to check player HWID: %s", e.what());
            }
        }

        return std::make_pair(playerFound, player_id);
    });
}

std::future<void> GMysqlManager::TruncateeEventForbidTable() 
{
    if (!enabled_gmysql) {
        if (enabled_log) {
            GLog::log(GLOG_ERR, "GMysqlManager::TruncateeEventForbidTable GMysqlManager is not enabled");
        }
        return std::async([]() {});
    }

    return std::async(std::launch::async, [this]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::TruncateeEventForbidTable Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "TRUNCATE TABLE event_forbid";

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::TruncateeEventForbidTable Truncated event_forbid table successfully");
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::TruncateeEventForbidTable Error truncating event_forbid table: %s", e.what());
            }
        }
    });
}

std::future<void> GMysqlManager::AddPlayerEventForbid(int player_id, unsigned long long player_hwid, int event_type) 
{
    return std::async(std::launch::async, [this, player_id, player_hwid, event_type]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerEventForbid Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "INSERT INTO event_forbid (player_id, player_hwid, event_type) VALUES ("
                  << player_id << ", " << player_hwid << ", " << event_type << ")";

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::AddPlayerEventForbid Player event forbid added successfully");
            }
            
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddPlayerEventForbid Failed to execute insert query for player event forbid: %s", e.what());
            }
        }
    });
}

std::future<void> GMysqlManager::RemovePlayerEventForbid(int player_id) 
{
    return std::async(std::launch::async, [this, player_id]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::RemovePlayerEventForbid Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "DELETE FROM event_forbid WHERE player_id = " << player_id;

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::RemovePlayerEventForbid Player event forbid removed successfully");
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::RemovePlayerEventForbid Failed to execute delete query for player event forbid: %s", e.what());
            }
        }
    });
}

std::future<std::pair<bool, int>> GMysqlManager::CheckPlayerEventForbid(unsigned long long player_hwid) 
{
    return std::async(std::launch::async, [this, player_hwid]() 
    {
        bool playerFound = false;
        int event_type = -1;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);

            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerEventForbid Connection failed");
                }
                return std::make_pair(false, -1);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT event_type FROM event_forbid WHERE player_hwid = '" << player_hwid << "'";

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckPlayerEventForbid No player event forbid found for given HWID");
                }
            } else {
                playerFound = true;
                event_type = result[0]["event_type"];
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerEventForbid Error executing query to check player event forbid: %s", e.what());
            }
        }

        return std::make_pair(playerFound, event_type);
    });
}

// New Arena
std::future<std::pair<bool, int>> GMysqlManager::CheckPlayerArenaTeam(int player_id) 
{
    return std::async(std::launch::async, [this, player_id]() 
    {
        bool playerFound = false;
        int team_id = -1;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerArenaTeam Connection failed");
                }
                return std::make_pair(false, -1);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT team_id FROM newarena WHERE leader_id = " << player_id
                  << " OR member_id_0 = " << player_id
                  << " OR member_id_1 = " << player_id
                  << " OR member_id_2 = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (!result.empty()) {
                playerFound = true;
                team_id = result[0]["team_id"];
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckPlayerArenaTeam Player found in team with team_id: %d", team_id);
                }
            } else {
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckPlayerArenaTeam Player not found in any team");
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerArenaTeam Error executing query to check if player is in teams: %s", e.what());
            }
        }

       
        return std::make_pair(playerFound, team_id);
    });
}

std::future<std::pair<bool, int>> GMysqlManager::CheckNameArenaTeam(const char name[], size_t name_len) 
{
    return std::async(std::launch::async, [this, name, name_len]() 
    {
        bool teamFound = false;
        int team_id = -1;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckNameArenaTeam Connection failed");
                }
                return std::make_pair(false, -1);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT team_id FROM newarena WHERE team_name = '";
            query << mysqlpp::escape << std::string(name, name_len);
            query << "'";

            mysqlpp::StoreQueryResult result = query.store();

            if (!result.empty()) {
                teamFound = true;
                team_id = result[0]["team_id"];
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckNameArenaTeam Team found with team_id: %d", team_id);
                }
            } else {
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckNameArenaTeam No team found with name: %s", std::string(name, name_len).c_str());
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckNameArenaTeam Error executing query to find team by name: %s", e.what());
            }
        }

        
        return std::make_pair(teamFound, team_id);
    });
}

std::future<void> GMysqlManager::CreateArenaTeam(int team_id, int leader_id, int cls, int patent, const char team_name[], std::size_t team_name_len) 
{
    return std::async(std::launch::async, [this, team_id, leader_id, cls, patent, team_name, team_name_len]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CreateArenaTeam Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "INSERT INTO newarena (team_id, leader_id, team_score, team_name, member_id_0, member_leader_0, member_cls_0, member_patent_0, member_name_0) VALUES (";
            query << team_id << ", ";
            query << leader_id << ", ";
            query << "1000, ";
            query << "'" << mysqlpp::escape << std::string(team_name, team_name_len) << "', ";
            query << leader_id << ", ";
            query << "1, ";
            query << cls << ", ";
            query << patent << ", ";

            LuaManager *lua = LuaManager::GetInstance();
            const char * player_name = lua->game__GetName(0, leader_id);

            query << "'" << mysqlpp::escape << std::string(player_name, 16) << "')";

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::CreateArenaTeam Arena team created successfully with team_id: %d", team_id);
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CreateArenaTeam Error executing query to create arena team: %s", e.what());
            }
        }

       
    });
}

std::future<void> GMysqlManager::GetArenaTeamInfoByLeaderOrMember(
    int leader_or_member_id,
    GMysqlManager::DB_ARENA_TEAM& arenaTeam)
{
    return std::async(std::launch::async, [this, leader_or_member_id, &arenaTeam]() 
    {
        try {
           
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::GetArenaTeamInfoByLeaderOrMember Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT * FROM newarena WHERE "
                  << "leader_id = " << leader_or_member_id
                  << " OR member_id_0 = " << leader_or_member_id
                  << " OR member_id_1 = " << leader_or_member_id
                  << " OR member_id_2 = " << leader_or_member_id;

            auto result = query.store();

            if (result.empty()) 
            {
                arenaTeam.team_id = 0;
                arenaTeam.leader_id = 0;
                memset(arenaTeam.team_name, 0, sizeof(arenaTeam.team_name));
                arenaTeam.team_score = 0;
                arenaTeam.team_win = 0;
                arenaTeam.team_battle = 0;
                arenaTeam.team_battle_week = 0;

                for (int i = 0; i < 3; ++i) {

                    arenaTeam.members[i].player_id = 0;
                    arenaTeam.members[i].is_leader = 0;
                    arenaTeam.members[i].cls = 0;
                    arenaTeam.members[i].patent_level = 0;
                    arenaTeam.members[i].score = 0;
                    memset(arenaTeam.members[i].player_name, 0, sizeof(arenaTeam.members[i].player_name));
                }

                arenaTeam.today_count_battle = 0;
                arenaTeam.today_count_max_battle = 0;
                return;
            }

            const mysqlpp::Row& row = result[0];

            arenaTeam.team_id = row["team_id"];
            arenaTeam.leader_id = row["leader_id"];
            arenaTeam.team_score = row["team_score"];
            arenaTeam.team_win = row["team_wins"];
            arenaTeam.team_battle = row["team_battle_total"];
            arenaTeam.team_battle_week = row["team_battle_week"];            
            memcpy(arenaTeam.team_name, row["team_name"], sizeof(arenaTeam.team_name));

            for (int i = 0; i < 3; i++) 
            {
                std::string memberIdField = "member_id_" + std::to_string(i);
                std::string memberLeaderField = "member_leader_" + std::to_string(i);
                std::string memberScoreField = "member_score_" + std::to_string(i);
                std::string memberNameField = "member_name_" + std::to_string(i);
                std::string memberClsField = "member_cls_" + std::to_string(i);
                std::string memberPatentField = "member_patent_" + std::to_string(i);

                if (atoi(row[memberIdField.c_str()]) > 0) 
                {
                    arenaTeam.members[i].is_leader = atoi(row[memberLeaderField.c_str()]);
                    arenaTeam.members[i].score = atoi(row[memberScoreField.c_str()]);
                    arenaTeam.members[i].player_id = atoi(row[memberIdField.c_str()]);
                    arenaTeam.members[i].cls = atoi(row[memberClsField.c_str()]);
                    arenaTeam.members[i].patent_level = atoi(row[memberPatentField.c_str()]);
                    memcpy(arenaTeam.members[i].player_name, row[memberNameField.c_str()], sizeof(arenaTeam.members[i].player_name));
                } else 
                {
                    arenaTeam.members[i].is_leader = 0;
                    arenaTeam.members[i].score = 0;
                    arenaTeam.members[i].player_id = 0;
                    arenaTeam.members[i].cls = 0;
                    arenaTeam.members[i].patent_level = 0;
                    memset(arenaTeam.members[i].player_name, 0, sizeof(arenaTeam.members[i].player_name));
                }
            }

            arenaTeam.today_count_battle = row["today_count_battle"];
            arenaTeam.today_count_max_battle = EmulateSettings::GetInstance()->GetTodayCountMaxBattle();

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::GetArenaTeamInfoByLeaderOrMember Error retrieving arena team info: %s", e.what());
            }
        }

    });
}

std::future<void> GMysqlManager::SetArenaTeamInfo(const GMysqlManager::DB_ARENA_TEAM& arenaTeam) 
{
    return std::async(std::launch::async, [this, arenaTeam]() 
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::SetArenaTeamInfo Connection failed");
                }
                return;
            }
            mysqlpp::Query query = conn.query();

            query << "SELECT * FROM newarena WHERE team_id = " << arenaTeam.team_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                query.reset();
                query << "INSERT INTO newarena (team_id, leader_id, team_score, team_wins, "
                      << "team_battle_total, team_battle_week, today_count_battle, team_name, "
                      << "member_id_0, member_name_0, member_score_0, member_wins_0, member_leader_0, member_cls_0, member_patent_0, "
                      << "member_id_1, member_name_1, member_score_1, member_wins_1, member_leader_1, member_cls_1, member_patent_1, "
                      << "member_id_2, member_name_2, member_score_2, member_wins_2, member_leader_2, member_cls_2, member_patent_2) "
                      << "VALUES (" << arenaTeam.team_id << ", "
                      << arenaTeam.leader_id << ", "
                      << arenaTeam.team_score << ", "
                      << arenaTeam.team_win << ", "
                      << arenaTeam.team_battle << ", "
                      << arenaTeam.team_battle_week << ", "
                      << arenaTeam.today_count_battle << ", '"
                      << std::string(arenaTeam.team_name, sizeof(arenaTeam.team_name) - 1) << "', ";

                for (int i = 0; i < 3; ++i) {
                    query << arenaTeam.members[i].player_id << ", '"
                          << std::string(arenaTeam.members[i].player_name, sizeof(arenaTeam.members[i].player_name) - 1) << "', "
                          << arenaTeam.members[i].score << ", "
                          << arenaTeam.team_win << ", "
                          << arenaTeam.members[i].is_leader << ", "
                          << arenaTeam.members[i].cls << ", "
                          << arenaTeam.members[i].patent_level << ", ";
                }

                query.seekp(-2, std::ios_base::end);
                query << ")";

                query.execute();

            } else {
                query.reset();
                query << "UPDATE newarena SET "
                      << "leader_id = " << arenaTeam.leader_id << ", "
                      << "team_score = " << arenaTeam.team_score << ", "
                      << "team_wins = " << arenaTeam.team_win << ", "
                      << "team_battle_total = " << arenaTeam.team_battle << ", "
                      << "team_battle_week = " << arenaTeam.team_battle_week << ", "
                      << "today_count_battle = " << arenaTeam.today_count_battle << ", "
                      << "team_name = '"
                      << std::string(arenaTeam.team_name, sizeof(arenaTeam.team_name) - 1) << "', ";

                for (int i = 0; i < 3; ++i) {
                    query << "member_id_" << i << " = " << arenaTeam.members[i].player_id << ", ";
                    query << "member_name_" << i << " = '"
                          << std::string(arenaTeam.members[i].player_name, sizeof(arenaTeam.members[i].player_name) - 1) << "', ";
                    query << "member_score_" << i << " = " << arenaTeam.members[i].score << ", ";
                    query << "member_wins_" << i << " = " << arenaTeam.team_win << ", ";
                    query << "member_cls_" << i << " = " << arenaTeam.members[i].cls << ", ";
                    query << "member_patent_" << i << " = " << arenaTeam.members[i].patent_level << ", ";
                }

                query.seekp(-2, std::ios_base::end);
                query << " WHERE team_id = " << arenaTeam.team_id;

                query.execute();
            }

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::SetArenaTeamInfo Successfully set arena team info for team_id: %d", arenaTeam.team_id);
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::SetArenaTeamInfo Error setting arena team info: %s", e.what());
            }
        }

    });
}

std::future<std::pair<bool, int>> GMysqlManager::CheckPlayerNewInviter(int player_id, int cls) 
{
    return std::async(std::launch::async, [this, player_id, cls]() 
    {
        int errorCode = 0;
        bool canAddPlayer = false;

        try {
           
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerNewInviter Connection failed");
                }
                return std::make_pair(false, 0);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT team_id, member_id_0, member_id_1, member_id_2, "
                  << "member_cls_0, member_cls_1, member_cls_2 "
                  << "FROM newarena "
                  << "WHERE leader_id = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) 
            {
                errorCode = 1;
                canAddPlayer = false;
            } else {
                const auto& row = result[0];
                int emptySlot = -1;

                if (atoi(row["member_id_0"]) == 0) {
                    emptySlot = 0;
                } else if (atoi(row["member_id_1"]) == 0) {
                    emptySlot = 1;
                } else if (atoi(row["member_id_2"]) == 0) {
                    emptySlot = 2;
                }

                if (emptySlot == -1) {
                    errorCode = 2;
                    canAddPlayer = false;
                } else {
                    
                    if (atoi(row["member_cls_0"]) == cls ||
                        atoi(row["member_cls_1"]) == cls ||
                        atoi(row["member_cls_2"]) == cls) {
                        errorCode = 3;
                        canAddPlayer = false;
                    } else {
                        canAddPlayer = true;
                    }
                }
            }
        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckPlayerNewInviter Erro ao verificar equipe de arena do jogador: %s", e.what());
            }
            return std::make_pair(false, -1);
        }

        return std::make_pair(canAddPlayer, errorCode);
    });
}

std::future<std::pair<bool, int>> GMysqlManager::AddRemoveMember(int player_id, int member_id, const char* member_name, int member_cls, int member_patent, OperationType op) 
{
    return std::async(std::launch::async, [this, player_id, member_id, member_name, member_cls, member_patent, op]() 
    {
        bool success = false;
        int resultType = -1;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Connection failed");
                }
                return std::make_pair(success, resultType);
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT team_id FROM newarena WHERE leader_id = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) 
            {
                if (op == OperationType::ADD) {
                    if (enabled_log) {
                        GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Cannot find team for leader_id: %d", player_id);
                    }
                    return std::make_pair(success, resultType);
                } else {
                    if (enabled_log) {
                        GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Cannot find team for leader_id: %d", player_id);
                    }                  
                    return std::make_pair(success, resultType);
                }
            }

            int team_id = result[0]["team_id"];

            if (op == OperationType::ADD) {

                query.reset();
                query << "SELECT * FROM newarena WHERE team_id = " << team_id;
                result = query.store();

                if (result.empty()) {
                    if (enabled_log) {
                        GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Team not found for team_id: %d", team_id);
                    }
                    return std::make_pair(success, resultType);
                }

                const auto& row = result[0];
                int emptySlot = -1;

                for (int i = 0; i < 3; ++i) {
                    std::string memberIdField = "member_id_" + std::to_string(i);
                    int memberId = std::atoi(row[memberIdField.c_str()]);
                    if (memberId == 0) {
                        emptySlot = i;
                        break;
                    }
                }

                if (emptySlot == -1) {
                    if (enabled_log) {
                        GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember No space to add new member in team_id: %d", team_id);
                    }
                    return std::make_pair(success, resultType);
                }

                query.reset();
                query << "UPDATE newarena SET ";
                query << "member_id_" << emptySlot << " = " << member_id;
                query << ", member_name_" << emptySlot << " = '" << member_name << "'";
                query << ", member_cls_" << emptySlot << " = " << member_cls;
                query << ", member_patent_" << emptySlot << " = " << member_patent;
                query << " WHERE team_id = " << team_id;

                query.execute();

                success = true;
                resultType = OperationType::ADD;

                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Added new member with id: %d to team_id: %d", member_id, team_id);
                }

            } else if (op == OperationType::REMOVE) 
            {

                query.reset();
                query << "SELECT * FROM newarena WHERE team_id = " << team_id;
                result = query.store();

                if (result.empty()) {
                    if (enabled_log) {
                        GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember No team found with team_id: %d", team_id);
                    }
                    return std::make_pair(success, resultType);
                }

                const auto& row = result[0];

                bool memberFound = false;

                for (int i = 0; i < 3; ++i) 
                {
                    std::string memberIdField = "member_id_" + std::to_string(i);
                    int memberId = std::atoi(row[memberIdField.c_str()]);

                     if (enabled_log) {
                            GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Could not find member with id: %d in team_id: %d", member_id, team_id);
                    }

                    if (memberId == member_id) 
                    {
                        query.reset();
                        query << "UPDATE newarena SET ";
                        query << "member_id_" << i << " = 0, ";
                        query << "member_name_" << i << " = NULL, ";
                        query << "member_cls_" << i << " = 0, ";
                        query << "member_patent_" << i << " = 0, ";
                        query << "member_wins_" << i << " = 0, ";
                        query << "member_score_" << i << " = 0 ";
                        query << "WHERE team_id = " << team_id;

                        query.execute();

                        success = true;
                        resultType = OperationType::REMOVE;

                        if (enabled_log) {
                            GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Removed member with id: %d from team_id: %d", member_id, team_id);
                        }

                        memberFound = true;
                        break;
                    }
                }

                if (!memberFound && enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Could not find member with id: %d in team_id: %d", member_id, team_id);
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddRemoveMember Error executing Add/Remove Member operation: %s", e.what());
            }
        }

        return std::make_pair(success, resultType);
    });
}

std::future<int> GMysqlManager::GetTeamLeader(int player_id) 
{
    return std::async(std::launch::async, [this, player_id]() 
    {
        int leader_id = -1;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::GetTeamLeader Connection failed");
                }
                return leader_id;
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT leader_id FROM newarena WHERE ";
            query << "member_id_0 = " << player_id << " OR ";
            query << "member_id_1 = " << player_id << " OR ";
            query << "member_id_2 = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (!result.empty()) {
                leader_id = result[0]["leader_id"];
            } else {
                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::GetTeamLeader No team found for player_id: %d", player_id);
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::GetTeamLeader Error retrieving leader_id: %s", e.what());
            }
        }

        return leader_id;
    });
}

std::future<bool> GMysqlManager::RemoveArenaTeam(int team_id)
{
    return std::async(std::launch::async, [this, team_id]()
    {
        bool success = false;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::RemoveArenaTeam Connection failed");
                }
                return success;
            }

            mysqlpp::Query query = conn.query();
            query << "DELETE FROM newarena WHERE team_id = " << team_id;

            query.execute();

            success = true;

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::RemoveArenaTeam Successfully removed team with team_id: %d", team_id);
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::RemoveArenaTeam Error executing Remove Arena Team operation: %s", e.what());
            }
        }

        return success; 
    });
}

std::future<bool> GMysqlManager::KickArenaMember(int leader_id, int pos) 
{
    return std::async(std::launch::async, [this, leader_id, pos]() 
    {
        bool success = false;

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember Connection failed");
                }
                return success;
            }

            mysqlpp::Query query = conn.query();

            query << "SELECT team_id FROM newarena WHERE leader_id = " << leader_id;
            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember No team found for leader_id: %d", leader_id);
                }
                return success;
            }

            int team_id = result[0]["team_id"];

            if (pos == 0) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember Cannot expel the leader from team_id: %d", team_id);
                }
                return success;
            }

            if (pos < 0 || pos > 2) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember Invalid position: %d for team_id: %d", pos, team_id);
                }
                return success;
            }

            query.reset();
            query << "UPDATE newarena SET ";
            query << "member_id_" << pos << " = 0, ";
            query << "member_name_" << pos << " = NULL, ";
            query << "member_cls_" << pos << " = 0, ";
            query << "member_patent_" << pos << " = 0, ";
            query << "member_score_" << pos << " = 0, ";
            query << "member_wins_" << pos << " = 0 ";
            query << "WHERE team_id = " << team_id;

            query.execute();

            success = true;

            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember Successfully expelled member at position: %d from team_id: %d", pos, team_id);
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::KickArenaMember Error executing Kick Arena Member operation: %s", e.what());
            }
        }

        return success;
    });
}

std::future<void> GMysqlManager::GetArenaRankingData(int &teamCount, std::vector<GMysqlManager::DB_TEAM_INFO> &teamData)
{
    return std::async(std::launch::async, [this, &teamData, &teamCount]()
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::GetArenaRankingData Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "SELECT leader_id, team_score, team_wins, team_name, member_name_0 "
                     "FROM newarena "
                     "ORDER BY team_score DESC "
                     "LIMIT 11";

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::GetArenaRankingData No team data found");
                }
                return;
            }

            teamData.clear();

            for (const auto& row : result) 
            {
                GMysqlManager::DB_TEAM_INFO teamInfo;
                teamInfo.leader_id = row["leader_id"];
                teamInfo.team_score = row["team_score"];
                teamInfo.team_win = row["team_wins"];
                memcpy(teamInfo.team_name, row["team_name"], sizeof(teamInfo.team_name));
                memcpy(teamInfo.leader_name, row["member_name_0"], sizeof(teamInfo.leader_name));
                teamData.push_back(teamInfo);
            }

            teamCount = teamData.size();

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::GetArenaRankingData Error while fetching top 11 teams: %s", e.what());
            }
        }

    });
}

std::future<void> GMysqlManager::ResetDailyArenaPoints() 
{
    return std::async(std::launch::async, [this]()
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::ResetDailyArenaPoints Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "UPDATE newarena SET today_count_battle = 0";

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::ResetDailyArenaPoints Successfully reset daily arena points for all teams.");
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::ResetDailyArenaPoints Error resetting daily arena points: %s", e.what());
            }
        }

    });
}

std::future<void> GMysqlManager::ResetWeeklyArenaPoints() 
{
    return std::async(std::launch::async, [this]()
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::ResetWeeklyArenaPoints Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();
            query << "UPDATE newarena SET team_battle_week = 0";

            query.execute();

            if (enabled_log) {
                GLog::log(GLOG_INFO, "GMysqlManager::ResetWeeklyArenaPoints Successfully reset daily arena points for all teams.");
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::ResetWeeklyArenaPoints Error resetting daily arena points: %s", e.what());
            }
        }

    });
}

std::future<void> GMysqlManager::AddOrUpdateArenaPlayer(const GMysqlManager::DB_ARENA_PLAYER &player)
{
    return std::async(std::launch::async, [this, player]()
    {
        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdateArenaPlayer Connection failed");
                }
                return;
            }

            mysqlpp::Query query = conn.query();

            query << "SELECT * FROM arena_players WHERE player_id = " << player.player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                query.reset();
                query << "INSERT INTO arena_players (player_id, team_id, cls, score, win_count, team_score, week_battle_count, invite_time, last_visite, battle_count, name) "
                      << "VALUES (" << player.player_id << ", "
                      << player.team_id << ", "
                      << player.cls << ", "
                      << player.score << ", "
                      << player.win_count << ", "
                      << player.team_score << ", "
                      << player.week_battle_count << ", "
                      << player.invite_time << ", "
                      << player.last_visite << ", "
                      << player.battle_count << ", '"
                      << std::string(player.name, sizeof(player.name) - 1) << "')";
                
                query.execute();

                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::AddOrUpdateArenaPlayer Successfully added new arena player with player_id: %d", player.player_id);
                }

            } else {
                query.reset();
                query << "UPDATE arena_players SET ";
                query << "team_id = " << player.team_id << ", ";
                query << "cls = " << player.cls << ", ";
                query << "score = " << player.score << ", ";
                query << "win_count = " << player.win_count << ", ";
                query << "team_score = " << player.team_score << ", ";
                query << "week_battle_count = " << player.week_battle_count << ", ";
                query << "invite_time = " << player.invite_time << ", ";
                query << "last_visite = " << player.last_visite << ", ";
                query << "battle_count = " << player.battle_count << ", ";
                query << "name = '"
                      << std::string(player.name, sizeof(player.name) - 1) << "' ";
                
                query << "WHERE player_id = " << player.player_id;

                query.execute();

                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::AddOrUpdateArenaPlayer Successfully updated arena player with player_id: %d", player.player_id);
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::AddOrUpdateArenaPlayer Error executing AddOrUpdateArenaPlayer operation: %s", e.what());
            }
        }

    });
}

std::future<bool> GMysqlManager::CheckAndResetDailyArenaPoints(int player_id, int tm_day) 
{
    return std::async(std::launch::async, [this, player_id, tm_day]() 
    {
        bool reset_needed = false; 

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetDailyArenaPoints Connection failed");
                }
                return reset_needed;
            }

            mysqlpp::Query query = conn.query();

            query << "SELECT team_id, reset_day FROM newarena WHERE leader_id = " << player_id
                  << " OR member_id_0 = " << player_id
                  << " OR member_id_1 = " << player_id
                  << " OR member_id_2 = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetDailyArenaPoints No team found for player_id: %d", player_id);
                }
                return reset_needed;
            }

            int team_id = result[0]["team_id"];
            int current_reset_day = result[0]["reset_day"];

            if (current_reset_day != tm_day) {
               
                reset_needed = true;

                query.reset();
                query << "UPDATE newarena SET today_count_battle = 0, reset_day = " << tm_day 
                      << " WHERE team_id = " << team_id;

                query.execute();

                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckAndResetDailyArenaPoints Successfully reset daily arena points for team_id: %d", team_id);
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetDailyArenaPoints Error while checking and resetting daily arena points: %s", e.what());
            }
        }

        return reset_needed;
    });
}

std::future<bool> GMysqlManager::CheckAndResetWeekArenaPoints(int player_id, int tm_day) 
{
    return std::async(std::launch::async, [this, player_id, tm_day]() 
    {
        bool reset_needed = false; 

        try {
            
            std::lock_guard<std::mutex> lock(mysql_mutex);
            Connect();
            MySQLConnectionGuard connGuard(conn);

            if (!conn.connected())
            {
                if (enabled_log)
                {
                 GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetWeekArenaPoints Connection failed");
                }
                return reset_needed;
            }

            mysqlpp::Query query = conn.query();

            query << "SELECT team_id, reset_week FROM newarena WHERE leader_id = " << player_id
                  << " OR member_id_0 = " << player_id
                  << " OR member_id_1 = " << player_id
                  << " OR member_id_2 = " << player_id;

            mysqlpp::StoreQueryResult result = query.store();

            if (result.empty()) {
                if (enabled_log) {
                    GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetWeekArenaPoints No team found for player_id: %d", player_id);
                }
                return reset_needed;
            }

            int team_id = result[0]["team_id"];
            int current_reset_week = result[0]["reset_week"];

            if (current_reset_week != tm_day) {
               
                reset_needed = true;

                query.reset();
                query << "UPDATE newarena SET team_battle_week = 0, reset_week = " << tm_day 
                      << " WHERE team_id = " << team_id;

                query.execute();

                if (enabled_log) {
                    GLog::log(GLOG_INFO, "GMysqlManager::CheckAndResetWeekArenaPoints Successfully reset daily arena points for team_id: %d", team_id);
                }
            }

        } catch (const mysqlpp::Exception& e) {
            if (enabled_log) {
                GLog::log(GLOG_ERR, "GMysqlManager::CheckAndResetWeekArenaPoints Error while checking and resetting daily arena points: %s", e.what());
            }
        }

        return reset_needed;
    });
}