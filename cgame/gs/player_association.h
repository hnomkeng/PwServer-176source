#ifndef __ONLINEGAME_GS_PLAYER_ASSOCIATION_H__
#define __ONLINEGAME_GS_PLAYER_ASSOCIATION_H__

#include <uchar.h>
#include <db_if.h>
#include <gsp_if.h>
#include <glog.h>

class gplayer_association
{
public:
	enum
	{
		MAX_CONTROBUTION = 100,
	};

public:
	struct ASSOCIATION
	{
		int enter_time;
		int leave_time;

		int camp_type;
		int camp_level;
		int camp_exp;

		int camp_contribution;
		int camp_money;
		int camp_supriments_count;

		int week_camp_money;
	};

private:
	ASSOCIATION association;

public:
	gplayer_association() { memset(this, 0x00, sizeof(*this)); }
	~gplayer_association() {}

	inline void Init()
	{
		// TODO
	}

	inline ASSOCIATION * GetAssociation() { return &association; }
	inline void SetAssociation(ASSOCIATION * pAssociation) { memcpy(&association, pAssociation, sizeof(association)); }	
};

#endif
