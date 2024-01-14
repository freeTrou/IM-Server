#ifndef _TCPSERVER_HPP_
#define _TCPSERVER_HPP_

//跨平台
#ifdef _WIN32//windows平台
#define FD_SETSIZE 10240 //覆盖select默认链接数
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>//windows系统库  在使用时要加入库文件，不然会链接错误
#include <WinSock2.h>//windows网络库
#pragma comment(lib,"ws2_32.lib")//  在使用windows系统库时要加入ws2_32.lib，不然会链接错误

#else//其他平台
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
#define RECV_BUFF_SIZE 4096 //缓冲区大小

#endif // !RECV_BUFF_SIZE


//通用
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
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)//默认给一个无效的值
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
	//第二缓冲区 消息缓冲区 ==》将接受缓冲区的包组成一个完整的然后进行处理
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	int _lastPos;//数据末尾位置
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
		this->_sock = INVALID_SOCKET; //初始化为一个无效的宏

	}
	~TcpServer()
	{

	}
	//初始化socket
	SOCKET initSocket()
	{
		//启动windows下socket网络环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//创建版本号 表示2.2
		WSADATA data;
		int ret = -1;
		ret = WSAStartup(ver, &data);//启动socket网络环境
		if (SOCKET_ERROR == ret)
		{
			printf("启动网络环境失败\n");
			return -1;
		}
#endif 
		if (_sock != INVALID_SOCKET)//初始化的时候发现是一个有效的socket，先清理
		{
			printf("<_sock = %d>关闭了之前的连接\n", _sock);
			Close();
		}
		//建立一个socket，获取套接字
		this->_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//参数：ipv4/ipv6 数据流 选择协议TCP
		if (INVALID_SOCKET == _sock)
		{
			printf("建立套接字失败\n");
			return (SOCKET)-1;
		}
		else
		{
			printf("建立套接字成功\n");
			return _sock;
		}

	}

	//绑定ip和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//连接服务器connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//ipv4协议
		_sin.sin_port = htons(port);//端口号 主机字节序转为网络字节序 host to net ushort
