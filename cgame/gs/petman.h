#ifndef __ONLINE_GAME_GS_PET_MANAGER_H__
#define __ONLINE_GAME_GS_PET_MANAGER_H__

#include <common/types.h>
#include <amemory.h>
#include <common/base_wrapper.h>

class world;
class pet_gen_pos
{
public:
	static bool FindGroundPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool FindWaterPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool FindAirPos(A3DVECTOR & pos,float dis,A3DVECTOR & offset, world * plane);
	static bool IsValidInhabit(char leader_layer, int inhabit_type);
	static int FindValidPos(A3DVECTOR & pos, char & inhabit_mode, char leader_layer, int inhabit_type, world * plane, float dis=0.f, A3DVECTOR offset=A3DVECTOR(0.f,0.f,0.f));
};

struct extend_prop;
struct msg_pet_hp_notify;
#pragma pack(1)
struct pet_data
{
	enum
	{
		MAX_PET_SKILL_COUNT = 8,
		MAX_NAME_LEN = 16,
	};

	int honor_point;	//Favorability will change frequently
	int hunger_gauge;	//Hunger will change frequently
	int feed_period;	//elapsed time since last feed
	int pet_tid;		//Pet's Template ID
	int pet_vis_tid;	//The pet's visible ID (if 0, it means there is no special visible ID)
	int pet_egg_tid;	//The ID of the pet egg
	int pet_class;		//Pet type battle pet, riding pet, ornamental pet
	float hp_factor;	//HP ratio (used when reviving and recovering) 0 means death
	short level;		//pet level
	unsigned short color;   //Pet color, the highest bit is 1, which means it is valid, currently it is only valid for riding pets
	int exp;		//Pets Current Experience
	int skill_point;	//remaining skill points
	char is_bind;		//Whether it is in the binding state, it is now a mask 0x01 binding 0x02 can be traded on Treasure Hunting Network
	char unused;		
	unsigned short name_len;//name length
	char name[MAX_NAME_LEN];//name content
	struct __skills
	{
		int skill;
		int level;
	} skills[MAX_PET_SKILL_COUNT];
	struct __evo_prop
	{
		int r_attack;
		int r_defense;
		int r_hp;
		int r_atk_lvl;
		int r_def_lvl;
		int nature;
		int r_pen_lvl_f;
		int r_pen_lvl_m;
		int r_chi_point;
	} evo_prop;

	int reserved[7];//unused
	
	enum
	{
		PET_CLASS_MOUNT,		//Riding a pet
		PET_CLASS_COMBAT,		//battle pet
		PET_CLASS_FOLLOW,		//follow pets
		PET_CLASS_SUMMON,		//Summoner
		PET_CLASS_PLANT,		//plant
		PET_CLASS_EVOLUTION,	//evolution pet
		PET_CLASS_MAX,
	};

	enum
	{
		HUNGER_LEVEL_0,			//satiated
		HUNGER_LEVEL_1,			//normal
		HUNGER_LEVEL_2,			//level of hunger 
		HUNGER_LEVEL_3,	
		HUNGER_LEVEL_4,			//Hunger level two
		HUNGER_LEVEL_5,
		HUNGER_LEVEL_6,
		HUNGER_LEVEL_7,			//Hunger level three
		HUNGER_LEVEL_8,
		HUNGER_LEVEL_9,
		HUNGER_LEVEL_A,
		HUNGER_LEVEL_B,			//level four 
		HUNGER_LEVEL_COUNT
	};

	enum 
	{
		FEED_TIME_UNIT = 300,		//300	unit of time for feeding
	};

	enum
	{
		HONER_LEVEL_0,			//wild	0-50
		HONER_LEVEL_1,			//Repeated gratuitous	51-150
		HONER_LEVEL_2,			//obedient	151-500
		HONER_LEVEL_3,			//loyal	501-999
		HONOR_LEVEL_COUNT,

		HONOR_POINT_MAX	= 999
	};

};

#pragma pack()

class gplayer_imp;
class pet_manager
{
public:
	enum
	{
		MAX_PET_CAPACITY = 40,
		NOTIFY_MASTER_TIME = 16,
		MAX_SUMMON_CAPACITY = 1,				//There can only be one summoned object, which cannot be changed
		SUMMON_SLOT	= 20, 		//Summons are indexed in pet_list
	};

