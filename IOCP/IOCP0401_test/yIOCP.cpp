#include "yIOCP.h"

//#define COUT_DEBUG

yIOCP* instance = NULL;

std::threadpool pool{ 32 };

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
	size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
	char* bytes = new char[size];
	snprintf(bytes, size, format.c_str(), args ...);
	std::string res = std::string(bytes);
	delete[] bytes;
	return res;
}

void local_time(std::string& time)
{
	tm _localTime;
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	localtime_r(&in_time_t, &_localTime);
	//if(_localTime.tm_mon + 1 < 10)
	time = std::to_string(_localTime.tm_year + 1900) + '-'
		+ string_format(std::to_string(_localTime.tm_mon + 1), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_mday), "%02d") + ' '
		+ string_format(std::to_string(_localTime.tm_hour), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_min), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_sec), "%02d");
}

/*
* ��ڲ�����
*		num_of_boards��				��������
*		channel_map_path��			ͨ��ӳ���(δ��)
*		save_path��					�����ļ�·��
*		save_file_time��			ÿ���ļ��������ݵ�ʱ�䳤��
*		sample_rate��				������
*		real_num_of_boards��		ʵ�ʵİ�������(��num_of_boards��������п��ܲ���11�����ӣ����Ǵ���������11�����ӵ���������Ҫ�������ӵİ��ӵ��������浽�������)
* ˵����
*/
yIOCP::yIOCP(uint32_t num_of_boards, char channel_map_path[], const char save_path[], uint32_t save_file_time, uint32_t sample_rate, uint32_t real_num_of_boards)
	:_num_of_boards(num_of_boards), _save_file_time(save_file_time), _sample_rate_to_FPGA(sample_rate), _real_num_of_boards(real_num_of_boards)
{
	_per_sample_data_lenth = num_of_boards * BYTE_OF_PER_BOARD;							// ÿ��ʱ�̲ɼ������ݵ�������Ҳ����plÿ��ʱ�̵������� 11*(16+2)*4 (�ֽ�)
	
	// Ϊ�˲��� 0.1s ���һ��ͬ���룬���� 5000 ���һ��ͬ�����жϲ����жϣ����ÿ 4096 �ж�һ�Σ���ͬ�����Ƿ���ͬ
	//_save_file_nums = 4096;																// Ҫ�����ļ��������� 4096
	//_save_file_Bytes = _save_file_nums * _per_sample_data_lenth;						// Ҫ�����ļ����ֽ��� 10s*50k*11*(16+2)*4 (�ֽ�)
	
	_save_file_nums = save_file_time * sample_rate;										// Ҫ�����ļ��������� 10s*50k (��)
	_save_file_Bytes = _save_file_nums * _per_sample_data_lenth;						// Ҫ�����ļ����ֽ��� 10s*50k*11*(16+2)*4 (�ֽ�)
	_save_file_channel = num_of_boards * 18;											// �����ļ���ͨ���� 11*18 (��)
	save_dir_path = save_path;

	m_IOCompletionPort = NULL;															// ��ɶ˿� ȫ��һ��
	ServerIP = "192.168.118.177";
	ServerPort = 8081;
	//ServerIP = "127.0.0.1";
	//ServerPort = 9000;
	m_pListenContext = new PER_SOCKET_CONTEXT;											// Ψһ�ļ��� PER_SOCKET_CONTEXT 

	/*ListenThread = new std::future<bool>;*/

	lpfnAcceptEx = nullptr;
	GuidAcceptEx = WSAID_ACCEPTEX;
	lpfnGetAcceptExSockaddrs = nullptr;
	GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	lpfnDisconnectEx = nullptr;
	GuidDisconnectEx = WSAID_DISCONNECTEX;

	m_RingBuffer = new RingBuffer(RINGBUFFER_LENGTH, _per_sample_data_lenth);			// һά�Ļ��λ����� | RINGBUFFER_LENGTH�����λ��������� | _per_sample_data_lenth�����ڽ綨ÿһ��ʱ�����ݳ���

	wsabuff1 = new char[_per_sample_data_lenth * PER_RECV_TIME_COUNT];					// ѭ��Ͷ�ݵ�wsabuf��Ϊ���첽
	wsabuff2 = new char[_per_sample_data_lenth * PER_RECV_TIME_COUNT];					// ��������鳤��Ҫ�� MAX_BUFFER_LEN ���ȶ�Ӧ����Ϊwsabufÿ��Ͷ�ݵ�ʱ�򶼻�ResetBuffer()

	num_of_receptions = 0;																// ���ڴ��ļ�ʱ����dwBytesTransfered���ۼ�ֵ

	start_store_data_flag = false;														// ���ļ��ı�־
	create_file_flag = false;															// WASBUFtrans2Ringbuffer�н����������ﵽ���ļ��������� notify_all 
	is_stopSaveBuff = true;																// ���ļ��߳����еı�־

	saveFile_buffing = nullptr;															// ���ڴ��ļ�
	saveFile_buffed = nullptr;
	saveFile_buff_reserve1 = new char[_save_file_Bytes];
	saveFile_buff_reserve2 = new char[_save_file_Bytes];
	saveFile_buffing = saveFile_buff_reserve1;
	saveFile_buffed = saveFile_buff_reserve2;

	workerThread_stop = false;															// ���� WorkerThread �̵߳�����ֹͣ
	is_stopSaveBuff_thread = false;														// ���� save_buff �̵߳�����ֹͣ

	selfcheck_state = 0x0;																// �ṩ��qt���Լ�״̬
}

