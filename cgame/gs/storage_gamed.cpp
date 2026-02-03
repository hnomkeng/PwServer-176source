#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "storage_gamed.h"
#include "gsp_if.h"
#include <vector>
#include <glog.h>
#include "player_imp.h"
#include "emulate_settings.h"

using namespace GNET;

StorageGamed* StorageGamed::instance = NULL;

static const std::string location = "./storage/";
std::mutex storage_mutex;

void StorageGamed::Init()
{
	memset(this,0x00,sizeof(StorageGamed));	
	enabled_log = false;
}

std::future<void> StorageGamed::AddOrUpdatePatentePlayer(
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
    return std::async(std::launch::async, 
    [this, player_id, player_name, player_cls, player_gender, player_lvl_elo, player_exp_elo, player_kill_elo, player_death_elo, kda]() 
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_RANKING> data;
        bool playerFound = false;

        std::ifstream infile(location + "ranking.dat", std::ios::binary);
        if (infile.is_open())
        {
            DB_RANKING existingPlayer;
            while (infile.read(reinterpret_cast<char*>(&existingPlayer), sizeof(DB_RANKING)))
            {
                if (existingPlayer.player_id == player_id)
                {
                    existingPlayer.player_cls = player_cls;
                    existingPlayer.player_gender = player_gender;
                    existingPlayer.player_lvl_elo = player_lvl_elo;
                    existingPlayer.player_exp_elo = player_exp_elo;
                    existingPlayer.player_kill_elo = player_kill_elo;
                    existingPlayer.player_death_elo = player_death_elo;
                    existingPlayer.kda = kda;
                    strncpy(existingPlayer.player_name, player_name.c_str(), sizeof(existingPlayer.player_name) - 1);
                    existingPlayer.player_name[sizeof(existingPlayer.player_name) - 1] = '\0';
                    playerFound = true;
                }
                data.push_back(existingPlayer);
            }
            infile.close();
        }

        if (!playerFound)
        {
            DB_RANKING newPlayer;
            newPlayer.player_id = player_id;
            newPlayer.player_cls = player_cls;
            newPlayer.player_gender = player_gender;
            newPlayer.player_lvl_elo = player_lvl_elo;
            newPlayer.player_exp_elo = player_exp_elo;
            newPlayer.player_kill_elo = player_kill_elo;
            newPlayer.player_death_elo = player_death_elo;
            newPlayer.kda = kda;
            strncpy(newPlayer.player_name, player_name.c_str(), sizeof(newPlayer.player_name) - 1);
            newPlayer.player_name[sizeof(newPlayer.player_name) - 1] = '\0';
            data.push_back(newPlayer);
        }

        std::ofstream outfile(location + "ranking.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::AddOrUpdatePatentePlayer Falha ao abrir o arquivo ranking.dat");
            return;
        }

        for (const auto& player : data)
        {
            outfile.write(reinterpret_cast<const char*>(&player), sizeof(DB_RANKING));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::AddOrUpdatePatentePlayer Jogador adicionado/atualizado com sucesso");
    });
}

std::future<void> StorageGamed::GetRankingData(
    int type,
    int& count_ranking,
    std::vector<StorageGamed::DB_RANKING>& rankingData)
{
    return std::async(std::launch::async, [this, type, &count_ranking, &rankingData]() 
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "ranking.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::GetRankingData Falha ao abrir o arquivo ranking.dat");
            return;
        }

        DB_RANKING playerData;
        while (file.read(reinterpret_cast<char*>(&playerData), sizeof(DB_RANKING)))
        {
            if (type == 0 || playerData.player_lvl_elo == type)
            {
                rankingData.push_back(playerData);
            }
        }

        count_ranking = rankingData.size();

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::GetRankingData Dados de ranking recuperados com sucesso");
    });
}

std::future<void> StorageGamed::TruncateCashRewardTable()
{
    return std::async(std::launch::async, [this]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ofstream file(location + "cash_reward.dat", std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::TruncateCashRewardTable Falha ao abrir o arquivo cash_reward.dat");
            return;
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::TruncateCashRewardTable Arquivo cash_reward.dat truncado com sucesso");
    });
}

