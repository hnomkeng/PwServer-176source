
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "arenaofaurora_ctrl.h"

#include <glog.h>

#define MAX_TEAM_NAME_LENGTH 36

#define MAX_SCORE 100
#define MAX_WAIT_QUEUE 600
#define BASE_SCORE 0
#define QUALIFICATION_COUNT_REQUIRE 10

void arenaofaurora_ctrl::Tick(world* pPlane)
{
	spin_autolock l(_lock);
	if ((++_tick_counter) % 20 != 0)
		return;

	_battle_timer += 1;

	SyncArenaStat();

	// Inicia a Batalha
	if (_battle_timer == 0)
	{
		HideAreaGFX(pPlane);
	}

	// Finaliza a Batalha
	if (_battle_timer >= 0)
	{
		CheckBattleResult(pPlane);
	}
}

void arenaofaurora_ctrl::ArenaPlayerFaction()
{
	char red_team_name[MAX_TEAM_NAME_LENGTH]; memcpy(red_team_name, _red_team_data.team_name, MAX_TEAM_NAME_LENGTH);
	char blue_team_name[MAX_TEAM_NAME_LENGTH]; memcpy(blue_team_name, _blue_team_data.team_name, MAX_TEAM_NAME_LENGTH);
	
	if(_red_list.size())
	{
		packet_wrapper h4(256);
		using namespace S2C;

		unsigned int team_type = RED_TYPE_IDX;
		int red_count_alive = (GetAliveMembers(_red_alive_list) == 0 ? -1 : GetAliveMembers(_red_alive_list));
		int blue_count_alive = (GetAliveMembers(_blue_alive_list) == 0 ? -1 : GetAliveMembers(_blue_alive_list));
		int reserved1 = 0;
		int reserved2 = IDX_ACTIVATE;
		const char *red_team_name2 = red_team_name;
		const char *blue_team_name2 = blue_team_name;

		CMD::Make<CMD::arena_player_faction_info>::From(
			h4,
			team_type,
			red_count_alive,
			blue_count_alive,
			reserved1,
			reserved2,
			red_team_name2,
			blue_team_name2);

		multi_send_ls_msg(_red_list, h4.data(), h4.size(), 0);
	}
		
	if (_blue_list.size())
	{
		packet_wrapper h4(256);
		using namespace S2C;

		unsigned int team_type = BLUE_TYPE_IDX;
		int red_count_alive = (GetAliveMembers(_red_alive_list) == 0 ? -1 : GetAliveMembers(_red_alive_list));
		int blue_count_alive = (GetAliveMembers(_blue_alive_list) == 0 ? -1 : GetAliveMembers(_blue_alive_list));
		int reserved1 = 0;
		int reserved2 = IDX_ACTIVATE;
		const char *red_team_name2 = red_team_name;
		const char *blue_team_name2 = blue_team_name;

		CMD::Make<CMD::arena_player_faction_info>::From(
			h4,
			team_type,
			red_count_alive,
			blue_count_alive,
			reserved1,
			reserved2,
			red_team_name2,
			blue_team_name2);

		multi_send_ls_msg(_blue_list, h4.data(), h4.size(), 0);

	}
}