/*
* ˵����
* ���������������uninitialize()��qt����
*/
yIOCP::~yIOCP()
{
	std::cout << "~yIOCP()" << std::endl;

	start_store_data_flag = false;													
	create_file_flag = false;															
	is_stopSaveBuff = true;
	workerThread_stop = true;
	is_stopSaveBuff_thread = true;

	if (m_pListenContext != NULL && m_pListenContext->m_Socket != INVALID_SOCKET)
	{
		PostQueuedCompletionStatus(m_IOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		_ClearContextList();
	}

	delete m_RingBuffer;
	delete[] wsabuff1;
	delete[] wsabuff2;
	delete[] saveFile_buff_reserve1;
	delete[] saveFile_buff_reserve2;
}

/*
* ���ڲ�����
*		bool						����initIOCP״̬
* ˵����
* Ͷ����һ��accept����
*/
bool yIOCP::initIOCP()
{
	m_IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (m_IOCompletionPort == nullptr)
	{
		std::cout << "CreateIoCompletionPort error" << std::endl;
		return false;
	}

	// �̳߳ش��� ����
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);
	//int m_nProcessors = si.dwNumberOfProcessors;
	//pool = std::threadpool{m_nProcessors};

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	sockaddr_in ServerAddress;

	m_pListenContext->m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_pListenContext->m_Socket == NULL)
	{
		std::cout << "WSASocket error" << std::endl;
		return false;
	}

	// �����ڶ������� CompletionKey ��Ҫע�⣬������GetQueuedCompletionStatus�л�ȡ���ֵ
	CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_IOCompletionPort, (ULONG_PTR)m_pListenContext, 0);

	ZeroMemory((char*)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	inet_pton(AF_INET, ServerIP.c_str(), &ServerAddress.sin_addr.s_addr);//���ص�ַ
	ServerAddress.sin_port = htons(ServerPort);

	if (SOCKET_ERROR == bind(m_pListenContext->m_Socket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)))
	{
		std::cout << "bindʧ��: " << WSAGetLastError() << std::endl;
		return false;
	}


	if (listen(m_pListenContext->m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listenʧ��" << std::endl;
		return false;
	}

	std::cout << "TCP server start... ... ..." << std::endl;

	DWORD dwBytes;
	int iResult = WSAIoctl(
		m_pListenContext->m_Socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx),
		&lpfnAcceptEx, 
		sizeof(lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL);
	iResult = WSAIoctl(
		m_pListenContext->m_Socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockaddrs, 
		sizeof(GuidGetAcceptExSockaddrs),
		&lpfnGetAcceptExSockaddrs, 
		sizeof(lpfnGetAcceptExSockaddrs), 
		&dwBytes, 
		NULL,
		NULL);
	//iResult = WSAIoctl(m_pListenContext->m_Socket, 
	//	SIO_GET_EXTENSION_FUNCTION_POINTER, 
	//	&GuidDisconnectEx, 
	//	sizeof(GuidDisconnectEx),
	//	&lpfnDisconnectEx, 
	//	sizeof(lpfnDisconnectEx), 
	//	&dwBytes, 
	//	NULL,
	//	NULL);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "��ȡAcceptEx����ָ��ʧ��" << std::endl;
		return false;
	}

	// Ͷ��һ�� accept 
	for (int i = 0; i < 1; i++)
	{
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext(); // m_pListenContext->m_arrayIoContext[1].

		if (false == _postAccept(pAcceptIoContext))
		{
			std::cout << "Ͷ�ݵ�һ��acceptʧ��" << std::endl;
			return false;
		}
	}

	// ��������̣߳�
	//ListenThread = new std::future<bool>[THREAD_NUM];
	//for (int i = 0; i < THREAD_NUM; i++)
	//{
	//	THREADPARAMS_WORKER* ThreadParams = new THREADPARAMS_WORKER;
	//	ThreadParams->IOCPModel = this;
	//	ThreadParams->nThreadNumber = i + 1;
	//	ListenThread[i] = executor.commit(WorkerThread, ThreadParams);
	//}

	// �������������߳�
	THREADPARAMS_WORKER* ThreadParams = new THREADPARAMS_WORKER;
	ThreadParams->IOCPModel = this;
	ThreadParams->nThreadNumber = 1;
	//ListenThread = pool.commit(WorkerThread, ThreadParams);

	std::thread work_thread(&WorkerThread, ThreadParams);
	work_thread.detach();
	std::thread save_thread(&yIOCP::save_buff, this);
	save_thread.detach();
	
	return true;
}

