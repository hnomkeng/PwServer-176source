#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <threadpool.h>
#include <conf.h>
#include <io/pollio.h>
#include <io/passiveio.h>
#include <gsp_if.h>
#include <db_if.h>
#include <amemory.h>
#include <meminfo.h>
#include <strtok.h>
#include <glog.h>

#include "../template/itemdataman.h"
#include "../world.h"
#include "../player_imp.h"
#include "../npc.h"
#include "../matter.h"
#include "../playertemplate.h"
#include "instance_config.h"
#include "instance_manager.h"
#include "../template/globaldataman.h"
#include <factionlib.h>
#include "faction_world_ctrl.h"
#include "../aei_filter.h"


bool 
instance_world_manager::InitNetClient(const char * gmconf)
{
	extern unsigned int _task_templ_cur_version;
	char version[1024];
	int ele_version = ELEMENTDATA_VERSION;
	int task_version = _task_templ_cur_version;
	int gshop_version = globaldata_getmalltimestamp();
	int gdividendshop_version = globaldata_getmall2timestamp();
	int gcashvipshop_version = globaldata_getmall3timestamp();
	sprintf(version, "%x%x%x%x%x", ele_version, task_version, gshop_version, gdividendshop_version, gcashvipshop_version);

	rect rt = _plane_template->GetLocalWorld();
	ASSERT(rt.left < rt.right && rt.top < rt.bottom);
	GMSV::InitGSP(gmconf,world_manager::GetWorldIndex(),_world_tag,rt.left,rt.right,rt.top,rt.bottom,version);
	GDB::init_gamedb();
	return true;
}

static bool quit_flag = false; 
static void timer_thread()
{
	g_timer.timer_thread();
}

static void poll_thread()
{
	for(;!quit_flag;)
	{
		ONET::PollIO::Poll(50);
	}
	__PRINTF("poll thread  terminated\n");
}
static void str2rect(rect & rt,const char * str)
{
	sscanf(str,"{%f,%f} , {%f,%f}",&rt.left,&rt.top,&rt.right,&rt.bottom);
}
	
void 
instance_world_manager::timer_tick(int index,void *object,int remain)
{
	class World_Tick_Task : public ONET::Thread::Runnable, public abase::ASmallObject
	{
		public:
		instance_world_manager * _manager;
		World_Tick_Task(instance_world_manager * manager):_manager(manager){}
		virtual void Run()
		{
			_manager->Heartbeat();
			delete this;
		}
	};
	ONET::Thread::Pool::AddTask(new World_Tick_Task((instance_world_manager*)object));
}

world * 
instance_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);
	//�����������������
	pPlane->w_life_time = _life_time;
	return pPlane;
}

world_message_handler * 
instance_world_manager::CreateMessageHandler()
{
	return new instance_world_message_handler(this);
}

