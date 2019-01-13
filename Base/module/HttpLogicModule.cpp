#include "HttpLogicModule.h"
#include "MsgModule.h"
#include "EventModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "HttpCgiModule.h"
#include <fstream>
#include "LPFile.h"

void HttpMsg::init(FactorManager * fm)
{
	request.Init();
	response.Init();
}

void HttpMsg::recycle(FactorManager * fm)
{
	request.Recycle();
	response.Recycle();
}

void HttpMsg::Clear()
{
	request.Clear();
	response.Clear();
}

void HttpRequest::Init()
{
	buff.buf = NULL;
	buff.len = 0;
	buff.use = 0;
	state = HTTP_POINT::FIRST_LINE;
	bodySize = 0;
	scanned = 0;
}
void HttpRequest::Clear()
{
	state = HTTP_POINT::FIRST_LINE;
	buff.moveHalf(scanned);
	scanned = buff.scan = 0;
	math.clear();
	url.clear();
	uvals.clear();
	heads.clear();
	body.clear();
}

void HttpRequest::RecvBuff(char* buf, const int& len)
{
	buff.combin(buf, len);
}
int HttpRequest::Decode()
{
	int st;
	while ((st = TryDecode()) == HTTP_STATE::CONTINUE);
	return st;
}
int HttpRequest::TryDecode()
{
	switch (state)
	{
	case HTTP_POINT::FIRST_LINE:
	{
		if (buff.use < 2)
			return HTTP_STATE::NOT_FINISH;
		for (uint32_t i = scanned; i <= buff.use - 2; i++)
		{
			if (buff.buf[i] == '\r' && buff.buf[i + 1] == '\n')
			{
				Slice hline(buff.buf, buff.buf + i);
				math = move(hline.CutWord());
				Slice uline = hline.CutWordSlice();
				url = move(uline.CutToChar('?'));
				if (url.size() == 0 || url[0] != '/')
					return HTTP_STATE::HERROR;
				uline.CutHead(1);
				while (!uline.Over())
				{
					auto k = uline.CutToChar('=');
					uline.CutHead(1);
					auto v = uline.CutToChar('&');
					uline.CutHead(1);
					uvals[k] = move(v);
				}

				buff.scan = scanned = i + 2;
				state = HTTP_POINT::HEADS;
				return HTTP_STATE::CONTINUE;
			}
		}
		scanned = buff.use - 1;
		return HTTP_STATE::NOT_FINISH;
	}
	break;
	case HTTP_POINT::HEADS:
	{
		char* p = buff.buf;
		for (uint32_t i = scanned; i <= buff.use - 4; i++)
		{
			if (p[i] == '\r' && memcmp(p + i, "\r\n\r\n", 4)==0)
			{
				Slice hs(p + buff.scan, p + i);
				while (!hs.Over())
				{
					auto k = hs.CutToChar(':');
					hs.CutHead(2);
					auto v = hs.CutToChar('\r');
					hs.CutHead(2);
					if (k.size() == 0 || v.size() == 0)
						return HTTP_STATE::HERROR;
					heads[k] = v;
				}
				bodySize = atoi(GetHead("Content-Length").c_str());
				buff.scan = scanned = i + 4;
				state = HTTP_POINT::BODY;
				return HTTP_STATE::CONTINUE;
			}
		}
		scanned = buff.use - 4;
		return HTTP_STATE::NOT_FINISH;
	}
	break;
	case HTTP_POINT::BODY:
	{
		if (bodySize == 0)
		{
			state = HTTP_POINT::END;
			return HTTP_STATE::FINISH;
		}
		else if (bodySize > 0)
		{
			if (buff.use - scanned < bodySize)
				return HTTP_STATE::NOT_FINISH;
			body.assign(buff.buf + scanned, bodySize);
			scanned += bodySize;
			state = HTTP_POINT::END;
			return HTTP_STATE::FINISH;
		}
	}
	break;
	case HTTP_POINT::END:
		return HTTP_STATE::FINISH;
	}
	return HTTP_STATE::HERROR;
}

void HttpResponse::Init()
{

}

void HttpResponse::Clear()
{
	heads.clear();
	body.clear();
	buff.use = 0;
}

void HttpResponse::Encode(NetBuffer& nbuff)
{
	stringstream hline;
	hline << HTTP_VERSION << " " << state << " " << statusInfo << "\r\n";
	nbuff.append(hline.str());
	for (auto& it:heads)
		nbuff.append(it.first).append(": ").append(it.second).append("\r\n");

	nbuff.append("Connection: Keep-Alive\r\n");
	nbuff.append("Content-Length: ").append(loop_cast<string>(buff.use)).append("\r\n\r\n");
	nbuff.append(buff.buf,buff.use);
}