void arenaofaurora_ctrl::ArenaBattleGroup()
{
	if (_red_list.size())
	{		
		packet_wrapper h2(64);
		using namespace S2C;

		int red_members[] = { 0, 0, 0, 0, 0, 0, 0 };
		int blue_members[] = { 0, 0, 0, 0, 0, 0, 0 };
		for (int i = 0; i < _data.red_members.size(); ++i)
		{
			red_members[i] = _data.red_members[i].player_id;			
		}
		for (int i = 0; i < _data.blue_members.size(); ++i)
		{
			blue_members[i] = _data.blue_members[i].player_id;
		}

		CMD::Make<CMD::arena_battle_group_info>::From(h2, 
		_battle_timer,
		_blue_damage, 
		_red_damage,
		_data.red_members.size(),
		_data.blue_members.size(),
		red_members,
		blue_members
		);

		multi_send_ls_msg(_red_list, h2.data(), h2.size(), 0);	

		packet_wrapper h3(24);
		using namespace S2C;

		CMD::Make<CMD::new_arena_players_alive>::From(h3,
			GetAliveMembers(_red_alive_list),
			_red_enter_list.size(),
			GetAliveMembers(_blue_alive_list),
			_blue_enter_list.size());

		multi_send_ls_msg(_red_list, h3.data(), h3.size(), 0);
	}
	if(_blue_list.size())
	{
		packet_wrapper h2(64);
		using namespace S2C;

		int red_members[] = { 0, 0, 0, 0, 0, 0, 0 };
		int blue_members[] = { 0, 0, 0, 0, 0, 0, 0 };
		for (int i = 0; i < _data.red_members.size(); ++i)
		{
			red_members[i] = _data.red_members[i].player_id;
		}
		for (int i = 0; i < _data.blue_members.size(); ++i)
		{
			blue_members[i] = _data.blue_members[i].player_id;
		}

		CMD::Make<CMD::arena_battle_group_info>::From(h2,
			_battle_timer,
			_blue_damage,
			_red_damage,
			_data.red_members.size(),
			_data.blue_members.size(),
			red_members,
			blue_members
		);

		multi_send_ls_msg(_blue_list, h2.data(), h2.size(), 0);

		packet_wrapper h3(24);
		using namespace S2C;

		CMD::Make<CMD::new_arena_players_alive>::From(h3,
			GetAliveMembers(_red_alive_list),
			_red_enter_list.size(),
			GetAliveMembers(_blue_alive_list),
			_blue_enter_list.size());

		multi_send_ls_msg(_blue_list, h3.data(), h3.size(), 0);
	}
}

void arenaofaurora_ctrl::SyncArenaStat()
{
	spin_autolock keeper(_user_list_lock);	
		
	ArenaBattleGroup();
}

void arenaofaurora_ctrl::SendPlayerLeave(int playerid)
{
	//
}

void arenaofaurora_ctrl::HideAreaGFX(world* pPlane)
{
	pPlane->ClearSpawn(1001);
}

void arenaofaurora_ctrl::CheckBattleResult(world * pPlane)
{
	if (_battle_result)
		return;

	if (GetAliveMembers(_red_alive_list) == 0 || GetAliveMembers(_blue_alive_list) == 0)
	{
		_battle_result = 1;
		BattleEnd(pPlane);
		return;
	}

	if (_battle_timer >= TIMER_MAX_BATTLE)
	{
		_battle_result = 1;
		BattleEnd(pPlane);
		return;
	}
}

void arenaofaurora_ctrl::BattleEnd(world * pPlane)
{
	// Interface de resultado
	ArenaRelaese();

	// Reseta o pareamento
	GMSV::EC_SendArenaBattleEnd(_data.arena_id, world_manager::GetWorldTag(), GetWinner(),
		GetAliveMembers(_red_alive_list), GetAliveMembers(_blue_alive_list),
		_red_damage, _blue_damage);
	
	// Mapa
	pPlane->w_destroy_timestamp = g_timer.get_systime() + 105;
	pPlane->w_battle_result = _battle_result;
	_battle_end_timer = 5;

}

int arenaofaurora_ctrl::GetWinner()
{
	int count_alive_red = GetAliveMembers(_red_alive_list);
	int count_alive_blue = GetAliveMembers(_blue_alive_list);

	int count_enter_red = GetEnterMembers(_red_enter_list);
	int count_enter_blue = GetEnterMembers(_blue_enter_list);

	if (count_alive_red == count_alive_blue)
	{
		if (_red_damage == _blue_damage)
		{
			return ARENA_RESULT_NOBODY;
		}
		
		if (_red_damage > _blue_damage)
		{
			return ARENA_RESULT_WINNER_RED;
		}

		if (_blue_damage > _red_damage)
		{
			return ARENA_RESULT_WINNER_BLUE;
		}

		return ARENA_RESULT_ERR;
	}

	if (count_alive_blue == 0 && count_alive_red >= 1)
	{
		return ARENA_RESULT_WINNER_RED;
	}

	if (count_alive_red == 0 && count_alive_blue >= 1)
	{
		return ARENA_RESULT_WINNER_BLUE;
	}


	return ARENA_RESULT_ERR;
}

