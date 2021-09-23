#include "LogLayer.h"
#include "MsgModule.h"
#include "LogModule.h"
#include "ScheduleModule.h"

LogLayer::LogLayer():BaseLayer(LY_LOG), m_init(false)
{
}

LogLayer::~LogLayer()
{
	m_th.join();
}

void LogLayer::start(ServerNode* ser, LoopServer* server)
{
	std::lock_guard<std::mutex> _g(m_lock);

	if (m_init)
		return;

	SetServer(ser);
	SetLoopServer(server);
	m_th = std::thread([this]() {
		StartRun();
	});

	m_init = true;
}

void LogLayer::init()
{
	m_msg_mod = CreateModule<MsgModule>();
	CreateModule<LogModule>();
	CreateModule<ScheduleModule>();

	regPipe(LY_LOG, &m_pipe, NULL);	
}

void LogLayer::loop()
{
}

void LogLayer::close()
{
}

void LogLayer::GetDefaultTrans(int32_t & ltype, int32_t & lid)
{
}

void LogLayer::log(BaseData * msg)
{
	std::lock_guard<std::mutex> _g(m_lock);
	m_pipe.write(msg);
}