std::future<void> StorageGamed::AddPlayerHwid(int player_id, unsigned long long player_hwid)
{
    return std::async(std::launch::async, [this, player_id, player_hwid]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ofstream file(location + "cash_reward.dat", std::ios::binary | std::ios::app);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::AddPlayerHwid Falha ao abrir o arquivo cash_reward.dat");
            return;
        }

        file.write(reinterpret_cast<const char*>(&player_id), sizeof(player_id));
        file.write(reinterpret_cast<const char*>(&player_hwid), sizeof(player_hwid));

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::AddPlayerHwid HWID do jogador adicionado com sucesso");
    });
}

std::future<void> StorageGamed::RemovePlayerHwid(int player_id)
{
    return std::async(std::launch::async, [this, player_id]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<std::pair<int, unsigned long long>> data;

        std::ifstream infile(location + "cash_reward.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::RemovePlayerHwid Falha ao abrir o arquivo cash_reward.dat");
            return;
        }

        int pid;
        unsigned long long hwid;
        while (infile.read(reinterpret_cast<char*>(&pid), sizeof(pid)))
        {
            infile.read(reinterpret_cast<char*>(&hwid), sizeof(hwid));
            if (pid != player_id)
                data.push_back(std::make_pair(pid, hwid));
        }
        infile.close();

        std::ofstream outfile(location + "cash_reward.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::RemovePlayerHwid Falha ao abrir o arquivo cash_reward.dat");
            return;
        }

        for (const auto& entry : data)
        {
            outfile.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));
            outfile.write(reinterpret_cast<const char*>(&entry.second), sizeof(entry.second));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::RemovePlayerHwid HWID do jogador removido com sucesso");
    });
}

std::future<std::pair<bool, int>> StorageGamed::CheckPlayerHwid(unsigned long long player_hwid)
{
    return std::async(std::launch::async, [this, player_hwid]()
    {
        bool playerFound = false;
        int player_id = -1;

        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "cash_reward.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckPlayerHwid Falha ao abrir o arquivo cash_reward.dat");
            return std::make_pair(false, -1);
        }

        int pid;
        unsigned long long hwid;
        while (file.read(reinterpret_cast<char*>(&pid), sizeof(pid)))
        {
            file.read(reinterpret_cast<char*>(&hwid), sizeof(hwid));
            if (hwid == player_hwid)
            {
                playerFound = true;
                player_id = pid;
                break;
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerHwid HWID do jogador verificado com sucesso");

        return std::make_pair(playerFound, player_id);
    });
}

std::future<void> StorageGamed::TruncateeEventForbidTable()
{
    return std::async(std::launch::async, [this]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ofstream file(location + "event_forbid.dat", std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::TruncateeEventForbidTable Falha ao abrir o arquivo event_forbid.dat");
            return;
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::TruncateeEventForbidTable Arquivo event_forbid.dat truncado com sucesso");
    });
}

std::future<void> StorageGamed::AddPlayerEventForbid(int player_id, unsigned long long player_hwid, int event_type)
{
    return std::async(std::launch::async, [this, player_id, player_hwid, event_type]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ofstream file(location + "event_forbid.dat", std::ios::binary | std::ios::app);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::AddPlayerEventForbid Falha ao abrir o arquivo event_forbid.dat");
            return;
        }

        file.write(reinterpret_cast<const char*>(&player_id), sizeof(player_id));
        file.write(reinterpret_cast<const char*>(&player_hwid), sizeof(player_hwid));
        file.write(reinterpret_cast<const char*>(&event_type), sizeof(event_type));

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::AddPlayerEventForbid Evento proibido de jogador adicionado com sucesso");
    });
}