	enum RECALL_REASON
	{
		NONE = 0,
		PET_DEATH,			//pet death
		PET_LIFE_EXHAUST,	//pet life
		PET_SACRIFICE,		//pet sacrifice
	};
	
protected:
	pet_data * 	_pet_list[MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY];		//Pet List + Summon List
	unsigned int		_active_pet_slot;			//The maximum number of pet packages can be activated
	int 		_cur_active_pet;			//Index of currently swapped out pets

public:
	XID		_cur_pet_id;				//Current Pet ID
	unsigned char	_pet_summon_stamp;			//The number of times pets are summoned, used to distinguish between different summons
	bool		_is_on_underwater;			//Whether to carry out special treatment under water
	char		_cur_pet_aggro_state;			//current pet aggro
	char		_cur_pet_stay_state;			//The current pet's follow method
	char		_cur_pet_combat_state;			//The current pet's attack status
	bool		_need_feed;				//pets need to be fed
	short		_cur_pet_counter;			//The current pet summon timing, used to count the experience value
	short		_cur_pet_notify_counter;		//Timing of messages currently sent to pet update data
	unsigned int		_cur_pet_state;				//This flag is set if the pet has notified itself
	int		_cur_pet_hp;				//The current blood value of the pet 
	int		_cur_pet_inhabit;			//Current Pet Habitat
	float	_cur_pet_mp_factor;
	int		_cur_pet_mp;
	int 	_cur_pet_life;				//Current pet life, 0 is permanent
	int		_summon_skill_level;		//Summons the skill level used by the current pet
	int		_codex_active_pet_slot;

protected:
	void ClearCurPet(gplayer_imp * pImp)
	{
		_cur_active_pet = -1;
		_cur_pet_id = XID(-1,-1);
		//do others ....
	}
	void HandleFeedTimeTick(gplayer_imp * pImp, pet_data * pData);
	void ModifyHonor(pet_data * pData, int offset);
	void ModifyHungerGauge(pet_data * pData, int offset);
public:
	pet_manager();
	~pet_manager();

	void ActiveNoFeed(bool nofeed)
	{
		_need_feed = !nofeed;
	}

	void SetAvailPetSlot(unsigned int slot)
	{
		if(slot > _active_pet_slot && slot <= MAX_PET_CAPACITY)
		{
			_active_pet_slot = slot;
		}
	}

	unsigned int GetAvailPetSlot()
	{
		return _active_pet_slot;
	}

	bool IsPetActive() 
	{
		return _cur_active_pet >= 0;
	}

	int GetCurActivePet()
	{
		return _cur_active_pet;
	}

	void SetTestUnderWater(bool val)
	{
		_is_on_underwater = val;
	}

	const XID & GetCurPet()
	{
		return _cur_pet_id;
	}

	pet_data * GetPetData(unsigned int index)
	{
		if(index < MAX_PET_CAPACITY + MAX_SUMMON_CAPACITY)
		{
			return _pet_list[index];
		}
		else
		{
			return NULL;
		}
	}

	unsigned int GetPetsCount()
	{
		unsigned int cnt = 0;
		for(unsigned int i=0; i<MAX_PET_CAPACITY; i++)
			if(_pet_list[i]) ++cnt;
		return cnt;
	}

	bool DBSetPetData(unsigned int index, const void * data, unsigned int size, bool codex = false);
	void ClientGetPetRoom(gplayer_imp * pImp);

protected:
	bool DoActiveMount(gplayer_imp * pImp,pet_data * pData);
	void TestUnderWater(gplayer_imp * pImp, float offset);
	void DoHeartbeat(gplayer_imp * pImp);

public:
	void OnUnderWater(gplayer_imp * pImp, float offset)
	{
		if(!_is_on_underwater) return;
		TestUnderWater(pImp,offset);
	}	
	void Save(archive & ar);
	void Load(archive & ar);
	void Swap(pet_manager & rhs);

public:
	int AddPetData(const pet_data & data, bool codex = false);
	bool BanishPet(gplayer_imp * pImp, unsigned int index);
	void FreePet(gplayer_imp * pIMp, unsigned int index, bool codex = false);		//check-less release

	bool ActivePet(gplayer_imp * pImp, unsigned int index);
	bool RecallPet(gplayer_imp * pImp);
	bool RecallPetWithoutFree(gplayer_imp * pImp, char reason=0);	//only called internally

