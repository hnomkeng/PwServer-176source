#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "instance_settings.h"
#include "gsp_if.h"
#include <glog.h>

InstanceSettings* InstanceSettings::instance = NULL;

void InstanceSettings::Init()
{	
	memset(this,0x00,sizeof(InstanceSettings));	
		
	const char * conf_file = "settings.conf";
	Conf *instance_settings = Conf::GetInstance(conf_file);	
	
	// Ativa/Desativa o sistema de limitador
	enable_system = atoi(instance_settings->find("InstanceSettings", "enable_system").c_str());
	// Ativa/Desativa o sistema de limitador
	enable_block_hwid = atoi(instance_settings->find("InstanceSettings", "enable_block_hwid").c_str());	
	// Ativa/Desativa o sistema de limitador
	enable_block_ip = atoi(instance_settings->find("InstanceSettings", "enable_block_ip").c_str());
	// Quantidade de personagens/contas permitidas na instância
	count_allowed_players = atoi(instance_settings->find("InstanceSettings", "count_allowed_players").c_str());

	// Posição de Mapa que os jogadores serão levados
	pos.tag_id = atoi(instance_settings->find("RemoveMap", "tag_id").c_str());
	pos.pos_1 = atoi(instance_settings->find("RemoveMap", "pos_1").c_str());
	pos.pos_2 = atoi(instance_settings->find("RemoveMap", "pos_2").c_str());
	pos.pos_3 = atoi(instance_settings->find("RemoveMap", "pos_3").c_str());

	int count_maps = atoi(instance_settings->find("InstanceMaps", "count_maps").c_str());

	// Mapas Habilitados
	for (int i = 0; i < count_maps && i < MAX_MAPS_BLOCK; i++)
    {
        char section[MAX_MAPS_BLOCK];
        snprintf(section, sizeof(section), "map_%d", i);
        maps_enable[i] = atoi(instance_settings->find("InstanceMaps", section).c_str());
    }	

}