std::future<void> StorageGamed::RemovePlayerEventForbid(int player_id)
{
    return std::async(std::launch::async, [this, player_id]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<std::tuple<int, unsigned long long, int>> data;

        std::ifstream infile(location + "event_forbid.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::RemovePlayerEventForbid Falha ao abrir o arquivo event_forbid.dat");
            return;
        }

        int pid;
        unsigned long long hwid;
        int event_type;
        while (infile.read(reinterpret_cast<char*>(&pid), sizeof(pid)))
        {
            infile.read(reinterpret_cast<char*>(&hwid), sizeof(hwid));
            infile.read(reinterpret_cast<char*>(&event_type), sizeof(event_type));
            if (pid != player_id)
                data.push_back(std::make_tuple(pid, hwid, event_type));
        }
        infile.close();

        std::ofstream outfile(location + "event_forbid.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::RemovePlayerEventForbid Falha ao abrir o arquivo event_forbid.dat");
            return;
        }

        for (const auto& entry : data)
        {
            outfile.write(reinterpret_cast<const char*>(&std::get<0>(entry)), sizeof(std::get<0>(entry)));
            outfile.write(reinterpret_cast<const char*>(&std::get<1>(entry)), sizeof(std::get<1>(entry)));
            outfile.write(reinterpret_cast<const char*>(&std::get<2>(entry)), sizeof(std::get<2>(entry)));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::RemovePlayerEventForbid Evento proibido de jogador removido com sucesso");
    });
}

std::future<std::pair<bool, int>> StorageGamed::CheckPlayerEventForbid(unsigned long long player_hwid)
{
    return std::async(std::launch::async, [this, player_hwid]()
    {
        bool playerFound = false;
        int event_type = -1;

        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "event_forbid.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckPlayerEventForbid Falha ao abrir o arquivo event_forbid.dat");
            return std::make_pair(false, -1);
        }

        int pid;
        unsigned long long hwid;
        int etype;
        while (file.read(reinterpret_cast<char*>(&pid), sizeof(pid)))
        {
            file.read(reinterpret_cast<char*>(&hwid), sizeof(hwid));
            file.read(reinterpret_cast<char*>(&etype), sizeof(etype));
            if (hwid == player_hwid)
            {
                playerFound = true;
                event_type = etype;
                break;
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerEventForbid Evento proibido de jogador verificado com sucesso");

        return std::make_pair(playerFound, event_type);
    });
}

std::future<std::pair<bool, int>> StorageGamed::CheckPlayerArenaTeam(int player_id)
{
    return std::async(std::launch::async, [this, player_id]()
    {
        bool playerFound = false;
        int team_id = -1;

        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckPlayerArenaTeam Falha ao abrir o arquivo newarena.dat");
            return std::make_pair(false, -1);
        }

        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == player_id || team.members[0].player_id == player_id || team.members[1].player_id == player_id || team.members[2].player_id == player_id)
            {
                playerFound = true;
                team_id = team.team_id;
                break;
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerArenaTeam Equipe de arena do jogador verificada com sucesso");

        return std::make_pair(playerFound, team_id);
    });
}

std::future<std::pair<bool, int>> StorageGamed::CheckNameArenaTeam(const char name[], size_t name_len)
{
    return std::async(std::launch::async, [this, name, name_len]()
    {
        bool teamFound = false;
        int team_id = -1;

        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckNameArenaTeam Falha ao abrir o arquivo newarena.dat");
            return std::make_pair(false, -1);
        }

        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (strncmp(team.team_name, name, name_len) == 0)
            {
                teamFound = true;
                team_id = team.team_id;
                break;
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CheckNameArenaTeam Nome da equipe de arena verificado com sucesso");

        return std::make_pair(teamFound, team_id);
    });
}

std::future<void> StorageGamed::CreateArenaTeam(int team_id, int leader_id, int cls, int patent, const char team_name[], std::size_t team_name_len)
{
    return std::async(std::launch::async, [this, team_id, leader_id, cls, patent, team_name, team_name_len]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CreateArenaTeam Falha ao abrir o arquivo newarena.dat para leitura");
            return;
        }

        DB_ARENA_TEAM team;
        bool teamExists = false;

        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.team_id == team_id)
            {
                teamExists = true;
                break;
            }
        }
        infile.close();

        if (teamExists)
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CreateArenaTeam A equipe com o ID: %d já existe", team_id);
            return;
        }

        std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::app);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CreateArenaTeam Falha ao abrir o arquivo newarena.dat para escrita");
            return;
        }

        memset(&team, 0x00, sizeof(DB_ARENA_TEAM));
        team.team_id = team_id;
        team.leader_id = leader_id;
        team.team_score = 1000;
        strncpy(team.team_name, team_name, team_name_len);
        team.members[0].player_id = leader_id;
        team.members[0].is_leader = 1;
        team.members[0].cls = cls;
        team.members[0].patent_level = patent;

        LuaManager *lua = LuaManager::GetInstance();
        const char * player_name = lua->game__GetName(0, leader_id);
        strncpy(team.members[0].player_name, player_name, sizeof(team.members[0].player_name) - 1);
        team.members[0].player_name[sizeof(team.members[0].player_name) - 1] = '\0';

        outfile.write(reinterpret_cast<const char*>(&team), sizeof(DB_ARENA_TEAM));

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CreateArenaTeam Equipe de arena criada com sucesso com o ID: %d", team_id);
    });
}