	bool FeedCurPet(gplayer_imp * pImp, int food_type, int honor);
	bool RelocatePos(gplayer_imp * pImp,const XID & who , int stamp,bool is_disappear);
	bool NotifyPetHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_pet_hp_notify & info);
	bool PetDeath(gplayer_imp * pImp,const XID & who , int stamp);
	bool PlayerPetCtrl(gplayer_imp * pImp,int cur_target,int pet_cmd, const void * buf, unsigned int size);
	void KillMob(gplayer_imp * pImp, int mob_level);
	int ResurrectPet(gplayer_imp * pImp, unsigned int index);
	int ResurrectPet(gplayer_imp * pImp);
	void NotifyMasterInfo(gplayer_imp * pImp,bool atonce);	
	void PreSwitchServer(gplayer_imp * pImp);
	void PostSwitchServer(gplayer_imp * pImp);
	void PlayerBeAttacked(gplayer_imp * pImp, const XID & target);
	void PetSetCoolDown(gplayer_imp *pImp, const XID & who, int idx, int msec);
	void NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack);
	int ChangePetName(gplayer_imp * pImp, unsigned int index, const char * name, unsigned int name_len);
	int ForgetPetSkill(gplayer_imp * pImp,int skill_id);
	int LearnSkill(gplayer_imp * pImp,int skill_id, int * level_result);
	void NotifyInvisibleData(gplayer_imp *pImp);

	/*166+*/
	int GetSkillsPetCount();
	bool SkillDataInc(gplayer_imp * pImp, int index_from, int index_to);
	bool PetBuySkinInc(gplayer_imp * pImp, int index, int pos_data);
	bool PetUseSkinInc(gplayer_imp * pImp, int index, int pos_data);
	int GetSkinsPetCount(gplayer_imp * pImp);
	bool RebuildStorage(gplayer_imp * pImp);

	void RecvExp(gplayer_imp * pImp, unsigned int pet_index,int exp);	
	bool AddExp(gplayer_imp *pImp,unsigned int pet_index,int exp);
	int x_LevelUp(gplayer_imp *pImp, unsigned int index, int level_counter);
	inline void Heartbeat(gplayer_imp * pImp);
	friend class petdata_imp;

	bool ActivePet2(gplayer_imp * pImp, pet_data & data, int life, int skill_level, const XID & target, char force_attack);
	void TryFreePet(gplayer_imp * pImp);		//Attempt to release a summon that has been recalled
	bool PetSacrifice(gplayer_imp * pImp);
	bool DyePet(gplayer_imp * pImp, unsigned int index, unsigned short color);
	bool EvolutionPet(gplayer_imp *pImp,unsigned int index,int evolution_id,int pet_nature,int skill1,int level1,int skill2,int level2);
	bool RebuildInheritRatio(int pet_id,int &r_attack,int &r_defense,int &r_hp,int &r_atk_lvl,int &r_def_lvl/*, int &r_pen_lvl_f, int &r_pen_lvl_m, int &r_chi_point*/);
	void PetAcceptInheritRatioResult(gplayer_imp *pImp,unsigned int index,int r_attack,int r_defense,int r_hp,int r_atk_lvl,int r_def_lvl/*, int r_pen_lvl_f, int r_pen_lvl_m, int r_chi_point*/);
	void PetAcceptNatureResult(gplayer_imp *pImp,unsigned int index,int nature,int skill1,int level1,int skill2,int level2);
	int GetNormalSkillNum(gplayer_imp *pImo,pet_data *pData);
	bool IsSkillNormal(gplayer_imp *pImp,pet_data *pData,int skill_id);
	void OnMountSpeedEnChanged(gplayer_imp *pImp);
	
	bool ActivePet3(gplayer_imp * pImp, pet_data & data, int life, int skill_level, const XID & target, char force_attack);
};

