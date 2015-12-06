#include "stdafx.h"

#include "WorldOfTanks.h"

#include "Player.h"
#include "GameCore.h"

using namespace std;

void AnalyseCommandLine(int,TCHAR**,TCHAR*,TCHAR*);

void JudgeMain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	Player playerA,playerB;
	GameCore gameCore;
	TCHAR* pszCommand1 = new TCHAR[1024];
	TCHAR* pszCommand2 = new TCHAR[1024];
	ZeroMemory(pszCommand1,1024*sizeof(TCHAR));
	ZeroMemory(pszCommand2,1024*sizeof(TCHAR));

	AnalyseCommandLine(argc,argv,pszCommand1,pszCommand2);
	/*AfxMessageBox(pszCommand1);
	AfxMessageBox(pszCommand2);*/

	cout<<"Initializing...";
	if(!playerA.Initialize(pszCommand1) || !playerB.Initialize(pszCommand2))
	{
		cerr<<"启动选手程序失败！(0x0001)"<<endl;
		goto ReleaseResource;
	}

	gameCore.SetPlayer(&playerA,&playerB);
	gameCore.SetPlayerName("a","b");

	if(!gameCore.Initialize())
	{
		cerr<<"游戏状态初始化失败!(0x0002)"<<endl;
		goto ReleaseResource;
	}
	cout<<"OK!"<<endl;

	gameCore.GenerateInitialData();
	gameCore.DistributeData();

	cout<<"The game is running....."<<endl;

	while( true )
	{
		gameCore.PlayerAsTurn();
		gameCore.PlayerBsTurn();

		if(gameCore.IsGameOver())
		{
			cout<<"The report:"<<endl;
			gameCore.ReportResult();
			break;
		}
	}
	
	//Cleaning.
	ReleaseResource:
	delete []pszCommand1;
	delete []pszCommand2;
	return;
}
//分析命令行参数，解析生成选手程序的启动命令行.
void AnalyseCommandLine(int argc,TCHAR* argv[],TCHAR* psz1,TCHAR* psz2)
{
	lstrcpy(psz1,_T("a.exe"));
	lstrcpy(psz2,_T("b.exe"));
	return;
	//当前只是默认 a.exe 和 b.exe ，命令行解析后待开发.
}
/*
程序交互：
输入地图信息，敌我坦克信息。
输出三阶段动作指令。
*/
/*
选手运行过程.
1.读入先后手.
2.选择阵营.
*/