std::future<void> StorageGamed::GetArenaTeamInfoByLeaderOrMember(int leader_or_member_id, DB_ARENA_TEAM& arenaTeam)
{
    return std::async(std::launch::async, [this, leader_or_member_id, &arenaTeam]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::GetArenaTeamInfoByLeaderOrMember Falha ao abrir o arquivo newarena.dat");
            return;
        }

        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == leader_or_member_id || team.members[0].player_id == leader_or_member_id || team.members[1].player_id == leader_or_member_id || team.members[2].player_id == leader_or_member_id)
            {
                arenaTeam = team;
                if (enabled_log)
                    GLog::log(GLOG_INFO, "StorageGamed::GetArenaTeamInfoByLeaderOrMember Equipe de arena encontrada para o ID: %d", leader_or_member_id);
                return;
            }
        }

        arenaTeam = DB_ARENA_TEAM{};
        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::GetArenaTeamInfoByLeaderOrMember Nenhuma equipe de arena encontrada para o ID: %d", leader_or_member_id);
    });
}

std::future<void> StorageGamed::SetArenaTeamInfo(const DB_ARENA_TEAM& arenaTeam)
{
    return std::async(std::launch::async, [this, arenaTeam]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_TEAM> data;
        bool teamFound = false;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (infile.is_open())
        {
            DB_ARENA_TEAM team;
            while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
            {
                if (team.team_id == arenaTeam.team_id)
                {
                    data.push_back(arenaTeam);
                    teamFound = true;
                }
                else
                {
                    data.push_back(team);
                }
            }
            infile.close();
        }

        if (!teamFound)
        {
            data.push_back(arenaTeam);
        }

        std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::SetArenaTeamInfo Falha ao abrir o arquivo newarena.dat");
            return;
        }

        for (const auto& team : data)
        {
            outfile.write(reinterpret_cast<const char*>(&team), sizeof(DB_ARENA_TEAM));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::SetArenaTeamInfo Informações da equipe de arena definidas com sucesso para o ID da equipe: %d", arenaTeam.team_id);
    });
}

std::future<std::pair<bool, int>> StorageGamed::CheckPlayerNewInviter(int player_id, int cls)
{
    return std::async(std::launch::async, [this, player_id, cls]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckPlayerNewInviter Falha ao abrir o arquivo newarena.dat");
            return std::make_pair(false, -1);
        }

        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == player_id)
            {
                for (int i = 0; i < 3; ++i)
                {
                    if (team.members[i].player_id == 0)
                    {
                        for (int j = 0; j < 3; ++j)
                        {
                            if (team.members[j].cls == cls)
                            {
                                if (enabled_log)
                                    GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerNewInviter Classe já existente na equipe");
                                return std::make_pair(false, 3);
                            }
                        }
                        if (enabled_log)
                            GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerNewInviter Pode adicionar jogador à equipe");
                        return std::make_pair(true, 0);
                    }
                }
                if (enabled_log)
                    GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerNewInviter Não há espaço na equipe");
                return std::make_pair(false, 2);
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::CheckPlayerNewInviter Jogador não encontrado como líder de equipe");
        return std::make_pair(false, 1);
    });
}

