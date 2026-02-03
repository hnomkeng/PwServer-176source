#ifndef __GNET_QUESTION_MANAGER_H
#define __GNET_QUESTION_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>

class QuestionManager
{
public:
	enum
	{
		GROUP_IDX0 = 5232,	
		QUESTION_MAX_COUNT = 300,
		QUESTION_MAX_ANSWER = 10,
		GROUP_REWARD_IDX0 = 5233,
		QUESTION_MAX_REWARD = 10,
	};
	static QuestionManager * instance;

private:

	struct LIST
	{
		int question;
		int answer_a;
		int answer_b;
		int answer_c;
		int answer_d;
	};

	struct REWARD
	{
		int exp;
		int sp;
		int money;
	};

private:

	int max_answer;
	LIST list[QUESTION_MAX_COUNT];
	REWARD award[QUESTION_MAX_REWARD];
	
public:
	void Init();

	int GetMaxAnswer() { return max_answer; }
	LIST * GetList(int pos) { return &list[pos]; }
	REWARD * GetReward(int pos) { return &award[pos-1]; }
	inline int GetCountValidList()
	{ 
		int res = 0;
		for (int i = 0; i < QUESTION_MAX_COUNT; i++)
		{
			if (list[i].question > 0)
			{
				res++;
			}
		}		
		return res; 
	}
	
	
QuestionManager()
{

}

~QuestionManager()
{
	
}

static QuestionManager * GetInstance()
{
	if (!instance)
	{
		instance = new QuestionManager();
	}
	return instance;
}

};


#endif