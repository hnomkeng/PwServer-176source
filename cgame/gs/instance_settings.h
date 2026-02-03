#ifndef __GNET_INSTANCE_SETTINGS_H
#define __GNET_INSTANCE_SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>

class InstanceSettings
{
public:
	enum
	{	
		MAX_MAPS_BLOCK = 30,
	};

	static InstanceSettings * instance;

private:
	struct POS_REMOVED
	{
		int tag_id;
		int	pos_1;
		int	pos_2;
		int pos_3;
	};

private:
	bool enable_system;
	bool enable_block_hwid;
	bool enable_block_ip;
	int count_allowed_players;
    int maps_enable[MAX_MAPS_BLOCK];
	POS_REMOVED pos;
	
public:
	void Init();
	inline bool CheckEnableBlock() { return enable_system; };
	inline bool CheckEnableBlockHwid() { return enable_block_hwid; };
	inline bool CheckEnableBlockIp() { return enable_block_ip; };
	inline int CountMaxAllowedPlayers() { return count_allowed_players; };
	inline bool CheckMapEnable(int getmap_id)
	{
		for (unsigned int i = 0; i < MAX_MAPS_BLOCK; i++)
		{
			if(getmap_id == maps_enable[i]) return true;
		}		
		return false;
	}

	inline POS_REMOVED * GetMapPos() { &pos; };
	
InstanceSettings()
{

}

~InstanceSettings()
{
	
}

static InstanceSettings * GetInstance()
{
	if (!instance)
	{
		instance = new InstanceSettings();
	}
	return instance;
}

};


#endif