int
instance_world_manager::Init(const char * gmconf_file,const char *  servername)
{
	_message_handler = CreateMessageHandler();
	_manager_instance = this;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	/*
	 *      ��ʼ��ʼ������
	 */
	Conf *conf = Conf::GetInstance();
	Conf::section_type section = "Instance_";
	section += servername;

	PreInit(servername);

	//��ȡ��������
	_player_limit_per_instance = atoi(conf->find(section,"player_per_instance").c_str());
	if(_player_limit_per_instance <= 0) 
	{
		__PRINTINFO("ÿ����������������������0\n");
		return -1010;
	}
	_effect_player_per_instance = atoi(conf->find(section,"effect_player_per_instance").c_str());
	if(_effect_player_per_instance < 0) 
	{
		__PRINTINFO("ÿ��������Ч�����������ڻ����0\n");
		return -1010;
	}
	
	unsigned int instance_count = atoi(conf->find(section,"instance_capacity").c_str());
	if(instance_count > MAX_INSTANCE_PER_SVR) instance_count = MAX_INSTANCE_PER_SVR;
	_planes_capacity = instance_count;

	_pool_threshold_low  =  atoi(conf->find(section,"pool_threshold_low").c_str());
	_pool_threshold_high =  atoi(conf->find(section,"pool_threshold_high").c_str());
	int pool_threadhold_init = atoi(conf->find(section,"pool_threshold_init").c_str());

	if(_pool_threshold_low <= 0 ||  _pool_threshold_high <=0
			|| _pool_threshold_low > _pool_threshold_high)
	{
		__PRINTINFO("���绺�����ֵ����\n");
		return -1011;
	}

	if(pool_threadhold_init > _pool_threshold_high) pool_threadhold_init = _pool_threshold_high;
	if(pool_threadhold_init <= 0) pool_threadhold_init = (_pool_threshold_low + _pool_threshold_high)/2;
	

	_player_max_count = atoi(conf->find(section,"player_capacity").c_str());
	_npc_max_count = atoi(conf->find(section,"npc_count").c_str());
	_matter_max_count = atoi(conf->find(section,"matter_count").c_str());
	if(_player_max_count > INS_MAX_PLAYER_COUNT)  _player_max_count = INS_MAX_PLAYER_COUNT;
	if(_npc_max_count > INS_MAX_NPC_COUNT) _npc_max_count = INS_MAX_NPC_COUNT;
	if(_matter_max_count > INS_MAX_MATTER_COUNT) _matter_max_count = INS_MAX_MATTER_COUNT;
	if( _player_max_count ==0 || _npc_max_count ==0 || _matter_max_count == 0)
	{
		__PRINTINFO("invalid argument player_count npc_count matter_count\n");
		return -1002;
	}

	int itime = atoi(conf->find(section,"idle_time").c_str());
	if(itime >= 30) _idle_time = itime;

	int ltime = atoi(conf->find(section,"life_time").c_str());
	if(ltime == -1 || ltime >= 300)			//���Ƹ�����������С��5���� 
		_life_time = ltime;
	
	__PRINTINFO("idle_time %d\n",_idle_time);
	__PRINTINFO("life_time %d\n",_life_time);
	
	if(strcmp(conf->find(section,"owner_mode").c_str(),"single") == 0)
	{
		_owner_mode = OWNER_MODE_SINGLE;
		__PRINTINFO("����������ģʽ:����\n");
	}
	else
	{
		_owner_mode = OWNER_MODE_TEAM;
		__PRINTINFO("����������ģʽ:����\n");
	}
	
	glb_verbose = 200;
	time_t ct = time(NULL);
	__PRINTINFO("%s\n",ctime(&ct));

	//��ʼ�����еĹ�����
	world_manager::Init();
	if(int irst = world_manager::InitBase(section.c_str()))
	{
		//��ʼ����������
		return irst;
	}
	
	//�õ���Ŀ¼
	std::string root = conf->find("Template","Root");

	//�õ�����Ŀ¼
	std::string base_path;
	base_path = root + conf->find(section,"base_path");
	__PRINTINFO("��Դ��Ŀ¼:'%s'\n", base_path.c_str());
	
	//�õ����������Ĳ���
	_restart_shell  = base_path + conf->find("Template","RestartShell");

	//�õ����������cid
	if(!_cid.Init(conf->find(section,"cid").c_str()))
	{
		__PRINTINFO("�����classid �� 'cid'��\n");
		return -1008;
	}

	//��ʼ����һ������ 
	_plane_template = CreateWorldTemplate();
	world & plane = *_plane_template;

	/*
	 *      ��ʼ����������
	 *
	 */
	std::string str = conf->find(section,"grid");
	int row=800,column=800;
	float xstart=0.f,ystart=0.f,step=12.5f;
	sscanf(str.c_str(),"{%d,%d,%f,%f,%f}",&column,&row,&step,&xstart,&ystart);

	if(!plane.CreateGrid(row,column,step,xstart,ystart)){
		__PRINTINFO("Can not create world grid!\n");
		return -1;
	}

	rect rt = plane.GetGrid().grid_region;
	__PRINTINFO("Create grid: %d*%d with step %f\n",row,column,step);
	__PRINTINFO("Grid Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	
	rect local_rt,base_rt;
	str2rect(base_rt,conf->find(section,"base_region").c_str());
	str2rect(local_rt,conf->find(section,"local_region").c_str());
	if(!plane.GetGrid().SetRegion(local_rt,base_rt,100.f))
	{
		__PRINTINFO("�����ļ��е��������ݲ���ȷ(base_region/local_region)\n");
		return -2;
	}

	str2rect(rt,conf->find(section,"inner_region").c_str());
	plane.GetGrid().inner_region = rt;
	float grid_sight_range = atof(conf->find(section, "grid_sight_range").c_str()); 
	if(grid_sight_range < 20.f || grid_sight_range > 100.f) grid_sight_range = 100.f;
	plane.BuildSliceMask((grid_sight_range>40.f ? 40.f : grid_sight_range), grid_sight_range); 

	rt = plane.GetGrid().local_region;
	__PRINTINFO("Local Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	rt = plane.GetGrid().inner_region;
	__PRINTINFO("Inner Region: {%.2f,%.2f} - {%.2f,%.2f}\n",rt.left,rt.top,rt.right,rt.bottom);
	if(rt.left > rt.right - 100.f || rt.top > rt.bottom - 100.f)
	{
		__PRINTINFO("�ڲ������С\n");
		return -5;
	}

	//����ͳ��һ���ڴ�ռ�ã���������˱�Ҫֵ�����ܽ���
	unsigned long long mem_need = row*column*sizeof(slice);

	mem_need *= _pool_threshold_high;
	mem_need += _player_max_count * sizeof(gplayer);
	mem_need += _npc_max_count * sizeof(gnpc);
	mem_need += _matter_max_count * sizeof(gmatter);
	mem_need += instance_count * sizeof(world);

	unsigned long long mem_std = mem_need;
	mem_std += _player_max_count * sizeof(gplayer_imp);
	mem_std += (unsigned long long)(0.5*_npc_max_count *sizeof(gnpc_imp)); 
	mem_std += _matter_max_count * sizeof(gmatter_imp);

	unsigned long long mem_max = mem_need;
	mem_max += (instance_count - _pool_threshold_high)*row*column*sizeof(slice);
	mem_max += (unsigned long long)(0.5*_npc_max_count *sizeof(gnpc_imp)); 
	mem_max += (_player_max_count + _npc_max_count)*1536;

	unsigned int mem_now = GetMemTotal();
	float mem_ratio1 = ((mem_need/1024.f)/mem_now)*100.f;
	float mem_ratio2 = ((mem_std/1024.f)/mem_now)*100.f;
	float mem_ratio3 = ((mem_max/1024.f)/mem_now)*100.f;
	__PRINTINFO("�ܼ�%d�������ռ�\n",instance_count);
	__PRINTINFO("Ԥ���ڴ�����:\n");
	__PRINTINFO("��״̬  \t %dkB(%.2f%%)\n",(int)(mem_need/1024), mem_ratio1);
	__PRINTINFO("��׼״̬\t %dKb(%.2f%%)\n",(int)(mem_std/1024), mem_ratio2);
	__PRINTINFO("�����\t %dKb(%.2f%%)\n",(int)(mem_max/1024), mem_ratio3);
	
	if(mem_now * 0.6 < mem_std/1024)
	{
		__PRINTINFO("��׼״̬������ڴ泬�������������ڴ����ֵ(need:%dmB/threshold:%dmB/total:%dmB)\n",(int)(mem_std/(1024*1024)), (int)(mem_now*0.6/1024), mem_now/1024);
		return -1001;
	}

	//��ʼ����ͼ��Դ: ���Ρ�Ѱ·����ײ������ͼ
	if(_mapres.Init(servername, base_path, plane.GetLocalWorld(), &plane) != 0)
	{
		__PRINTF("��ʼ����ͼ��Դʧ��\n");
		return -6;	
	}

	std::string  regionfile= base_path + conf->find("Template","RegionFile");
	std::string  regionfile2= base_path + conf->find("Template","RegionFile2");
	std::string pathfile = base_path + conf->find("Template","PathFile");
	//����player_temp���޸������ļ�������Ҫ�ȱ���һ���ļ�����

	if(!player_template::Load("ptemplate.conf",&_dataman))
	{
		__PRINTINFO("can not load player template data from file template file or 'ptemplate.conf'\n");
		return -7;
	}

	//װ�س�������
	if(!player_template::LoadRegionData(regionfile.c_str(),regionfile2.c_str()))
	{
		__PRINTINFO("can not load city region data from file '%s'\n",regionfile.c_str());
		return -7;
	}

	player_template::GetRegionTime(_region_file_tag,_precinct_file_tag);
    //��ʼ��ÿ�������ڵĴ��͵� 
    world_manager::InitRegionWayPointMap();

	//��ȡ·���ļ�
	if(!_pathman.Init(pathfile.c_str()))
	{
		__PRINTINFO("�޷���·���ļ�\n");
		return -9;
	}

	__PRINTINFO("��ʼ������������\n");

	//���ɼ��������б�
	_cur_planes.insert(_cur_planes.end(),instance_count,0);
	_planes_state.insert(_planes_state.end(),instance_count,0);

	//���ɱ�׼�����
	for(int i = 0; i < pool_threadhold_init; i ++)
	{
		_planes_pool.push_back(new world);
		_plane_template->DuplicateWorld(_planes_pool[i]);
	}

	for(int i = 0; i < pool_threadhold_init; i ++)
	{
		int rst = _planes_pool[i]->RebuildMapRes();
		sleep(1);
		ASSERT(rst == 0);
		_planes_pool[i]->w_plane_index = -1;
		_mapres.BuildNpcGenerator(_planes_pool[i]);
	}
	
	_max_active_index = 0;

	/*
	 *       ������ʱ���߳� 
	 */
	ONET::Thread::Pool::CreateThread(timer_thread);

	//��ʼ����������tick
	g_timer.set_timer(1,0,0,timer_tick,this);

	/**
	 *      ��ʼ��PollIO��������Ӧ�� Poll�߳�
	 */
	ONET::PollIO::Init();
	ONET::Thread::Pool::CreateThread(poll_thread);

	/*
	 *      ��ʼ����Ϸ������֮������ӳ�
	 */
	if(!InitNetIO(servername))
	{
		return -7;
	}

	/**
	 *      ��ʼ��������Ϣ���� 
	 */
	_msg_queue.Init();

	//�������һ�����������в����Ժ�ſ�ʼ�������з�����
	InitNetClient(gmconf_file);
	GLog::init();

	FinalInit(servername);

	trace_manager2::ReleaseElement();        
	return 0;
}

bool 
instance_world_manager::InitNetIO(const char * servername)
{
	_ioman.SetPlane(this);
	//�ӵ�һ������ȡ������
	grid & g = _plane_template->GetGrid();
	return _ioman.Init(servername,g.local_region,g.inner_region);
}

void 
instance_world_manager::SendRemoteMessage(int id, const MSG & msg)
{
	//������Ҫ������Ϣ����
	return _ioman.SendMessage(id,msg);
}

int  
instance_world_manager::BroadcastSvrMessage(const rect & rt,const MSG & message,float extend_size)
{
	//��������Ҫ���з������㲥
	return 0;
}

int 
instance_world_manager::GetServerNear(const A3DVECTOR & pos) const
{
	//����������Server near
	return -1;
}

int 
instance_world_manager::GetServerGlobal(const A3DVECTOR & pos) const
{
	return _ioman.GetGlobalServer(pos,_world_tag);
}


void 
instance_world_manager::RestartProcess()
{
	//�����������˶����� 
	gplayer * pPool = GetPlayerPool();
	for(unsigned int i = 0; i < world_manager::GetMaxPlayerCount(); i ++)
	{
		if(pPool[i].IsEmpty()) continue;
		if(!pPool[i].imp) continue;
		int cs_index = pPool[i].cs_index;
		if(cs_index <=0) continue;
		GMSV::SendDisconnect(cs_index,pPool[i].ID.id,pPool[i].cs_sid,0);
	}

	if(!fork())
	{
		for(int i =3;i < getdtablesize(); i ++)
		{
			close(i);
		}
		sleep(1);
		system(_restart_shell.c_str());
	}
}

void 
instance_world_manager::ShutDown()
{
	unsigned int ins_count = _max_active_index;
	for(unsigned int i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		if(pPlane->w_ctrl) pPlane->w_ctrl->OnServerShutDown();
	}

	world_manager::ShutDown();
}

gplayer* 
instance_world_manager::FindPlayer(int uid, int & world_index)
{	
	int widx = GetPlayerWorldIdx(uid);
	if(widx < 0) return NULL;
	world * pPlane = _cur_planes[widx];

	if(pPlane == NULL) return NULL;
	int index = pPlane->FindPlayer(uid);
	if(index < 0) return NULL;
	world_index = widx;
	return pPlane->GetPlayerByIndex(index);
}

/*
gplayer* 
instance_world_manager::FindPlayerByIdx(int i)
{	
	int widx = GetPlayerWorldIdx(uid);
	if(widx < 0) return NULL;
	
	world * pPlane = _cur_planes[widx];
	if(pPlane == NULL) return NULL;
	
	if(i < 0 || i > pPlane->GetObjCountPool()) return NULL;
	return pPlane->GetPlayerByIndex(i);
}
*/

int
instance_world_manager::SendRemotePlayerMsg(int uid, const MSG & msg)
{
	int w_idx = GetPlayerServerIdx(uid);
	if( w_idx >= 0)
	{
		//����Ҫ����һЩ��Ϣ
		if(w_idx != _world_index)
		{
			//ת�����ⲿ������
			SendRemoteMessage(w_idx,msg);
		}
		else
		{
			//ת�������������һ��λ�� ?			
			int widx = GetPlayerWorldIdx(uid);
			if(widx < 0) return -1;
			world * pPlane = _cur_planes[widx];
			if(pPlane) pPlane->PostLazyMessage(msg);
		}
	}
	else
	{
		__PRINTF("can not find extern player %d\n",uid);
	}
	return w_idx;
}

unsigned int
instance_world_manager::GetWorldCapacity()
{
	return _planes_capacity;
}

int 
instance_world_manager::GetOnlineUserNumber()
{
	return w_player_man.GetAllocedCount();
}

world * 
instance_world_manager::GetWorldByIndex(unsigned int index)
{
	return _cur_planes[index];
}

void 
instance_world_manager::GetPlayerCid(player_cid & cid)
{
	cid = _cid;
}

void 
instance_world_manager::FreeWorld(world * pPlane, int index)
{
	mutex_spinlock(&_key_lock);
	if(_planes_state[index] == 0 || _cur_planes[index] != pPlane 
			|| pPlane->w_player_count || pPlane->w_obsolete == 0 ) 
	{
		mutex_spinunlock(&_key_lock);
		return;
	}
	pPlane->w_obsolete = 0;
	pPlane->w_index_in_man = -1;
	pPlane->w_activestate = 2;
	_key_map.erase(pPlane->w_ins_key);
	pPlane->w_create_timestamp = -1;	//���������ʱ���
	_cur_planes[index] = 0;
	_planes_state[index] = 0;
	mutex_spinunlock(&_key_lock);

	mutex_spinlock(&_pool_lock);
	_active_plane_count --; 
	mutex_spinunlock(&_pool_lock);

	//��������
	pPlane->ResetWorld();
	//�����������ȴ�б�
	_planes_cooldown.push_back(pPlane);
			
	__PRINTF("������%d������ȴ����\n",index);
}
void 
instance_world_manager::RegroupCoolDownWorld()
{
	if(unsigned int count = _planes_cooldown2.size())
	{
		//���ڲ����н��Ѿ���ȴ�������������صĲ����ˣ�����һ��ֱ���ͷ�
		for(unsigned int i = 0; i < count; i ++)
		{
			__PRINTF("�ͷ�������%p\n",_planes_cooldown2[i]);
			_planes_cooldown2[i]->Release();
			delete _planes_cooldown2[i];
		}
		_planes_cooldown2.clear();
	}
	_planes_cooldown2.swap(_planes_cooldown);
}

void
instance_world_manager::FillWorldPool()
{
	mutex_spinlock(&_pool_lock);
	int pool_count = _planes_pool.size();
	int r1 = _pool_threshold_low - pool_count;
	int r2 =  _planes_capacity - (1 + pool_count +  (int)_planes_cooldown.size() + _active_plane_count);
	mutex_spinunlock(&_pool_lock);

	if(r1 > r2) r1 = r2;
	if(r1 > 0)
	{
		abase::vector<world*> list;
		list.reserve(r1);
		for(int i = 0; i < r1; i ++)
		{
			world * pPlane = new world;
			_plane_template->DuplicateWorld(pPlane);
			int rst = pPlane->RebuildMapRes();
			ASSERT(rst == 0);
			pPlane->w_plane_index = -1;
			_mapres.BuildNpcGenerator(pPlane);
			list.push_back(pPlane);
			__PRINTF("����������%p\n",pPlane);
		}

		//���²�����������뵽�������
		mutex_spinlock(&_pool_lock);
		for(int i = 0; i < r1; i ++)
		{
			_planes_pool.push_back(list[i]);
		}
		mutex_spinunlock(&_pool_lock);

	}

}

void
instance_world_manager::Heartbeat()
{
	_msg_queue.OnTimer(0,100);
	world_manager::Heartbeat();
	unsigned int ins_count = _max_active_index;
	for(unsigned int i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		pPlane->RunTick();
	}

	mutex_spinlock(&_heartbeat_lock);
	
	if((++_heartbeat_counter) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//ÿ10�����һ��
		for(unsigned int i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//������
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//���ڵȴ��ϳ�״̬
				if(pPlane->w_player_count)
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if((pPlane->w_obsolete += HEARTBEAT_CHECK_INTERVAL) > _idle_time)  // 20 * 60
					{
						//û����Ҳ���ʱ�䳬ʱ�ˣ����ͷ�����
						FreeWorld(pPlane,i);
					}
				}
			}
			else
			{
				if(!pPlane->w_player_count)
				{
					pPlane->w_obsolete = 1;
				}
			}
			
			//���и����������
			if(pPlane->w_life_time > 0)
			{
				pPlane->w_life_time -= HEARTBEAT_CHECK_INTERVAL;
				if(pPlane->w_life_time < 0)
					pPlane->w_life_time = 0;
			}			
		}
		_heartbeat_counter = 0;

		//������ȴ�б��Ĵ���
		RegroupCoolDownWorld();
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//�������ص��������㣬��������´�������
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);
}

