#ifndef __ONLINEGAME_GS_SOLO_CHALLENGE_H__
#define __ONLINEGAME_GS_SOLO_CHALLENGE_H__

#include <cstring>
#include <vector.h>
#include <common/base_wrapper.h>
#include "timer.h"
#include <db_if.h>
#include "template/exptypes.h"

#define GET_CV(type) pImp->_plane->GetCommonValue(COMMON_VALUE_ID_SOLO_CHALLENGE_##type)
#define SET_CV(type,value) pImp->_plane->SetCommonValue(COMMON_VALUE_ID_SOLO_CHALLENGE_##type,value)
#define FILTER_SOLO_START FILTER_SOLO_INCATTACKANDMAGIC

namespace
{
	const static int filter_max_num[16] = {20,20,20,20,
											1, 1, 1, 1,
										   20,20, 0, 0,
											0, 0, 0, 0
	};
}

class gplayer_imp;
class playersolochallenge
{
public:
	enum
	{
		STAGE_STATE_INIT     =0,
		STAGE_WAIT_START,
		STAGE_PROCESSING,
		STAGE_COMPLETE,

		STAGE_STATE_MAX,
	};

	enum 
	{
		CUR_STAGE_START_TIMESTAMP,
		CUR_STAGE_LEVEL,
		CUR_STAGE_STATE,
		MAX_STAGE_LEVEL,
		CUR_STAGE_PLAY_MODE,
	};
	
	enum
	{
		SOLO_CHALLENGE_NOTIFY_START_SUCCESS = 0,
		SOLO_CHALLENGE_NOTIFY_START_FAILED,
		SOLO_CHALLENGE_NOTIFY_COMPLETE_SUCCESS,
		SOLO_CHALLENGE_NOTIFY_COMPLETE_FAILED,
	};

	struct player_solo_challenge_award
	{
		int item_id;
		int item_count;
		player_solo_challenge_award():item_id(-1),item_count(-1)
		{
		}
		player_solo_challenge_award(int tmp_id, int tmp_count):item_id(tmp_id),item_count(tmp_count)
		{
		}
	};
private:
	int _max_stage_level;//�Ѵ�������߹ؿ�
	int _max_stage_cost_time;//��߹ؿ��Ĵ���ʱ��
	int _total_score;//��������
	int _total_first_climbing_time;//������ʱ�䣬ֻ��¼��һ�δ���ʱ����ܺ�
	int _left_draw_award_times;
	int _play_modes;//�����淨
	abase::vector<player_solo_challenge_award> award_info;//�����߹ؿ��ѳ鵽�Ľ���
	
	int _cur_stage_cost_time;//��ǰ�ؿ��Ĵ���ʱ��
	int _cur_stage_play_mode;//��ǰ�ؿ����淨
	int _cur_score;
	int _filter_num[16];

private:
	bool IsTopLevel(int cur_stage_level)
	{
		return (cur_stage_level == (_max_stage_level + 1));
	}
	
public:
	playersolochallenge():_max_stage_level(0),_max_stage_cost_time(0),_total_score(0),_total_first_climbing_time(0),_left_draw_award_times(0),_play_modes(0),_cur_stage_cost_time(0),_cur_stage_play_mode(-1)
	{
		memset(&_filter_num, 0, sizeof(_filter_num));
	}
	~playersolochallenge(){}

	void Save(archive & ar);
	void Load(archive & ar);

	void Swap(playersolochallenge & rhs);

	void GetDBSoloChallengeInfo(GDB::base_info::               solo_challenge_info_t& solo_challenge_info);
	void SetDBSoloChallengeInfo(const GDB::base_info::               solo_challenge_info_t& solo_challenge_info);
	
	void SelectStage(gplayer_imp *pImp,int stage_level);
	void StageComplete(gplayer_imp *pImp, bool isCompleteSuccess);//������ɺ󣬽��н��������
	void StageStart(gplayer_imp *pImp, bool isStartSuccess);//����ʼ��ʱ

	int UserSelectAward(gplayer_imp *pImp, int num, int args[]);//�û�ѡ����
	int ScoreCost(gplayer_imp *pImp, int filter_index, int args[]);

	int ClearFilter(gplayer_imp *pImp, int args[]);
	void SetFilterData(int filter_id, int num, gplayer_imp *pImp);

	void NotifySoloChallengeData(gplayer_imp *pImp);//֪ͨ�ͻ��˵��˸�������
	void PlayerEnterSoloChallengeInstance(gplayer_imp *pImp);//��ҽ��뵥�˸���
	void PlayerLeaveSoloChallengeInstance(gplayer_imp *pImp);//��ҽ����뿪���˸���
	void PlayerDeliverSoloChallengeScore(gplayer_imp *pImp, int score);
	int PlayerSoloChallengeLeaveTheRoom(gplayer_imp *pImp);

	void OnClock(gplayer_imp *pImp);
	void OnPassClock(gplayer_imp *pImp, int lastupdate,int now);

	void SetMaxStageLevel(int max_stage_level, gplayer_imp *pImp)
	{
		_max_stage_level = max_stage_level;
		StageComplete(pImp, true);
	}

	void SetResetSolloTower(gplayer_imp *pImp);

private:
	void UpdateTimer(gplayer_imp *pImp);//�����������¼����ʱ��
	void UpdateMaxStageLevel(int stage_level, gplayer_imp *pImp);
	void CalcPlayMode(SOLO_TOWER_CHALLENGE_LEVEL_CONFIG *conf, int stage_level, gplayer_imp *pImp);

	void ReSetSoloChallengeData();//ÿ������ʱ��������
	void RecordReSetLog(gplayer_imp *pImp);
};

#endif