std::future<std::pair<bool, int>> StorageGamed::AddRemoveMember(int player_id, int member_id, const char* member_name, int member_cls, int member_patent, OperationType op)
{
    return std::async(std::launch::async, [this, player_id, member_id, member_name, member_cls, member_patent, op]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_TEAM> data;
        bool success = false;
        int resultType = -1;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (infile.is_open())
        {
            DB_ARENA_TEAM team;
            while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
            {
                if (team.leader_id == player_id)
                {
                    if (op == OperationType::ADD)
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            if (team.members[i].player_id == 0)
                            {
                                team.members[i].player_id = member_id;
                                strncpy(team.members[i].player_name, member_name, sizeof(team.members[i].player_name) - 1);
                                team.members[i].cls = member_cls;
                                team.members[i].patent_level = member_patent;
                                success = true;
                                resultType = OperationType::ADD;
                                break;
                            }
                        }
                    }
                    else if (op == OperationType::REMOVE)
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            if (team.members[i].player_id == member_id)
                            {
                                team.members[i] = DB_ARENA_TEAM::MEMBER_INFO{};
                                success = true;
                                resultType = OperationType::REMOVE;
                                break;
                            }
                        }
                    }
                }
                data.push_back(team);
            }
            infile.close();
        }

        if (success)
        {
            std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
            if (!outfile.is_open())
            {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "StorageGamed::AddRemoveMember Falha ao abrir o arquivo newarena.dat");
                return std::make_pair(false, -1);
            }

            for (const auto& team : data)
            {
                outfile.write(reinterpret_cast<const char*>(&team), sizeof(DB_ARENA_TEAM));
            }

            if (enabled_log)
                GLog::log(GLOG_INFO, "StorageGamed::AddRemoveMember Operação %s membro com sucesso",
                          (op == OperationType::ADD ? "adicionar" : "remover"));
        }
        else
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::AddRemoveMember Falha ao %s membro",
                          (op == OperationType::ADD ? "adicionar" : "remover"));
        }

        return std::make_pair(success, resultType);
    });
}

std::future<int> StorageGamed::GetTeamLeader(int player_id)
{
    return std::async(std::launch::async, [this, player_id]()
    {
        int leader_id = -1;

        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::GetTeamLeader Falha ao abrir o arquivo newarena.dat");
            return leader_id;
        }

        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.members[0].player_id == player_id || team.members[1].player_id == player_id || team.members[2].player_id == player_id)
            {
                leader_id = team.leader_id;
                break;
            }
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::GetTeamLeader Líder da equipe encontrado com sucesso para o ID do jogador: %d", player_id);

        return leader_id;
    });
}

std::future<bool> StorageGamed::RemoveArenaTeam(int team_id)
{
    return std::async(std::launch::async, [this, team_id]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_TEAM> data;
        bool success = false;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (infile.is_open())
        {
            DB_ARENA_TEAM team;
            while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
            {
                if (team.team_id != team_id)
                {
                    data.push_back(team);
                }
                else
                {
                    success = true;
                }
            }
            infile.close();
        }

        if (success)
        {
            std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
            if (!outfile.is_open())
            {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "StorageGamed::RemoveArenaTeam Falha ao abrir o arquivo newarena.dat");
                return false;
            }

            for (const auto& team : data)
            {
                outfile.write(reinterpret_cast<const char*>(&team), sizeof(DB_ARENA_TEAM));
            }

            if (enabled_log)
                GLog::log(GLOG_INFO, "StorageGamed::RemoveArenaTeam Equipe de arena removida com sucesso para o ID da equipe: %d", team_id);
        }
        else
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::RemoveArenaTeam Nenhuma equipe de arena encontrada para o ID da equipe: %d", team_id);
        }

        return success;
    });
}

