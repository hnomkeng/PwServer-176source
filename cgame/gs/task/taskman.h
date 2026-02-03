#ifndef __ONLINEGAME_GS_TASK_MAN_H__
#define __ONLINEGAME_GS_TASK_MAN_H__

#include "TaskServer.h"
class elementdataman;
bool InitQuestSystem(const char * filename,const char * filename2, elementdataman * pMan);
void PublicQuestRunTick();

struct gplayer;
class gplayer_imp;
class PlayerTaskInterface : public TaskInterface
{
gplayer_imp * _imp;
public:
	virtual unsigned int GetPlayerLevel();
	virtual void* GetActiveTaskList();
	virtual void* GetFinishedTaskList();
	virtual void* GetFinishedTimeList();
	virtual void* GetFinishedCntList();
	virtual void* GetStorageTaskList();
	virtual unsigned int* GetTaskMask();
	virtual unsigned int GetTaskMaskInt();
	virtual void DeliverGold(unsigned int ulGoldNum);
	virtual void DeliverExperience(unsigned int ulExp);
	virtual void DeliverSP(unsigned int ulSP);
	virtual void DeliverReputation(int lReputation);
//	virtual bool HasTaskItem(unsigned int ulTaskItem); 	//xxxxx
//	virtual bool HasCommonItem(unsigned int ulCommonItem); //xxxx
	virtual int GetTaskItemCount(unsigned int ulTaskItem);
	virtual int GetTeamVisibleStateParam(unsigned int ulStateId);
	virtual int GetCommonItemCount(unsigned int ulCommonItem);
	virtual bool IsInFaction(unsigned int);
	virtual int GetFactionRole();
	//virtual bool GoldConvert(unsigned int);
	virtual unsigned int GetGoldNum();
	virtual void TakeAwayGold(unsigned int ulNum);
	virtual unsigned int GetGoldNumNew();
	virtual void TakeAwayGoldNew(unsigned int ulNum);
	virtual void TakeAwayTaskItem(unsigned int ulTemplId, unsigned int ulNum);
	virtual void TakeAwayCommonItem(unsigned int ulTemplId, unsigned int ulNum);
	virtual int GetReputation();
	virtual unsigned int GetCurPeriod();
	virtual unsigned int GetCurPeriodFlag();
	virtual void SetCurPeriod(unsigned int per);
	virtual unsigned int GetPlayerId();
	virtual unsigned int GetPlayerRace();
	virtual unsigned int GetPlayerOccupation();
	virtual bool CheckHwidLimit(int task_id);
	virtual void RemoveHwidLimit();

	virtual bool CanDeliverCommonItem(unsigned int ulItemTypes);
	virtual bool CanDeliverTaskItem(unsigned int ulItemTypes);
//	virtual bool CanDeliverCommonItem(unsigned int ulItem,unsigned int count);
//	virtual bool CanDeliverTaskItem(unsigned int ulItem,unsigned int count);
	virtual void DeliverCommonItem(unsigned int ulItemId, unsigned int ulCount, int lPeriod);
	virtual void DeliverTaskItem(unsigned int ulItem,unsigned int count);
	virtual void NotifyClient(const void* pBuf, unsigned int sz);
	virtual float UnitRand();
	virtual int RandNormal(int low, int high);
	virtual int  RandSelect(const float * option, int size);
	virtual bool HasLivingSkill(unsigned int ulSkill); 
	virtual int GetLivingSkillProficiency(unsigned int ulSkill);
	virtual int GetLivingSkillLevel(unsigned int ulSkill);
	virtual void SetNewRelayStation(unsigned int ulStationId);
	virtual void SetStorehouseSize(unsigned int ulSize);
	virtual void SetStorehouseSize2(unsigned int ulSize);
	virtual void SetStorehouseSize3(unsigned int ulSize);
	virtual void SetAccountStorehouseSize(unsigned int ulSize);//�ʺŲֿ����ÿɲ����ĸ�������С
	virtual void AddDividend(int nDividend); // ����ֵ
	virtual void SetFuryUpperLimit(unsigned int ulValue);
	virtual void TransportTo(unsigned int ulWorldId, const float pos[3], int lController);
	virtual void SetInventorySize(int lSize);

	virtual int GetTeamMemberNum();
	virtual void NotifyPlayer(unsigned int ulPlayerId, const void* pBuf, unsigned int sz);
	virtual void GetTeamMemberInfo(int nIndex, task_team_member_info* pInfo);
	virtual bool IsDeliverLegal();
	virtual unsigned int GetTeamMemberId(int nIndex);
	virtual bool IsInTeam();
	virtual bool IsCaptain();
	virtual bool IsMale();
	virtual unsigned int GetPos(float pos[3]);
	virtual bool GetTeasoureCheckRadius(int tag_id, int tag_required, float radius_size, float old_pos[3]);
	virtual unsigned int GetTeamMemberPos(int nIndex, float pos[3]);
	virtual void GetSpecailAwardInfo(special_award* p);
	virtual void SetPetInventorySize(unsigned int ulSize);
	virtual bool IsGM();
	virtual void SetMonsterController(int id, bool bTrigger);

	virtual bool IsMarried();
	virtual bool IsWeddingOwner();
	virtual void CheckTeamRelationship(int nReason);
	virtual void Divorce();
	virtual void SendMessage(unsigned int task_id, int channel, int param);

	virtual void LeaveCarrier();
	virtual void CheckEventValidTask(int task_id);
	virtual void UnlockRepositorySystem();
	virtual void TaskExpCelestialMemorial(int swallow_exp);

	virtual void PassBattleTaskManager();
	virtual void PassBattleTaskAddItemPass(int itemid, int itemcount);
	
	// Novo Sistema Exclusivo
	virtual void SetAnecdotePoints();