void arenaofaurora_ctrl::RefreshStateRedTeam()
{
	for (unsigned int i = 0; i < _data.red_members.size(); i++)
	{
		int index;
		gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(_data.red_members[i].player_id, index);
		if (pPlayer) {
			gactive_imp* pImp = (gactive_imp*)pPlayer->imp;
			if (pImp) {
				pImp->SetRefreshState();
			}
		}
	}
}

void arenaofaurora_ctrl::RefreshStateBlueTeam()
{
	for (unsigned int i = 0; i < _data.blue_members.size(); i++)
	{
		int index;
		gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(_data.blue_members[i].player_id, index);
		if (pPlayer) {
			gactive_imp* pImp = (gactive_imp*)pPlayer->imp;
			if (pImp) {
				pImp->SetRefreshState();
			}
		}
	}
}

void arenaofaurora_ctrl::NotifyMembers()
{
	ArenaPlayerFaction();
}

void arenaofaurora_ctrl::OnPlayerEnter(gplayer* pPlayer)
{
    StorageGamed * storagegamed = StorageGamed::GetInstance();

	spin_autolock keeper(_user_list_lock);
	AddMapNode(_all_list, pPlayer);

	int playerId = pPlayer->ID.id;
	gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;

	switch (GetTeamType(playerId))
	{
	case ARENA_TEAM_RED:
	{
		AddVectorInt(_red_enter_list, pPlayer);
		AddArenaMember(_red_alive_list, pPlayer);
		AddMapNode(_red_list, pPlayer);		
		RefreshStateBlueTeam();
		
		StorageGamed *storagegamed = StorageGamed::GetInstance();
		StorageGamed::DB_ARENA_TEAM arena_team;
		//pImp->GetArenaTeam()->GetTeamInfo(arena_team);
		auto future = storagegamed->GetArenaTeamInfoByLeaderOrMember(playerId, arena_team);
    	future.get();

		AddRedTeamData(arena_team, pPlayer);
	}
	break;
	case ARENA_TEAM_BLUE:
	{
		AddVectorInt(_blue_enter_list, pPlayer);
		AddArenaMember(_blue_alive_list, pPlayer);
		AddMapNode(_blue_list, pPlayer);
		RefreshStateRedTeam();

		StorageGamed *storagegamed = StorageGamed::GetInstance();
		StorageGamed::DB_ARENA_TEAM arena_team;
		//pImp->GetArenaTeam()->GetTeamInfo(arena_team);
		auto future = storagegamed->GetArenaTeamInfoByLeaderOrMember(playerId, arena_team);
    	future.get();

		AddBlueTeamData(arena_team, pPlayer);
	}
	break;

	default:
		break;
	}

	NotifyMembers();
}

void arenaofaurora_ctrl::OnPlayerLeave(gplayer* pPlayer)
{
	spin_autolock keeper(_user_list_lock);
	DelMapNode(_all_list, pPlayer);

	int playerId = pPlayer->ID.id;

	switch (GetTeamType(playerId))
	{
	case ARENA_TEAM_RED:
		DelArenaMember(_red_alive_list, pPlayer);
		DelMapNode(_red_list, pPlayer);
		DelVectorInt(_red_enter_list, pPlayer);

		//printf(" [ arenaofaurora_ctrl::OnPlayerLeave ] %d is red team\n", playerId);
		break;
	case ARENA_TEAM_BLUE:
		DelArenaMember(_blue_alive_list, pPlayer);
		DelMapNode(_blue_list, pPlayer);
		DelVectorInt(_blue_enter_list, pPlayer);

		//printf(" [ arenaofaurora_ctrl::OnPlayerLeave ] %d is blue team\n", playerId);
		break;
	default:
		//printf(" [ arenaofaurora_ctrl::OnPlayerLeave ] %d is undefined player\n", playerId);
		break;
	}

	AddVectorInt(_death_players, pPlayer);

	SendPlayerLeave(playerId);
}