class petdata_imp
{
public:
	virtual bool DoActivePet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData, extend_prop * pProp) = 0;
	virtual bool DoRecallPet(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void TestUnderWater(gplayer_imp * pImp, pet_manager * pMan,pet_data * pData,float offset) = 0;
	virtual void LevelUp(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void Heartbeat(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData) = 0;
	virtual void OnHonorModify(gplayer_imp *pImp, pet_manager * pMan,pet_data * pData, int old_honor) = 0;
	virtual bool OnPetRelocate(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,bool is_disappear) = 0; 
	virtual bool OnPetNotifyHP(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp,const msg_pet_hp_notify & info) = 0;
	virtual bool OnPetDeath(gplayer_imp *pImp, pet_manager * pMan,pet_data *pData,const XID &who,int stamp) = 0;
	virtual bool OnPetCtrl(gplayer_imp * pImp, pet_manager * pMan,pet_data *pData,int cur_target,int pet_cmd, const void * buf, unsigned int size) = 0;
	virtual void OnKillMob(gplayer_imp * pImp, pet_manager * pMan, pet_data * pData, int mob_level) = 0;
	virtual void OnNotifyMasterInfo(gplayer_imp * pImp,pet_manager * pMan, pet_data * pData, bool at_once) = 0;
	virtual void PreSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData) = 0;
	virtual void PostSwitchServer(gplayer_imp * pImp, pet_manager * pman, pet_data * pData) = 0;
	virtual void OnMasterBeAttacked(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const XID &who) = 0; 
	virtual bool OnChangeName(gplayer_imp * pImp,pet_manager *pman, pet_data *pData,const char *name, unsigned int len) = 0; 
	virtual bool OnForgetSkill(gplayer_imp * pImp,pet_manager *pman, pet_data *pData, int skill_id) = 0;
	virtual bool OnLearnSkill(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,int skill_id, int * level_result) = 0;
	virtual bool OnDyePet(gplayer_imp * pImp,pet_manager *pMan,pet_data *pData,unsigned short color) = 0;
	virtual bool OnEvolution(gplayer_imp *pImp,pet_manager *pMan,pet_data *pData,int evolution_pet_id,int pet_nature,int skill1,int level1,int skill2,int level2) = 0;
	virtual void OnMountSpeedEnChanged(gplayer_imp *pImp,pet_manager * pMan, pet_data * pData) = 0;
protected:
	inline void ClearCurPet(gplayer_imp * pImp, pet_manager * pMan)
	{
		pMan->ClearCurPet(pImp);
	}
};

inline void pet_manager::Heartbeat(gplayer_imp * pImp)
{
	if(_cur_active_pet >= 0)
	{
		DoHeartbeat(pImp);
	}
}


//The following is the plant pet manager, because the plant switching server disappears, so there is no need to save
struct msg_plant_pet_hp_notify;
struct plant_pet_data
{
	XID id;
	int tid;
	int plant_group;
	int group_limit;
	int plant_attack_type;
	int life;
	float hp_factor;
	int hp;
	float mp_factor;
	int mp;
	unsigned int plant_state;
	bool is_suicide;
};

class plant_pet_manager
{
public:
	enum
	{
		PET_STATE_THRESHOLD		 = 15,	//If the pet has no notification information within 15 seconds, the pet will be recalled
		NOTIFY_MASTER_TIME		 = 15,	//Notify pet master info every 15 seconds
	};
	
	enum
	{
		PLANT_DEATH = 0,	//plant death
		PLANT_LIFE_EXHAUST,	//plant life to
		PLANT_OUT_OF_RANGE,	//plants out of bounds
		PLANT_SUICIDE,		//plant blew up
		PLANT_GROUP_LIMIT,	//The number of plants exceeds the group limit
	};
	
	typedef abase::vector<plant_pet_data, abase::fast_alloc<> > PLANT_LIST;
	PLANT_LIST _plant_list;					//plant list
	unsigned int _plant_notify_counter;

public:
	plant_pet_manager():_plant_notify_counter(0){}

	bool ActivePlant(gplayer_imp * pImp, pet_data & data, int life, int skill_level, const XID & target, char force_attack);
	bool PlantSuicide(gplayer_imp * pImp, float distance, const XID & target, char force_attack);
	void Heartbeat(gplayer_imp * pImp);
	void NotifyMasterInfo(gplayer_imp * pImp);	
	void PreSwitchServer(gplayer_imp * pImp);
	void NotifyStartAttack(gplayer_imp *pImp, const XID & target, char force_attack);
	void PlayerBeAttacked(gplayer_imp * pImp, const XID & target);
public:
	bool PlantDeath(gplayer_imp * pImp,const XID & who , int stamp);
	bool NotifyPlantHP(gplayer_imp * pImp,const XID & who , int stamp,const msg_plant_pet_hp_notify & info);
	bool PlantDisappear(gplayer_imp * pImp,const XID & who , int stamp);

};	
#endif

