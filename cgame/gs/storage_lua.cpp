#include <threadpool.h>
#include <malloc.h>
#include <unordered_map>
#include "threadusage.h"
#include "world.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "threadusage.h"
#include "player_imp.h"
#include "storage_lua.h"
#include "gsp_if.h"
#include <glog.h>


StorageLua* StorageLua::instance = NULL;

void StorageLua::Init()
{	
	memset(this,0x00,sizeof(StorageLua));	
}