bool 
instance_world_manager::CompareInsKey(const instance_key & key, const instance_hash_key & hkey)
{
	instance_hash_key key2;
	TransformInstanceKey(key.essence,key2);
	return key2 == hkey;
}

int instance_world_manager::GetInstanceReenterTimeout(world* plane)
{
	if(_world_limit.can_reenter && (plane->w_life_time<0 || plane->w_life_time>INSTANCE_REENTER_INTERVAL)
		&& (!plane->w_obsolete || _idle_time - plane->w_obsolete >= INSTANCE_REENTER_INTERVAL))
		return g_timer.get_systime() + INSTANCE_REENTER_INTERVAL;
	else
		return 0;
}

int
instance_world_manager::CheckPlayerSwitchRequest(const XID & who, const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	int rst = 0;
	//���ȼ��key�Ƿ����
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		//���粻���ڣ� �ж��Ƿ��п��е�world�ȴ����䣬���û����֪ͨ���
		bool bRst = _planes_pool.size() >= 1;
		mutex_spinunlock(&_key_lock);
		if(!bRst || (_active_plane_count + 1) >= _planes_capacity)
		{
			rst = S2C::ERR_TOO_MANY_INSTANCE;
		}
		else
		{
			if(ins_timer == 0 || (ikey->special_mask & IKSM_REENTER)) // �޷�����
			{
				rst = S2C::ERR_CAN_NOT_RESET_INSTANCE;
			}
		}
		return rst;
	}
	pPlane = _cur_planes[*pTmp];
	mutex_spinunlock(&_key_lock);
	//�����ҵ������Ƿ�ƥ��
	if(pPlane)
	{
		return ((ikey->special_mask & IKSM_GM) || pPlane->w_player_count < _player_limit_per_instance)?0:S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
	}
	else
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
}

