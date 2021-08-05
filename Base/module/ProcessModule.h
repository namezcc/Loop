#ifndef PROCESS_MODULE_H
#define PROCESS_MODULE_H

#include "BaseModule.h"

class MsgModule;

class LOOP_EXPORT ProcessModule:public BaseModule
{
public:
	ProcessModule(BaseLayer* l);
	~ProcessModule();

	void CreateServer(const SERVER_TYPE& sertype,const int32_t& nid,const int32_t& port);
	void CreateLoopProcess(const std::string& proname, const std::string& logname,const std::vector<std::string>& args);
private:
	// ͨ�� BaseModule �̳�
	virtual void Init() override;
	virtual void Execute() override;

private:
	std::string m_error;
	std::string m_logdir;

	MsgModule* m_msgModule;

	std::map<int32_t, std::string> m_server_name;
};

#endif