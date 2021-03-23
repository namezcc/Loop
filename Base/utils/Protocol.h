#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "BaseModule.h"
#include "ProtoDefine.h"

typedef std::function<void(int32_t, char*, int32_t)> DecodeDataCall;

class Protocol
{
	typedef void(*EncodeFuncPtr)(LocalBuffBlock&, NetMsg*);
	typedef bool(*DecodeFuncPtr)(NetBuffer&, const DecodeDataCall&);
public:
	
	Protocol(ProtoType ptype)
	{
		switch (ptype)
		{
		case PT_MY_PROTO:
			m_encodeFunc = Protocol::EecodeMyProto;
			m_decodeFunc = Protocol::DecodeMyProto;
			break;
		case PT_HTTP:
			m_encodeFunc = Protocol::EecodeHttpProto;
			m_decodeFunc = Protocol::DecodeHttpProto;
			break;
		}
	}

	void EncodeSendData(LocalBuffBlock& lb, NetMsg* msg)
	{
		m_encodeFunc(lb, msg);
	}

	bool DecodeReadData(NetBuffer& buff,const DecodeDataCall& call)
	{
		return m_decodeFunc(buff, call);
	}

private:

	static void EecodeMyProto(LocalBuffBlock& lb, NetMsg* nMsg)
	{
		lb.makeRoom(nMsg->getLen() + MsgHead::HEAD_SIZE);
		MsgHead::Encode(lb, nMsg->mid, nMsg->getLen());
		nMsg->getCombinBuff(&lb);
	}

	static bool DecodeMyProto(NetBuffer& buff, const DecodeDataCall& call)
	{
		bool res = true;
		int32_t read = 0;

		while (buff.use - read >= MsgHead::HEAD_SIZE)
		{
			MsgHead head;
			if (!MsgHead::Decode(head, buff.buf + read))
			{
				res = false;
				break;
			}

			if (head.size <= buff.use - read)
			{
				call(head.mid, buff.buf + read + MsgHead::HEAD_SIZE, head.size - MsgHead::HEAD_SIZE);
				read += head.size;
			}
			else
			{
				if(buff.len < head.size)
					buff.MakeRoome(head.size);
				break;
			}
		}
		buff.moveHalf(read);
		return res;
	}

	// consider optimise no copy data
	static void EecodeHttpProto(LocalBuffBlock& lb, NetMsg* nMsg)
	{
		lb.makeRoom(nMsg->getLen());
		nMsg->getCombinBuff(&lb);
	}

	static bool DecodeHttpProto(NetBuffer& buff, const DecodeDataCall& call)
	{
		call(N_RECV_HTTP_MSG, buff.buf, buff.use);
		buff.moveHalf(buff.use);
		return true;
	}

private:
	EncodeFuncPtr m_encodeFunc;
	DecodeFuncPtr m_decodeFunc;
};

#endif