void HttpResponse::EncodePHP(NetBuffer& nbuff)
{
	stringstream hline;
	hline << HTTP_VERSION << " " << state << " " << statusInfo << "\r\n";
	nbuff.append(hline.str());
	for (auto& it : heads)
		nbuff.append(it.first).append(": ").append(it.second).append("\r\n");

	nbuff.append("Connection: Keep-Alive\r\n");

	uint32_t len=0;
	for (uint32_t i = 0; i <= buff.use-4; i++)
	{
		if (buff.buf[i] == '\r' && memcmp(buff.buf + i, "\r\n\r\n", 4) == 0)
		{
			len = buff.use - i - 4;
			break;
		}
	}

	nbuff.append("Content-Length: ").append(loop_cast<string>(len)).append("\r\n");
	nbuff.append(buff.buf, buff.use);
}

void FileCash::init(FactorManager * fm)
{
	cashTime = GetMilliSecend();
}

void FileCash::recycle(FactorManager * fm)
{
	buff.Clear();
}

void HttpLogicModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
	m_eventModule = GetLayer()->GetModule<EventModule>();
	m_netObjModule = GetLayer()->GetModule<NetObjectModule>();
	m_scheduleModule = GetLayer()->GetModule<ScheduleModule>();
	m_httpCgiModule = GetLayer()->GetModule<HttpCgiModule>();

	m_msgModule->AddMsgCallBack(N_RECV_HTTP_MSG, this, &HttpLogicModule::OnRecvHttpMsg);
	
	m_eventModule->AddEventCallBack(E_SOCKEK_CONNECT, this, &HttpLogicModule::OnHttpClientConnect);
	m_eventModule->AddEventCallBack(E_CLIENT_HTTP_CLOSE, this, &HttpLogicModule::OnHttpClientClose);

	m_scheduleModule->AddTimePointTask(this, &HttpLogicModule::CheckCash,-1,1);

	InitPath();

	m_defaultCall[GET] = AnyFuncBind::Bind(&HttpLogicModule::OnGetFileResouce, this);

	m_fileType["html"] = "text/html";
	m_fileType["htm"] = "text/htm";
	m_fileType["css"] = "text/css";
	m_fileType["gif"] = "image/gif";
	m_fileType["jpg"] = "image/jpeg";
	m_fileType["jpeg"] = "image/jpeg";
	m_fileType["png"] = "image/png";
	m_fileType["js"] = "application/x-javascript";

	m_index = {"/index.html","/index.htm","/index.php"};

	m_lastCheck = m_cashIndex = 0;
	m_useCash = false;
}

void HttpLogicModule::Execute()
{
}

void HttpLogicModule::SetWebRoot(const string & root)
{
	m_webRoot.append(root);
	//���Ŀ¼�Ƿ����

}

void HttpLogicModule::OnHttpClientConnect(const int & sock)
{
	auto obj = GetLayer()->GetSharedLoop<HttpMsg>();
	obj->socket = sock;
	m_cliens[sock] = obj;
	m_eventModule->SendEvent(E_CLIENT_HTTP_CONNECT, sock);
}

void HttpLogicModule::OnHttpClientClose(const int & sock)
{
	m_cliens.erase(sock);
}

void HttpLogicModule::OnRecvHttpMsg(NetMsg * msg)
{
	auto it = m_cliens.find(msg->socket);
	if (it == m_cliens.end())
		return;

	auto buff = msg->m_buff;
	while(buff){
		it->second->request.RecvBuff(buff->m_buff, buff->m_size);
		buff = buff->m_next;	
	}
	auto st = it->second->request.Decode();
	
	if (st == HTTP_STATE::FINISH)
		OnRequest(it->second.get());
	else if (st == HTTP_STATE::HERROR)
		CloseClient(msg->socket);
	else if (st == HTTP_STATE::CONTINUE_100)
	{

	}
}

void HttpLogicModule::OnRequest(HttpMsg* msg)
{
	if (m_reqCheck && !m_reqCheck->operator()(msg))
		return DoResPonse(msg);

	msg->opration = HttpMsg::SEND;
	for (auto& reg:m_httpCall)
	{
		auto it = reg.second.find(msg->request.math);
		if (it != reg.second.end())
		{
			smatch res;
			if (regex_match(msg->request.url, res, reg.first))
			{
				it->second(msg);
				return DoResPonse(msg);
			}
		}
	}

	auto it = m_defaultCall.find(msg->request.math);
	if (it != m_defaultCall.end())
		it->second(msg);
	else
		msg->opration = HttpMsg::CLOSE;
	DoResPonse(msg);
}

void HttpLogicModule::DoResPonse(HttpMsg * msg)
{
	switch (msg->opration)
	{
	case HttpMsg::SEND_AND_CLOSE:
	{
		NetBuffer buff;
		msg->response.Encode(buff);
		m_netObjModule->SendHttpMsg(msg->socket, buff);
		msg->Clear();

		CloseClient(msg->socket);
	}
	break;
	case HttpMsg::SEND:
	{
		NetBuffer buff;
		msg->response.Encode(buff);
		m_netObjModule->SendHttpMsg(msg->socket, buff);
		msg->Clear();
	}
	break;
	case HttpMsg::NONE:
	{
		msg->Clear();
	}
	break;
	default:
		CloseClient(msg->socket);
		break;
	}
}

