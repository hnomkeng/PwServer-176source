#ifndef __GNET_STORAGE_LUA_NEW_H
#define __GNET_STORAGE_LUA_NEW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <sstream>

#include <glog.h>

class StorageLua
{
public:
	enum
	{	
		//
	};

	static StorageLua * instance;

public:

	struct CASH_REWARD
	{
		int player_id;
		unsigned long long player_hwid;

		CASH_REWARD(int player_id, unsigned long long player_hwid)
		{
			this->player_id = player_id;
			this->player_hwid = player_hwid;
		}
	};

public:
	void Init();

	inline std::vector<CASH_REWARD> LoadCashReward(const std::string &filename)
	{
		std::vector<CASH_REWARD> cash_reward;
		std::ifstream file(filename);
		std::string line;
		while (std::getline(file, line))
		{
			if (line.find("player_id") != std::string::npos)
			{
				int player_id;
				unsigned long long player_hwid;
				std::istringstream iss(line);
				std::string temp;
				iss >> temp >> player_id >> temp >> player_hwid;
				cash_reward.push_back(CASH_REWARD(player_id, player_hwid));
			}
		}
		return cash_reward;
	}

	inline bool AddCashReward(const std::string &filename, const CASH_REWARD &cash_reward)
	{
		std::ofstream file(filename, std::ios_base::app);
		if (!file.is_open())
		{
			return false;
		}

		GLog::log(GLOG_ERR, "AddCashReward: %d, %llu", cash_reward.player_id, cash_reward.player_hwid);

		file << "{ player_id = " << cash_reward.player_id << ", player_hwid = " << std::to_string(cash_reward.player_hwid) << " },\n";
		file.close();
		return true;
	}

	inline bool RemoveCashReward(const std::string &filename, int player_id)
	{
		std::ifstream file(filename);
		if (!file.is_open())
		{
			return false;
		}
		std::ofstream temp("/home/gamed/storage/temp.lua");
		std::string line;

		GLog::log(GLOG_ERR, "RemoveCashReward: %d", player_id);

		while (std::getline(file, line))
		{
			if (line.find("player_id = " + std::to_string(player_id)) == std::string::npos)
			{
				temp << line << std::endl;
			}
		}
		file.close();
		temp.close();
		remove(filename.c_str());
		rename("/home/gamed/storage/temp.lua", filename.c_str());
		return true;
	}

	inline std::pair<bool, int> CheckHwidExist(const std::string &filename, unsigned long long player_hwid)
	{
		std::ifstream file(filename);
		if (!file.is_open())
		{
			return {false, -1};
		}

		std::string line;
		while (std::getline(file, line))
		{
			if (line.find("player_hwid = " + std::to_string(player_hwid)) != std::string::npos)
			{
				int player_id;
				std::istringstream iss(line);
				std::string temp;
				iss >> temp >> player_id;
				file.close();
				return {true, player_id};
			}
		}
		file.close();
		return {false, -1};
	}

StorageLua()
{

}

~StorageLua()
{
	
}

static StorageLua * GetInstance()
{
	if (!instance)
	{
		instance = new StorageLua();
	}
	return instance;
}

};


#endif