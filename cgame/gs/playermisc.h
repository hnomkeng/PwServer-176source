#ifndef __ONLINEGAME_GS_PLAYER_MISC_IMP_H__
#define __ONLINEGAME_GS_PLAYER_MISC_IMP_H__


extern const A3DVECTOR aExts[USER_CLASS_COUNT*2];
extern const A3DVECTOR aExts2[USER_CLASS_COUNT*2];
class gplayer_imp;
class phase_control  
{
	enum
	{
		STATE_GROUND,
		STATE_JUMP,
		STATE_FALL,
		STATE_FLY,
		STATE_WATER,
	};

	enum
	{
		MAX_JUMP_COUNT = 2,	
	};

	int   state;            //��ǰ״̬�������� վ�������棿������(����/����)
	float jump_distance;    //����ľ���͸߶Ⱥ�
	int   jump_time;        //�����ʱ���
	float drop_speed;       //�����ٶȿ���
	int   jump_count;		//��ǰ��Ծ����
	float max_jump_distance;//��worldmanager�е�_world_limit.lowjump����
	int   max_jump_time;	//��worldmanager�е�_world_limit.lowjump����
	float speed_ctrl_factor;//����y�����ٶȵ�ͳ�����ӣ������ǵ�ǰ����ù������Ϸ��ٶȵ�ƽ��
	int   fall_to_fly_time;	//����������״̬��ʱ������ڵ�������п����ɻ����տ�ʼ��һ��ʱ��(ʱ�䳤�̺������ӳ��й�)�ͻ�����Ϊ��Ҵ�ʱ�ڵ��䣬��������Ϊ��Ҵ�ʱ�ڷ��У�Ϊ��ֹ���д˶�ʱ���ڷ�����ֻ�����ҵ������ٶȣ������Ӵ��������
	int   fly_to_fall_time;	//����������״̬��ʱ����������Ϸ��й����йرշɻ����տ�ʼ��һ��ʱ��(ʱ�䳤�̺������ӳ��й�)�ͻ�����Ϊ��Ҵ�ʱ�ڷ��У���������Ϊ��Ҵ�ʱ�ڲ��ڷ��У�Ϊ��ֹ���д˶�ʱ���ڷ�����������Ҽ������Ϸɣ������Ӵ��������
public:
	phase_control():state(STATE_GROUND),jump_distance(0.0f),jump_time(0),drop_speed(0.0f),jump_count(0),max_jump_distance(0.0f),max_jump_time(0),speed_ctrl_factor(0.0f),fall_to_fly_time(0),fly_to_fall_time(0){}

	void Swap(phase_control & rhs)
	{
		phase_control tmp(*this);
		*this = rhs;
		rhs = tmp;
	}

	void Load(archive & ar);
	void Save(archive & ar);
	
	void Initialize(gplayer_imp * pImp);
	int PhaseControl(gplayer_imp * pImp, const A3DVECTOR & target, float theight, int mode, int use_time); //ÿ���ƶ����״̬�������ƶ������ɹ� ������Լ���״̬
	bool CheckLevitate()
	{
		return state == STATE_JUMP || state == STATE_FALL;
	}
};

#endif

