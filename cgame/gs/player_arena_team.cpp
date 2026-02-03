#include <common/utf.h>
#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include <db_if.h>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "usermsg.h"
#include "luamanager.h"
#include <glog.h>
#include "player_arena_team.h"

void gplayer_arena_team::Init()
{
	memset(&_my_arena_team,0,sizeof(_my_arena_team));
	memset(&_my_arena_reward_week,0,sizeof(_my_arena_reward_week));
	memset(&_my_arena_reward_season,0,sizeof(_my_arena_reward_season));
}
