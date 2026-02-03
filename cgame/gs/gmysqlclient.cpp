#include <common/utf.h>
#include <threadpool.h>
#include <malloc.h>
#include <mysql/mysql.h>
#include <unordered_map>
#include <db_if.h>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "gmysqlclient.h"
#include "octets.h"
#include "base64.h"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include <glog.h>


using namespace GNET;
//----------------------------------------------------------------------------------------------------------
GMysqlClient* GMysqlClient::instance = NULL;
//----------------------------------------------------------------------------------------------------------
void GMysqlClient::Init(const char * ip, const int port, const char * user, const char * passwd, const char * db)
{
	int size = 0;
	
	pthread_mutex_init(&mysql_mutex,NULL);
	pthread_mutex_lock(&mysql_mutex);
	
	this->mysql_log = false;
	
	this->active = false;
	this->DBMysql = NULL;

	memset(this->ip,0x00,sizeof(this->ip)); size = strlen(ip); if(size < sizeof(this->ip)) memcpy(this->ip, ip, size);
	this->port = port;
	memset(this->user,0x00,sizeof(this->user)); size = strlen(user); if(size < sizeof(this->user)) memcpy(this->user, user, size);
	memset(this->passwd,0x00,sizeof(this->passwd)); size = strlen(passwd); if(size < sizeof(this->passwd)) memcpy(this->passwd, passwd, size);
	memset(this->db,0x00,sizeof(this->db)); size = strlen(db); if(size < sizeof(this->db)) memcpy(this->db, db, size);
	
	this->mysql_state = 0;

	this->patente_players.clear();
	this->patente_players.reserve(5000);

	this->cash_reward.clear();
	this->cash_reward.reserve(1000);
	
	pthread_mutex_unlock(&mysql_mutex);
}
//----------------------------------------------------------------------------------------------------------
bool GMysqlClient::Connect()
{
	bool res = false;
	pthread_mutex_lock(&mysql_mutex);
	try
	{
		if ( !active )
		{
			if (!DBMysql)
			{
				DBMysql = mysql_init(NULL);
				if(DBMysql)
				{
					if( mysql_real_connect(DBMysql,ip,user,passwd,db,port,0,0) )
					{
						active = true;
						mysql_state = 1;
						res = true;
					}
					else
					{
						if (mysql_log) printf("GMysqlClient::Gamed::Connect: error: mysql_real_connect! ip = %s , user = %s , pass = %s , db = %s , port = %d \n", ip, user, passwd, db, port );
							
					}
				}
				else
				{
					if (mysql_log) printf("GMysqlClient::Gamed::Connect: error: mysql_init \n");
				}
			}
			else
			{
				if (DBMysql)
				{
					mysql_close(DBMysql);
				}
				if (mysql_log) printf("GMysqlClient::Gamed::Connect: error: DBMysql != NULL \n");
			}
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::Gamed::Connect: error: active = true \n");
		}
		
		if ( res )
		{
			active = true;
			mysql_state = 1;
			printf("GMysqlClient::Gamed::OnAddSession \n");
		}
		else
		{
			printf("GMysqlClient::Gamed::OnAbortSession \n");
		}
	}
	catch ( ... )
	{
		if (mysql_log) printf("GMysqlClient::Gamed::Connect: exception error !!! \n");
	}
	pthread_mutex_unlock(&mysql_mutex);
	return res;
}
//----------------------------------------------------------------------------------------------------------
void GMysqlClient::Disconnect()
{
	pthread_mutex_lock(&mysql_mutex);
	try
	{
		active = false;
		mysql_state = 0;
		if (DBMysql)
		{
			mysql_close(DBMysql);
		}
		printf("GMysqlClient::Gamed::OnDelSession \n");
	}
	catch ( ... )
	{
		if (mysql_log) printf("GMysqlClient::Gamed::Disconnect: exception error !!! \n");
	}
	pthread_mutex_unlock(&mysql_mutex);
}
//----------------------------------------------------------------------------------------------------------
void GMysqlClient::Reconnect() {
    Disconnect();
    for (int i = 0; i < MAX_RECONNECT_ATTEMPTS; ++i) {
        if (Connect()) {
            mysql_state = 2;
            if (mysql_log) printf("GMysqlClient::Gamed::Reconnect: Connect ERROR... Reconnect... OK!!! \n");
            return;
        } else {
          	if (mysql_log) printf("GMysqlClient::Gamed::Reconnect: Connect ERROR... Reconnect... FAILED!!! \n");
            usleep(1000);
        }
    }
}
//----------------------------------------------------------------------------------------------------------
bool GMysqlClient::MysqlQuery()
{
	bool res = false;
	size_t len = strlen(QUERY);
	const char * str = QUERY;
	try
	{
		if (active && DBMysql && mysql_state && str && len )
		{
			query_state = mysql_real_query(DBMysql, str, len);
			if ( !query_state )
			{
				if (mysql_log) printf("GMysqlClient::Gamed::MysqlQuery: str='%s' , len=%d OK! \n", str, len );
				res = true;
			}
			else
			{
				if (mysql_log) printf("GMysqlClient::Gamed::MysqlQuery: query_state = %d , error: %s \n", query_state, mysql_error(DBMysql) );
				Reconnect();
				MysqlQuery();
			}
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::Gamed::MysqlQuery: str=%s , len=%d ERROR!!! \n", str, len );
		}
	}
	catch ( ... )
	{
		if (mysql_log) printf("GMysqlClient::Gamed::MysqlQuery: exception error !!! \n");
	}
	return res;
}
//----------------------------------------------------------------------------------------------------------

