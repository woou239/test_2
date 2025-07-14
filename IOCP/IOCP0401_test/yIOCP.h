#pragma once
#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <cstring>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <mutex>
#include <future>
#include "RingBuffer.h"
#include "threadpool.h"

// note:�����2�����ļ�ʱû�н�64λ��485���ݽ��д��������Ļ�csv�ļ�ʱ����д���

#define CMD_CONNECT_4NODE_25K_SAMPLE_RATE			1, 4, 1
#define CMD_CONNECT_11NODE_25K_SAMPLE_RATE			1, 11, 1
#define CMD_CONNECT_4NODE_50K_SAMPLE_RATE			1, 4, 2
#define CMD_CONNECT_11NODE_50K_SAMPLE_RATE			1, 11, 2
#define CMD_CONNECT_4NODE_100K_SAMPLE_RATE			1, 4, 3
#define CMD_CONNECT_11NODE_100K_SAMPLE_RATE			1, 11, 3

#define CMD_SELFCHECK_4NODE_25K						2, 4, 1
#define CMD_SELFCHECK_11NODE_25K					2, 11, 1
#define CMD_SELFCHECK_4NODE_50K						2, 4, 2
#define CMD_SELFCHECK_11NODE_50K					2, 11, 2
#define CMD_SELFCHECK_4NODE_100K					2, 4, 3
#define CMD_SELFCHECK_11NODE_100K					2, 11, 3

#define CMD_BEGIN_4NODE_25K							3, 4, 1
#define CMD_BEGIN_11NODE_25K						3, 11, 1
#define CMD_BEGIN_4NODE_50K							3, 4, 2
#define CMD_BEGIN_11NODE_50K						3, 11, 2
#define CMD_BEGIN_4NODE_100K						3, 4, 3
#define CMD_BEGIN_11NODE_100K						3, 11, 3

#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}



#define MAX_POST_ACCEPT 12
#define EXIT_CODE NULL
#define THREAD_NUM 12

#define REF_LISTENER (1.1111*0.000000488292)
#define REF_OTHER (((1.1111*0.000000488292)/(0.4)))*(300/16)

#define BYTE_OF_PER_BOARD 72										// ÿ�����ӵ��ֽ��� (16 + 2) * 4
#define NUM_OF_CHANNELS 16											// ÿ������ͨ������ 16
#define ADC_DATA_WIDTH 24											// adc����λ�� 24λ
#define SYNC_COPDE_DATA_WIDTH 8										// ͬ����λ�� 8λ
#define SENSOR_485_DATA_WIDTH 64									// 485����λ�� 64λ
#define TRANS_FREQUENCY 50000										// ������ 50K
#define PER_RECV_TIME_COUNT 2000									// ÿ��tcp���ն���ʱ�̵����� 1000��ʱ��

#define RINGBUFFER_LENGTH 2147483648								// ringbuffer��length 2��27�η� 

#define MAX_BUFFER_LEN 11 * 72 * PER_RECV_TIME_COUNT

extern "C" _declspec(dllexport) bool init(uint32_t num_of_boards, char channel_map_path[], char save_path[], uint32_t save_file_time, uint32_t sample_rate);
extern "C" _declspec(dllexport) void uninitialize();
extern "C" _declspec(dllexport) bool get_data(uint32_t * buffer, uint32_t num);
extern "C" _declspec(dllexport) void begin_save(bool flag);
extern "C" _declspec(dllexport) bool send_msg(int mode);
extern "C" _declspec(dllexport) uint32_t get_selfcheck_state_for_qt();

struct CMD {
	char _type_of_cmd;
	char _num_of_node;
	char _sample_rate;
	char _result_of_selfcheck[4];
	char _null[3]; //û���õ�
};



typedef enum _OPERATION_TYPE {
	IO_ACCEPT,
	IO_SEND_CONNECT,
	IO_SEND_SELFCHECK,
	IO_SEND_BEGIN,
	IO_RECV_SELFCHECKED,
	IO_RECV_BEIGNED,
	IO_RECV,
	IO_CONNECT,
	IO_DISCONNECT,
	NULL_POSTED
}OPERATION_TYPE;

