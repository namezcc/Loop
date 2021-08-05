#include "InputCmd.h"
#include <iostream>


InputCmd::InputCmd()
{
}

InputCmd::~InputCmd()
{
	m_thr.join();
}

void InputCmd::Run()
{
	m_thr = std::thread([this]() {
		while (true)
		{
			std::string cmd;
			std::getline(std::cin, cmd, '\n');
			if (cmd.empty())
			{
				continue;
			}

			InputPam* pam = NULL;
			if (!m_cash.pop(pam))
				pam = new InputPam();
			pam->pams = cmd;
			m_deal.write(pam);
		}
	});
}
