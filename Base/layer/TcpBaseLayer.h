#ifndef TCP_BASE_LAYER_H
#define TCP_BASE_LAYER_H

#include "BaseLayer.h"
#include "MsgModule.h"
#include "ProtoDefine.h"

class TcpBaseLayer:public BaseLayer
{
public:
	TcpBaseLayer(const int32_t& _port, const ProtoType& _potype):BaseLayer(LY_NET),
		m_port(_port),m_protoType(_potype)
	{
		CreateModule<MsgModule>();
	}

	virtual ~TcpBaseLayer() {};

protected:

	virtual void GetDefaultTrans(int32_t & ltype, int32_t & lid)
	{
		auto it = m_pipes.begin();
		ltype = it->first;
		lid = 0;
	}

	int32_t m_port;
	ProtoType m_protoType;
};

#endif