//��IO���ݣ�ÿ���ص�IO��Ҫ��Ӧ����һ������
typedef struct _PER_IO_CONTEXT {
	OVERLAPPED      m_Overlapped;
	SOCKET          m_sockAccept;
	WSABUF          m_wsaBuf;
	char            m_szBuffer[MAX_BUFFER_LEN];
	OPERATION_TYPE  m_OpType;

	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType = NULL_POSTED;
	}
	// �ͷŵ�Socket
	~_PER_IO_CONTEXT()
	{
		if (m_sockAccept != INVALID_SOCKET)
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}
	// ���û���������
	void ResetBuffer()
	{
		ZeroMemory(m_wsaBuf.buf, MAX_BUFFER_LEN);
	}
} PER_IO_CONTEXT, * PPER_IO_CONTEXT;

// ���������
typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET                    m_Socket;							// ÿһ���ͻ������ӵ�Socket
	SOCKADDR_IN               m_ClientAddr;						// ����ͻ��˵ĵ�ַ
	std::vector<_PER_IO_CONTEXT*>  m_arrayIoContext;			// ���飬���пͻ���IO�����Ĳ�����Ҳ����˵����ÿһ���ͻ���Socket,�ǿ���������ͬʱͶ�ݶ��IO�����

	_PER_SOCKET_CONTEXT()
	{
		m_Socket = INVALID_SOCKET;
		memset(&m_ClientAddr, 0, sizeof(m_ClientAddr));
	}

	~_PER_SOCKET_CONTEXT()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
		}
		for (int i = 0; i < m_arrayIoContext.size(); i++)
		{
			delete m_arrayIoContext[i];
		}
		m_arrayIoContext.shrink_to_fit();
	}
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		m_arrayIoContext.push_back(p);

		return p;
	}
	void RemoveContext(_PER_IO_CONTEXT* pContext)
	{
		for (int i = 0; i < m_arrayIoContext.size(); i++)
		{
			if (pContext == m_arrayIoContext[i])
			{
				delete pContext;
				pContext = NULL;
				m_arrayIoContext.erase(m_arrayIoContext.begin() + i);
				break;
			}
		}
	}

} PER_SOCKET_CONTEXT, * PPER_SOCKET_CONTEXT;

class yIOCP;
typedef struct _tagThreadParams_WORKER
{
	yIOCP* IOCPModel;      // ��ָ�룬���ڵ������еĺ���
	int nThreadNumber;         // �̱߳��

} THREADPARAMS_WORKER, * PTHREADPARAM_WORKER;

class yIOCP
{
	friend bool WINAPI WorkerThread(LPVOID lpParam);
	friend bool WINAPI WASBUFtrans2Ringbuffer(yIOCP* iocp, char* buffer, DWORD dwBytesTransfered);

public:
	// real_num_of_boards��ʾ��ʵ�ӵİ��ӣ�num_of_boards = 11
	yIOCP(uint32_t num_of_boards, char channel_map_path[], const char save_path[], uint32_t save_file_time, uint32_t sample_rate, uint32_t real_num_of_boards);
	~yIOCP();
	bool initIOCP();
	bool _postAccept(PER_IO_CONTEXT* pAcceptIoContext);
	bool _postRecv(PER_IO_CONTEXT* pIoContext, int rec_type);

	// �ɰ汾
	//bool _postSend(PER_IO_CONTEXT* pIoContext);

	bool _postSend(PER_IO_CONTEXT* pIoContext, char* cmd, unsigned long length);
	bool _doAccept(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);
	bool _doRecv(PER_IO_CONTEXT* pIoContext, DWORD dwBytesTransfered, int rec_type);
	void _doDisconnect();
	bool _doSend(PER_IO_CONTEXT* pIoContext, int send_type);
	bool _AssociateWithIOCP(PER_SOCKET_CONTEXT* pSocketContext);
	void _AddToContextList(PER_SOCKET_CONTEXT* pSocketContext);
	void _RemoveContext(PER_SOCKET_CONTEXT* pSocketContext);
	void _ClearContextList();

