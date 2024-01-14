#pragma once
#include <iostream>

//CMD命令
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//包头
struct DataHeader
{
	short dataLength;//数据长度
	short cmd;//数据命令
};

//简易结构化的网络数据
struct Login : public DataHeader
{
public:
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
		memset(userName, 0, 32);
		memset(passWord, 0, 32);
	}

	char userName[32];
	char passWord[32];
	char data[932] = { 0 };
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = -1;
	}
	int result;//登录结果
	char data[992] = { 0 };
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
		memset(userName, 0, 32);
	}

	char userName[32];//登出用户
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = -1;
	}
	int result;//登出结果
};

//新用户加入
struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = -1;
	}
	int sock;//新用户socket id
};