void arenaofaurora_ctrl::OnDeath(gplayer* pPlayer)
{
	spin_autolock keeper(_user_list_lock);

	int playerId = pPlayer->ID.id;
	
	switch (GetTeamType(playerId))
	{
	case ARENA_TEAM_RED:
		DeathArenaMember(_red_alive_list, pPlayer);
		//printf(" [ arenaofaurora_ctrl::OnDeath ] %d is red team\n", playerId);
		break;
	case ARENA_TEAM_BLUE:
		DeathArenaMember(_blue_alive_list, pPlayer);
		//printf(" [ arenaofaurora_ctrl::OnDeath ] %d is blue team\n", playerId);
		break;
	default:
		//printf(" [ arenaofaurora_ctrl::OnDeath ] %d is undefined player\n", playerId);
		break;
	}

	AddVectorInt(_death_players, pPlayer);

	SendPlayerLeave(playerId);
}

void arenaofaurora_ctrl::OnDamage(int attacker, int damage)
{
	int teamType = GetTeamType(attacker);

	switch (teamType)
	{
	case ARENA_TEAM_RED:
		_red_damage += damage;
		break;
	case ARENA_TEAM_BLUE:
		_blue_damage += damage;
		break;
	}
}

int arenaofaurora_ctrl::GetTeamType(int player_id)
{
	for (unsigned int i = 0; i < _data.red_members.size(); i++)
	{
		if ( _data.red_members[i].player_id == player_id )
		{
			return ARENA_TEAM_RED;
		}
	}

	for (unsigned int i = 0; i < _data.blue_members.size(); i++)
	{
		if ( _data.blue_members[i].player_id == player_id )
		{
			return ARENA_TEAM_BLUE;
		}
	}

	return ARENA_ERR;
}

bool arenaofaurora_ctrl::IsAlivePlayer(int player_id)
{
	return std::find(_death_players.begin(), _death_players.end(), player_id) == _death_players.end();
}

bool arenaofaurora_ctrl::IsDeathPlayer(int player_id)
{
	return std::find(_death_players.begin(), _death_players.end(), player_id) != _death_players.end();
}


// result 

int arenaofaurora_ctrl::CalculateQualScore(int win_count)
{
	if (win_count >= 0 && win_count <= 1)
		return 1300;

	if (win_count >= 2 && win_count <= 3)
		return 1400;

	if (win_count >= 4 && win_count <= 6)
		return 1500;

	if (win_count >= 7 && win_count <= 8)
		return 1600;

	if (win_count >= 9)
		return 1700;

	return 1000;
}

double arenaofaurora_ctrl::CalculateScore(int red_score, int blue_score)
{
	return 1 / (1 + pow(10, (blue_score - red_score) / 400.0));
}

int arenaofaurora_ctrl::GetScoreDelta(int red_score, int blue_score, int result)
{
	return (int)((MAX_SCORE + 1) * ((int)result - CalculateScore(red_score, blue_score)));
}

