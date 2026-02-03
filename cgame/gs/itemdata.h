#ifndef __ONLINEGAME_GS_ITEM_DATA_H__
#define __ONLINEGAME_GS_ITEM_DATA_H__

#pragma pack(push, 1)
struct item_data 
{
	int type;		//����
	unsigned int count;		//����
	unsigned int pile_limit;	//�ѵ�����
	int equip_mask;		//װ����־	��ʾ����װ�����ĸ��ط� 0x80000000��ʾ�Ƿ�64λ��չ 0x40000000��ʾ�Ƿ��и�������
	int proc_type;		//��Ʒ�Ĵ�����ʽ
	int classid;		//��Ӧitem�����GUID �������-1����ʾ����Ҫitem����
	struct 
	{
		int guid1;
		int guid2;
	}guid;			//GUID
	unsigned int price;		//���� ���������Ϊһ���ο�ֵ,ʵ��ֵ��ģ���е�Ϊ׼
	int expire_date; 	//��Ʒ����
	unsigned int content_length;	//�������ݵĴ�С
	char * item_content;	//�������� ��item����ʹ��

};
#pragma pack(pop)

#endif

