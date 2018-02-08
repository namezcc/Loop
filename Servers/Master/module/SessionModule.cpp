#include "SessionModule.h"
#include "HttpLogicModule.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "ReflectData.h"
#include "Crypto/md5.h"


SessionModule::SessionModule(BaseLayer * l):BaseModule(l)
{

}

SessionModule::~SessionModule()
{
}

void SessionModule::Init()
{
	m_httpModule = GetLayer()->GetModule<HttpLogicModule>();
	m_mysqlModule = GetLayer()->GetModule<MysqlModule>();
	m_msgModule = GET_MODULE(MsgModule);

	m_httpModule->AddRequestCheck(this, &SessionModule::OnCheckSession);
	m_httpModule->AddUrlCallBack("^/login.html$", POST, this, &SessionModule::OnHttpLogin);
}

void SessionModule::BeforExecute()
{
	LoadAdmin();
}

void SessionModule::Execute()
{
}

void SessionModule::LoadAdmin()
{
	vector<SHARE<Admin>> res;
	m_mysqlModule->Select(res, "select * from Admin;");
	if (res.size() == 0)
	{
		LP_WARN(m_msgModule)<< "Error LoadAdmin";
		return;
	}
	
	for (auto& m:res)
		m_admins[m->name] = m;
}

bool SessionModule::OnCheckSession(HttpMsg * msg)
{
	bool ret = false;
	ExitCall _call([&ret,&msg]() {
		if (!ret)
		{
			msg->response.SetStatus(302, "");
			msg->response.heads["Location"] = "/login.html";
			msg->opration = HttpMsg::SEND_AND_CLOSE;
		}
	});
	if (msg->request.url == "/login.html")
		return ret = true;

	auto cookie = msg->request.GetHead("Cookie");
	if (cookie.size() == 0)
		return ret;

	auto pos = cookie.find(COOKIE_NAME);
	if (pos == string::npos)
		return ret;
	char* beg = const_cast<char*>(cookie.data());
	Slice slice(beg + pos, beg + cookie.size());
	slice.CutToChar('=');
	slice.CutHead(1);
	auto strid=slice.CutWord();
	auto sid=loop_cast<int64_t>(strid);
	auto s = GetSession(sid);
	if (s)
		ret = true;
	return ret;
}

void SessionModule::OnHttpLogin(HttpMsg * msg)
{
	//¼ì²éÃÜÂë
	if (!CheckLogin(msg))
	{
		msg->response.SetStatus(403, "Unauthorized");
		msg->response.buff.append("none user or wrong password");
		msg->opration = HttpMsg::SEND_AND_CLOSE;
		return;
	}

	auto s = GetLayer()->GetSharedLoop<Session>();
	s->loseTime = GetSecend() + SESSION_LOSE_TIME;
	auto key = rand();
	s->id = (s->loseTime << 32) | key;
	m_session[s->id] = s;

	string cookie(COOKIE_NAME);
	cookie.append("=").append(loop_cast<string>(s->id)).append("; path=/");

	msg->response.SetStatus(302, "");
	msg->response.heads["Set-Cookie"] = move(cookie);
	msg->response.heads["Location"] = "/";
	msg->opration = HttpMsg::SEND_AND_CLOSE;
}

bool SessionModule::CheckLogin(HttpMsg * msg)
{
	Slice s(msg->request.body.data(), msg->request.body.data()+ msg->request.body.size());
	s.CutToChar('=');
	s.CutHead(1);
	auto name = s.CutToChar('&');
	s.CutToChar('=');
	s.CutHead(1);
	auto pass = s.CutWord();

	auto it = m_admins.find(name);
	if (it == m_admins.end())
		return false;
	char passmd5[MD5_SIZE+1];
	MD5_stringEx(const_cast<char*>(pass.data()), passmd5);
	return it->second->pass==passmd5;
}

Session * SessionModule::GetSession(int64_t & sid)
{
	auto it = m_session.find(sid);
	if (it != m_session.end())
		return it->second.get();
	return nullptr;
}
