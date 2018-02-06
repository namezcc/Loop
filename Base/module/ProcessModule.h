#ifndef PROCESS_MODULE_H
#define PROCESS_MODULE_H

#include "BaseModule.h"

class LOOP_EXPORT ProcessModule:public BaseModule
{
public:
	ProcessModule(BaseLayer* l);
	~ProcessModule();

	bool CreateServer(const string& name, const string& line="");

private:


	// ͨ�� BaseModule �̳�
	virtual void Init() override;

	virtual void Execute() override;

};

#endif