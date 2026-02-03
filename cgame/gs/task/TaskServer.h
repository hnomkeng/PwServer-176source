#ifndef _TASKSERVER_H_
#define _TASKSERVER_H_

#include "TaskInterface.h"

#ifndef _TASK_CLIENT

class elementdataman;

// ���������ļ�
bool LoadTasksFromPack(elementdataman* pDataMan, const char* szPackPath, const char* szDynPackPath);

// �ͷ������ļ�
void ReleaseTaskTemplMan();

// ֪ͨ�ӳ�����
void OnTeamSetup(TaskInterface* pTask);

// ֪ͨ�ӳ���ɢ
void OnTeamDisband(TaskInterface* pTask);

// ֪ͨ�¶ӳ��ӳ�ת��
void OnTeamTransferCaptain(TaskInterface* pTask);

// ֪ͨ�ӳ������Ա
void OnTeamAddMember(TaskInterface* pTask, const task_team_member_info* pInfo);

// ֪ͨ�ӳ���Ա�˳�
void OnTeamDismissMember(TaskInterface* pTask, unsigned int ulPlayerId);

// ֪ͨ��Ա�˳�
void OnTeamMemberLeave(TaskInterface* pTask);

// �����������֪ͨ
void OnPlayerNotify(TaskInterface* pTask, unsigned int ulPlayerId, const void* pBuf, unsigned int sz);

// ���տͻ���֪ͨ
void OnClientNotify(TaskInterface* pTask, const void* pBuf, unsigned int sz);

// ��������ܷ񷢷�
bool OnTaskCheckDeliver(TaskInterface* pTask, unsigned int ulTaskId,unsigned int ulStorageId,bool bNotifyErr=true);

// NPC����������Ʒ
void OnNPCDeliverTaskItem(TaskInterface* pTask, unsigned int ulNPCId, unsigned int ulTaskId);

// �����NPC����ɵ�����
void OnTaskCheckAward(TaskInterface* pTask, unsigned int ulTasklId, int nChoice);

// ��ұ�ɱ
void OnTaskPlayerKilled(TaskInterface* pTask);

// ��ʱ�����������ϵͳ��ʱ���н��������������״̬
void OnTaskCheckAllTimeLimits(unsigned int ulCurTime);

// ����ɱ��
void OnTaskKillMonster(TaskInterface* pTask, unsigned int ulTemplId, unsigned int ulLev, float fRand,int dps,int dph);

void OnTaskEventTaskKillMonster(TaskInterface* pTask, unsigned int ulTemplId, bool bTeam);
void OnTaskEventTaskCollectMining(TaskInterface* pTask, unsigned int ulTemplId, bool bTeam);
void OnTaskEventTaskFinishTask(TaskInterface* pTask, unsigned int ulTemplId, bool bTeam);

void OnTaskUseItemsFinishTask(TaskInterface* pTask, unsigned int ulItemId);

// ���ɱ��
void OnTaskTeamKillMonster(TaskInterface* pTask, unsigned int ulTemplId, unsigned int ulLev, float fRand);

// �������
void OnTaskMining(TaskInterface* pTask, unsigned int ulTaskId);

void OnDeliverTask(TaskInterface* pTask, unsigned int ulTaskId);

// ��������ȫ������
void TaskQueryGlobalData(
	unsigned int ulTaskId,
	unsigned int ulPlayerId,
	const void* pPreservedData,
	unsigned int size);

// ��������ȫ������
void TaskUpdateGlobalData(
	unsigned int ulTaskId,
	const unsigned char pData[TASK_GLOBAL_DATA_SIZE]);

// ���ȫ������
void OnTaskReceivedGlobalData(
	TaskInterface* pTask,
	unsigned int ulTaskId,
	unsigned char pData[TASK_GLOBAL_DATA_SIZE],
	const void* pPreservedData,
	unsigned int size);

// ���������
void OnForgetLivingSkill(TaskInterface* pTask);

// �������
void OnTaskPlayerLevelUp(TaskInterface* pTask);

// PQ�����ʼ��
bool InitPublicQuestSystem();

// PQ�����heartbeat
void OnTaskPublicQuestHeartbeat();

// PQ����ɱ�ֹ��׶ȷ���
void OnTaskKillPQMonster(TaskInterface* pTask, unsigned int ulTemplId, bool bOwner,float fTeamContrib,int iTeamMemberCnt,float fPersonalContib);

// PQ�����ʼ��link��Ϣ
void OnTaskCheckPQEnterWorldInit(TaskInterface* pTask, unsigned int ulTaskId);

// �뿪��������ʧ��
void OnTaskLeaveForce(TaskInterface* pTask);

enum{
	EX_TK_SENDAUMAIL_LEVEL1 = 1,	//	��AU�����ٻغ����ʼ�����1����������δ��¼������
	EX_TK_SENDAUMAIL_LEVEL2 = 2,
	EX_TK_SENDAUMAIL_LEVEL3 = 3,
	EX_TK_SENDAUMAIL_EXAWARD = 4,	//	��AU�����ٻغ����ʼ���Ŀ���������������ʼ���Ŀ��
	EX_TK_WELCOMBACK_REWARD	= 5,	//	���߻ظ��ٻ��ʼ�ʱ�յ���ϵͳ��ӭ�ع齱��
};
// 
void OnTaskExternEvent(TaskInterface* pTask, int Event);

// ĳЩ���������񷢷ţ������˺Ų���
void OnTaskSpecialAwardMask(TaskInterface* pTask,unsigned int ulTaskID);

// ɱ������
void OnTaskKillPlayer(TaskInterface* pTask, int iOccupation, bool bGender, int iLevel, int iForce, float fRand);

// �ƺ�����
void OnTaskTitle(TaskInterface* pTask, unsigned int ulTaskID);

// ��ͨ��NPC��������
void OnTaskManualTrig(TaskInterface* pTask,unsigned int ulTaskId, bool bNotifyErr=true);

// ������е���������
void ClearAllTowerTask(TaskInterface* pTask);

#endif

#endif
