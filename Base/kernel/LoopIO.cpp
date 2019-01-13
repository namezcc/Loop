#include "LoopIO.h"

void IOCP::SendOVSData(void * ovs)
{
	m_sendData[m_sendIndex].write((OVS*)ovs);
	++m_sendIndex;
	if (m_sendIndex >= IO_SEND_THREAD_NUM)
		m_sendIndex = 0;
}

void IOCP::DoRecv(const uint32_t & tidx)
{
	OVERLAPPED_ENTRY* _entry = &m_entry[LP_ENTRY_SIZE*tidx];
	while (true)
	{
		int bytes = 0;
		ULONG_PTR key = 0;
		int getent = 0;
		int ret = GetQueuedCompletionStatusEx(m_iocp, _entry, LP_ENTRY_SIZE, (PULONG)&getent, INFINITE, 0);
		//int ret = GetQueuedCompletionStatus(m_iocp, (LPDWORD)&bytes, (PULONG_PTR)&key, (LPOVERLAPPED*)&ov, INFINITE);
		if (!ret)
		{
			ret = WSAGetLastError();
			printf("GetQueuedCompletionStatus error %d", ret);
		}

		for (size_t i = 0; i < getent; i++)
		{
			OVR* ovr = (OVR*)_entry[i].lpOverlapped;
			bytes = _entry[i].dwNumberOfBytesTransferred;
			ovr->_err = ret;
			ovr->_recv_size = bytes;
			((UdpSocket*)ovr->_socket)->m_ctx->PushDownRecv(tidx, UdpSocket::Do_recv, ovr);
		}
	}
}

void IOCP::DoSend(const uint32_t & tidx)
{
	OVS* sdata = NULL;
	UdpSocket* usock = NULL;
	int ret = 0;
	while (true)
	{
		while (m_sendData[tidx].pop(sdata))
		{
			usock = (UdpSocket*)sdata->_socket;
			ret = ::sendto(usock->m_socket, sdata->_buf, sdata->_send_size, 0, (sockaddr*)&sdata->_addr._addr, sizeof(sdata->_addr._addr));
			usock->m_ctx->PushDownSend(tidx, UdpSocket::Do_send, sdata);
			if (ret == SOCKET_ERROR)
			{
				ret = WSAGetLastError();
				if (ret != ERROR_IO_PENDING)
					printf("SendTo Error %d", ret);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

UdpSocket::UdpSocket(const LoopUDPIO & ctx):m_ctx(const_cast<LoopUDPIO*>(&ctx)), m_wsaflag(0)
{
	m_insiocp = Single::GetInstence<IOCP>();
	m_ovrpool = Single::LocalInstance<LoopFactor<OVR>>();
	m_ovspool = Single::LocalInstance<LoopFactor<OVS>>();
	m_addr_size = sizeof(sockaddr_in);

	m_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(m_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
}

int UdpSocket::Bind(const int32_t & port)
{
	sockaddr_in local = { 0 };
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	int ret = ::bind(m_socket, (sockaddr*)&local, sizeof(local));
	if (ret == SOCKET_ERROR)
		assert(0);

	auto rhand = CreateIoCompletionPort((HANDLE)m_socket, m_ctx->GetIocp(), 1, 0);

	return ret;
}