world * 
instance_world_manager::AllocWorld(const instance_hash_key & key, int & world_index,int ctrl_id)
{
	spin_autolock keeper(_key_lock);
	return AllocWorldWithoutLock(key,world_index,ctrl_id);
}

world * 
instance_world_manager::AllocWorldWithoutLock(const instance_hash_key &key, int & world_index, int ctrl_id)
{
	//��Ȼ��without lock ���Ǳ��׶α��뱣�ּ���
	ASSERT(_key_lock);
	//���Ȳ�ѯ���߷���һ������
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(key);
	world_index = -1;
	if(!pTmp)
	{
		if(!_planes_pool.size()) 
		{
			return NULL;
		}
		
		//���ǿ��������������磩
		//����Ƿ���Ҫkey
		bool need_key = world_manager::GetWorldLimit().ctrlid_open_instance;
		if(need_key && ctrl_id <= 0)
		{
			return NULL;
		}
		//���Կ���
		mutex_spinlock(&_pool_lock);
		if(_planes_pool.size())
		{
			pPlane = _planes_pool.back();
			_planes_pool.pop_back();
			_active_plane_count ++;
		}
		mutex_spinunlock(&_pool_lock);
		if(pPlane)
		{
			pPlane->w_destroy_timestamp = g_timer.get_systime() + 120;//��ֹ����destroy_time������ͷ�

			//Ѱ�ҿ��Է�������Ŀ�λ
			unsigned int i = 1;	
			for(; i < (unsigned int)_planes_capacity; i ++)
			{
				if(_cur_planes[i]) continue;
				_cur_planes[i] = pPlane;
				_planes_state[i]  = 1; 
				pPlane->w_index_in_man = i;
				pPlane->w_plane_index = i;
				pPlane->w_activestate = 1;
				world_index = i;
				break;
			}
			if(i != (unsigned int)_planes_capacity)
			{
				__PRINTF("����������%d %p\n",i,pPlane);
				_key_map.put(key,i);
				pPlane->w_ins_key = key;
				pPlane->w_obsolete = 0;
				pPlane->w_create_timestamp = time(NULL);
				if((unsigned int) _max_active_index < i +1)
				{
					_max_active_index = i + 1;
				}
			}
			else
			{
			//	ASSERT(false);

				//�޷��������磬���½����ɵ�������뵽�������
				mutex_spinlock(&_pool_lock);
				_planes_pool.push_back(pPlane);
				_active_plane_count --; 
				mutex_spinunlock(&_pool_lock);
				return NULL;
			}
			if(need_key)
			{
				bool bRst = pPlane->TriggerSpawn(ctrl_id);
				__PRINTF("��������ʱͬʱ�����˿�����%d %d\n",ctrl_id, bRst?1:0);
			}
		}
	}
	else
	{
		//�������������� 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

world * 
instance_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int ctrl_id)
{
	return AllocWorld(ikey,world_index,ctrl_id);
	
}

world * 
instance_world_manager::GetWorldOnLogin(const instance_hash_key & ikey,int & world_index)
{
	return AllocWorld(ikey,world_index,0);
}

void 
instance_world_manager::HandleSwitchRequest(int link_id,int user_id,int localsid,int source,const instance_key &ikey)
{
	instance_hash_key key;
	TransformInstanceKey(ikey.target,key);
	int world_index;
	world * pPlane = GetWorldInSwitch(key,world_index,ikey.control_id);
	if(!pPlane ) 
	{
		//�޷��������磬������Ŀ�ﵽ������� 
		MSG msg;
		int error = S2C::ERR_TOO_MANY_INSTANCE;
		if(world_manager::GetWorldLimit().ctrlid_open_instance)
		{
			error = S2C::ERR_CANNOT_ENTER_INSTANCE;
		}
		BuildMessage(msg,GM_MSG_SWITCH_FAILED,XID(GM_TYPE_PLAYER,user_id),XID(0,0),A3DVECTOR(0,0,0),error);
		SendRemotePlayerMsg(user_id,msg);
		return;
	}

	ASSERT(pPlane == _cur_planes[world_index]);
	int index1 = pPlane->FindPlayer(user_id);
	if(index1 >= 0) 
	{
		//����û���Ӧ�ô��ڵ�(��������ʱ���ƺ�Ҳ�п���)
		//���link��game�Ͽ�����ô����������ܻᷢ��
		return;
	}
	gplayer * pPlayer = pPlane->AllocPlayer();
	if(pPlayer == NULL)
	{
		//����û�������ռ��������Player����Ϣ
		__PRINTF("�û��ﵽ�����������ֵ\n");
		GLog::log(GLOG_ERR,"����%d�û���Ŀ�ﵽ������ޣ����û�%dת�Ʒ�����ʱ",GetWorldTag(),user_id);
		//������ת����Ϣ��������һ���Ϊ��ʱ������
		return;
	}

	__PRINTF("player %d switch from %d\n",user_id,source );
	pPlayer->cs_sid = localsid;
	pPlayer->cs_index = link_id;
	pPlayer->ID.id = user_id;
	pPlayer->ID.type = GM_TYPE_PLAYER;
	pPlayer->login_state = gplayer::WAITING_SWITCH;
	pPlayer->pPiece = NULL;
	if(!pPlane->MapPlayer(user_id,pPlane->GetPlayerIndex(pPlayer)))
	{	
		__PRINTF("������ת��ʱmap playerʧ��\n");
		GLog::log(GLOG_ERR,"����%d��ִ�з�����ת��ʱmap playerʧ�ܣ��û�%d",GetWorldTag(),user_id);

		//����������ʱ��ʹ�ú�������Ϣ�ȴ������Ӷ����е��ظ��Ĵ��� 
		pPlane->FreePlayer(pPlayer);
		pPlayer->Unlock();
		return;
	}

	//������û�����Ϊ�����λ��
	SetPlayerWorldIdx(user_id,world_index);
	
	class switch_task : public ONET::Thread::Runnable, public abase::timer_task , public abase::ASmallObject
	{
		gplayer *_player;
		int _userid;
		world * _plane;
		instance_world_manager * _manager;
		public:
			switch_task(gplayer * pPlayer,world * pPlane,instance_world_manager * manager)
				:_player(pPlayer),_userid(pPlayer->ID.id),_plane(pPlane),_manager(manager)
			{
				//�����ʱʱ����2.5s�ӳ���5s����ֹgs���ظ�����ĳ�ʱ��modify by liuguichen 20131224
				SetTimer(g_timer,TICK_PER_SEC*5,1);
				__PRINTF("timer %p %d\n",this,_timer_index);
			}
			~switch_task()
			{
				if(_timer_index >=0) RemoveTimer();
			}
		public:
			virtual void OnTimer(int index,int rtimes)
			{
				ONET::Thread::Pool::AddTask(this);
			}

			virtual void Run()
			{
				spin_autolock keeper(_player->spinlock);
				if(_player->IsActived() && _player->ID.id == _userid && _player->login_state == gplayer::WAITING_SWITCH)
				{
					_plane->UnmapPlayer(_userid);
					_plane->FreePlayer(_player);

					//ͬʱȡ�������λ��ļ�¼  ������ʱ��������ܻ���Щ���⣩$$$$$$$$
					//Ҫ�ٿ���һ��
					_manager->RemovePlayerWorldIdx(_userid);
				}
				delete this;
			}
	};
	//�����ȴ���Ϣ
	MSG msg;
	BuildMessage(msg,GM_MSG_SWITCH_GET,pPlayer->ID,XID(GM_TYPE_SERVER,world_manager::GetWorldIndex()),A3DVECTOR(0,0,0),_world_tag,0, &ikey,sizeof(ikey));
	pPlane->SendRemoteMessage(source,msg);

	//���ó�ʱ
	switch_task *pTask = new switch_task(pPlayer,pPlane,this);
	pPlayer->base_info.race = (long)(abase::timer_task*)pTask;
	pPlayer->base_info.faction = pTask->GetTimerIndex();
	pPlayer->Unlock();
	
}


void 
instance_world_manager::PlayerLeaveThisWorld(int plane_index, int userid)
{
	RemovePlayerWorldIdx(userid,plane_index);
}


void 
instance_world_manager::PostMessage(world * plane, const MSG & msg)
{
	_msg_queue.AddMsg(plane,msg);
}

void 
instance_world_manager::PostMessage(world * plane, const MSG & msg,int latancy)
{
	_msg_queue.AddMsg(plane,msg,latancy);
}

void 
instance_world_manager::PostMessage(world * plane, const XID * first, const XID * last, const MSG & msg)
{
	_msg_queue.AddMultiMsg(plane,first,last,msg);
}

void 
instance_world_manager::PostPlayerMessage(world * plane, int * player_list, unsigned int count, const MSG & msg)
{
	_msg_queue.AddPlayerMultiMsg(plane,count, player_list,msg);
}

void
instance_world_manager::PostMultiMessage(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg)
{
	_msg_queue.AddMultiMsg(plane,list, msg);
}

void 
instance_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	if(_world_limit.savepoint && _save_point.tag > 0)
	{
		world_tag = _save_point.tag;
		pos = _save_point.pos;
	}
	else
	{
		pos = pImp->GetLogoutPos(world_tag);
	}
}

