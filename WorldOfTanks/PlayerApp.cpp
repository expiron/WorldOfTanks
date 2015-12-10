#include "stdafx.h"

#include "PlayerApp.h"

using namespace std;

//构造函数.
PlayerApp::PlayerApp()
{
	//匿名管道句柄初始化.
	hInputRead = NULL;
	hInputWrite = NULL;
	hOutputRead = NULL;
	hOutputWrite = NULL;
	//IO缓冲区初始化.
	pszInput = new char [2048];
	pszOutput = new char [2048];
	ZeroMemory(pszInput,2048);
	ZeroMemory(pszOutput,2048);
	//文件名初始化.
	ZeroMemory(strName,128);
	//DEBUG文件指针初始化.
	fdebug = NULL;
	//日志文件指针初始化.
	flog = NULL;
}
//析构函数.
PlayerApp::~PlayerApp()
{
	TerminateProcess(pi.hProcess,0);
	if(fdebug)
	{
		fdebug->close();
		delete fdebug;
	}
	delete []pszInput;
	delete []pszOutput;
}
//初始化函数.
bool PlayerApp::Initialize(LPWSTR lpCommandLine)
{
	//定义安全描述符
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;    //使用系统默认的安全描述符
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	
	//创建子进程输入匿名管道
	if(!CreatePipe(&hInputRead,&hInputWrite,&sa,0))
	{
		//cerr<<"Can not create input pipe!"<<endl;
		return false;
	}
	//创建子进程输出匿名管道
	if(!CreatePipe(&hOutputRead,&hOutputWrite,&sa,0))
	{
		//cerr<<"Can not create output pipe!"<<endl;
		return false;
	}
	//创建进程
	//启动信息
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hInputRead;
	si.hStdOutput = hOutputWrite;
	si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
	//进程信息
	//PROCESS_INFORMATION pi;
	if (!CreateProcess(NULL,lpCommandLine,NULL,NULL,TRUE,CREATE_NO_WINDOW/*CREATE_NEW_CONSOLE*/,NULL,NULL,&si,&pi))
	{
		//cerr<<"Can not create target process!"<<endl;
		return false;
	}
	return true;
}
bool PlayerApp::SetName(const char* pStr)
{
	strcpy(strName,pStr);
	char tmp[128];
	if(flog)
		return false;

	/*memset(tmp,0,sizeof(tmp));
	strcpy(tmp,pStr);
	strcat(tmp,"_debug.txt");
	fdebug = new ofstream(tmp);
	if(!fdebug)
		return false;*/

	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,pStr);
	strcat(tmp,"_log.txt");
	flog = new ofstream(tmp);
	if(!flog)
		return false;

	return true;
}
bool PlayerApp::WriteFirsthand(bool initiative)
{
	firsthand = initiative;
	DWORD dwWrite;
	ZeroMemory(pszInput,2048);
	//序列化输出内容.
	pszInput[0] = (firsthand ? 1 : 2) + 48;
	pszInput[1] = '\n';
	//写入选手程序输入缓冲区.
	WriteFile(hInputWrite,pszInput,strlen(pszInput),&dwWrite,NULL);
	//写入日志文件.
	if(flog)
		(*flog)<<">>"<<pszInput<<endl;

	if(dwWrite == strlen(pszInput))      //写入成功.
		return true;
	else                                 //写入失败.
		return false;

}
bool PlayerApp::ChooseFaction(Faction& fac)
{
	if(!InputStringsPreProcess())
		return false;

	if(pszOutput[0] == '1')
		fac = Axis;              //轴心国.
	else
	if(pszOutput[0] == '2')
		fac = Allies;            //同盟国.
	else
		return false;

	return true;
}
bool PlayerApp::WriteGameData(GameData& gdata)
{
	DWORD dwWrite;
	ZeroMemory(pszInput,2048);
	ostrstream sout(pszInput,2048);
	sout<<gdata;

	WriteFile(hInputWrite,pszInput,strlen(pszInput),&dwWrite,NULL);
	//写入日志文件.
	if(flog)
		(*flog)<<">>"<<pszInput<<endl;

	if(dwWrite == strlen(pszInput))      //写入成功.
		return true;
	else                                 //写入失败.
		return false;
}
bool PlayerApp::WriteLastCommand(Command& cmd,CmdRpt& rpt)
{
	DWORD dwWrite;
	ZeroMemory(pszInput,2048);
	ostrstream sout(pszInput,2048);
	sout<<cmd<<rpt;

	WriteFile(hInputWrite,pszInput,strlen(pszInput),&dwWrite,NULL);
	//写入日志文件.
	if(flog)
		(*flog)<<">>"<<pszInput<<endl;

	if(dwWrite == strlen(pszInput))      //写入成功.
		return true;
	else                                 //写入失败.
		return false;
}
bool PlayerApp::ReadCommand(Command& cmd)
{
	if(!InputStringsPreProcess())
		return false;
	istrstream strin(pszOutput);

	strin>>cmd;

	return true;
}
bool PlayerApp::WriteCommandReport(CmdRpt& rpt)
{
	DWORD dwWrite;
	ZeroMemory(pszInput,2048);
	ostrstream sout(pszInput,2048);
	sout<<rpt;

	WriteFile(hInputWrite,pszInput,strlen(pszInput),&dwWrite,NULL);
	//写入日志文件.
	if(flog)
		(*flog)<<">>"<<pszInput<<endl;

	if(dwWrite == strlen(pszInput))      //写入成功.
		return true;
	else                                 //写入失败.
		return false;
}
bool PlayerApp::InputStringsPreProcess()
{
	DWORD dwRead = 0;
	Sleep(25);    //Timeout

	ZeroMemory(pszOutput,2048);
	DWORD dwExitCode;
	GetExitCodeProcess(pi.hProcess,&dwExitCode);
	if(dwExitCode != STILL_ACTIVE)
	{
		cerr<<"选手程序异常结束！"<<endl;
		return false;
	}
	//BOOL bRet = ReadFile(hOutputRead,pszOutput,2048,&dwRead,NULL);
	char tmp[2048];
	ZeroMemory(tmp,2048);
	ReadFile(hOutputRead,tmp,2048,&dwRead,NULL);
	if(tmp[0] == '{')
		strcpy(pszOutput,tmp + 3);
	else
		strcpy(pszOutput,tmp);
	while(!strchr(tmp,'}'))
	{
		ZeroMemory(tmp,2048);
		ReadFile(hOutputRead,tmp,2048,&dwRead,NULL);
		strcat(pszOutput,tmp);
	}
	pszOutput[strlen(pszOutput)-3] = '\0';
	//写入日志文件.
	if(flog)
		(*flog)<<"<<"<<pszOutput<<endl;
	else
		return false;
	return true;
}
