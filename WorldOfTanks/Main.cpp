#include "stdafx.h"

#include "WorldOfTanks.h"

#include "Player.h"
#include "GameCore.h"

using namespace std;

void AnalyseCommandLine(int,TCHAR**,TCHAR*,TCHAR*,char*,char*);

void JudgeMain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	Player playerA,playerB;
	GameCore gameCore;
	TCHAR* pszCommand1 = new TCHAR[1024];
	TCHAR* pszCommand2 = new TCHAR[1024];
	char* pszPlayerNameA = new char[128];
	char* pszPlayerNameB = new char[128];
	ZeroMemory(pszCommand1,1024*sizeof(TCHAR));
	ZeroMemory(pszCommand2,1024*sizeof(TCHAR));

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwSize = 0;
	COORD coo;
	coo.Y = 2;

	AnalyseCommandLine(argc,argv,pszCommand1,pszCommand2,pszPlayerNameA,pszPlayerNameB);
	/*AfxMessageBox(pszCommand1);
	AfxMessageBox(pszCommand2);*/

	cout<<"Initializing...";
	if(!playerA.Initialize(pszCommand1) || !playerB.Initialize(pszCommand2))
	{
		cerr<<"启动选手程序失败！(0x0001)"<<endl;
		goto ReleaseResource;
	}

	gameCore.SetPlayer(&playerA,&playerB);
	gameCore.SetPlayerName(pszPlayerNameA,pszPlayerNameB);

	if(!gameCore.Initialize())
	{
		cerr<<"游戏状态初始化失败!(0x0002)"<<endl;
		goto ReleaseResource;
	}
	cout<<"OK!"<<endl;

	gameCore.GenerateInitialData();
	gameCore.DistributeData();

	cout<<"The game is running....."<<endl;
	cout<<"Round.01..."<<endl;
	int round = 1;

	while( true )
	{
		coo.X = 6;
		FillConsoleOutputCharacterA(hOut,round / 10 + 48,1,coo,&dwSize);
		coo.X = 7;
		FillConsoleOutputCharacterA(hOut,round % 10 + 48,1,coo,&dwSize);

		if(!gameCore.PlayerAsTurn())
			goto ReportResult;
		if(!gameCore.PlayerBsTurn())
			goto ReportResult;

		if(gameCore.IsGameOver())
		{
			goto ReportResult;
		}
		round++;
	}
	
	//Report
ReportResult:
	cout<<"The report:"<<endl;
	gameCore.ReportResult();
	::Sleep(2000);
	//Cleaning.
ReleaseResource:
	delete []pszCommand1;
	delete []pszCommand2;
	return;
}
//分析命令行参数，解析生成选手程序的启动命令行.
void AnalyseCommandLine(int argc,TCHAR* argv[],TCHAR* psz1,TCHAR* psz2,char* name1,char* name2)
{
	lstrcpy(psz1,_T("a"));
	strcpy(name1,"a");
	lstrcpy(psz2,_T("b"));
	strcpy(name2,"b");
	
	return;
}
