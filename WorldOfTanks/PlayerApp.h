#pragma once

#include "stdafx.h"

class PlayerApp
{
	friend class GameCore;

public:
	PlayerApp();
	~PlayerApp();
	//初始化及选手程序启动.
	bool Initialize(LPWSTR);
	//设置文件名.
	bool SetName(const char*);
	//写入先后手信息.
	bool WriteFirsthand(bool);
	//读入选手阵营选择.
	bool ChooseFaction(Faction&);
	//写入游戏初始数据.
	bool WriteGameData(GameData&);
	//写入上一条(对手)命令.
	bool WriteLastCommand(Command&,CmdRpt&);
	//读入选手命令.
	bool ReadCommand(Command&);
	//写入命令执行结果.
	bool WriteCommandReport(CmdRpt&);
	//输入Tank.
	bool InputTank(Tank&);
	
protected:
	bool InputStringsPreProcess();

private:
	//TCHAR* pszCommandLine;      //应用程序启动命令行.
	PROCESS_INFORMATION pi;     //进程信息.
	HANDLE hInputRead;          //程序输入读管道句柄.
	HANDLE hInputWrite;         //程序输入写管道句柄.
	HANDLE hOutputRead;         //程序输出读管道句柄.
	HANDLE hOutputWrite;        //程序输出写管道句柄.
	char* pszInput;             //程序输入缓冲区.(2048 Bytes)
	char* pszOutput;            //程序输出缓冲区.(2048 Bytes)
	char  strName[128];         //文件名.
	ofstream* fdebug;           //调试模式输出文件.
	ofstream* flog;             //选手输入输出信息日志文件.

	bool firsthand;             //是否为先手.
	//static bool bTimeOut;
};
