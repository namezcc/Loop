#ifndef INPUT_CMD_H
#define INPUT_CMD_H

#include "LoopArray.h"
#include <string>
#include <thread>

struct InputPam
{
	std::string pams;
};

typedef LoopList<InputPam*> LoopPam;

class InputCmd
{
public:
	InputCmd();
	~InputCmd();

	void Run();
	
	InputPam* PopInputCmd()
	{
		InputPam* pam = NULL;
		m_deal.pop(pam);
		return pam;
	}

	void PushPamCash(InputPam* pam)
	{
		m_cash.write(pam);
	}

private:

	LoopPam m_cash;
	LoopPam m_deal;
	std::thread m_thr;

};

#endif