void instance_world_manager::SwitchServerCancel(int link_id,int user_id, int localsid)
{
	int index1;
	gplayer * pPlayer = FindPlayer(user_id,index1);
	if(!pPlayer)
	{
		ASSERT(false);
		//û���ҵ� ���ʵ��û�
		//��������£�����û�Ӧ�ô��ڵ�
		return;
	}
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != user_id || !pPlayer->IsActived() || !pPlayer->imp)
	{
		ASSERT(false);
		return;
	}
	pPlayer->imp->CancelSwitch();
}

void	instance_user_login(int cs_index,int cs_sid,int uid,const void * auth_data, unsigned int auth_size, bool isshielduser, char flag);
void 
instance_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data,unsigned int auth_size,bool isshielduser, char flag)
{
	instance_user_login(cs_index,cs_sid,uid,auth_data,auth_size,isshielduser,flag);
}

void 
instance_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	//���븱��key���filter,���filter���л�������ʱ������б���ͻָ�
	pImp->_filters.AddFilter(new aei_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
}

bool instance_world_manager::IsUniqueWorld()
{
	return false;
}

void 
instance_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & pos,int special_mask)
{
	if( (0 == (special_mask & IKSM_REENTER)) &&
		pPlayer->plane->SetIncomingPlayerPos(pPlayer,pos))	return;
	
	pPlayer->pos = pos;

	//��pos�ĸ߶Ƚ�������
	float height = pPlayer->plane->GetHeightAt(pos.x,pos.z);
	if(pPlayer->pos.y < height) pPlayer->pos.y = height;
}

