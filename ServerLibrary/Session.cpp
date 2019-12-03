#include "stdafx.h"

namespace ServerLibrary
{
	void Session::Init(const SOCKET listenSocket, const int index, const SessionConfig* config, ILog* log)
	{
		Log = log;
		ListenSocket = listenSocket;
		Index = index;
		RecvBufSize		= config->MaxRecvBufferSize;
		SendBufSize		= config->MaxSendBufferSize;
		MaxPacketSize	= config->MaxPacketSize;

		RingRecvBuffer.Create(RecvBufSize);
		RingSendBuffer.Create(SendBufSize);
		Init();

		RecvOverlappedEx = new OverlappedEx(Index);
		SendOverlappedEx = new OverlappedEx(Index);

		ConnectionMsg.Type = MessageType::CONNECTION;
		ConnectionMsg.Contents = nullptr;
		CloseMsg.Type = MessageType::CLOSE;
		CloseMsg.Contents = nullptr;

		AcceptExSocket();
	}

	void Session::Init()
	{
		memset(Ip, 0, kMaxIpLength);

		RingRecvBuffer.Init();
		RingSendBuffer.Init();

		Connected = FALSE;
		Sendable = TRUE;

		AcceptIoCount = 0;
		RecvIoCount = 0;
		SendIoCount = 0;
	}

	void Session::SetLog(ILog* log)
	{
		Log = log;
	}

