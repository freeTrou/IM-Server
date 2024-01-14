#ifndef _TCPSERVER_HPP_
#define _TCPSERVER_HPP_

//��ƽ̨
#ifdef _WIN32//windowsƽ̨
#define FD_SETSIZE 10240 //����selectĬ��������
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>//windowsϵͳ��  ��ʹ��ʱҪ������ļ�����Ȼ�����Ӵ���
#include <WinSock2.h>//windows�����
#pragma comment(lib,"ws2_32.lib")//  ��ʹ��windowsϵͳ��ʱҪ����ws2_32.lib����Ȼ�����Ӵ���

#else//����ƽ̨
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif // _WIN32

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 4096 //��������С

#endif // !RECV_BUFF_SIZE


//ͨ��
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include "messageHeader.hpp"
#include "CELLTimestamp.hpp"
using namespace std;

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)//Ĭ�ϸ�һ����Ч��ֵ
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return this->_sockfd;
	}
	char* msgBuf()
	{
		return this->_szMsgBuf;
	}

	int getLastPos()
	{
		return this->_lastPos;
	}

	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������ ==�������ܻ������İ����һ��������Ȼ����д���
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	int _lastPos;//����ĩβλ��
};

class CellServer
{
public:
	CellServer()
	{

	}

	~CellServer()
	{

	}
private:

};

class TcpServer
{
public:
	TcpServer()
	{
		this->_sock = INVALID_SOCKET; //��ʼ��Ϊһ����Ч�ĺ�

	}
	~TcpServer()
	{

	}
	//��ʼ��socket
	SOCKET initSocket()
	{
		//����windows��socket���绷��
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//�����汾�� ��ʾ2.2
		WSADATA data;
		int ret = -1;
		ret = WSAStartup(ver, &data);//����socket���绷��
		if (SOCKET_ERROR == ret)
		{
			printf("�������绷��ʧ��\n");
			return -1;
		}
#endif 
		if (_sock != INVALID_SOCKET)//��ʼ����ʱ������һ����Ч��socket��������
		{
			printf("<_sock = %d>�ر���֮ǰ������\n", _sock);
			Close();
		}
		//����һ��socket����ȡ�׽���
		this->_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//������ipv4/ipv6 ������ ѡ��Э��TCP
		if (INVALID_SOCKET == _sock)
		{
			printf("�����׽���ʧ��\n");
			return (SOCKET)-1;
		}
		else
		{
			printf("�����׽��ֳɹ�\n");
			return _sock;
		}

	}