void arenaofaurora_ctrl::CalculateBattleResult( StorageGamed::DB_ARENA_TEAM winteam, StorageGamed::DB_ARENA_TEAM loseteam)
{
	// increment win
	winteam.team_win += 1;

	// increment battle count
	winteam.team_battle += 1;
	loseteam.team_battle += 1;
	
	// increment week battle count
	winteam.team_battle_week += 1;
	loseteam.team_battle_week += 1;

	// increment today battle count
	winteam.today_count_battle += 1;
	loseteam.today_count_battle += 1;

	StorageGamed *storagegamed = StorageGamed::GetInstance();

	bool add_win = false;

	for (int i = 0; i < 3; i++)
	{
		if(winteam.members[i].player_id > 0)
		{
			int windex1;
			gplayer *gPlayer = world_manager::GetInstance()->FindPlayer(winteam.members[i].player_id, windex1);
			if (gPlayer && gPlayer->imp)
			{
				gplayer_imp *pImp = (gplayer_imp *)gPlayer->imp;
				//pImp->GetArenaTeam()->SetTeamInfo(winteam);

				auto futureteam = storagegamed->SetArenaTeamInfo(winteam);
				futureteam.get();

				// Recompensa Win
				gplayer_arena_team::ARENA_REWARD arena_award;
				pImp->GetArenaTeam()->GetRewardSeason(arena_award);

				if(!add_win)
				{
					arena_award.reward_id = gplayer_arena_team::REWARD_DEFAULT_ID;
					arena_award.reward_count += gplayer_arena_team::REWARD_WIN_COUNT;
					arena_award.reward_season = gplayer_arena_team::REWARD_DEFAULT_SEASON;
					arena_award.reward_date = g_timer.get_systime();					
				}

				pImp->GetArenaTeam()->SetRewardSeason(arena_award);
				pImp->ProtocolGetArenaTeamInfo();
			}
		}				
	}	

	bool add_lose = false;

	for (int i = 0; i < 3; i++)
	{
		if(loseteam.members[i].player_id > 0)
		{
			int windex2;
			gplayer *gPlayer = world_manager::GetInstance()->FindPlayer(loseteam.members[i].player_id, windex2);
			if (gPlayer && gPlayer->imp)
			{
				gplayer_imp *pImp = (gplayer_imp *)gPlayer->imp;
				//pImp->GetArenaTeam()->SetTeamInfo(loseteam);
				auto futureteam = storagegamed->SetArenaTeamInfo(loseteam);
				futureteam.get();

				// Recompensa Lose
				gplayer_arena_team::ARENA_REWARD arena_award;
				pImp->GetArenaTeam()->GetRewardSeason(arena_award);

				if(!add_lose)
				{
					arena_award.reward_id = gplayer_arena_team::REWARD_DEFAULT_ID;
					arena_award.reward_count += gplayer_arena_team::REWARD_LOSE_COUNT;
					arena_award.reward_season = gplayer_arena_team::REWARD_DEFAULT_SEASON;
					arena_award.reward_date = g_timer.get_systime();

					add_lose = true;
				}				
				
				pImp->GetArenaTeam()->SetRewardSeason(arena_award);
				pImp->ProtocolGetArenaTeamInfo();
			}
		}
	}
}

int arenaofaurora_ctrl::GetArenaResultScore(int result)
{
	int delta = 0;
	int delta_r = 0;
	int delta_b = 0;

	switch (result)
	{
	case ARENA_RESULT_WINNER_RED: // red winner
		delta = GetScoreDelta(_red_team_data.team_score, _blue_team_data.team_score, 1);
		delta_r = delta;
		delta_b = -delta;
		break;
	case ARENA_RESULT_WINNER_BLUE: // blue winner
		delta = GetScoreDelta(_red_team_data.team_score, _blue_team_data.team_score, 0);
		delta_r = -delta;
		delta_b = delta;
		break;
	}

	_red_team_data.team_score += delta;
	_blue_team_data.team_score -= delta;

	if (_red_team_data.team_score < 0)
		_red_team_data.team_score = 0;
	if (_blue_team_data.team_score < 0)
		_blue_team_data.team_score = 0;

	switch (result)
	{
	case ARENA_RESULT_WINNER_RED: // red winner
	{
		for (size_t i = 0; i < _red_enter_list.size(); i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				if (_red_team_data.members[j].player_id == _red_enter_list[i])
				{
					_red_team_data.members[j].score += GetScoreDelta(_red_team_data.members[j].score, _blue_team_data.team_score, 1);
				}
			}
		}

		for (size_t i = 0; i < _blue_enter_list.size(); i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				if (_blue_team_data.members[j].player_id == _blue_enter_list[i])
				{
					_blue_team_data.members[j].score -= GetScoreDelta(_blue_team_data.members[j].score, _red_team_data.team_score, 0);
				}
			}
		}
	}
	break;
	case ARENA_RESULT_WINNER_BLUE: // blue winner
	{
		for (size_t i = 0; i < _blue_enter_list.size(); i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				if (_blue_team_data.members[j].player_id == _blue_enter_list[i])
				{
					_blue_team_data.members[j].score += GetScoreDelta(_blue_team_data.members[j].score, _red_team_data.team_score, 1);
				}
			}
		}

		for (size_t i = 0; i < _red_enter_list.size(); i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				if (_red_team_data.members[j].player_id == _red_enter_list[i])
				{
					_red_team_data.members[j].score -= GetScoreDelta(_red_team_data.members[j].score, _blue_team_data.team_score, 0);
				}
			}
		}
	}
	break;
	}

	switch (result)
	{
	case ARENA_RESULT_WINNER_RED: // red winner
		CalculateBattleResult(_red_team_data, _blue_team_data);
		break;
	case ARENA_RESULT_WINNER_BLUE: // blue winner
		CalculateBattleResult(_blue_team_data, _red_team_data);
		break;
	default:
		break;
	}

	return delta;
}