bool yIOCP::_postAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	DWORD dwBytes = 0;
	pAcceptIoContext->m_OpType = IO_ACCEPT;
	WSABUF* p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED* p_ol = &pAcceptIoContext->m_Overlapped;
	pAcceptIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);


	//setsockopt(pAcceptIoContext->m_sockAccept, SOL_SOCKET,)
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		std::cout << "Accept��socket����ʧ��" << std::endl;
		return false;
	}
	//bool flag = lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2),
	//	sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol);

	bool flag = lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol);

	if ((FALSE == flag) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		std::cout << "AcceptExerror: " << WSAGetLastError() << std::endl;
		return false;
	}/*
	std::cout << "AcceptExerror: " << WSAGetLastError() << std::endl;*/
	return true;
}

// rec_type = 2 Ͷ�ݽ����Լ�ظ�������
// rec_type = 3 Ͷ�ݽ����������ݵ�����
bool yIOCP::_postRecv(PER_IO_CONTEXT* pIoContext, int rec_type)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	//pIoContext->ResetBuffer();
	if (rec_type == 3)
	{
		pIoContext->m_OpType = IO_RECV_BEIGNED; 
	}
	else if (rec_type == 2)
	{
		pIoContext->m_OpType = IO_RECV_SELFCHECKED;
	}
	else
	{
		std::cout << "error type of rec_type" << std::endl;
	}

	int nBytesRecv = WSARecv(pIoContext->m_sockAccept, &pIoContext->m_wsaBuf, 1, NULL, &dwFlags, &pIoContext->m_Overlapped, NULL);

	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		std::cout << "WSARecv error: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}


bool yIOCP::_postSend(PER_IO_CONTEXT* pIoContext, char* cmd, unsigned long length)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	pIoContext->m_wsaBuf.buf = cmd;
	pIoContext->m_wsaBuf.len = length;

	int returnFlag = WSASend(pIoContext->m_sockAccept, &pIoContext->m_wsaBuf, 1, NULL, dwFlags, &pIoContext->m_Overlapped, NULL);
	if ((SOCKET_ERROR == returnFlag) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		std::cout << "WSASend error: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}



//bool yIOCP::_postSend(PER_IO_CONTEXT* pIoContext)
//{
//	DWORD dwFlags = 0;
//	DWORD dwBytes = 0;
//	char wasbuf1[] = "1";
//	char wasbuf2[] = "2";
//	switch (_sample_rate_to_FPGA)
//	{
//	case 50000: pIoContext->m_wsaBuf.buf = wasbuf1; pIoContext->m_wsaBuf.len = 1; break;
//	case 100000: pIoContext->m_wsaBuf.buf = wasbuf2; pIoContext->m_wsaBuf.len = 1; break;
//	default: pIoContext->m_wsaBuf.buf = wasbuf1; pIoContext->m_wsaBuf.len = 1; break;
//	}
//
//	//char wasbuf[] = "123";
//	//pIoContext->m_wsaBuf.buf = wasbuf;
//	//pIoContext->m_wsaBuf.len = strlen("123");
//
//	int returnFlag = WSASend(pIoContext->m_sockAccept, &pIoContext->m_wsaBuf, 1, NULL, dwFlags, &pIoContext->m_Overlapped, NULL);
//	if ((SOCKET_ERROR == returnFlag) && (WSA_IO_PENDING != WSAGetLastError()))
//	{
//		std::cout << "WSASend error: " << WSAGetLastError() << std::endl;
//		return false;
//	}
//	return true;
//}

bool yIOCP::_AssociateWithIOCP(PER_SOCKET_CONTEXT* pSocketContext)
{
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pSocketContext->m_Socket, m_IOCompletionPort, (ULONG_PTR)pSocketContext, 0);
	if (NULL == hTemp)
	{
		std::cout << "CreateIoCompletionPort error: " << WSAGetLastError() <<std::endl;
		return false;
	}
	return true;
}

void yIOCP::_AddToContextList(PER_SOCKET_CONTEXT* pSocketContext)
{
	std::unique_lock<std::mutex> lock(mtx);
	m_arrayClientContext .push_back(pSocketContext);
}

