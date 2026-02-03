#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "emulate_settings.h"
#include "gsp_if.h"
#include <glog.h>

EmulateSettings* EmulateSettings::instance = NULL;

void EmulateSettings::Init()
{	
	memset(this,0x00,sizeof(EmulateSettings));	
		
	const char * conf_file = "settings.conf";
	Conf *emulate_settings = Conf::GetInstance(conf_file);	
	
	// Versão do Emulador
	emulate_version = atoi(emulate_settings->find("Static_Setting", "emulate_version").c_str()) > MAX_EMULATE_VERSION ? MAX_EMULATE_VERSION : atoi(emulate_settings->find("Static_Setting", "emulate_version").c_str());

	// Define as novas rates
	rate_config.mob_exp = atoi(emulate_settings->find("Rates_Setting", "mob_exp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "mob_exp").c_str());
	rate_config.mob_sp = atoi(emulate_settings->find("Rates_Setting", "mob_sp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "mob_sp").c_str());
	rate_config.mob_money = atoi(emulate_settings->find("Rates_Setting", "mob_money").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "mob_money").c_str());
	rate_config.mob_drop = atoi(emulate_settings->find("Rates_Setting", "mob_drop").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "mob_drop").c_str());
	rate_config.task_exp = atoi(emulate_settings->find("Rates_Setting", "task_exp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "task_exp").c_str());
	rate_config.task_sp = atoi(emulate_settings->find("Rates_Setting", "task_sp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "task_sp").c_str());
	rate_config.task_money = atoi(emulate_settings->find("Rates_Setting", "task_money").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "task_money").c_str());
	rate_config.realm_exp = atoi(emulate_settings->find("Rates_Setting", "realm_exp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "realm_exp").c_str());
	rate_config.pet_exp = atoi(emulate_settings->find("Rates_Setting", "pet_exp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "pet_exp").c_str());
	rate_config.genie_exp = atoi(emulate_settings->find("Rates_Setting", "genie_exp").c_str()) > MAX_RATE_VALUE ? MAX_RATE_VALUE : atoi(emulate_settings->find("Rates_Setting", "genie_exp").c_str());

	// Habilitar tradução do chat (Obsoleto)
	enabled_translation = atoi(emulate_settings->find("Static_Setting", "enabled_translation").c_str()) > MAX_ENABLED_TRANSLATE ? MAX_ENABLED_TRANSLATE : atoi(emulate_settings->find("Static_Setting", "enabled_translation").c_str());

	// Configurações de Nw
	nw_config.enabled_team = atoi(emulate_settings->find("Nw_Setting", "enabled_team").c_str()) > MAX_ENABLED_NW ? MAX_ENABLED_NW : atoi(emulate_settings->find("Nw_Setting", "enabled_team").c_str());
	nw_config.min_level_required_nw = atoi(emulate_settings->find("Nw_Setting", "min_level_required_nw").c_str()) > MAX_LEVEL ? MAX_LEVEL : atoi(emulate_settings->find("Nw_Setting", "min_level_required_nw").c_str());
	nw_config.min_level2_required_nw = atoi(emulate_settings->find("Nw_Setting", "min_level2_required_nw").c_str()) > MAX_LEVEL_2 ? MAX_LEVEL_2 : atoi(emulate_settings->find("Nw_Setting", "min_level2_required_nw").c_str());
	nw_config.min_realm_level_required_nw = atoi(emulate_settings->find("Nw_Setting", "min_realm_level_required_nw").c_str()) > MAX_REALM_LEVEL ? MAX_REALM_LEVEL : atoi(emulate_settings->find("Nw_Setting", "min_realm_level_required_nw").c_str());

	// Mensagens de Refino
	enabled_refine_msg = atoi(emulate_settings->find("Static_Setting", "enabled_refine_msg").c_str()) > MAX_ENABLED_REFINE ? MAX_ENABLED_REFINE : atoi(emulate_settings->find("Static_Setting", "enabled_refine_msg").c_str());
	refine_msg_lvl_min = atoi(emulate_settings->find("Static_Setting", "refine_msg_lvl_min").c_str()) > MAX_REFINE_LVL ? MAX_REFINE_LVL : atoi(emulate_settings->find("Static_Setting", "refine_msg_lvl_min").c_str());
	msg_language = atoi(emulate_settings->find("Static_Setting", "msg_language").c_str()) > MAX_MSG_LANGUAGE ? MAX_MSG_LANGUAGE : atoi(emulate_settings->find("Static_Setting", "msg_language").c_str());

	killed_global_message = atoi(emulate_settings->find("Static_Setting", "killed_global_message").c_str()) > MAX_ENABLED_KILLED_GLOBAL_MESSAGE ? MAX_ENABLED_KILLED_GLOBAL_MESSAGE : atoi(emulate_settings->find("Static_Setting", "killed_global_message").c_str());
	enabled_event_forbid = atoi(emulate_settings->find("Static_Setting", "enabled_event_forbid").c_str()) > MAX_ENABLED_EVENT_FORBID ? MAX_ENABLED_EVENT_FORBID : atoi(emulate_settings->find("Static_Setting", "enabled_event_forbid").c_str());
	
	// Configurações padrão
	enabled_codex = atoi(emulate_settings->find("Default_Setting", "enabled_codex").c_str()) > MAX_ENABLED_CODEX ? MAX_ENABLED_CODEX : atoi(emulate_settings->find("Default_Setting", "enabled_codex").c_str());
	enabled_memorial_reset = atoi(emulate_settings->find("Default_Setting", "enabled_memorial_reset").c_str()) > MAX_ENABLED_MEMORIAL_RESET ? MAX_ENABLED_MEMORIAL_RESET : atoi(emulate_settings->find("Default_Setting", "enabled_memorial_reset").c_str());

	// Configurações personalizadas
	enabled_camp_token = atoi(emulate_settings->find("Custom_Setting", "enabled_camp_token").c_str()) > MAX_ENABLED_CAMP_TOKEN ? MAX_ENABLED_CAMP_TOKEN : atoi(emulate_settings->find("Custom_Setting", "enabled_camp_token").c_str());
	enabled_child = atoi(emulate_settings->find("Custom_Setting", "enabled_child").c_str()) > MAX_ENABLED_CHILD ? MAX_ENABLED_CHILD : atoi(emulate_settings->find("Custom_Setting", "enabled_child").c_str());
	child_awakening_days = atoi(emulate_settings->find("Custom_Setting", "child_awakening_days").c_str()) > MAX_CHILD_AWAKENING_DAYS ? MAX_CHILD_AWAKENING_DAYS : atoi(emulate_settings->find("Custom_Setting", "child_awakening_days").c_str());
	
	enabled_ranking = atoi(emulate_settings->find("Custom_Setting", "enabled_ranking").c_str()) > MAX_ENABLED_RANKING ? MAX_ENABLED_RANKING : atoi(emulate_settings->find("Custom_Setting", "enabled_ranking").c_str());
	enabled_newhistory = atoi(emulate_settings->find("Custom_Setting", "enabled_newhistory").c_str()) > MAX_ENABLED_NEWHISTORY ? MAX_ENABLED_NEWHISTORY : atoi(emulate_settings->find("Custom_Setting", "enabled_newhistory").c_str());
	enabled_new_dungeons = atoi(emulate_settings->find("Custom_Setting", "enabled_new_dungeons").c_str()) > MAX_ENABLED_NEW_DUNGEONS ? MAX_ENABLED_NEW_DUNGEONS : atoi(emulate_settings->find("Custom_Setting", "enabled_new_dungeons").c_str());
	boss_rush_mutiple = atoi(emulate_settings->find("Custom_Setting", "boss_rush_mutiple").c_str()) > MAX_BOSS_RUSH_MUTIPLE ? MAX_BOSS_RUSH_MUTIPLE : atoi(emulate_settings->find("Custom_Setting", "boss_rush_mutiple").c_str());

	enabled_reward_gold = atoi(emulate_settings->find("Custom_Setting", "enabled_reward_gold").c_str()) > MAX_ENABLED_REWARD_GOLD ? MAX_ENABLED_REWARD_GOLD : atoi(emulate_settings->find("Custom_Setting", "enabled_reward_gold").c_str());
	enabled_blatte_pass = atoi(emulate_settings->find("Custom_Setting", "enabled_blatte_pass").c_str()) > MAX_ENABLED_BLATTE_PASS ? MAX_ENABLED_BLATTE_PASS : atoi(emulate_settings->find("Custom_Setting", "enabled_blatte_pass").c_str());
	enabled_arena = atoi(emulate_settings->find("Custom_Setting", "enabled_arena").c_str()) > MAX_ENABLED_ARENA ? MAX_ENABLED_ARENA : atoi(emulate_settings->find("Custom_Setting", "enabled_arena").c_str());
	totay_count_max_battle = atoi(emulate_settings->find("Custom_Setting", "totay_count_max_battle").c_str()) > TODAY_COUNT_MAX_BATTLE ? TODAY_COUNT_MAX_BATTLE : atoi(emulate_settings->find("Custom_Setting", "totay_count_max_battle").c_str());

	// Configurações de Missões
	task_default_count = atoi(emulate_settings->find("Tasks_Setting", "task_default_count").c_str()) > MAX_TASK_COUNT ? MAX_TASK_COUNT : atoi(emulate_settings->find("Tasks_Setting", "task_default_count").c_str());
	task_expand_count = atoi(emulate_settings->find("Tasks_Setting", "task_expand_count").c_str()) > MAX_TASK_COUNT ? MAX_TASK_COUNT : atoi(emulate_settings->find("Tasks_Setting", "task_expand_count").c_str());
	task_hidden_count = atoi(emulate_settings->find("Tasks_Setting", "task_hidden_count").c_str()) > MAX_TASK_COUNT ? MAX_TASK_COUNT : atoi(emulate_settings->find("Tasks_Setting", "task_hidden_count").c_str());
	task_title_count = atoi(emulate_settings->find("Tasks_Setting", "task_title_count").c_str()) > MAX_TASK_COUNT ? MAX_TASK_COUNT : atoi(emulate_settings->find("Tasks_Setting", "task_title_count").c_str());

}