bool GMysqlClient::CheckDBActive()
{
	bool res = false;
	pthread_mutex_lock(&mysql_mutex);
	try
	{
		if (active && DBMysql && mysql_state)
		{
			if (mysql_ping(DBMysql) == 0)
			{
				res = true;
			}			
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::Gamed::CheckDBActive: error: active = false \n");
		}
	}
	catch ( ... )
	{
		if (mysql_log) printf("GMysqlClient::Gamed::CheckDBActive: exception error !!! \n");
	}
	pthread_mutex_unlock(&mysql_mutex);
	return res;
}

//----------------------------------------------------------------------------------------------------------
bool GMysqlClient::CheckFailedExcept()
{
	bool res = false;
	if ( DBMysql )
	{
		if ( mysql_errno(DBMysql) )
		{
			active == false;
			const char * err = mysql_error(DBMysql);
			if (err)
			{
				if (mysql_log) printf("GMysqlClient: error: aa \n");			
			}
			res = true;
		}
	}
	else
	{
		active == false;
		if (mysql_log) printf("GMysqlClient: error: MYSQL == NULL \n");
		res = true;
	}
	return res;
}

//----------------------------------------------------------------------------------------------------------

bool GMysqlClient::SetRankingValue(const PATENTE_RANKING& playerData)
{
	bool res = false, mcs = false;
	MYSQL_MUTEX_BEGIN
	try
	{
		if (!CheckDBActive())
            return false;

		if (active && DBMysql && mysql_state)
		{
			while ( mysql_next_result(DBMysql) == 0 );	

			sprintf(QUERY,"REPLACE INTO patente_players (player_id, name_len, player_name, player_cls, player_gender, player_exp_elo, player_kill_elo, player_death_elo, player_lvl_elo, kda, updated_at) VALUES (%d, %d, '%s', %d, %d, %d, %d, %d, %d, %f, NOW())", playerData.player_id, playerData.name_len, playerData.player_name, playerData.player_cls, playerData.player_gender, playerData.player_exp_elo, playerData.player_kill_elo, playerData.player_death_elo, playerData.player_lvl_elo, playerData.kda);
			mcs = MysqlQuery(); 
			if (mcs)
			{
				result = mysql_store_result(DBMysql);
				if (result)
				{
					row = mysql_fetch_row(result);
					if( row )
					{
						res = true;
					}
					else
					{
						if (mysql_log) printf("GMysqlClient::SetRankingValue: error: row \n");
					}
					mysql_free_result(result);
				}
				else
				{
					if (mysql_log) printf("GMysqlClient::SetRankingValue: error: result \n");
				}
			}
			else
			{
				if (mysql_log) printf("GMysqlClient::SetRankingValue: error: %s \n", mysql_error(DBMysql) );
				Reconnect();
			}
	
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::SetRankingValue: error: mysql connect FAILED!!! \n");
			Reconnect();
		}
	}
	catch ( ... )
	{
		if (mysql_log) printf("GMysqlClient::SetRankingValue: exception error !!! \n");
	}
	MYSQL_MUTEX_END
	return res;
}

