#ifndef HTTP_LOGIC_MODULE_H
#define HTTP_LOGIC_MODULE_H
#include "BaseModule.h"
#include <regex>

#define MAX_LINE_LEN 4096
#define HTTP_VERSION "HTTP/1.1"

#define GET "GET"
#define POST "POST"

class MsgModule;
class EventModule;
class NetObjectModule;
class ScheduleModule;
class HttpCgiModule;

enum HTTP_POINT
{
	FIRST_LINE,
	HEADS,
	BODY,
	END,
};

enum HTTP_STATE
{
	CONTINUE,
	NOT_FINISH,
	CONTINUE_100,
	FINISH,
	HERROR,
};


struct HttpBase
{
	int state;
	map<string, string> heads;
	string body;
	NetBuffer buff;

	virtual void Init() = 0;
	virtual void Clear() = 0;
	virtual void Recycle();
	string GetHead(const string& head);
};

struct HttpRequest :public HttpBase
{
	int bodySize;
	int scanned;
	string math;
	string url;
	map<string, string> uvals;

	virtual void Init();
	virtual void Clear();
	void RecvBuff(char* buf, const int& len);
	int Decode();
	int TryDecode();
};

struct HttpResponse :public HttpBase
{
	NetBuffer encode;
	string statusInfo;

	virtual void Init();
	virtual void Clear();

	void Encode(NetBuffer& nbuff);
	void EncodePHP(NetBuffer& nbuff);

	void SetStatus(int code, const string& info);
};

struct HttpMsg:public LoopObject
{
	enum
	{
		CLOSE,
		SEND_AND_CLOSE,
		SEND,
		NONE,
	};

	int socket;
	int opration;
	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;

	void Clear();

	HttpRequest request;
	HttpResponse response;
};

struct FileCash:public LoopObject
{
	string file;
	int64_t cashTime;
	NetBuffer buff;

	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
};

#define CASH_SIZE 10
#define CHECK_TIME 300000

class HttpLogicModule:public BaseModule
{
public:
	typedef function<void(HttpMsg*)> HttpCall;

public:
	HttpLogicModule(BaseLayer* l) :BaseModule(l)
	{};
	~HttpLogicModule()
	{};
	
	void SetWebRoot(const string& root);
	void OnGetPHPContent(const int& sock,NetBuffer& content);
protected:
	virtual void Init() override;
	virtual void Execute() override;

	void OnHttpClientConnect(const int& sock);
	void OnHttpClientClose(const int& sock);
	void OnRecvHttpMsg(NetMsg* msg);
	void OnRequest(HttpMsg* msg);
	void CloseClient(const int& sock);

	void OnGetFileResouce(HttpMsg* msg);
	bool SendFile(HttpMsg* msg,const string& file);

	void InitPath();

	bool GetFromCash(const string& file, NetBuffer& buff);
	void AddCashFile(const string& file, NetBuffer& buff);
	void CheckCash(int64_t& nt);
private:
	MsgModule* m_msgModule;
	EventModule* m_eventModule;
	NetObjectModule* m_netObjModule;
	ScheduleModule* m_scheduleModule;
	HttpCgiModule* m_httpCgiModule;

	string m_webRoot;

	map<string, string> m_fileType;
	vector<string> m_index;

	map<int, SHARE<HttpMsg>> m_cliens;
	map<regex, map<string, HttpCall>> m_httpCall;
	map<string, HttpCall> m_defaultCall;

	int m_cashIndex;
	int64_t m_lastCheck;
	map<string, SHARE<FileCash>> m_fileCash;
	list<SHARE<FileCash>> m_cashCheck[CASH_SIZE];
};

#endif