	Result Session::AcceptExSocket()
	{
		memset(&RecvOverlappedEx->Overlapped, 0, sizeof OVERLAPPED);

		RecvOverlappedEx->Wsabuf.buf = AddrBuf;
		RecvOverlappedEx->Wsabuf.len = RecvBufSize;
		RecvOverlappedEx->Mode = IoMode::ACCEPT;
		RecvOverlappedEx->SessionIndex = Index;

		ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (ClientSocket == INVALID_SOCKET)
		{
			Log->Write(LogType::L_ERROR, "%s | WSASocket() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASOCKET;
		}
		IncrementAcceptIoCount();
		DWORD bytes = 0;
		auto result = AcceptEx(
			ListenSocket,
			ClientSocket,
			RecvOverlappedEx->Wsabuf.buf,
			0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&bytes,
			reinterpret_cast<OVERLAPPED*>(RecvOverlappedEx)
		);

		if (!result && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementAcceptIoCount();
			Log->Write(LogType::L_ERROR, "%s | AcceptEx() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_ACCEPTEX;
		}
		return Result::SUCCESS;
	}

	bool Session::CloseCompletely()
	{
		if (Connected && AcceptIoCount == 0 && RecvIoCount == 0 && SendIoCount == 0)
		{
			Disconnect();
			return true;
		}

		// 카운트가 남았다면 소켓은 끊고 iocp에서 완료를 기다린다
		else if (Connected && (AcceptIoCount != 0 || RecvIoCount != 0 || SendIoCount != 0))
		{
			Disconnect();
			return false;
		}

		// 한 번만 접속 종료 처리 하기 위함
		else if (Connected == FALSE && AcceptIoCount == 0 && RecvIoCount == 0 && SendIoCount == 0)
		{
			return true;
		}
		return false;
	}

	void Session::Disconnect(bool forced)
	{
		SetStateDisconnected();
		LockGuard lock(Cs);
		if (ClientSocket != INVALID_SOCKET)
		{
			if (forced == true)
			{
				LINGER linger = { 0 };
				linger.l_onoff = 1;
				setsockopt(ClientSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof LINGER);
			}
			closesocket(ClientSocket);
		}
	}

	Result Session::ResetSession()
	{
		LockGuard lock(Cs);

		RecvOverlappedEx->Remain = 0;
		RecvOverlappedEx->TotalBytes = 0;
		SendOverlappedEx->Remain = 0;
		SendOverlappedEx->TotalBytes = 0;
		Init();
		return AcceptExSocket();
	}

	bool Session::BindIocp(const HANDLE WorkerIocp)
	{
		LockGuard lock(Cs);
		auto iocp = CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(ClientSocket),
			WorkerIocp,
			reinterpret_cast<ULONG_PTR>(this),
			0
		);
		if (iocp == INVALID_HANDLE_VALUE || iocp != WorkerIocp)
		{
			Log->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	Result Session::PostRecv(const int forwardLength, const DWORD remain)
	{
		assert(Connected == TRUE && RecvOverlappedEx != nullptr);

		RecvOverlappedEx->Mode = IoMode::RECV;
		RecvOverlappedEx->Remain = remain;

		RecvOverlappedEx->Wsabuf.len = MaxPacketSize;
		RecvOverlappedEx->Wsabuf.buf = RingRecvBuffer.ForwardRecvPos(forwardLength);
		assert(RecvOverlappedEx->Wsabuf.buf != nullptr);

		memset(&RecvOverlappedEx->Overlapped, 0, sizeof WSAOVERLAPPED);
		IncrementRecvIoCount();

		DWORD flag = 0;
		DWORD recvBytes = 0;
		auto result = WSARecv(
			ClientSocket,
			&RecvOverlappedEx->Wsabuf,
			1,
			&recvBytes,
			&flag,
			&RecvOverlappedEx->Overlapped,
			nullptr
		);

		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementRecvIoCount();
			Log->Write(LogType::L_ERROR, "%s | WSARecv() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::POSTRECV_NULL_SOCKET_ERROR;
		}
		else
		{
			// Main::Run()에서 sleep을 걸어도 패킷을 제대로 처리 못하는 상황이 생기는데
			// 이 부분에서 Log->Write 함수를 호출하거나 sleep을 걸면 오류가 안 생긴다.
			// 멀티스레드 환경이기 때문에 순서상의 어떤 문제가 있는 것으로 생각됨
			// 아직까지는 최선의 해결책
			this_thread::sleep_for(chrono::milliseconds(1));
			//Log->Write(LogType::L_WARN, "%s | WSARecv() completed", __FUNCTION__, WSAGetLastError());
		}
		return Result::SUCCESS;
	}

	bool Session::PostSend(const int sendSize, char* buffer)
	{
		if (InterlockedCompareExchange(reinterpret_cast<long*>(&Sendable), FALSE, TRUE))
		{
			if (buffer == nullptr)
			{
				InterlockedExchange(reinterpret_cast<long*>(&Sendable), TRUE);
				Log->Write(LogType::L_ERROR, "%s | RingSendBuffer.GetBuffer() failure", __FUNCTION__);
				return false;
			}

			memset(&SendOverlappedEx->Overlapped, 0, sizeof WSAOVERLAPPED);

			SendOverlappedEx->Wsabuf.buf = buffer;
			SendOverlappedEx->Wsabuf.len = sendSize;
			SendOverlappedEx->SessionIndex = Index;

			SendOverlappedEx->Remain = 0;
			SendOverlappedEx->TotalBytes = sendSize;
			SendOverlappedEx->Mode = IoMode::SEND;

			IncrementSendIoCount();

			DWORD sendBytes = 0;
			auto result = WSASend(
				ClientSocket,
				&SendOverlappedEx->Wsabuf,
				1,
				&sendBytes,
				0,
				&SendOverlappedEx->Overlapped,
				nullptr
			);

			if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				DecrementSendIoCount();
				Log->Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				return false;
			}
			else
			{
				// Main::Run()에서는 어디에 sleep을 걸어도 패킷을 제대로 처리 못하는 상황이 생기는데
				// 이 부분에서 Log->Write 함수를 호출하거나 sleep을 걸면 오류가 안 생긴다.
				// 멀티스레드 환경이기 때문에 순서상의 어떤 문제가 있는 것으로 생각됨
				this_thread::sleep_for(chrono::milliseconds(1));
				//Log->Write(LogType::L_WARN, "%s | WSASend() completed", __FUNCTION__);
			}
		}
		return true;
	}

	Result Session::ReserveSendPacketBuffer(OUT char** buf, const int size)
	{
		if (!Connected)
		{
			*buf = nullptr;			
			return Result::RESERVED_BUFFER_NOT_CONNECTED;
		}

		*buf = RingSendBuffer.ForwardSendPos(size);
		if (*buf == nullptr)
		{			
			return Result::RESERVED_BUFFER_EMPTY;
		}
		return Result::SUCCESS;
	}

	bool Session::SetAddressInfo()
	{
		SOCKADDR* localAddr		= nullptr;
		SOCKADDR* remoteAddr	= nullptr;
		int localAddrLen = 0;
		int remoteAddrLen = 0;

		GetAcceptExSockaddrs(
			AddrBuf, 0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&localAddr, &localAddrLen,
			&remoteAddr, &remoteAddrLen
		);

		if (remoteAddrLen != 0)
		{
			SOCKADDR_IN* remoteAddrIn = reinterpret_cast<SOCKADDR_IN*>(remoteAddr);
			if (remoteAddrIn != nullptr)
			{
				char ip[kMaxIpLength] = { 0 };
				inet_ntop(AF_INET, &remoteAddrIn->sin_addr, ip, kMaxIpLength);
				SetIp(ip);
			}
			return true;
		}
		return false;
	}
}
