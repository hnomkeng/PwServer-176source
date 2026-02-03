#ifndef __GNET_GMYSQLCLIENT_H
#define __GNET_GMYSQLCLIENT_H

#include <mysql/mysql.h>
#include <vector>

#define MYSQL_MUTEX_BEGIN Lock(); \
							try \
							{ 
								//TODO 
#define MYSQL_MUTEX_END		} \
							catch (...) \
							{ \
								printf("MYSQL::PANIC::EXCEPTION: ERROR \n"); \
							} \
							Unlock();

using namespace GNET;

class GMysqlClient
{
private:
	enum : size_t
	{
		MAX_IP_LEN = 20u,
		MAX_LOGIN_LEN = 64u,
		MAX_STR_LEN = 128u,
		
		MAX_QUERY_LEN = 2048,
		MAX_CLEAN_SIZE = 65535,
		MAX_RECONNECT_ATTEMPTS = 3,
	};

	bool active;
	bool mysql_log;
	int mysql_state;
	pthread_mutex_t mysql_mutex;
	pthread_mutex_t query_mutex;
	MYSQL *DBMysql;
	MYSQL_RES * result;
	MYSQL_ROW row;
	int query_state;
	char QUERY[MAX_QUERY_LEN];
	char RES[MAX_QUERY_LEN];

	char ip[MAX_IP_LEN];
	int  port;
	char user[MAX_STR_LEN];
	char passwd[MAX_STR_LEN];
	char db[MAX_STR_LEN];

public:
	struct PATENTE_RANKING
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

		PATENTE_RANKING(int player_id, int player_cls , int player_gender, int player_exp_elo, int player_kill_elo, int player_death_elo, int player_lvl_elo, float kda, short name_len, char player_name[16])
		{
			this->player_id = player_id ;			
			this->player_cls = player_cls;
			this->player_gender = player_gender;
			this->player_exp_elo = player_exp_elo;
			this->player_kill_elo = player_kill_elo;
			this->player_death_elo = player_death_elo;
			this->player_lvl_elo = player_lvl_elo;
			this->kda = kda;
			this->name_len = name_len;
			memcpy(this->player_name, player_name, name_len);
		};	
	};  std::vector <PATENTE_RANKING> patente_players;	

	struct CASH_REWARD
	{
		int player_id;		
		unsigned long long player_hwid;

		CASH_REWARD(int player_id, unsigned long long player_hwid)
		{
			this->player_id = player_id ;			
			this->player_hwid = player_hwid;
		};	
	};  std::vector <CASH_REWARD> cash_reward;	

public:
	void Init(const char * ip, const int port, const char * user, const char * passwd, const char * db);
	bool Connect();
	void Disconnect();
	void Reconnect();
	bool IsActive() { return active; }
	bool MysqlQuery();	
	bool CheckDBActive();
	bool CheckFailedExcept();

	inline std::vector<GMysqlClient::PATENTE_RANKING> GetAllRanking() const
	{
   		return patente_players;
	};
		
	inline int GetRankingSize() 
	{ 
		return patente_players.size(); 
	};

	bool SetRankingValue(const PATENTE_RANKING& playerData);
	bool MakeRankingTopList(int patent_type);
	inline void ClearRanking() 
	{
		std::vector<GMysqlClient::PATENTE_RANKING>::iterator it2 = patente_players.begin();
		for(; it2 != patente_players.end(); )
		patente_players.erase(it2);
	}

	inline std::vector<GMysqlClient::CASH_REWARD> GetAllCashReward() const
	{
   		return cash_reward;
	};

	inline int GetCashRewardSize() 
	{ 
		return cash_reward.size(); 
	};

	bool SetCashRewardValue(const CASH_REWARD& playerData);
	inline void ClearCashReward() 
	{
		std::vector<GMysqlClient::CASH_REWARD>::iterator it2 = cash_reward.begin();
		for(; it2 != cash_reward.end(); )
		cash_reward.erase(it2);
	}

	bool DropCashReward(int player_id);
	std::pair<bool, int> CheckHwidExist(unsigned long long player_hwid);

	inline int ATOI(const char * ptr)
	{
		if(ptr) 
		return atoi(ptr);
		return 0;
	}

	void AddrToStr(unsigned int ip, char * address)
	{    
		unsigned char bytes[4];
		bytes[0] = ip & 0xFF;
		bytes[1] = (ip >> 8) & 0xFF;
		bytes[2] = (ip >> 16) & 0xFF;
		bytes[3] = (ip >> 24) & 0xFF;
		sprintf(address,"%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
	}	

	void Lock();
	void Unlock();

	static GMysqlClient * GetInstance()
	{
		if (!instance)
		instance = new GMysqlClient();
		return instance;
	}
	static GMysqlClient * instance;
};


#endif