bool 
instance_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & world_tag)
{
	return pImp->_plane->GetTownPosition(pImp,opos,pos,world_tag) ||
		   world_manager::GetTownPosition(pImp,opos,pos,world_tag);
}
bool 
instance_world_manager::ClearSpawn(int sid)
{
	unsigned int ins_count = _max_active_index;
	for(unsigned int i = 0; i < ins_count; i++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world *pPlane = _cur_planes[i];
		if(!pPlane) continue;
		pPlane->ClearSpawn(sid);
	}
	return true;
}

/*---------------------------------   ���ɸ��������� ---------------------------------*/


bool 
faction_world_manager::FactionLogin(const instance_hash_key &hkey,const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2)
{
	//����ȡ�û��ߴ���һ������ 
	mutex_spinlock(&_key_lock);
	//�������ȫ�̱��ּ�����ȷ��״̬��ȷ�����ڴ��ֲ�������������ȫ�̼���Ӧ�ò������̫��ĳ�ͻ 
	//��������д��������Ļ���wait list �����޷�������ȷ״̬

	//��ȡ���ɻ�������ʧ�ܻ���ɻ��ز�����
	if(data == NULL)
	{
		ClearWaitList(hkey, S2C::ERR_CANNOT_ENTER_INSTANCE);
		mutex_spinunlock(&_key_lock);
		return false;
	}
	
	//��ʼ��������,��������ķ��䷽ʽҪ������ͬ�Ŷ�(���߿�������NPC����������,�ٸ��ݲ�ͬ������������Ƿ���ĳЩNPC��ʧ��) 
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);

	if(pPlane == NULL)
	{
		//��������ʧ�ܣ� �����޷�������Ϸ�� ��Ҫ���WaitList�������޷��������Ϣ
		ClearWaitList(hkey, S2C::ERR_CANNOT_ENTER_INSTANCE);
		mutex_spinunlock(&_key_lock);
		return false;
	}

	//���η���������������
	bool verify_key = false;	// �Ƿ�ܾ��Ǳ����ɵ��˽���
	bool bRst = SendReplyToWaitList(hkey,verify_key);
	if(!bRst)
	{
		//�����ܳ����������
		ASSERT(false);
		mutex_spinunlock(&_key_lock);
		return false;
	}
	mutex_spinunlock(&_key_lock);
	
	//����Ϣ����ʽ֪ͨ����������, �ô�������������ȷ��������߼���ĳ��NPC
	faction_world_ctrl * pCtrl = dynamic_cast<faction_world_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		//�����ڲ���ctrl���ǺϷ���
		ASSERT(false);
		return false;
	}
	pCtrl->Init(pPlane,data,data2);
	__PRINTF("Faction Login. factionid=%d world_index=%d world=%p\n",data->factionid,world_index,pPlane);
	return true;
}