	//��ip�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//���ӷ�����connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//ipv4Э��
		_sin.sin_port = htons(port);//�˿ں� �����ֽ���תΪ�����ֽ��� host to net ushort
#ifdef _WIN32
		if (ip != nullptr)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);//�󶨵��ĸ�ip��ַ��   INADDR_ANY �󶨱�������(����)ip
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; //�󶨵��ĸ�ip��ַ��   INADDR_ANY �󶨱�������(����)ip
		}
	
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) //�׽��� Ҫ���ӵķ�������Ϣ �ṹ���С
		{
			printf("���󣬰�����˿�<%d>ʧ��\n",port);
			return -1;
		}
		else
		{
			printf("������˿�<%d>�ɹ�\n",port);
		}

		return 0;
	}

	//�����˿ں�
	int Listen(int n)
	{
		//listen�����󶨵�����˿�
		int ret = listen(_sock, n);//�ȴ����������
		if (SOCKET_ERROR == ret)
		{
			printf("��������˿�ʧ��\n");
			return -1;
		}
		else
		{
			printf("��������˿ڳɹ�\n");
			return ret;
		}

	}

	//���տͻ�������
	SOCKET Accept()
	{
		// accept�ȴ����ܿͻ�������  ==> ��ȡһ���µ��׽���
		sockaddr_in clientAddr = {};
		int addrlen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//�ȵ���һ����Ч��ֵ
		cSock = accept(_sock, (sockaddr*)&clientAddr, &addrlen);
		if (INVALID_SOCKET == cSock)
		{
			printf("��ȡ������ʧ��\n");
			return -1;

		}
		else//��ȡ�����ӳɹ�
		{
			NewUserJoin userjoin;
			userjoin.sock = cSock;
			sendDataToAll(&userjoin);

			//����ȡ�������Ӵ洢����̬������
			_clients.push_back(new ClientSocket(cSock));
			//�ɹ�����ӡ���ӿͻ�����Ϣ
			//printf("�¿ͻ���<Socket = %d>����,����<%d>��ip = %s, port = %u\n", cSock,_clients.size(), inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		}

		return cSock;
	}

	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)//����һ����Чֵ���Ŷ�������
		{
#ifdef _WIN32
			//�ȹر����пͻ�������
			for (int i = 0; i < _clients.size(); i++)
			{
				closesocket(_clients[i]->sockfd());
				delete _clients[i];//�ͷſ��ٿռ�
			}
			//�ر��׽��� closesocket
			closesocket(_sock);
			//�ر�windows��socket���绷��
			WSACleanup();//���socket���绷��
#else
			close(_sock);
#endif // _WIN32
			this->_sock = INVALID_SOCKET;//��ʼ��Ϊһ����Ч�ĺ�
			_clients.clear();//��������
		}
	}

	//��ѯ�Ƿ���������Ϣ
	bool onRun()
	{
		if (isRun())
		{
			//������������
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExcept;

			//��ռ���   �ײ�ֻ�ǽ��ṹ���е�count��Ϊ0
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			//���������ŵ�������
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExcept);

			SOCKET maxSock = _sock;
			//����̬�����е�socket�ŵ���������
			for (int i = 0; i < _clients.size(); i++)
			{
				FD_SET(_clients[i]->sockfd(), &fdRead);
				//�ж�һ��maxSockֵ�Ƿ�������
				if (maxSock < _clients[i]->sockfd())
				{
					maxSock = _clients[i]->sockfd();//��������socket
				}
			}

			//select�������������������ݵ������Ὣû�����ݵ������������ֻ����Ҫ�������������ڼ�����
			//���һ������Ϊnullptr����һֱ�ȴ������ݣ������� select(_sock + 1, &fdRead, &fdWrite, &fdExcept, nullptr);
			timeval t = { 1,0 };//��һ���� �ڶ���΢��
			int retSelect = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);//������mģʽ������ʱ��Ϊ0 û�����ݻ���������
			if (retSelect < 0)//��������
			{
				Close();
				printf("select�������\n");
				return false;
			}
			//�ж�����������ڲ������������
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);//������һ���������ļ��������Ӽ�����ɾ��

				// accept�ȴ����ܿͻ�������  ==> ��ȡһ���µ��׽���
				//SOCKET cSock = INVALID_SOCKET;//�ȵ���һ����Ч��ֵ
				Accept();//�Ѿ�����ȡ�������Ӵ浽��������
			}

			//��������ʹ�ú���������������������
			for (int i = 0; i < _clients.size(); i++)
			{
				//�ж�����������ڲ������������ select�Ὣû�����ݵ���������Ϊ0��Ҳ�����ڼ�����ɾ����
				if (FD_ISSET(_clients[i]->sockfd(),&fdRead))
				{
					if (-1 == recvData(_clients[i]))//��ʾ��ǰ��������Ӧ�Ŀͻ����˳�
					{
						//�Ӷ�̬������ɾ��
						auto iter = _clients.begin() + i;//��������������������е�λ��
						if (iter != _clients.end())//���������һ�����ҵ��ˣ�
						{
							delete _clients[i];
							_clients.erase(iter);
						}
					}
				}
				
			}

			return true;
		}

		return false;
	}


	//�ж���������Ƿ���Ч �Ƿ���������
	bool isRun()//��Ч-�� ��Ч-��
	{
		return _sock != INVALID_SOCKET;
	}

	

	//�������� ����ճ�� ��ְ�
	int recvData(ClientSocket* pClinet)
	{
		
		//���տͻ�������
		int nLen = recv(pClinet->sockfd(), _szRecv, sizeof(_szRecv), 0);
		if (nLen <= 0)
		{
			printf("�ͻ���<Socket = %d>�Ͽ����ӣ��������\n", pClinet->sockfd());
			return -1;
		}
		//printf("server nLen = %d\n", nLen);
		//LoginResult ret;
		//ret.result = 1234;
		//sendData(pClinet->sockfd(), &ret);

		//����Ϣ���������ݿ������ڶ�������
		memcpy(pClinet->msgBuf() + pClinet->getLastPos(),_szRecv,nLen);
		//��Ϣ������β����Ϣ����
		pClinet->setLastPos(pClinet->getLastPos() + nLen);
		//���յ���Ϣ�Ѿ�����һ����Ϣͷ�ĳ��ȣ����Խ���������,Ҳ����֪����ǰ��Ϣ�ĳ����ˣ�����������������
		while (pClinet->getLastPos() >= sizeof(DataHeader))//����ճ��(���)���ٰ�
		{
			DataHeader* header = (DataHeader*)pClinet->msgBuf();
			if (pClinet->getLastPos() >= header->dataLength)//��һ�����������ݰ����д���ֱ�����ѭ�����������������������д���
			{
				//�ȼ��㴦������Ϣ֮�󻺳���ʣ�����ݵĳ���
				int nSize = pClinet->getLastPos() - header->dataLength;
				onNetMsg(pClinet->sockfd(),header);//����������Ϣ
				//ʣ����Ϣǰ��
				memcpy(pClinet->msgBuf(), pClinet->msgBuf() + header->dataLength, nSize);
				pClinet->setLastPos(nSize);//ĩβλ��ƫ��
			}
			else
			{
				//ʣ�໺�������ݲ���һ�����������ݰ���ֱ������ѭ��
				break;
			}
		}

		/*
		DataHeader* header = (DataHeader*)buf;
		recv(cSock, buf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(cSock, header);
		*/

		return 0;
	}

	//��Ӧ��������
	virtual void onNetMsg(SOCKET cSock,DataHeader* header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();//��ȡ��
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,cSock,this->_clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		//��������
		switch (header->cmd)
		{
		case CMD_LOGIN://��¼
		{
			//���տͻ�����Ϣ
			//���������Ϣ�Ķ���
			//���ݰ�ǰ����Ѿ������ˣ���Ҫƫ��������
			
			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket = %d>��¼�������ݳ���:%d���յ�����:CMD_LOGIN��username = %s | password = %s\n", cSock, login->dataLength, login->userName, login->passWord);
			/*
			������֤�û����������Ƿ���ȷ�Ĺ���
			*/
			//���͸��ͻ�����Ӧ
			LoginResult ret;
			ret.result = 1;//��¼��Ӧ��Ϣ 1-> �ɹ� 0 - ʧ��
			sendData(cSock, &ret);
			//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT://�ǳ�
		{
			//���տͻ�����Ϣ
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<Socket = %d>ע������,���ݳ���:%d���յ�����:CMD_LOGOUT��username = %s\n", cSock, logout->dataLength, logout->userName);

			//������֤�û����������Ƿ���ȷ�Ĺ���
			//���͸��ͻ�����Ӧ
			LogoutResult ret;//��¼��Ӧ��Ϣ 1-> �ɹ� 0 - ʧ��
			ret.result = 1;
			sendData(cSock, &ret);
			//send(cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		break;
		default://������
		{
			DataHeader headerErr = { 0,CMD_ERROR };
			sendData(cSock,&headerErr);
			printf("δ֪�������\n");
		}
		break;
		}
	}

	//��������
	int sendData(SOCKET cSock,DataHeader* header)
	{
		if (isRun() && header != nullptr)
		{
			//�������ݸ�header��д��һ��ȡ��ַ��������������
			return send(cSock, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}

	//�������ݸ����пͻ���
	void  sendDataToAll(DataHeader* header)
	{
		if (isRun() && header != nullptr)
		{
			//ÿ�����¿ͻ��˼��룬����ϢȺ����֮ǰ���еĿͻ���

			//���͸���̬�����е������ͻ��������� 
			for (int i = 0; i < _clients.size(); i++)
			{
				sendData(_clients[i]->sockfd(), header);
			}
		}

		return;
	}

	//���ص�ǰ�ͻ��˸���
	int getClinentNum()
	{
		return this->_clients.size();
	}

private:
	SOCKET _sock;
	
	char _szRecv[RECV_BUFF_SIZE] = { 0 };//ʹ�û���������������

	//�洢ָ�룬�ÿռ俪�ٵ����ڴ���
	std::vector<ClientSocket*> _clients;//�ö�̬�������洢�����ӵĿͻ���������
	CELLTimestamp _tTime;
	int _recvCount;
};

#endif // !_TCPSERVER_HPP_