void GMysqlClient::Lock()
{
	pthread_mutex_lock(&query_mutex);
}

void GMysqlClient::Unlock()
{
	pthread_mutex_unlock(&query_mutex);
}

//----------------------------------------------------------------------------------------------------------
bool GMysqlClient::MakeRankingTopList(int patent_type)
{
    bool res = false, mcs = false;
    MYSQL_MUTEX_BEGIN
    try
    {
		if (!CheckDBActive())
            return false;

		ClearRanking();

        if (active && DBMysql && mysql_state)
        {
            while (mysql_next_result(DBMysql) == 0);

			if (patent_type == 0)
			{
				sprintf(QUERY, "SELECT * FROM `patente_players` ORDER BY `kda` DESC LIMIT 100");
			}
			else
			{
				sprintf(QUERY, "SELECT * FROM `patente_players` WHERE `player_lvl_elo` = %d ORDER BY `kda` DESC LIMIT 100", patent_type);
			}

			mcs = MysqlQuery();
            if (mcs)
            {
                result = mysql_store_result(DBMysql);

				if (result)
				{
					int i = 0;

					while ((row = mysql_fetch_row(result)) != nullptr && i < 200)
					{			
						PATENTE_RANKING player(
							atoi(row[0]),							
							atoi(row[3]),
							atoi(row[4]),
							atoi(row[5]),
							atoi(row[6]),
							atoi(row[7]),
							atoi(row[8]),
							atof(row[9]),
							atoi(row[1]),
							row[2]);

						patente_players.push_back(player);
						i++;
					}

					res = true;
					mysql_free_result(result);
				}
				else
                {
                    if (mysql_log) printf("GMysqlClient::MakeRankingTopList: error: result \n");
                }
            }
            else
            {
                if (mysql_log) printf("GMysqlClient::MakeRankingTopList: error: %s \n", mysql_error(DBMysql));
                Reconnect();
            }
        }
        else
        {
            if (mysql_log) printf("GMysqlClient::SetRankingValue: error: mysql connect FAILED!!! \n");
            Reconnect();
        }
    }
    catch (...)
    {
        if (mysql_log) printf("GMysqlClient::SetRankingValue: exception error !!! \n");
    }
    MYSQL_MUTEX_END
    return res;
}

//----------------------------------------------------------------------------------------------------------