std::future<bool> StorageGamed::KickArenaMember(int leader_id, int pos)
{
    return std::async(std::launch::async, [this, leader_id, pos]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);
        bool success = false;

        if (pos == 0)
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::KickArenaMember Não é possível expulsar o líder da equipe.");
            return success;
        }

        if (pos < 0 || pos > 2)
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::KickArenaMember Posição inválida: %d", pos);
            return success;
        }

        std::vector<DB_ARENA_TEAM> data;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::KickArenaMember Falha ao abrir o arquivo newarena.dat");
            return success;
        }

        DB_ARENA_TEAM team;
        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == leader_id)
            {
                team.members[pos] = DB_ARENA_TEAM::MEMBER_INFO{};
                success = true;
            }
            data.push_back(team);
        }
        infile.close();

        if (success)
        {
            std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
            if (!outfile.is_open())
            {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "StorageGamed::KickArenaMember Falha ao abrir o arquivo newarena.dat");
                return false;
            }

            for (const auto& t : data)
            {
                outfile.write(reinterpret_cast<const char*>(&t), sizeof(DB_ARENA_TEAM));
            }

            if (enabled_log)
                GLog::log(GLOG_INFO, "StorageGamed::KickArenaMember Membro expulso com sucesso da posição: %d", pos);
        }
        else
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::KickArenaMember Nenhuma equipe encontrada para o líder_id: %d", leader_id);
        }

        return success;
    });
}

std::future<void> StorageGamed::GetArenaRankingData(int &teamCount, std::vector<DB_TEAM_INFO> &teamData)
{
    return std::async(std::launch::async, [this, &teamData, &teamCount]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::ifstream file(location + "newarena.dat", std::ios::binary);
        if (!file.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::GetArenaRankingData Falha ao abrir o arquivo newarena.dat");
            return;
        }

        std::vector<DB_TEAM_INFO> teams;
        DB_ARENA_TEAM team;
        while (file.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            DB_TEAM_INFO teamInfo;
            teamInfo.leader_id = team.leader_id;
            teamInfo.team_score = team.team_score;
            teamInfo.team_win = team.team_win;
            strncpy(teamInfo.team_name, team.team_name, sizeof(teamInfo.team_name));
            strncpy(teamInfo.leader_name, team.members[0].player_name, sizeof(teamInfo.leader_name));
            teams.push_back(teamInfo);
        }

        std::sort(teams.begin(), teams.end(), [](const DB_TEAM_INFO &a, const DB_TEAM_INFO &b)
        {
            return a.team_score > b.team_score;
        });

        teamCount = std::min(11, static_cast<int>(teams.size()));
        teamData.assign(teams.begin(), teams.begin() + teamCount);

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::GetArenaRankingData Dados de classificação da arena obtidos com sucesso.");
    });
}

std::future<void> StorageGamed::ResetDailyArenaPoints()
{
    return std::async(std::launch::async, [this]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_TEAM> data;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::ResetDailyArenaPoints Falha ao abrir o arquivo newarena.dat");
            return;
        }

        DB_ARENA_TEAM team;
        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            team.today_count_battle = 0;
            data.push_back(team);
        }
        infile.close();

        std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::ResetDailyArenaPoints Falha ao abrir o arquivo newarena.dat");
            return;
        }

        for (const auto& t : data)
        {
            outfile.write(reinterpret_cast<const char*>(&t), sizeof(DB_ARENA_TEAM));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::ResetDailyArenaPoints Pontos diários da arena redefinidos com sucesso.");
    });
}

std::future<void> StorageGamed::ResetWeeklyArenaPoints()
{
    return std::async(std::launch::async, [this]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_TEAM> data;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::ResetWeeklyArenaPoints Falha ao abrir o arquivo newarena.dat");
            return;
        }

        DB_ARENA_TEAM team;
        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            team.team_battle_week = 0;
            data.push_back(team);
        }
        infile.close();

        std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::ResetWeeklyArenaPoints Falha ao abrir o arquivo newarena.dat");
            return;
        }

        for (const auto& t : data)
        {
            outfile.write(reinterpret_cast<const char*>(&t), sizeof(DB_ARENA_TEAM));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::ResetWeeklyArenaPoints Pontos semanais da arena redefinidos com sucesso.");
    });
}

