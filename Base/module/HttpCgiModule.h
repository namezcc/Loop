#ifndef HTTP_CGI_MODULE_H
#define HTTP_CGI_MODULE_H
#include "BaseModule.h"

inline unsigned int ord(int val) {
	return ((unsigned int)val & 0xff);
}

typedef struct {
	unsigned int version;
	unsigned int type;
	unsigned int requestId;
	unsigned int contentLength;
	unsigned int paddingLength;
	unsigned int reserved;
	unsigned int flag;
} Cgi_header;

#define CGI_VERSION				1

#define CGI_BEGIN_REQUEST       1      //开始请求
#define CGI_ABORT_REQUEST       2      //异常终止请求
#define CGI_END_REQUEST         3      //正常终止请求
#define CGI_PARAMS              4      //传递参数
#define CGI_STDIN               5      //POST内容传递
#define CGI_STDOUT              6      //正常响应内容
#define CGI_STDERR              7      //错误输出
#define CGI_DATA                8
#define CGI_GET_VALUES          9
#define CGI_GET_VALUES_RESULT   10
#define CGI_UNKNOWN_TYPE        11      //通知webserver所请求type非正常类型
#define CGI_MAXTYPE				11

#define CGI_RESPONDER			1
#define CGI_AUTHORIZER			2
#define CGI_FILTER				3

#define CGI_REQUEST_COMPLETE	0
#define CGI_CANT_MPX_CONN		1
#define CGI_OVERLOADED			2
#define CGI_UNKNOWN_ROLE		3

#define CGI_MAX_CONNS		"MAX_CONNS"
#define CGI_MAX_REQS		"MAX_REQS"
#define CGI_MPXS_CONNS		"MPXS_CONNS"

#define CGI_HEADER_LEN			8
#define CGI_MAX_LENGTH			0xffff

enum CGI_REQUEST
{
	CGI_CONTINUE,
	NOT_COMPLETE,
	END_REQUEST,
	CGI_ERROR,
};

typedef std::map<string, string> HeadData;

class MsgModule;
class EventModule;
class NetObjectModule;
class HttpLogicModule;

class LOOP_EXPORT HttpCgiModule:public BaseModule
{
public:
	HttpCgiModule(BaseLayer* l);
	~HttpCgiModule();

	void ConnectCgi(const string& ip, const int& port);
	void Request(const int& sock, HeadData& header,const string& content="");

protected:
	void OnCgiConnect(NetServer* ser);
	void OnCgiClose(const int& sock);
	void OnRecvCgiMsg(NetMsg* msg);

	int ReadPacket(int& reqid);

	void ReconnectCgi();
private:
	virtual void Init() override;
	virtual void AfterInit();
	virtual void Execute() override;

	void BuildRecord(NetBuffer& buff,HeadData& header, const string& nStdin, const int& requestId);
	void BuildPacket(NetBuffer& buff, const int& type, const string& content, const int& requestId);
	void BuildKVPair(string& res, const string& key, const string& val);

	NetServer m_phpcgi;

	NetBuffer m_recvPack;
	NetBuffer m_content;

	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netObjModule;
	HttpLogicModule* m_httpLogicModule;
};

#endif