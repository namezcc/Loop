#include "HttpCgiModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "HttpLogicModule.h"

HttpCgiModule::HttpCgiModule(BaseLayer * l):BaseModule(l)
{
}

HttpCgiModule::~HttpCgiModule()
{
}

void HttpCgiModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
	m_eventModule = GetLayer()->GetModule<EventModule>();
	m_netObjModule = GetLayer()->GetModule<NetObjectModule>();
	m_httpLogicModule = GetLayer()->GetModule<HttpLogicModule>();

	m_eventModule->AddEventCall(E_PHP_CGI_CLOSE,BIND_EVENT(OnCgiClose,int));
	
}

void HttpCgiModule::AfterInit()
{
	ReconnectCgi();
}

void HttpCgiModule::Execute()
{
}

void HttpCgiModule::OnCgiClose(const int& sock)
{
	m_phpcgi.state = CONN_STATE::CLOSE;
	m_phpcgi.socket = -1;
}

void HttpCgiModule::OnRecvCgiMsg(NetMsg* msg)
{
	if (msg->socket != m_phpcgi.socket)
		return;

	auto buff = msg->m_buff;
	while(buff){
		m_recvPack.combin(buff->m_buff, buff->getSize());
		buff = buff->m_next;
	}

	int reqId;
	int state;
	while ((state = ReadPacket(reqId)) == CGI_CONTINUE);

	switch (state)
	{
	case END_REQUEST:
	{
		NetBuffer pack;
		pack = move(m_content);
		m_recvPack.moveHalf(m_recvPack.scan);
		//֪ͨ�ص�
		m_httpLogicModule->OnGetPHPContent(reqId, pack);
		return;
	}
		break;
	case CGI_ERROR:
		break;
	}
}

int HttpCgiModule::ReadPacket(int& reqid)
{
	if (m_recvPack.use - m_recvPack.scan <= CGI_HEADER_LEN)
		return CGI_REQUEST::NOT_COMPLETE;

	char* pack = m_recvPack.buf + m_recvPack.scan;
	Cgi_header header;
	header.version = ord(pack[0]);
	header.type = ord(pack[1]);
	header.requestId = reqid = (ord(pack[2]) << 8) + ord(pack[3]);
	header.contentLength = (ord(pack[4]) << 8) + ord(pack[5]);
	header.paddingLength = ord(pack[6]);
	header.reserved = ord(pack[7]);

	auto nlest = m_recvPack.use - m_recvPack.scan - CGI_HEADER_LEN;

	if (nlest>= header.contentLength+header.paddingLength)
	{
		auto len = header.contentLength + header.paddingLength;
		m_recvPack.scan += CGI_HEADER_LEN + len;
		if (header.type == CGI_STDOUT || header.type == CGI_STDERR)
		{
			auto beg = m_recvPack.buf + (m_recvPack.scan - len);
			m_content.append(beg,len);
			return CGI_REQUEST::CGI_CONTINUE;
		}
		else if (header.type == CGI_END_REQUEST)
			return CGI_REQUEST::END_REQUEST;
		else
			return CGI_REQUEST::CGI_CONTINUE;
	}
	return CGI_REQUEST::NOT_COMPLETE;
}

void HttpCgiModule::ReconnectCgi()
{
	m_netObjModule->ConnectServer(m_phpcgi, [this](bool res, NetServer& ser) {
		if (res)
		{
			m_phpcgi.state = CONN_STATE::CONNECT;
			m_phpcgi.socket = ser.socket;
			m_netObjModule->AcceptConn(ser.socket,CONN_PHP_CGI);
		}else
			LP_ERROR << "PHP CGI not run on host:" << ser.ip << " port:" << ser.port;
	});
}

void HttpCgiModule::ConnectCgi(const string & ip, const int & port)
{
	m_phpcgi.ip = ip;
	m_phpcgi.port = port;
	m_phpcgi.state = CONN_STATE::CLOSE;
}
//���Ż�
void HttpCgiModule::Request(const int & sock, HeadData & header, const string & content)
{
	if (m_phpcgi.state == CONN_STATE::CLOSE)
	{
		ReconnectCgi();
		return;
	}

	NetBuffer buff;
	buff.MakeRoome(4096);
	BuildRecord(buff, header, content,sock);
	m_netObjModule->SendHttpMsg(m_phpcgi.socket, buff);
}

void HttpCgiModule::BuildRecord(NetBuffer & buff, HeadData & header, const string & nStdin, const int& requestId)
{
	header["CONTENT_LENGTH"] = to_string(nStdin.size());
	char head[CGI_HEADER_LEN] = {0,1,1,0,0,0,0,0};

	BuildPacket(buff, CGI_BEGIN_REQUEST, string(head, CGI_HEADER_LEN), requestId);

	string params;
	for (auto& it:header)
		BuildKVPair(params,it.first,it.second);
	
	BuildPacket(buff, CGI_PARAMS, params, requestId);
	BuildPacket(buff, CGI_PARAMS, "", requestId);
	if(nStdin.size()>0)
		BuildPacket(buff, CGI_STDIN, nStdin, requestId);
	BuildPacket(buff, CGI_STDIN, "", requestId);
}

void HttpCgiModule::BuildPacket(NetBuffer & buff, const int & type, const string & content, const int & requestId)
{
	int contentLength = (int)content.size();
	assert(contentLength >= 0 && contentLength <= CGI_MAX_LENGTH);

	std::string record;
	record += char(CGI_VERSION);                            // version
	record += char(type);									// type
	record += char((requestId >> 8) & 0xff);				// requestIdB1
	record += char((requestId) & 0xff);						// requestIdB0
	record += char((contentLength >> 8) & 0xff);			// contentLengthB1
	record += char((contentLength) & 0xff);					// contentLengthB0
	record += char(0);										// paddingLength
	record += char(0);										// reserved
	buff.append(record).append(content);
}

void HttpCgiModule::BuildKVPair(string& res, const string & key, const string & val)
{
	size_t nlen = key.size();
	if (nlen < 128) {
		res.push_back((unsigned char)nlen);                     // name LengthB0
	}
	else {
		res.push_back((unsigned char)((nlen >> 24) | 0x80));    // name LengthB3
		res.push_back((unsigned char)((nlen >> 16) & 0xff));    // name LengthB2
		res.push_back((unsigned char)((nlen >> 8) & 0xff));    // name LengthB1
		res.push_back((unsigned char)(nlen & 0xff));            // name LengthB0
	}

	size_t vlen = val.size();
	if (vlen < 128) {
		res.push_back((unsigned char)vlen);                     // value LengthB0
	}
	else {
		res.push_back((unsigned char)((vlen >> 24) | 0x80));    // value LengthB3
		res.push_back((unsigned char)((vlen >> 16) & 0xff));    // value LengthB2
		res.push_back((unsigned char)((vlen >> 8) & 0xff));    // value LengthB1
		res.push_back((unsigned char)(vlen & 0xff));            // value LengthB0
	}
	res.append(key).append(val);
}