void yIOCP::_RemoveContext(PER_SOCKET_CONTEXT* pSocketContext)
{
	for (int i = 0; i < m_arrayClientContext.size(); i++)
	{
		if (pSocketContext == m_arrayClientContext.at(i))
		{
			RELEASE(pSocketContext);
			m_arrayClientContext.erase(m_arrayClientContext.begin() + i);
			break;
		}
	}

}

void yIOCP::_ClearContextList()
{
	for (int i = 0; i < m_arrayClientContext.size(); i++)
	{
		delete m_arrayClientContext.at(i);
	}

	m_arrayClientContext.clear();
}

bool yIOCP::_doAccept(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	//int rec_size = 0;

	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	//lpfnGetAcceptExSockaddrs(pIoContext->m_wsaBuf.buf, 
	//	pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
	//	sizeof(SOCKADDR_IN) + 16, 
	//	sizeof(SOCKADDR_IN) + 16, 
	//	(LPSOCKADDR*)&LocalAddr,
	//	&localLen, 
	//	(LPSOCKADDR*)&ClientAddr, 
	//	&remoteLen);

	lpfnGetAcceptExSockaddrs(pIoContext->m_wsaBuf.buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&LocalAddr,
		&localLen,
		(LPSOCKADDR*)&ClientAddr,
		&remoteLen);

	/*char addrbuf[20] = { 0 };
	inet_ntop(AF_INET, &LocalAddr->sin_addr, addrbuf, sizeof(addrbuf));
	std::cout << "��������ַ�� " << addrbuf << ":" << LocalAddr->sin_port << std::endl;
	inet_ntop(AF_INET, &ClientAddr->sin_addr, addrbuf, sizeof(addrbuf));
	std::cout << "�ͻ��˵�ַ�� " << addrbuf << ":" << ClientAddr->sin_port << std::endl;*/
	
	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;

	pNewSocketContext->m_Socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));

	if (false == this->_AssociateWithIOCP(pNewSocketContext))
	{
		std::cout << "CreateIoCompletionPort error: " << WSAGetLastError() << std::endl;
		return false;
	}

	/* ----------------------------------------------------------------���ӷ���----------------------------------------------------------------------------------- */
	//PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	//pNewIoContext->m_OpType = IO_RECV;
	//pNewIoContext->m_sockAccept = pNewSocketContext->m_Socket;

	//int receiveBufferSize = 1152 * 1152 * 40;
	//DWORD dwBufferLength = sizeof(receiveBufferSize);
	//int result = setsockopt(pNewIoContext->m_sockAccept, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, dwBufferLength);
	//if (result == SOCKET_ERROR) {
	//	return false;
	//}

	//pNewIoContext->m_wsaBuf.buf = wsabuff1;

	//if (false == _postRecv(pNewIoContext))
	//{
	//	pNewSocketContext->RemoveContext(pNewIoContext);
	//	std::cout << "PostRecv fail" << std::endl;
	//	return false;
	//}

	//_AddToContextList(pNewSocketContext);
	/* ----------------------------------------------------------------���ӷ���----------------------------------------------------------------------------------- */

	/* ----------------------------------------------------------------�ӷ���----------------------------------------------------------------------------------- */
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType = IO_SEND_CONNECT;
	pNewIoContext->m_sockAccept = pNewSocketContext->m_Socket;

	int receiveBufferSize = 1024 * 1024 * 256;
	DWORD dwBufferLength = sizeof(receiveBufferSize);
	int result = setsockopt(pNewIoContext->m_sockAccept, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, dwBufferLength);
	if (result == SOCKET_ERROR) {
		return false;
	}

	CMD cmd;
	char cmd_address[10];
	switch (_sample_rate_to_FPGA)
	{
	case 50000: set_cmd_type(CMD_CONNECT_11NODE_50K_SAMPLE_RATE, cmd); break;   // 50k
	case 25000: set_cmd_type(CMD_CONNECT_11NODE_25K_SAMPLE_RATE, cmd); break;   // 25k
	case 100000: set_cmd_type(CMD_CONNECT_11NODE_100K_SAMPLE_RATE, cmd); break;   // 100k �ο�tcp��psЭ��
	default:break;
	}
	memcpy(cmd_address, &cmd, sizeof(CMD));

	if (false == _postSend(pNewIoContext, cmd_address, 10))
	{
		pSocketContext->RemoveContext(pNewIoContext);
		std::cout << "PostRecv fail" << std::endl;
		return false;
	}
	_AddToContextList(pNewSocketContext);
	/* ----------------------------------------------------------------����----------------------------------------------------------------------------------- */

	// ��������̰߳�����򿪣�����Ƕ��tcp���ӿ��Զ��Ͷ��
	//pIoContext->ResetBuffer();
	//return _postAccept(pIoContext);
	//delete[] rec_data;
	return true;
}