void HttpLogicModule::CloseClient(const int& sock)
{
	m_cliens.erase(sock);
	m_netObjModule->CloseNetObject(sock);
}

void HttpLogicModule::OnGetFileResouce(HttpMsg * msg)
{
	if (msg->request.url.size()==1)
	{
		for (auto& f:m_index)
		{
			if (SendFile(msg, f))
				return;
		}
	}
	else
		SendFile(msg, msg->request.url);
}

bool HttpLogicModule::SendFile(HttpMsg* msg, const string& file)
{
	auto pos = file.find_last_of('.');
	auto ftype = file.substr(pos == string::npos ? pos : ++pos);
	if (!ftype.empty()) // == "php"
	{//��php
		HeadData header;
		header["REQUEST_METHOD"] = GET;
		header["SCRIPT_FILENAME"] = m_webRoot + file;
		header["DOCUMENT_ROOT"] = m_webRoot;
		m_httpCgiModule->Request(msg->socket, header);
		msg->opration = HttpMsg::NONE;
		return true;
	}

	if (m_useCash && GetFromCash(file, msg->response.buff))
	{
		msg->response.SetStatus(200, "OK");
		msg->response.heads["Content-Type"] = GetContentType(ftype);
		return true;
	}
	auto path = m_webRoot + file;
	if (LoopFile::ExistFile(path))
	{
		if (LoopFile::GetContent(path, msg->response.buff) == 0)
		{
			if (m_useCash)
				AddCashFile(file, msg->response.buff);
			msg->response.SetStatus(200, "OK");
			msg->response.heads["Content-Type"] = GetContentType(ftype);
			return true;
		}
	}
	msg->response.SetStatus(404, "Not Find");
	return false;
}

string HttpLogicModule::GetContentType(const string& ext)
{
	auto it = m_fileType.find(ext);
	if (it == m_fileType.end())
		return "text/html";
	return it->second;
}

void HttpLogicModule::OnGetPHPContent(const int& sock, NetBuffer& content)
{
	auto it = m_cliens.find(sock);
	if (it == m_cliens.end())
		return;

	it->second->response.buff = move(content);
	it->second->response.SetStatus(200, "OK");

	NetBuffer buff;
	it->second->response.EncodePHP(buff);
	m_netObjModule->SendHttpMsg(sock, buff);
	it->second->Clear();
}

void HttpLogicModule::InitPath()
{
	auto tmp = m_webRoot;
	LoopFile::GetRootPath(m_webRoot);
	m_webRoot.append(tmp);
}

bool HttpLogicModule::GetFromCash(const string& file, NetBuffer& buff)
{
	auto it = m_fileCash.find(file);
	if (it == m_fileCash.end())
		return false;

	it->second->cashTime += CHECK_TIME;
	buff.append(it->second->buff.buf, it->second->buff.len);
	return true;
}

void HttpLogicModule::AddCashFile(const string& file, NetBuffer& buff)
{
	auto cash = GetLayer()->GetSharedLoop<FileCash>();
	cash->cashTime = GetMilliSecend() + CHECK_TIME;
	cash->file = file;
	cash->buff.append(buff.buf,buff.use);
	m_fileCash[file] = cash;

	auto idx = m_cashIndex + 1 == CASH_SIZE ? 0 : m_cashIndex + 1;
	m_cashCheck[idx].push_back(cash);
}

void HttpLogicModule::CheckCash(int64_t& nt)
{
	if (nt < m_lastCheck)
		return;

	m_lastCheck = nt + CHECK_TIME;
	m_cashIndex = m_cashIndex + 1 == CASH_SIZE ? 0 : m_cashIndex + 1;

	if (m_cashCheck[m_cashIndex].size() == 0)
		return;

	auto tmp = move(m_cashCheck[m_cashIndex]);
	for (auto& f:tmp)
	{
		if (nt >= f->cashTime)
			m_fileCash.erase(f->file);
		{
			auto step = (f->cashTime - nt) / CHECK_TIME + 1;
			auto nidx = (m_cashIndex + step) % CASH_SIZE;
			if (nidx > m_cashIndex && step > CASH_SIZE)
				nidx = m_cashIndex;
			m_cashCheck[m_cashIndex].push_back(f);
		}
	}
}

void HttpLogicModule::SendHttpMsg(const int& sock, HttpCall && call)
{
	auto it = m_cliens.find(sock);
	if (it == m_cliens.end())
		return;
	call(it->second.get());
	DoResPonse(it->second.get());
}