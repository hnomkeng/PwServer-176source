#ifndef __PLAYER_NEWHISTORY_H__
#define __PLAYER_NEWHISTORY_H__

class gplayer_newhistory
{
public:
	enum 
	{
		//
	};
private:	
	struct NEWHISTORY
	{
		char enabled_systeam;
		unsigned int player_level;
		unsigned int player_realm_level;
		char enabled_buff;
		char enabled_level[3];
		char reward_level[3];
		char enabled_realm_level[3];
		char reward_realm_level[5];
	};
private:	
	NEWHISTORY player_newhistory;
public:	
	void Init();

	void SetEnabledSysteam(char enabled_systeam) { player_newhistory.enabled_systeam = enabled_systeam; }
	bool GetEnabledSysteam() { return player_newhistory.enabled_systeam; }

	void SetPlayerLevel(unsigned int player_level) { player_newhistory.player_level = player_level; }
	unsigned int GetPlayerLevel() { return player_newhistory.player_level; }

	void SetPlayerRealmLevel(unsigned int player_realm_level) { player_newhistory.player_realm_level = player_realm_level; }
	unsigned int GetPlayerRealmLevel() { return player_newhistory.player_realm_level; }

	void SetEnabledBuff(char enabled_buff) { player_newhistory.enabled_buff = enabled_buff; }
	bool GetEnabledBuff() { return player_newhistory.enabled_buff; }

	void SetEnabledLevel(int pos, bool enabled_level) { player_newhistory.enabled_level[pos] = enabled_level; }
	bool GetEnabledLevel(int pos) { return player_newhistory.enabled_level[pos]; }
	char * GetEnabledLevel() { return player_newhistory.enabled_level; }

	void SetRewardLevel(int pos, bool reward_level) { player_newhistory.reward_level[pos] = reward_level; }
	bool GetRewardLevel(int pos) { return player_newhistory.reward_level[pos]; }
	char * GetRewardLevel() { return player_newhistory.reward_level; }

	void SetEnabledRealmLevel(int pos, bool enabled_realm_level) { player_newhistory.enabled_realm_level[pos] = enabled_realm_level; }
	bool GetEnabledRealmLevel(int pos) { return player_newhistory.enabled_realm_level[pos]; }
	char * GetEnabledRealmLevel() { return player_newhistory.enabled_realm_level; }

	void SetRewardRealmLevel(int pos, bool reward_realm_level) { player_newhistory.reward_realm_level[pos] = reward_realm_level; }
	bool GetRewardRealmLevel(int pos) { return player_newhistory.reward_realm_level[pos]; }
	char * GetRewardRealmLevel() { return player_newhistory.reward_realm_level; }
	
};


#endif