bool yIOCP::_doRecv(PER_IO_CONTEXT* pIoContext, DWORD dwBytesTransfered, int rec_type)
{
	//std::cout << dwBytesTransfered << std::endl;
	//return _postRecv(pIoContext);

#ifdef COUT_DEBUG
	static bool isFirsttime = true;

	if (isFirsttime)
	{
		isFirsttime = false;
		int rec_size = 0;
		int sizeof_rec_size = sizeof(rec_size);
		getsockopt(pIoContext->m_sockAccept, SOL_SOCKET, SO_RCVBUF, (char*)&rec_size, &sizeof_rec_size);
		std::cout << "SO_RCVBUF size:" << rec_size << std::endl;
	}
#endif


	static bool isFirstTime = true;
	static bool wasbuff_flag = true;

	static int rec_total_packet = 0;

	bool post_ret = false;

	// ���ļ��õ��ı���
	//start_store_data_flag = true;
	//static bool saveFile_buff_flag = true;
	if (rec_type == 2)
	{
		if (dwBytesTransfered == 10)
		{
			memcpy(&selfcheck_state, pIoContext->m_wsaBuf.buf + 3, dwBytesTransfered - 3);
			/*			selfcheck_state |= 0x80000000; */

			return true;
		}
		selfcheck_state &= 0x7fffffff;
		return false;
		//pIoContext->m_wsaBuf.buf
	}

	if (rec_type == 3)
	{
		if (dwBytesTransfered > MAX_BUFFER_LEN)
		{
			std::cout << "dwBytesTransfered > MAX_BUFFER_LEN" << std::endl;
		}
		//std::cout << dwBytesTransfered << std::endl;
		if (isFirstTime)
		{
			int sendbuf = 0;
			socklen_t opt_len = sizeof(sendbuf);
			getsockopt(pIoContext->m_sockAccept, SOL_SOCKET, SO_RCVBUF, (char*)&sendbuf, &opt_len);
			std::cout << "RCVBUF buffer size: " << sendbuf << std::endl;

			isFirstTime = false;
			wsabuff1_ret = pool.commit(WASBUFtrans2Ringbuffer, this, pIoContext->m_wsaBuf.buf, dwBytesTransfered);

			pIoContext->m_wsaBuf.buf = wsabuff2;
			post_ret = _postRecv(pIoContext, 3);
		}
		else
		{
			if (wasbuff_flag)
			{
				if (wsabuff1_ret.get())
				{
					wsabuff2_ret = pool.commit(WASBUFtrans2Ringbuffer, this, pIoContext->m_wsaBuf.buf, dwBytesTransfered);
					pIoContext->m_wsaBuf.buf = wsabuff1;
				}
				else
				{
					std::cout << "1last WASBUFtrans2Ringbuffer() func fail!" << std::endl;
				}
			}
			else
			{
				if (wsabuff2_ret.get())
				{
					wsabuff1_ret = pool.commit(WASBUFtrans2Ringbuffer, this, pIoContext->m_wsaBuf.buf, dwBytesTransfered);
					pIoContext->m_wsaBuf.buf = wsabuff2;
				}
				else
				{
					std::cout << "2last WASBUFtrans2Ringbuffer() func fail!" << std::endl;
				}
			}
			wasbuff_flag = !wasbuff_flag;
			post_ret = _postRecv(pIoContext, 3);
		}
		if (post_ret)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool WINAPI WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* m_Parm = (THREADPARAMS_WORKER*)lpParam;
	yIOCP* IOCPModel = (yIOCP*)m_Parm->IOCPModel;
	//std::thread::id Thread_ID = std::this_thread::get_id();
	//std::cout << "thread id: " << m_Parm->nThreadNumber << std::endl;

	OVERLAPPED* pOverlapped = nullptr;
	PER_SOCKET_CONTEXT* pSocketContext = nullptr;
	DWORD dwBytesTransfered = 0;

	//static PER_IO_CONTEXT* PER_IO_CONTEXT_address = nullptr;

	while (!IOCPModel->workerThread_stop)
	{

		bool gReturn = GetQueuedCompletionStatus(IOCPModel->m_IOCompletionPort, &dwBytesTransfered,
			(PULONG_PTR)&pSocketContext, &pOverlapped, INFINITE);
		//std::cout << dwBytesTransfered << std::endl;

		if (dwBytesTransfered == -1)
		{
			std::cout << "ERROR:	dwBytesTransfered == -1!" << std::endl;
			break;
		}

		//if (EXIT_CODE == (DWORD)pSocketContext)
		//{
		//	std::cout << " EXIT_CODE == (DWORD)pSocketContext" << std::endl;
		//	break;
		//}
		if (!gReturn)
		{
			std::cout << "GetQueuedCompletionStatus error: " << WSAGetLastError() << std::endl;
			continue;
		}
		else
		{
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);
			//if (PER_IO_CONTEXT_address != pIoContext)
			//{
			//	std::cout << "PER_IO_CONTEXT_address != pIoContext, only print once" << std::endl;
			//}
			//PER_IO_CONTEXT_address = pIoContext;
			
			if ((0 == dwBytesTransfered) && (IO_RECV_SELFCHECKED == pIoContext->m_OpType || IO_RECV_BEIGNED == pIoContext->m_OpType || IO_RECV == pIoContext->m_OpType))
			{
				std::cout << "�пͻ��˶Ͽ�" << std::endl;
				IOCPModel->_RemoveContext(pSocketContext);
				continue;
			}
			else
			{
				switch (pIoContext->m_OpType)
				{
				case IO_RECV_BEIGNED:
				{
					//std::cout << "IO_RECV_BEIGNED" << std::endl;
					std::cout << "dwBytesTransfered : " << dwBytesTransfered << std::endl;
					if (!IOCPModel->_doRecv(pIoContext, dwBytesTransfered, 3))
					{
						std::cout << "_doRecv failed" << std::endl;
						
					}
				}
				break;
				case IO_ACCEPT:
				{
					std::cout << "IO_ACCEPT" << std::endl;
					if (!IOCPModel->_doAccept(pSocketContext, pIoContext))
					{
						std::cout << "_doAccept failed" << std::endl;
					}
				}
				break;
				case IO_RECV_SELFCHECKED:
				{
					std::cout << "IO_RECV_SELFCHECKED" << std::endl;
					if (!IOCPModel->_doRecv(pIoContext, dwBytesTransfered, 2))
					{
						std::cout << "_doRecv2 failed" << std::endl;
					}
				}
				break;
				
				case IO_SEND_CONNECT:
				{
					std::cout << "IO_SEND_CONNECT" << std::endl;
					IOCPModel->_doSend(pIoContext, 1);
				}
				break;
				case IO_SEND_SELFCHECK:
				{
					std::cout << "IO_SEND_SELFCHECK" << std::endl;
					IOCPModel->_doSend(pIoContext, 2);
				}
				break;
				case IO_SEND_BEGIN:
				{
					std::cout << "IO_SEND_BEGIN" << std::endl;
					IOCPModel->_doSend(pIoContext, 3);
				}
				break;
				case IO_DISCONNECT:
				{
					std::cout << "IO_DISCONNECT" << std::endl;
					IOCPModel->_doDisconnect();
				}
				break;
				default:
					std::cout << "�������Ͳ����쳣" << std::endl;
					break;
				}
			}
		}
	}
	return true;
}