std::future<void> StorageGamed::AddOrUpdateArenaPlayer(const DB_ARENA_PLAYER &player)
{
    return std::async(std::launch::async, [this, player]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);

        std::vector<DB_ARENA_PLAYER> data;
        bool playerFound = false;

        std::ifstream infile(location + "arena_players.dat", std::ios::binary);
        if (infile.is_open())
        {
            DB_ARENA_PLAYER existingPlayer;
            while (infile.read(reinterpret_cast<char*>(&existingPlayer), sizeof(DB_ARENA_PLAYER)))
            {
                if (existingPlayer.player_id == player.player_id)
                {
                    data.push_back(player);
                    playerFound = true;
                }
                else
                {
                    data.push_back(existingPlayer);
                }
            }
            infile.close();
        }

        if (!playerFound)
        {
            data.push_back(player);
        }

        std::ofstream outfile(location + "arena_players.dat", std::ios::binary | std::ios::trunc);
        if (!outfile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::AddOrUpdateArenaPlayer Falha ao abrir o arquivo arena_players.dat");
            return;
        }

        for (const auto& p : data)
        {
            outfile.write(reinterpret_cast<const char*>(&p), sizeof(DB_ARENA_PLAYER));
        }

        if (enabled_log)
            GLog::log(GLOG_INFO, "StorageGamed::AddOrUpdateArenaPlayer Jogador da arena adicionado ou atualizado com sucesso, player_id: %d", player.player_id);
    });
}


std::future<bool> StorageGamed::CheckAndResetDailyArenaPoints(int player_id, int tm_day)
{
    return std::async(std::launch::async, [this, player_id, tm_day]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);
        bool reset_needed = false;

        std::vector<DB_ARENA_TEAM> data;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckAndResetDailyArenaPoints Falha ao abrir o arquivo newarena.dat");
            return reset_needed;
        }

        DB_ARENA_TEAM team;
        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == player_id || team.members[0].player_id == player_id || team.members[1].player_id == player_id || team.members[2].player_id == player_id)
            {
                if (team.today_count_battle != tm_day)
                {
                    team.today_count_battle = 0;
                    reset_needed = true;
                }
            }
            data.push_back(team);
        }
        infile.close();

        if (reset_needed)
        {
            std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
            if (!outfile.is_open())
            {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "StorageGamed::CheckAndResetDailyArenaPoints Falha ao abrir o arquivo newarena.dat");
                return false;
            }

            for (const auto& t : data)
            {
                outfile.write(reinterpret_cast<const char*>(&t), sizeof(DB_ARENA_TEAM));
            }

            if (enabled_log)
                GLog::log(GLOG_INFO, "StorageGamed::CheckAndResetDailyArenaPoints Pontos diários da arena redefinidos com sucesso.");
        }

        return reset_needed;
    });
}

std::future<bool> StorageGamed::CheckAndResetWeekArenaPoints(int player_id, int tm_day)
{
    return std::async(std::launch::async, [this, player_id, tm_day]()
    {
        std::lock_guard<std::mutex> lock(storage_mutex);
        bool reset_needed = false;

        std::vector<DB_ARENA_TEAM> data;

        std::ifstream infile(location + "newarena.dat", std::ios::binary);
        if (!infile.is_open())
        {
            if (enabled_log)
                GLog::log(GLOG_ERR, "StorageGamed::CheckAndResetWeekArenaPoints Falha ao abrir o arquivo newarena.dat");
            return reset_needed;
        }

        DB_ARENA_TEAM team;
        while (infile.read(reinterpret_cast<char*>(&team), sizeof(DB_ARENA_TEAM)))
        {
            if (team.leader_id == player_id || team.members[0].player_id == player_id || team.members[1].player_id == player_id || team.members[2].player_id == player_id)
            {
                if (team.team_battle_week != tm_day)
                {
                    team.team_battle_week = 0;
                    reset_needed = true;
                }
            }
            data.push_back(team);
        }
        infile.close();

        if (reset_needed)
        {
            std::ofstream outfile(location + "newarena.dat", std::ios::binary | std::ios::trunc);
            if (!outfile.is_open())
            {
                if (enabled_log)
                    GLog::log(GLOG_ERR, "StorageGamed::CheckAndResetWeekArenaPoints Falha ao abrir o arquivo newarena.dat");
                return false;
            }

            for (const auto& t : data)
            {
                outfile.write(reinterpret_cast<const char*>(&t), sizeof(DB_ARENA_TEAM));
            }

            if (enabled_log)
                GLog::log(GLOG_INFO, "StorageGamed::CheckAndResetWeekArenaPoints Pontos semanais da arena redefinidos com sucesso.");
        }

        return reset_needed;
    });
}