bool GMysqlClient::SetCashRewardValue(const CASH_REWARD& playerData)
{
	bool res = false, mcs = false;
	MYSQL_MUTEX_BEGIN
	try
	{
		if (!CheckDBActive())
			return false;

		if (active && DBMysql && mysql_state)
		{
			while (mysql_next_result(DBMysql) == 0);

			sprintf(QUERY, "REPLACE INTO cash_reward (player_id, player_hwid) VALUES (%d, %lld)", playerData.player_id, playerData.player_hwid);
			mcs = MysqlQuery();
			if (mcs)
			{
				result = mysql_store_result(DBMysql);
				if (result)
				{
					row = mysql_fetch_row(result);
					if (row)
					{
						res = true;
					}
					else
					{
						if (mysql_log) printf("GMysqlClient::SetCashRewardValue: error: row \n");
					}
					mysql_free_result(result);
				}
				else
				{
					if (mysql_log) printf("GMysqlClient::SetCashRewardValue: error: result \n");
				}
			}
			else
			{
				if (mysql_log) printf("GMysqlClient::SetCashRewardValue: error: %s \n", mysql_error(DBMysql));
				Reconnect();
			}
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::SetCashRewardValue: error: mysql connect FAILED!!! \n");
			Reconnect();
		}
	}
	catch (...)
	{
		if (mysql_log) printf("GMysqlClient::SetCashRewardValue: exception error !!! \n");
	}
	MYSQL_MUTEX_END
	return res;
}

//----------------------------------------------------------------------------------------------------------

bool GMysqlClient::DropCashReward(int player_id)
{
	bool res = false, mcs = false;
	MYSQL_MUTEX_BEGIN
	try
	{
		if (!CheckDBActive())
			return false;

		if (active && DBMysql && mysql_state)
		{
			while (mysql_next_result(DBMysql) == 0);

			sprintf(QUERY, "DELETE FROM `cash_reward` WHERE `player_id` = %d", player_id);
			mcs = MysqlQuery();
			if (mcs)
			{
				result = mysql_store_result(DBMysql);
				if (result)
				{
					row = mysql_fetch_row(result);
					if (row)
					{
						res = true;
					}
					else
					{
						if (mysql_log) printf("GMysqlClient::DropCashReward: error: row \n");
					}
					mysql_free_result(result);
				}
				else
				{
					if (mysql_log) printf("GMysqlClient::DropCashReward: error: result \n");
				}
			}
			else
			{
				if (mysql_log) printf("GMysqlClient::DropCashReward: error: %s \n", mysql_error(DBMysql));
				Reconnect();
			}
		}
		else
		{
			if (mysql_log) printf("GMysqlClient::DropCashReward: error: mysql connect FAILED!!! \n");
			Reconnect();
		}
	}
	catch (...)
	{
		if (mysql_log) printf("GMysqlClient::DropCashReward: exception error !!! \n");
	}
	MYSQL_MUTEX_END
	return res;
}
std::pair<bool, int> GMysqlClient::CheckHwidExist(unsigned long long player_hwid)
{
    bool res = false, mcs = false;
    int player_id = -1;
    MYSQL_MUTEX_BEGIN
    try
    {
        if (!CheckDBActive())
            return std::make_pair(false, -1);

        if (active && DBMysql && mysql_state)
        {
            while (mysql_next_result(DBMysql) == 0);

            sprintf(QUERY, "SELECT `player_id` FROM `cash_reward` WHERE `player_hwid` = %lld", player_hwid);
            mcs = MysqlQuery();
            if (mcs)
            {
                result = mysql_store_result(DBMysql);

                if (result)
                {
                    row = mysql_fetch_row(result);
                    if (row)
                    {
                        res = true;                 
                        player_id = atoi(row[0]);
                    }
                    else
                    {
                        if (mysql_log) printf("GMysqlClient::CheckHwidExist: error: row \n");
                    }
                    mysql_free_result(result);
                }
                else
                {
                    if (mysql_log) printf("GMysqlClient::CheckHwidExist: error: result \n");
                }
            }
            else
            {
                if (mysql_log) printf("GMysqlClient::CheckHwidExist: error: %s \n", mysql_error(DBMysql));
                Reconnect();
            }
        }
        else
        {
            if (mysql_log) printf("GMysqlClient::CheckHwidExist: error: mysql connect FAILED!!! \n");
            Reconnect();
        }
    }
    catch (...)
    {
        if (mysql_log) printf("GMysqlClient::CheckHwidExist: exception error !!! \n");
    }
    MYSQL_MUTEX_END
    return std::make_pair(res, player_id);
}
