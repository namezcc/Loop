#ifndef LOG_LATER_H
#define LOG_LATER_H
#include "BaseLayer.h"
#include <mutex>

class MsgModule;

class LogLayer:public BaseLayer
{
public:
	LogLayer();
	~LogLayer();

	void log(BaseData* msg);

	void start(ServerNode* ser,LoopServer* server);
private:
	// Í¨¹ý BaseLayer ¼Ì³Ð
	virtual void init() override;

	virtual void loop() override;

	virtual void close() override;

	virtual void GetDefaultTrans(int32_t & ltype, int32_t & lid) override;

	bool m_init;
	std::mutex m_lock;
	MsgModule* m_msg_mod;
	PIPE m_pipe;
	std::thread m_th;
};

#define LOG_LAYER Single::GetInstence<LogLayer>()

#endif