#ifndef __GNET_ASSOCIATION_MANAGER_H
#define __GNET_ASSOCIATION_MANAGER_H


class AssociationConfig
{
public:
	enum 
	{
		IDX_ATK_CAMP_LEVEL_CONFIG = 6146,
		IDX_DEF_CAMP_LEVEL_CONFIG = 6147,

		IDX_ATK_BATTLE_TECH_TREE_CONFIG = 6148,
		IDX_DEF_BATTLE_TECH_TREE_CONFIG = 6149,	
	};

protected:

	struct LEVEL_CONFIG
	{
		int require_scor;
		unsigned int award_item_id;
	};

	struct BATTLE_TECH_TREE_CONFIG
	{
		unsigned int parent_node;
		unsigned int common_value1;
		unsigned int common_value2;
	};

public:
	
	LEVEL_CONFIG atk_camp_level_config[16];
	LEVEL_CONFIG def_camp_level_config[16];

	BATTLE_TECH_TREE_CONFIG atk_battle_tech_tree_config[16];
	BATTLE_TECH_TREE_CONFIG def_battle_tech_tree_config[16];


public:
	void Init();
	LEVEL_CONFIG * GetAtkCampLevelConfig() { return atk_camp_level_config; }
	LEVEL_CONFIG * GetDefCampLevelConfig() { return def_camp_level_config; }

	BATTLE_TECH_TREE_CONFIG * GetAtkBattleTechTreeConfig() { return atk_battle_tech_tree_config; }
	BATTLE_TECH_TREE_CONFIG * GetDefBattleTechTreeConfig() { return def_battle_tech_tree_config; }

	
public:

	AssociationConfig();
	~AssociationConfig();

	static AssociationConfig * GetInstance()
	{
		if (!instance)
		instance = new AssociationConfig();
		return instance;
	}
	static AssociationConfig * instance;
};

#endif