	//???key/value
	virtual int GetGlobalValue(int lKey);
	virtual void PutGlobalValue(int lKey, int lValue);
	virtual void ModifyGlobalValue(int lKey, int lValue);

	virtual int SummonMonster(int mob_id, int count, int distance, int remain_time, bool die_with_self);
	virtual bool IsShieldUser();
	virtual void SetAwardDeath(bool bDeadWithLoss);
	virtual unsigned int GetRoleCreateTime();
	virtual unsigned int GetRoleLastLoginTime();
	virtual unsigned int GetAccountTotalCash();
	virtual unsigned int GetSpouseID();
	virtual bool CastSkill(int skill_id, int skill_level);
	virtual unsigned int GetInvEmptySlot();
	virtual void LockInventory(bool is_lock);
	virtual unsigned char GetShapeMask();
	virtual bool IsAtCrossServer();
	virtual bool IsKing();
	
	virtual int GetFactionContrib();
	virtual void DeliverFactionContrib(int iConsumeContrib,int iExpContrib);
	virtual int GetFactionConsumeContrib();
	virtual void TakeAwayFactionConsumeContrib(int ulNum);
	virtual int GetFactionExpContrib();
	virtual void TakeAwayFactionExpContrib(int ulNum);

	virtual int GetForce();
	virtual int GetForceContribution();
	virtual int GetForceReputation();
	virtual bool ChangeForceContribution(int iValue);
	virtual bool ChangeForceReputation(int iValue);
	virtual int GetExp();
	virtual int GetSP();
	virtual bool ReduceExp(int exp);
	virtual bool ReduceSP(int sp);
	virtual int GetForceActivityLevel();
	virtual void AddForceActivity(int activity);
// �ƺ����
	virtual bool HaveGotTitle(unsigned int id_designation);
	virtual void DeliverTitle(unsigned int id_designation, int lPeriod = 0);
	virtual bool CheckRefine(unsigned int level_refine, unsigned int num_equips);
// ��ʷ�ƽ����
	virtual void PutHistoryGlobalValue(int lKey, int lValue);
	virtual void ModifyHistoryGlobalValue(int lKey, int lValue);
	virtual int GetHistoryGlobalValue(int lKey);
	virtual void PutPersonGlobalValue(int bChangeType, int lKey, int lValue);
	virtual int GetPersonGlobalValue(int lKey);
	virtual int  GetCurHistoryStageIndex(); // ��ǰ��ʷ�׶ε����

	virtual bool CheckSimpleTaskFinshConditon(unsigned int task_id) const { return true; } // ������ֻ�ͻ��˼�飬��������ֱ�� ����ture��
	// ת��
	virtual unsigned int GetMaxHistoryLevel();
	virtual unsigned int GetReincarnationCount();
	// ����
	virtual unsigned int GetRealmLevel();
	virtual bool IsRealmExpFull();
	virtual void DeliverRealmExp(unsigned int exp);
	virtual void ExpandRealmLevelMax();
	virtual unsigned int GetObtainedGeneralCardCount(); // ��õĿ��������� ����ͼ���е�����
	virtual void AddLeaderShip(unsigned int leader_ship);
	virtual unsigned int GetObtainedGeneralCardCountByRank(int rank); // ���ϡ��������Ϳ��Ʋֿ���ĳƷ�׿��Ƶ�����֮��
	virtual bool CheckTaskForbid(unsigned int task_id);
	virtual int GetWorldContribution();	// ���繱�׶�
	virtual void DeliverWorldContribution(int contribution);
	virtual void TakeAwayWorldContribution(int contribution);
	virtual int GetWorldContributionSpend();
	virtual bool PlayerCanSpendContributionAsWill();
	//virtual void SoloChallengeTaskAward();
	virtual bool HasIconStateID(unsigned int ulIconStateID);
	 
	// �����������
	virtual void OnTowerTaskDeliver(bool bSuccess);		// �������񷢷�
	virtual void OnTowerTaskComplete(bool bSuccess);	// �����������
	virtual void DeliverSoloTowerChallengeScore(int score);	// ���ŵ����������ֽ���(�������޺͵�ǰ���ֶ�Ҫ����)

	// VIP
	virtual int GetVIPLevel();	
	virtual int GetPointsWeekArena();
	virtual int GetWinCountArena();

	virtual void UpdatePlayerEventTask(int task_id, int points);
	virtual int GetPlayerEventTask(int task_id);
	virtual void RemovePlayerEventTask(int task_id);

	virtual void UpdatePlayerUseItems(int task_id, int count);
	virtual int GetPlayerUseItems(int task_id);

	virtual unsigned int CheckHasCampTokenType();
	virtual unsigned int CheckHasCampTokenPower();
	virtual void UpdateCampTokenScore(unsigned int lValue);
	virtual void UpdateCampTokenMoney(unsigned int lValue);

	void LoadFixedEntry(const void* pActiveListBuf, const unsigned int iActiveListLen);
	void SaveFixedEntry(void* pActiveListBuf, unsigned int & iActiveListLen);
	
public:
	PlayerTaskInterface(gplayer_imp * imp):_imp(imp)
	{}

};	

struct PlayerTaskTeamInterface : public TaskTeamInterface
{
	int marriage_op;
	int couple[2];
	PlayerTaskTeamInterface()
	{
		marriage_op = 0;
		couple[0] =  couple[1] = 0;
	}

	virtual void SetMarriage(int nPlayer);

	void Execute(gplayer ** list, unsigned int count);
};

//�����Ա�İ�װ���� ;
inline void Task_OnTeamAddMember(TaskInterface* pTask, int index)
{
	task_team_member_info info;
	pTask->GetTeamMemberInfo(index,&info);
	OnTeamAddMember(pTask,&info);
}

#endif