void yIOCP::_doDisconnect()
{

}

// send_type = 1 �������������Ĳ���
// send_type = 2 �����Լ������Ĳ���
// send_type = 3 ���Ϳ�ʼ�ɼ������Ĳ���
bool yIOCP::_doSend(PER_IO_CONTEXT* pIoContext, int send_type)
{
#ifdef COUT_DEBUG1
	std::cout << "send successful" << std::endl;
#endif

	if (send_type == 1)
	{
		
	}
	else if (send_type == 2)
	{
		pIoContext->m_wsaBuf.buf = wsabuff1;
		pIoContext->m_wsaBuf.len = MAX_BUFFER_LEN;

		if (false == _postRecv(pIoContext, 2))
		{
			std::cout << "PostRecv fail" << std::endl;
			return false;
		}
	}
	else if (send_type == 3)
	{
		pIoContext->m_wsaBuf.buf = wsabuff1;
		pIoContext->m_wsaBuf.len = MAX_BUFFER_LEN;

		if (false == _postRecv(pIoContext, 3))
		{
			std::cout << "PostRecv fail" << std::endl;
			return false;
		}
	}
	return true;
}


/*
* ���ڲ�����
*		bool						û��ʲôʵ�ʺ���
* ˵����
* save_buff�̣߳��߳�����ֹͣ��is_stopSaveBuff_thread���ƣ�һֱwait��WorkerThread��notify������
*/
bool yIOCP::save_buff() 
{
	//static int save_last_num = -1;
	//static bool debug_flag1 = true;
	//static bool debug_flag2 = true;
	//static int debug_save_file_num1 = 0;
	//static int debug_save_file_num2 = 0;

	//static int per_100ms_check_sync_code_last = 0;

	while (!is_stopSaveBuff_thread)
	{
		std::unique_lock<std::mutex> lck(mtx_saveFile);
		cv_saveFile.wait(lck, [this] {
			return (!is_stopSaveBuff);
			});
		//save_num++;

		//if (per_100ms_check_sync_code_last != (int)saveFile_buffed[3]) 
		//{
		//	std::cout << "sync code discontinue" << std::endl;
		//}

		/*std::cout << "save file sync code 1:2 =  " << (int)saveFile_buffed[3] << ":" << (int)saveFile_buffed[7] << std::endl;*/

		//if ((int)saveFile_buffed[3] != (int)saveFile_buffed[7])
		//{
		//	//if (debug_flag1)
		//	//{
		//	//	debug_flag1 = false;
		//	//}
		//	
		//	if (debug_save_file_num1 > 9)
		//	{
		//		start_store_data_flag = false;
		//	}
		//	else
		//	{
		//		std::cout << "sync code is not same, begin save file............" << std::endl;
		//		start_store_data_flag = true;
		//		debug_save_file_num1++;
		//	}

		//}
		//if (saveFile_buffed[3] == 0)
		//{
		//	if (save_last_num == 0)
		//	{
		//		//if (debug_flag2)
		//		//{
		//		//	debug_flag2 = false;
		//		//}
		//		if (debug_save_file_num2 > 10)
		//		{
		//			start_store_data_flag = false;
		//		}
		//		else
		//		{
		//			std::cout << "sync code is 0 0, begin save file............" << std::endl;
		//			start_store_data_flag = true;
		//			debug_save_file_num2++;
		//		}
		//	}
		//	else
		//	{
		//		save_last_num = 0;
		//	}
		//}
		//else
		//{
		//	save_last_num = -1;
		//}

		if (start_store_data_flag)
		{
			set_file_name();
			
			if (this_file == nullptr)
			{
				std::cout << "create file failed!" << std::endl;
			}

			size_t returnNum;
			returnNum = fwrite(saveFile_buffed, sizeof(uint32_t), _save_file_channel * _save_file_nums, this_file);
			if (returnNum != _save_file_channel * _save_file_nums)
			{
				std::cout << "save file returnNum: " << returnNum << std::endl;
			}

			fclose(this_file);
		}
		is_stopSaveBuff = true;
	}
	return true;
}