	bool save_buff();
	void set_file_name();

	uint32_t get_selfcheck_state();

	bool start_store_data_flag;											// Ԥ������λ���ı�־λ������λ�������ʼ�������� 
	RingBuffer* m_RingBuffer;
	std::vector<PER_SOCKET_CONTEXT*> m_arrayClientContext;
	uint32_t _real_num_of_boards;										// ��ʾ��ʵ�ϵ�İ����� Ҫ���� zynq
	int _sample_rate_to_FPGA;
private:
	HANDLE m_IOCompletionPort; //��ɶ˿�
	std::string ServerIP;
	int ServerPort;
	PER_SOCKET_CONTEXT* m_pListenContext; //�����ṹ��
	
	std::future<bool> ListenThread;

	LPFN_ACCEPTEX lpfnAcceptEx;
	GUID GuidAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs ;
	GUID GuidGetAcceptExSockaddrs;
	LPFN_DISCONNECTEX lpfnDisconnectEx;
	GUID GuidDisconnectEx;

	std::mutex mtx;
	std::mutex mtx_saveFile;
	std::mutex mtx_trans2Ringbuffer;
	std::condition_variable cv_saveFile;

	int file_tag = 0;
	FILE* this_file = nullptr;

	int saveNums = 0;

	bool is_stopSaveBuff = false;	
	bool is_stopSaveBuff_thread = false;

	char* wsabuff1;														// Ԥ��������ڴ棬����Ͷ��ʱ�滻
	char* wsabuff2;

	//std::threadpool pool;												// �̳߳� ���� ���ն���Ϊȫ�ֱ���

	std::future<bool> wsabuff1_ret;										// �����첽���� WASBUFtrans2Ringbuffer() ����Ľ��
	std::future<bool> wsabuff2_ret;

	uint32_t num_of_receptions;											// ��¼����������
	
	bool create_file_flag;												// ��do_Recv�д����ļ���־

	char* saveFile_buffing;												// ���ڽ�������ʱ��ָ�룬ѭ��ָ�� saveFile_buff_reserve1 �� saveFile_buff_reserve2
	char* saveFile_buffed;												// ���ڱ��浽�ļ�������ָ�룬ѭ��ָ�� saveFile_buff_reserve1 �� saveFile_buff_reserve2
	char* saveFile_buff_reserve1;
	char* saveFile_buff_reserve2;

	bool workerThread_stop;												// WorkerThread�˳���־

	uint32_t _num_of_boards;											// ��������
	uint32_t _per_sample_data_lenth;									// ÿ��ʱ�����ݵĳ���(B)
	uint32_t _save_file_time;											// ���ļ���ʱ�� 10s ʵ�ʸ���ѡ�����	
	uint32_t _save_file_nums;											// ÿ���ļ��������� (_save_file_time * ������) ������
	uint32_t _save_file_Bytes;											// ÿ���ļ������ֽ��� (_save_file_time * ������) * _per_sample_data_lenth
	uint32_t _save_file_channel;										// ���ļ���� 32λ (16 + 2) * _num_of_boards ÿ��ʱ���� _save_file_channel ��32λ����

	std::string save_dir_path;

	uint32_t selfcheck_state;											// ���λΪ1��ʾ��Ч 11�����ӣ�ÿ����������λ��ʾϵͳ״̬ Ҳ���ǵ�22λΪϵͳ״̬

	
};


bool WINAPI WorkerThread(LPVOID lpParam);
bool WINAPI WASBUFtrans2Ringbuffer(yIOCP* iocp, char* buffer, DWORD dwBytesTransfered);

void set_cmd_type(char type_of_cmd, char num_of_node, char sample_rate, CMD& command);

typedef struct _IOCP_STATUS {
	OPERATION_TYPE op_type;

}IOCP_STATUS;


