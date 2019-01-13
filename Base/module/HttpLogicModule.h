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
	virtual void Recycle()
	{
		Clear();
		buff.Clear();
	}
	inline string GetHead(const string& head) { return heads[head];};
};

struct HttpRequest :public HttpBase
{
	uint32_t bodySize;
	uint32_t scanned;
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
	string statusInfo;

	virtual void Init();
	virtual void Clear();

	void Encode(NetBuffer& nbuff);
	void EncodePHP(NetBuffer& nbuff);

	void SetStatus(int code, const string& info)
	{
		state = code;
		statusInfo = move(info);
	};
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

class LOOP_EXPORT HttpLogicModule:public BaseModule
{
public:
	typedef function<void(HttpMsg*)> HttpCall;
	typedef function<bool(HttpMsg*)> HttpCheck;

	class regex_orderable : public regex 
	{
		std::string str;
	public:
		regex_orderable(const char *regex_cstr) : regex(regex_cstr), str(regex_cstr) {}
		regex_orderable(std::string regex_str) : regex(regex_str), str(std::move(regex_str)) {}
		bool operator<(const regex_orderable &rhs) const noexcept {
			return str < rhs.str;
		}
	};

public:
	HttpLogicModule(BaseLayer* l) :BaseModule(l)
	{};
	~HttpLogicModule()
	{};
	
	void SetWebRoot(const string& root);
	void OnGetPHPContent(const int& sock,NetBuffer& content);

	template<typename T,typename F>
	void AddUrlCallBack(const string& url, const string& math,T&&t, F&&f)
	{
		m_httpCall[url][math] = AnyFuncBind::Bind(forward<F>(f), forward<T>(t));
	}

	template<typename T,typename F>
	void AddRequestCheck(T&&t,F&&f)
	{
		m_reqCheck = SHARE<HttpCheck>(new HttpCheck(ANY_BIND(t,f)));
	}

	void SendHttpMsg(const int& sock,HttpCall&& call);
protected:
	virtual void Init() override;
	virtual void Execute() override;

	void OnHttpClientConnect(const int& sock);
	void OnHttpClientClose(const int& sock);
	void OnRecvHttpMsg(NetMsg* msg);
	void OnRequest(HttpMsg* msg);

	void DoResPonse(HttpMsg* msg);

	void CloseClient(const int& sock);

	void OnGetFileResouce(HttpMsg* msg);
	bool SendFile(HttpMsg* msg,const string& file);
	string GetContentType(const string& ext);

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
	map<regex_orderable, map<string, HttpCall>> m_httpCall;
	map<string, HttpCall> m_defaultCall;
	SHARE<HttpCheck> m_reqCheck;

	bool m_useCash;
	int m_cashIndex;
	int64_t m_lastCheck;
	map<string, SHARE<FileCash>> m_fileCash;
	list<SHARE<FileCash>> m_cashCheck[CASH_SIZE];
};

#endif