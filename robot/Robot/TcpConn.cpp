#include "TcpConn.h"

bool TcpConn::Connect(const std::string & ip, const int32_t & port)
{
	init();
	m_ip = ip;
	m_port = port;
	return RealConnect();
}

bool TcpConn::ReConnect()
{
	init();
	return RealConnect();
}

void TcpConn::SendPackData(const int32_t& mid,const char * buff, const int32_t & size)
{
	auto buf = GET_SHARE(NetBuffer);

	char encode[MsgHead::HEAD_SIZE];
	MsgHead::Encode(encode, mid, size);
	buf->combin(encode, MsgHead::HEAD_SIZE);
	buf->combin(const_cast<char*>(buff), size);
	
	m_socket.send(as::buffer(buf->buf, buf->use));

	/*auto len = m_socket.receive(as::buffer(m_cashBuff, 4096));
	if (len > 0)
		++len;*/

	/*auto self = shared_from_this();
	as::post(m_context, [this,buf, self]() {
		bool isempty = m_writeBuff.empty();
		m_writeBuff.push_back(buf);
		if (isempty)
			DoWrite();
	});*/
}

void TcpConn::Close()
{
	auto self(shared_from_this());
	as::post(m_context, [this]() {
		DoClose();
	});
}

void TcpConn::init()
{
	m_recvBuff.Clear();
}

bool TcpConn::RealConnect()
{
	auto addr = tcp::endpoint(as::ip::address_v4::from_string(m_ip), m_port);
	//auto res = as::connect(m_socket, addr);
	as::error_code ec;
	m_socket.connect(addr, ec);
	if (!ec)
	{
		//发送 23 个字节
		/*char buf[23] = { 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2 };
		m_socket.send(as::buffer(buf,23));*/
		DoRecv();
	}
	else
		std::cout << ec.message() << std::endl;
	return !ec;
}

void TcpConn::DoRecv()
{
	auto self(shared_from_this());

	as::async_read(m_socket, as::buffer(m_cashBuff, MsgHead::HEAD_SIZE),
		[this, self](as::error_code ec, std::size_t recvsize) {
		if (!ec && recvsize == MsgHead::HEAD_SIZE)
		{
			if (DoReadHead())
			{
				return;
			}
		}
		DoClose();
	});
}

void TcpConn::DoWrite()
{
	auto self(shared_from_this());
	as::async_write(m_socket, as::buffer(m_writeBuff.front()->buf, m_writeBuff.front()->use), 
	[this, self](as::error_code ec,std::size_t) {
		if (!ec)
		{
			if(!m_writeBuff.empty())
				m_writeBuff.pop_front();

			if (!m_writeBuff.empty())
				DoWrite();
		}else
		{
			DoClose();
		}
	});
}

bool TcpConn::DoReadHead()
{
	if (!m_msghead.Decode(m_cashBuff))
		return false;
	
	/*if (m_msghead.mid < 5000 || m_msghead.mid >= 10000)
	{
		std::cout << "error msg mid " << m_msghead.mid << std::endl;
		return false;
	}*/

	if (m_msghead.size < 0 || m_msghead.size > 1024 * 1024 * 5)
	{
		std::cout << "error msg size = " << m_msghead.size << std::endl;
		return false;
	}

	auto self(shared_from_this());
	auto bodysize = m_msghead.size - MsgHead::HEAD_SIZE;
	m_recvBuff.MakeRoome(bodysize);
	as::async_read(m_socket, as::buffer(m_recvBuff.buf, bodysize),
		[this, self](as::error_code ec, std::size_t recvsize) {
		if (!ec)
		{
			if (DoReadBody())
			{
				DoRecv();
				return;
			}
		}
		DoClose();
	});

	return true;
}

bool TcpConn::DoReadBody()
{
	if(m_onRead)
		m_onRead(m_msghead.mid, m_msghead.size - MsgHead::HEAD_SIZE, m_recvBuff.buf);
	return true;
}

void TcpConn::DoClose()
{
	if (m_socket.is_open())
		m_socket.close();
	else
		return;
	m_writeBuff.clear();
	if (m_onClose)
		m_onClose();
}