#ifdef _WIN32
		if (ip != nullptr)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);//绑定到哪个ip地址上   INADDR_ANY 绑定本机所有(任意)ip
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; //绑定到哪个ip地址上   INADDR_ANY 绑定本机所有(任意)ip
		}
	
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) //套接字 要连接的服务器信息 结构体大小
		{
			printf("错误，绑定网络端口<%d>失败\n",port);
			return -1;
		}
		else
		{
			printf("绑定网络端口<%d>成功\n",port);
		}

		return 0;
	}

	//监听端口号
	int Listen(int n)
	{
		//listen监听绑定的网络端口
		int ret = listen(_sock, n);//等待最大链接数
		if (SOCKET_ERROR == ret)
		{
			printf("监听网络端口失败\n");
			return -1;
		}
		else
		{
			printf("监听网络端口成功\n");
			return ret;
		}

	}

	//接收客户端连接
	SOCKET Accept()
	{
		// accept等待接受客户端连接  ==> 提取一个新的套接字
		sockaddr_in clientAddr = {};
		int addrlen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//先等于一个无效的值
		cSock = accept(_sock, (sockaddr*)&clientAddr, &addrlen);
		if (INVALID_SOCKET == cSock)
		{
			printf("提取新连接失败\n");
			return -1;

		}
		else//提取新连接成功
		{
			NewUserJoin userjoin;
			userjoin.sock = cSock;
			sendDataToAll(&userjoin);

			//将提取的新连接存储到动态数组中
			_clients.push_back(new ClientSocket(cSock));
			//成功，打印连接客户端信息
			//printf("新客户端<Socket = %d>加入,数量<%d>，ip = %s, port = %u\n", cSock,_clients.size(), inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		}

		return cSock;
	}

	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)//不是一个无效值，才对其清理
		{
#ifdef _WIN32
			//先关闭所有客户端连接
			for (int i = 0; i < _clients.size(); i++)
			{
				closesocket(_clients[i]->sockfd());
				delete _clients[i];//释放开辟空间
			}
			//关闭套接字 closesocket
			closesocket(_sock);
			//关闭windows下socket网络环境
			WSACleanup();//清除socket网络环境
#else
			close(_sock);
#endif // _WIN32
			this->_sock = INVALID_SOCKET;//初始化为一个无效的宏
			_clients.clear();//清理数组
		}
	}

	//查询是否有网络消息
	bool onRun()
	{
		if (isRun())
		{
			//定义三个集合
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExcept;

			//清空集合   底层只是将结构体中的count置为0
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			//将描述符放到集合中
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExcept);

			SOCKET maxSock = _sock;
			//将动态数组中的socket放到读集合中
			for (int i = 0; i < _clients.size(); i++)
			{
				FD_SET(_clients[i]->sockfd(), &fdRead);
				//判断一下maxSock值是否还是最大的
				if (maxSock < _clients[i]->sockfd())
				{
					maxSock = _clients[i]->sockfd();//更新最大的socket
				}
			}

			//select监听到有描述符有数据到来，会将没有数据的描述符情况，只保留要操作的描述符在集合中
			//最后一个参数为nullptr，会一直等待有数据（阻塞） select(_sock + 1, &fdRead, &fdWrite, &fdExcept, nullptr);
			timeval t = { 1,0 };//第一个秒 第二个微秒
			int retSelect = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);//非阻塞m模式，传入时间为0 没有数据会立即返回
			if (retSelect < 0)//出现问题
			{
				Close();
				printf("select任务结束\n");
				return false;
			}
			//判断这个描述符在不在这个集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);//用来将一个给定的文件描述符从集合中删除

				// accept等待接受客户端连接  ==> 提取一个新的套接字
				//SOCKET cSock = INVALID_SOCKET;//先等于一个无效的值
				Accept();//已经将提取的新连接存到容器中了
			}

			//将描述符使用函数处理，从描述符集合中
			for (int i = 0; i < _clients.size(); i++)
			{
				//判断这个描述符在不在这个集合中 select会将没有数据的描述符置为0，也就是在集合中删除了
				if (FD_ISSET(_clients[i]->sockfd(),&fdRead))
				{
					if (-1 == recvData(_clients[i]))//表示当前描述符对应的客户端退出
					{
						//从动态数组中删除
						auto iter = _clients.begin() + i;//查找这个描述符在数组中的位置
						if (iter != _clients.end())//不等于最后一个（找到了）
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


	//判断这个连接是否有效 是否正常工作
	bool isRun()//有效-真 无效-假
	{
		return _sock != INVALID_SOCKET;
	}

	

	//接收数据 处理粘包 拆分包
	int recvData(ClientSocket* pClinet)
	{
		
		//接收客户端数据
		int nLen = recv(pClinet->sockfd(), _szRecv, sizeof(_szRecv), 0);
		if (nLen <= 0)
		{
			printf("客户端<Socket = %d>断开连接，任务结束\n", pClinet->sockfd());
			return -1;
		}
		//printf("server nLen = %d\n", nLen);
		//LoginResult ret;
		//ret.result = 1234;
		//sendData(pClinet->sockfd(), &ret);

		//将消息缓冲区数据拷贝到第二缓冲区
		memcpy(pClinet->msgBuf() + pClinet->getLastPos(),_szRecv,nLen);
		//消息缓冲区尾部消息后移
		pClinet->setLastPos(pClinet->getLastPos() + nLen);
		//接收的消息已经大于一个消息头的长度，可以解析命令了,也可以知道当前消息的长度了，里面包含了这个数据
		while (pClinet->getLastPos() >= sizeof(DataHeader))//处理粘包(多包)和少包
		{
			DataHeader* header = (DataHeader*)pClinet->msgBuf();
			if (pClinet->getLastPos() >= header->dataLength)//将一个完整的数据包进行处理，直到外层循环不满足条件，将缓冲区中处理
			{
				//先计算处理完消息之后缓冲区剩余数据的长度
				int nSize = pClinet->getLastPos() - header->dataLength;
				onNetMsg(pClinet->sockfd(),header);//处理网络消息
				//剩余消息前移
				memcpy(pClinet->msgBuf(), pClinet->msgBuf() + header->dataLength, nSize);
				pClinet->setLastPos(nSize);//末尾位置偏移
			}
			else
			{
				//剩余缓冲区数据不够一个完整的数据包，直接跳出循环
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

	//响应网络数据
	virtual void onNetMsg(SOCKET cSock,DataHeader* header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();//获取秒
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,cSock,this->_clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		//处理请求
		switch (header->cmd)
		{
		case CMD_LOGIN://登录
		{
			//接收客户端消息
			//定义接收消息的对象
			//数据包前半截已经被读了，需要偏移数据量
			
			Login* login = (Login*)header;
			//printf("收到客户端<Socket = %d>登录请求，数据长度:%d，收到命令:CMD_LOGIN，username = %s | password = %s\n", cSock, login->dataLength, login->userName, login->passWord);
			/*
			忽略验证用户名和密码是否正确的过程
			*/
			//发送给客户端响应
			LoginResult ret;
			ret.result = 1;//登录响应消息 1-> 成功 0 - 失败
			sendData(cSock, &ret);
			//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT://登出
		{
			//接收客户端消息
			Logout* logout = (Logout*)header;
			//printf("收到客户端<Socket = %d>注销请求,数据长度:%d，收到命令:CMD_LOGOUT，username = %s\n", cSock, logout->dataLength, logout->userName);

			//忽略验证用户名和密码是否正确的过程
			//发送给客户端响应
			LogoutResult ret;//登录响应消息 1-> 成功 0 - 失败
			ret.result = 1;
			sendData(cSock, &ret);
			//send(cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		break;
		default://错误处理
		{
			DataHeader headerErr = { 0,CMD_ERROR };
			sendData(cSock,&headerErr);
			printf("未知命令，错误\n");
		}
		break;
		}
	}

	//发送数据
	int sendData(SOCKET cSock,DataHeader* header)
	{
		if (isRun() && header != nullptr)
		{
			//传输数据给header多写了一个取地址符？？？？谨记
			return send(cSock, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}

	//发送数据给所有客户端
	void  sendDataToAll(DataHeader* header)
	{
		if (isRun() && header != nullptr)
		{
			//每当有新客户端加入，将消息群发给之前所有的客户端

			//发送给动态数组中的其他客户端描述符 
			for (int i = 0; i < _clients.size(); i++)
			{
				sendData(_clients[i]->sockfd(), header);
			}
		}

		return;
	}

	//返回当前客户端个数
	int getClinentNum()
	{
		return this->_clients.size();
	}

private:
	SOCKET _sock;
	
	char _szRecv[RECV_BUFF_SIZE] = { 0 };//使用缓冲区来接收数据

	//存储指针，让空间开辟到堆内存上
	std::vector<ClientSocket*> _clients;//用动态数组来存储已连接的客户端描述符
	CELLTimestamp _tTime;
	int _recvCount;
};

#endif // !_TCPSERVER_HPP_