bool 
faction_world_manager::NotifyFactionData(GNET::faction_fortress_data2 * data2)
{
	instance_hash_key hkey;
	MakeInstanceKey(data2->factionid,hkey);
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(hkey);
	if(pTmp)
	{
		int world_index = *pTmp;
		world * pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		mutex_spinunlock(&_key_lock);
		faction_world_ctrl * pCtrl = dynamic_cast<faction_world_ctrl*>(pPlane->w_ctrl);
		if(pCtrl == NULL)
		{
			ASSERT(false);
			return false;
		}
		pCtrl->OnNotifyData(pPlane,data2);
		return true;
	}
	mutex_spinunlock(&_key_lock);
	return false;
}

bool
faction_world_manager::SendReplyToWaitList(const instance_hash_key & hkey , bool is_verify)
{
	bool bRst = false;
	//�����ٽ���
	spin_autolock keeper(_wait_queue_lock);
	
	//�ҵ�ָ���ĵȴ�����
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		
		//���ָ���Ķ��в����� ��ʲôҲ����
		return false;
	}
	else
	{
		//��������ҷ��ͻ�����Ϣ
		MSG msg;
		BuildMessage(msg,GM_MSG_PLANE_SWITCH_REPLY,XID(-1,-1),XID(GM_TYPE_SERVER,GetWorldIndex()),
				A3DVECTOR(0,0,0),0);

		WAIT_ENTRY & v = **(it.value());
		for(unsigned int i = 0; i < v.size(); i ++)
		{
			if(is_verify)
			{
				instance_hash_key okey; 
				TransformInstanceKey(v[i].second.essence, okey);
				if(!(okey == hkey))
				{
					MSG nmsg;
					BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,v[i].first,XID(0,0),A3DVECTOR(0,0,0),S2C::ERR_FACTION_BASE_DENIED);
					SendRemotePlayerMsg(msg.target.id, msg);
					continue;
				}
			}
			msg.target = v[i].first;
			msg.content = &(v[i].second);
			msg.pos = v[i].pos;
			msg.content_length = sizeof(instance_key);
			SendRemotePlayerMsg(msg.target.id, msg);
			bRst = true;
		}
		delete *(it.value());
		_wait_queue.erase(it);
	}
	return bRst;

}

void 
faction_world_manager::ClearWaitList(const instance_hash_key & hkey,int err_code)
{
	//�����ٽ���
	spin_autolock keeper(_wait_queue_lock);
	
	//�ҵ�ָ���ĵȴ�����
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		
		//���ָ���Ķ��в����� ��ʲôҲ����
		return;
	}
	else
	{
		//��������ҷ��ʹ�����Ϣ
		if(err_code != 0)
		{
			MSG msg;
			BuildMessage(msg,GM_MSG_ERROR_MESSAGE,XID(-1,-1),XID(0,0),A3DVECTOR(0,0,0),err_code);
			
			WAIT_ENTRY & v = **(it.value());
			for(unsigned int i = 0; i < v.size(); i ++)
			{
				msg.target = v[i].first;
				SendRemotePlayerMsg(msg.target.id, msg);
			}
		}
		delete *(it.value());
		_wait_queue.erase(it);
	}
	return ;
}

bool 
faction_world_manager::AddWaitList(const XID & who, const instance_hash_key & hkey, const instance_key & ikey,const A3DVECTOR & pos)
{
	class GetFactionFortress : public GNET::FactionFortressResult
	{
		instance_hash_key _hkey;
	public:
		GetFactionFortress(const instance_hash_key & hkey):_hkey(hkey)
		{}

		virtual void OnTimeOut()
		{
			OnGetData(NULL,NULL);
		}
		virtual void OnFailed()
		{
			OnGetData(NULL,NULL);
		}
		virtual void OnGetData(const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2)
		{
			//����һ������
			world_manager::GetInstance()->FactionLogin(_hkey,data,data2);
			delete this;
		}
	};
	//��鸱�������͵ȴ������е��������Ƿ񵽴����� ��δ���

	//�����ٽ���
	spin_autolock keeper(_wait_queue_lock);
	
	//�ҵ�ָ���ĵȴ�����
	WAIT_MAP::iterator it = _wait_queue.find(hkey);

	if(it == _wait_queue.end())
	{
		//���ָ���Ķ��в����� ����Ҽ��������,���������ݿ��ȡ����
		WAIT_ENTRY *v = new WAIT_ENTRY;
		v->push_back(wait_node(who,ikey,pos));
		_wait_queue.put(hkey,v);

		GNET::get_faction_fortress(ikey.target.key_level3, new GetFactionFortress(hkey));
	}
	else
	{
		//���ָ���Ķ��д��� ���ָ��������Ƿ����,������� ��ô���Ա��β���
		WAIT_ENTRY & v = **(it.value());
		for(unsigned int i = 0; i < v.size(); i ++)
		{
			if(v[i].first == who) return true;
		}
		v.push_back(wait_node(who,ikey,pos));
	}
	return true;
}