/*
* ˵����
* ����ʱ����ȷ���ļ���
*/
void yIOCP::set_file_name()
{
	std::string file_name;
	local_time(file_name);

	file_name = file_name + ".dat";

	file_name = "  " + file_name;

	file_name = save_dir_path + "\\" + file_name;

#ifdef COUT_DEBUG1
	std::cout << file_name << std::endl;
#endif

	this_file = fopen(file_name.c_str(), "wb+");	
}

/*
* ���ڲ�����
*		uint32_t					���ؽ��յ����Լ�״̬
* ˵����
* �Լ�״̬32λ������22λΪ11�����ӵ�A��B��״̬
*/
uint32_t yIOCP::get_selfcheck_state()
{
	return selfcheck_state;
}

/*
* ��ڲ�����
*		iocp��						ȫ��Ψһ��yIOCP�ṹ��
*		buffer��					iocp���յ���wsabuf
*		dwBytesTransfered��			ÿ�ν��������ֽ���
* ���ڲ�����
*		bool						����Ͷ�������״̬
* ˵����
* �ڹ����߳�û���յ�ps���������ݺ��������̳߳�Ͷ��WASBUFtrans2Ringbuffer���񣬸���wsabuf��
* ������Ͷ��WSARecv�������첽�������񣬲��Ͻ������紫��������
*/
bool WINAPI WASBUFtrans2Ringbuffer(yIOCP *iocp, char* buffer, DWORD dwBytesTransfered)
{
	static bool saveFile_buff_flag = true;

	// ���ﱣ���ļ������ݣ��ܹ��� _save_file_Bytes ���������ݾͿ�ʼ���棬Ϊ�˱��ⵥ���ڴ���ļ����������������saveFile_buffing��saveFile_buffed�����ڴ�����
	// saveFile_buffing��ʾ���ڴ������ݵĶ�����saveFile_buffed��ʾ�Ѿ��������ݣ������ļ�ʹ�õĶ���
	if ((iocp->num_of_receptions + dwBytesTransfered) >= iocp->_save_file_Bytes)
	{
		memcpy_safe((iocp->saveFile_buffing + iocp->num_of_receptions), iocp->_save_file_Bytes, buffer, iocp->_save_file_Bytes - iocp->num_of_receptions);
		memcpy_safe(iocp->saveFile_buffed, iocp->_save_file_Bytes, buffer + iocp->_save_file_Bytes - iocp->num_of_receptions, iocp->num_of_receptions + dwBytesTransfered - iocp->_save_file_Bytes);
		//memcpy((iocp->saveFile_buffing + iocp->num_of_receptions), buffer, iocp->_save_file_Bytes - iocp->num_of_receptions);
		//memcpy(iocp->saveFile_buffed, buffer + iocp->_save_file_Bytes - iocp->num_of_receptions, iocp->num_of_receptions + dwBytesTransfered - iocp->_save_file_Bytes);

		iocp->num_of_receptions = iocp->num_of_receptions + dwBytesTransfered - iocp->_save_file_Bytes;
		iocp->create_file_flag = true;
	}
	else
	{
		memcpy_safe((iocp->saveFile_buffing + iocp->num_of_receptions), iocp->_save_file_Bytes, buffer, dwBytesTransfered);
		//memcpy((iocp->saveFile_buffing + iocp->num_of_receptions), buffer, dwBytesTransfered);
		iocp->num_of_receptions += dwBytesTransfered;
	}

	if (iocp->create_file_flag)
	{
		iocp->create_file_flag = false;
		if (saveFile_buff_flag)
		{
			iocp->saveFile_buffing = iocp->saveFile_buff_reserve2;
			iocp->saveFile_buffed = iocp->saveFile_buff_reserve1;
		}
		else
		{
			iocp->saveFile_buffing = iocp->saveFile_buff_reserve1;
			iocp->saveFile_buffed = iocp->saveFile_buff_reserve2;
		}
		saveFile_buff_flag = !saveFile_buff_flag;

		iocp->is_stopSaveBuff = false;
		iocp->cv_saveFile.notify_all();
	}
	
	// ���ｫbuffer���ݸ���һ�ݸ�ringbuffer
	if (!iocp->m_RingBuffer->adds(buffer, dwBytesTransfered))
	{
		std::cout << "WASBUFtrans2Ringbuffer add ringbuffer failed" << std::endl;
		return false;
	}
	return true;
}

