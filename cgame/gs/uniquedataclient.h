#ifndef __ONLINEGAME_GS_UNIQUE_DATA_MAN_H__
#define __ONLINEGAME_GS_UNIQUE_DATA_MAN_H__

#include "amemobj.h"
#include <map>
#include <octets.h>

enum UNIQUE_DATA_ENUM   // ��ϵͳ��ռ����
{
	UDI_HISTORY_VERSION = 0,

	UDI_CARNIVAL_COUNT_LIMIT = 8500,  // ����ռ��8000~9000 ��localmacro.h
};

enum UNIQUE_DATA_TYPE // ͬlocalmacro.h
{
	UDT_INVALID,		// δʹ��
	UDT_INT,			// ����
	UDT_DOUBLE,			// ������
	UDT_OCTETS,			// �䳤

	UDT_COUNT	
};

enum UNIQUE_MODIFY_ERR // localmacro.h
{
	ERR_UNIQUE_CLASH = 436,         // Ψһֵ���ó�ͻ 
	ERR_UNIQUE_TYPE_INVALID = 437,  // Ψһֵ���ʹ���
	ERR_UNIQUE_VERSION_FAIL = 438,  // Ψһֵ�汾����
};

// -----------  abase::octets �ǣ��������Ͱ�ȫת�� ---------------
struct UDOctets
{
	UDOctets() : _type(UDT_INVALID) {}
	UDOctets(int n) : _type(UDT_INT), _data(&n,sizeof(int)) {}
	UDOctets(double n) : _type(UDT_DOUBLE), _data(&n,sizeof(double)) {}
	UDOctets(const void* p,unsigned int sz,int type = UDT_OCTETS) : _type(type), _data(p,sz) {}
	UDOctets(const UDOctets& rhs) : _type(rhs._type), _data(rhs._data) {}

	operator const int&() const { ASSERT(_type == UDT_INT && _data.size() == sizeof(int) && "UDOctets operator int fail!");  return *((const int*)data()); }
	operator const double&() const { ASSERT(_type == UDT_DOUBLE && _data.size() == sizeof(double) && "UDOctets operator double fail!"); return *((const double*)data());}

	UDOctets& operator +=(const UDOctets& rhs)
	{
		ASSERT( _type == rhs._type && "UDOctets operator += in UDOctets type fail!");
		
		_type = rhs._type;

		switch(_type)
		{
			case UDT_INT:
				{
					reserve(sizeof(int));
					int nval = *((int*)data());
					*((int*)data()) = nval + *((int*)rhs.data());
				}
				break;
			case UDT_DOUBLE:
				{
					reserve(sizeof(double));
					double dval = *((double*)data());
					*((double*)data()) = dval + *((double*)rhs.data());
				}
				break;
			case UDT_OCTETS:
				{
					_data = rhs._data;			
				}
				break;
		}
		
		return *this;
	}
	UDOctets& operator =(const UDOctets& rhs)
	{
		ASSERT(!(_type && _type != rhs._type) && "UDOctets operator = in UDOctets type fail!");
		
		_type = rhs._type;
		_data = rhs._data;
		
		return *this;
	}
	UDOctets& operator =(int irhs)
	{
		ASSERT(!(_type && _type != UDT_INT) && "UDOctets operator = in INT type fail!");
			
		_type = UDT_INT;
		reserve(sizeof(int));
		*((int*)data()) = irhs;
		
		return *this;
	}
	UDOctets& operator =(double drhs)
	{
		ASSERT(!(_type && _type != UDT_DOUBLE) && "UDOctets operator = in DOUBLE type fail!");
			
		_type = UDT_DOUBLE;
		reserve(sizeof(double));
		*((double*)data()) = drhs;
		
		return *this;
	}
	bool operator ==(const UDOctets& rhs)
	{
		return _type == rhs._type && _data == rhs._data;
	}
	bool operator !=(const UDOctets& rhs)
	{
		return !(*this == rhs);
	}

	int 	type() 	const { return _type;}
	unsigned int 	size() 	const { return _data.size();}
	void 	reserve(unsigned int n) { _data.reserve(n);}
	void*	data()	{ return _data.begin();}
	const void* data() const { return _data.begin();}
	
	int 		  _type;
	abase::octets _data;
};

class gplayer;
class UniqueDataClient
{
	enum 
	{
		UDC_STATE_UNINIT,
		UDC_STATE_OPEN,
		UDC_STATE_CLOSE,
	};
	
	friend class world_manager;
	UniqueDataClient(): _world_tag(-1),_modify_version(0), _state(UDC_STATE_UNINIT), _tickcount(0), _lock(0), _dbg_time(0) {}
	void  SetWorldTag(int wt) { _world_tag = wt ;}

	static const unsigned int MAX_UDPACKET_LENTH = 1024;  // ��Ϣ������
public:

	// -----------  ȫ�ֱ���ֵ�������� ---------------
	struct ModifyOperater
	{
		// -----------  ֵ��ʼ���ص����� ---------------
		// key ȫ�ֱ�����ֵ
		// val ȫ�ֱ���ֵ
		virtual bool  OnInit(int key, const UDOctets& val) 		{return false;}
		// -----------  ֵ�Ķ���ص����� ---------------
		// key ȫ�ֱ�����ֵ
		// val ȫ�ֱ���ֵ
		// retcode   |0 �ɹ� |436 ���ó�ͻ |437 �������Ͳ��� |438 �汾����
		// localflag |true �����޸�  | false ����޸�
		virtual bool  OnModify(int key, const UDOctets& val, int retcode, bool localflag) 	{ return false;}
		// -----------  ֵ�Ķ�ǰԤ�к��� --------------
		// key ȫ�ֱ�����ֵ
		// val ȫ�ֱ���ֵ
		// setflag �ı������
		virtual bool  CheckModify(int key,const UDOctets& val, bool setflag)		{ return false;}
	};

//	------------------------- ����ע�� ---------------------------------
//  initval ���ݳ�ʼ�� ��dbδ���ڴ�ֵʱ�����Ψһ�γ�ʼ��
//  noticeclient ��ɫ���߻��޸ĺ�֪ͨ�ͻ��� 
//  freeze �����Ƿ��ֹ�޸� 
//  callback ���ݸĶ�����
	void 	Register(int key,UDOctets initval, ModifyOperater* callback = NULL, bool noticeclient = false,bool freeze = false);	

/// ��ѯ  ȡ������ͬ�����ֵ ����ʧ��
	bool	GetData(int key,UDOctets & val);

/// �޸�
	int		ModifyData(int key, UDOctets val, bool setflag);
	int		GetValueKey(int key);
/// ModifyOperater ר��modify�ӿڣ���ֹ����
	int 	ModifyDataInCallback(int key, UDOctets val, bool setflag);
/// --------------------------- ��ʹ�ýӿ� ------------------------------------------------
public:	
/// tick
	void 	OnHeartbeat();
/// ��ʼ��	
	void  	OnDataLoad(int key,int type,int version, const void* p,unsigned int sz);
	void 	OnLoadFinish();
	void 	OnSystemClose();
/// �޸Ļص�
	void  	OnDataModify(int worldtag,int key, int type, const void* val, unsigned int sz,const void* oldval, unsigned int osz, int retcode, int version);
/// ��ҵ�½
	void 	OnRoleLogin(gplayer* prole);
/// ��Ҳ�ѯ
	void 	OnRoleQuery(gplayer* prole, int count,int keys[]);
/// ����
	int 	OnDump(int key,char* str,unsigned int len);
	void 	SetDebugTime(int dt) { _dbg_time = dt; }
private:
	int 	_ModifyDataNoLock(int key,UDOctets& val,bool setflag);
	void	_DebugModifySync();
private :
	struct SUniqueDataElem
	{
		SUniqueDataElem() : wlock(false), wsync(false), freeze(false), vtype(0),inittype(0), operater(NULL), 
						timeout(0), notifyrole(false) ,timever(0),clientver(0), serverver(0)
		{
		}
		~SUniqueDataElem() 
		{
			if(operater) delete operater;
		}
		

		void init(const UDOctets& val,int ver)
		{
			vtype = val._type;
			syncval = val;
			stubval = val;
			localval = val;
			timever = clientver = serverver = ver;
		}
		void update(const UDOctets& val)
		{
			syncval = val;
		}

		void dump(char* str,unsigned int len);

		bool wlock;         // ��ռд
		bool wsync;			// дͬ����
		bool freeze;		// �����޸Ľ�ֹ
		int  vtype;			// ��������	
		int  inittype;		// ����ʼ����������

		UDOctets syncval;  // sync��ͬ��ֵ
		UDOctets stubval;  // stub�ύ�ı�ʱ�̱�����ͬ��ֵ��
		UDOctets localval; // localͬ�������б��ظı䵽��ֵ
		
		ModifyOperater* operater;
		int	 timeout;
		bool notifyrole;
		
		int	 timever;		// timeout�ð汾
		int  clientver;		// �����޸İ汾
		int  serverver;		// �������汾
	};

	class SUniqueData
	{
	public:
		~SUniqueData()
		{
			SGUMap::iterator iter = _elem_map.begin();
			SGUMap::iterator iend = _elem_map.end();

			for(;iter != iend; ++iter)
			{
				delete iter->second;
			}

			_elem_map.clear();
		}
		
		SUniqueDataElem& operator[](int key) 
		{ 
			SGUMap::iterator iter = _elem_map.find(key);
			if(iter == _elem_map.end()) 
				iter = _elem_map.insert(_elem_map.begin(), std::make_pair(key,new SUniqueDataElem()));
			return *iter->second;
		}
		
		void init(int worldtag);
		void timeout(int worldtag);
		void syncrole(gplayer* prole);
		bool exist(int key) { return _elem_map.find(key) != _elem_map.end();}
		int  next(int key) 
		{ 
			SGUMap::iterator iter = _elem_map.find(key); 
			if(iter == _elem_map.end())
				return -1;
			if(++iter == _elem_map.end())
				return -1;
			return iter->first;
		}	
	private:
		typedef std::map<int,SUniqueDataElem* const> SGUMap;
		SGUMap _elem_map;
	} _data;

	int	 _world_tag;
	int  _modify_version;	
	int  _state;
	int  _tickcount;
	int  _lock;	// ud ���ݲ�����

	//// for debug
	int  _dbg_time;
};

#endif