world * 
faction_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int ctrl_id)
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(ikey);
	world_index = -1;
	if(pTmp)
	{
		//�������������� 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
	
}

int 
faction_world_manager::CheckPlayerSwitchRequest(const XID & who, const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	//����Ƿ�ȥ��ȷ�ĸ���
	if(ikey->target.key_level3 == 0 || ikey->essence.key_level3 == 0)
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	int factionid = ikey->essence.key_level3;
	
//�����ɸ������͹���
//���ȼ��Key�Ƿ����
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		//���粻���ڣ�������ȴ��б�
		//������ƷǱ�������Ա�������������
		int rst = -1;
		//if(ikey->target.key_level3 == ikey->essence.key_level3)
		//{
			bool bRst = AddWaitList(who,key,*ikey,pos);
			if(!bRst)
			{
				rst = S2C::ERR_TOO_MANY_INSTANCE;
			}
		//}
		//else 
		//{
			//Ӧ���Ǵ˰��ɻ���δ����
			//���ڲ������Ǳ����Ա�������ɻ���
		//	rst = S2C::ERR_FACTION_BASE_NOT_READY;
		//}
		mutex_spinunlock(&_key_lock);
		return rst;
	}
	pPlane = _cur_planes[*pTmp];
	mutex_spinunlock(&_key_lock);

	//�����ҵ������Ƿ�ƥ��
	int rst = 0;
	if(pPlane)
	{
		if(!(ikey->special_mask & IKSM_GM) && pPlane->w_player_count >= _player_limit_per_instance)
		{
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			faction_world_ctrl* pCtrl = (faction_world_ctrl*)pPlane->w_ctrl;
			
			if(factionid == pCtrl->factionid)
			{
				if(pCtrl->defender_count >= pCtrl->player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;	
				}
			}
			else if(pCtrl->inbattle && factionid == pCtrl->offense_faction)
			{
				if(pCtrl->attacker_count >= pCtrl->player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;	
				}
			}
			else
			{
				rst = S2C::ERR_FACTION_IS_NOT_MATCH;
			}
			//�����ǲ��Ǵ�������״̬
			if(!rst && pCtrl->iskick)
			{
				rst = S2C::ERR_FACTION_FORTRESS_ISKICK;
			}
		}
	}
	else
	{
		rst = S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	return rst;
}

void 
faction_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	//����Ӧ���ö�̬��savepoint ��������ʱ��Ҫָ����Щ����
	pImp->GetLastInstanceSourcePos(world_tag,pos);
	if(world_tag != 1)
	{
		//������ĺã� ����Ҳû�취�� ��Ȼ��������ô�죿
		world_tag = 1;
		pos = A3DVECTOR(320,0,3200);
	}
}

void
faction_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, unsigned int auth_size,bool isshielduser, char flag)
{
	//���ɻ����޷�ֱ�ӵ�½
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void 
faction_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aeff_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
}

void 
faction_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;
	
	if(pPlayer->id_mafia == pCtrl->factionid)
	{
		//�������д��
		pPlayer->pos = A3DVECTOR(-383.295f,35.f,1929.511f);
	}
	else if(pCtrl->inbattle && pPlayer->id_mafia == pCtrl->offense_faction)
	{
		//�������д��
		pPlayer->pos = A3DVECTOR(-188.f,35.f,400.f);
	}
	else
	{
		instance_world_manager::SetIncomingPlayerPos(pPlayer,origin_pos,special_mask);	
	}
}

bool 
faction_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	gplayer * pPlayer = (gplayer *)pImp->_parent;
	if(pPlayer->IsBattleDefence())
	{
		//�������д��
		pos = A3DVECTOR(-383.295f,35.f,1929.511f);
		tag = _world_tag;
		return true;
	}
	else if(pPlayer->IsBattleOffense())
	{
		//�������д��
		pos = A3DVECTOR(-188.f,35.f,400.f);
		tag = _world_tag;
		return true;
	}
	return false;
}

void 
faction_world_manager::OnDeliveryConnected()
{
	GNET::SendFactionServerRegister(GetWorldIndex(),GetWorldTag());
	GMSV::SendMafiaPvPRegister(GetWorldIndex(),GetWorldTag());
}

void faction_world_manager::OnMafiaPvPStatusNotice(int status,std::vector<int>& ctrl_list)
{
	if(status)
	{
		world_flags& flags = GetWorldFlag();
		flags.mafia_pvp_flag = true;
		flags.nonpenalty_pvp_flag = true;
	}
	else
	{
		world_flags& flags = GetWorldFlag();
		flags.mafia_pvp_flag = false;
		flags.nonpenalty_pvp_flag = false;
	}
}

world * 
faction_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);

	pPlane->SetWorldCtrl(new faction_world_ctrl());
	return pPlane;
}

world_message_handler * 
faction_world_manager::CreateMessageHandler()
{
	return new faction_world_message_handler(this);
}

int 
faction_world_manager::OnMobDeath(world * pPlane, int faction,  int tid)
{
	npc_template * pTemplate = npc_stubs_manager::Get(tid);
	if(!pTemplate || !pTemplate->faction_building_id) return 0;
	
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;
	pCtrl->OnBuildingDestroyed(pPlane, pTemplate->faction_building_id);	
	return 1;
}