// num_of_boards��ʾ��ʵ���˵İ�������
bool init(uint32_t num_of_boards, char channel_map_path[], char save_path[], uint32_t save_file_time, uint32_t sample_rate)
{
	instance = new yIOCP(11, channel_map_path, save_path, save_file_time, sample_rate, num_of_boards);
	return instance->initIOCP();
}

void uninitialize()
{
	if (instance)
	{
		delete instance;
		instance = NULL;
	}
}

bool get_data(uint32_t* buffer, uint32_t num)
{
	return(instance->m_RingBuffer->removes(buffer, num));
}

void begin_save(bool flag)
{
	instance->start_store_data_flag = flag;
}

// mode = 2 �Լ�����
// mode = 3 ��ʼ�ɼ�����
bool send_msg(int mode)
{
	if (!instance->m_arrayClientContext.size())
	{
		return false;
	}
	PER_IO_CONTEXT* pNewIoContext = instance->m_arrayClientContext[0]->GetNewIoContext();
	pNewIoContext->m_sockAccept = instance->m_arrayClientContext[0]->m_Socket;

	CMD cmd;
	char cmd_address[10];

	if (mode == 2)
	{
		switch (instance->_sample_rate_to_FPGA)
		{
		case 50000: set_cmd_type(CMD_SELFCHECK_11NODE_50K, cmd); break;   // 50k
		case 25000: set_cmd_type(CMD_SELFCHECK_11NODE_25K, cmd); break;   // 25k
		case 100000: set_cmd_type(CMD_SELFCHECK_11NODE_100K, cmd); break;   // 100k �ο�tcp��psЭ��
		default:break;
		}
		pNewIoContext->m_OpType = IO_SEND_SELFCHECK;
	}
	else if (mode == 3)
	{
		switch (instance->_sample_rate_to_FPGA)
		{
		case 50000: set_cmd_type(CMD_BEGIN_11NODE_50K, cmd); break;   // 50k
		case 25000: set_cmd_type(CMD_BEGIN_11NODE_25K, cmd); break;   // 25k
		case 100000: set_cmd_type(CMD_BEGIN_11NODE_100K, cmd); break;   // 100k �ο�tcp��psЭ��
		default:break;
		}
		pNewIoContext->m_OpType = IO_SEND_BEGIN; 
	}
	else
	{
		std::cout << "ERROR:	error type of cmd" << std::endl;
	}

	memcpy(cmd_address, &cmd, sizeof(CMD));

	return instance->_postSend(pNewIoContext, cmd_address, 10);
}

void set_cmd_type(char type_of_cmd, char num_of_node, char sample_rate, CMD& command)
{
	command._type_of_cmd = type_of_cmd;
	command._num_of_node = instance->_real_num_of_boards;
	command._sample_rate = sample_rate;
}

uint32_t get_selfcheck_state_for_qt()
{
	return instance->get_selfcheck_state();
}