void
arenaofaurora_ctrl::ArenaRelaese()
{
	if (_all_list.size())
	{
		int result = GetWinner();

		unsigned int red_damage = _red_damage;
		unsigned int blue_damage = _blue_damage;

		int red_score = _red_team_data.team_score;
		int blue_score = _blue_team_data.team_score;
		
		int delta = GetArenaResultScore(result);
		int delta_r = ( _red_team_data.team_score - red_score );
		int delta_b = ( _blue_team_data.team_score - blue_score );
			
		int red_player_count = _data.red_members.size();
		int blue_player_count = _data.blue_members.size();
		int red_alive_player_count = GetAliveMembers(_red_alive_list);
		int blue_alive_player_count = GetAliveMembers(_blue_alive_list);

		char red_team_name[MAX_TEAM_NAME_LENGTH]; 
		memset(red_team_name, 0, MAX_TEAM_NAME_LENGTH);
		strncpy(red_team_name, _red_team_data.team_name, MAX_TEAM_NAME_LENGTH - 1);
		
		char blue_team_name[MAX_TEAM_NAME_LENGTH]; 
		memset(blue_team_name, 0, MAX_TEAM_NAME_LENGTH);
		strncpy(blue_team_name, _blue_team_data.team_name, MAX_TEAM_NAME_LENGTH - 1);
		
		packet_wrapper h1(2048);
		using namespace S2C;
		CMD::Make<CMD::arena_battle_result>::From(h1);
	
		h1 << result;
		h1 << blue_damage << red_damage;
		h1 << red_score << blue_score;
		h1 << delta_r << delta_b;
		h1 << (_red_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_RED ? 1 : 0)) << (_blue_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_BLUE ? 1 : 0)); // Win Team Count 0 / 0
		h1 << red_player_count << blue_player_count;

		h1.push_back(red_team_name, MAX_TEAM_NAME_LENGTH);
		h1.push_back(blue_team_name, MAX_TEAM_NAME_LENGTH);

		for (int i = 0; i < _data.red_members.size(); ++i)
		{
			if ( _data.red_members[i].player_id >= 1024 )
			{
				int index;
				gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(_data.red_members[i].player_id, index);
				if (pPlayer && pPlayer->imp)
				{
					gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
					arena_team_player_result player_result;
					unsigned int  len = 0;
					const void * player_name = pImp->GetPlayerName(len);
					
					player_result.cls = _red_team_data.members[i].cls;
					player_result.solo_point = _red_team_data.members[i].score;
					player_result.solo_wins = _red_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_RED ? 1 : 0);
					player_result.team_wins = _red_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_RED ? 1 : 0);
					player_result.reserved = 0;
					player_result.reserved2 = 0;
					player_result.reserved3 = 0;
					
					if (player_name && len)
					{
						memcpy(player_result.name, player_name, len);
					}
					h1.push_back(&player_result, sizeof(player_result));
				}
			}
		}

		for (int i = 0; i < _data.blue_members.size(); ++i)
		{
			if ( _data.blue_members[i].player_id >= 1024 )
			{
				int index;
				gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(_data.blue_members[i].player_id, index);
				if (pPlayer && pPlayer->imp)
				{
					gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
					arena_team_player_result player_result;
					unsigned int  len = 0;
					const void * player_name = pImp->GetPlayerName(len);
					
					player_result.cls = _blue_team_data.members[i].cls;
					player_result.solo_point = _blue_team_data.members[i].score;
					player_result.solo_wins = _blue_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_BLUE ? 1 : 0);
					player_result.team_wins = _blue_team_data.team_win + (GetWinner() == ARENA_RESULT_WINNER_BLUE ? 1 : 0);
					player_result.reserved = 0;
					player_result.reserved2 = 0;
					player_result.reserved3 = 0;
					
					if (player_name && len)
					{
						memcpy(player_result.name, player_name, len);
					}
					h1.push_back(&player_result, sizeof(player_result));
				}
			}
		}

		multi_send_ls_msg(_all_list, h1.data(), h1.size(), 0);
	}
}



