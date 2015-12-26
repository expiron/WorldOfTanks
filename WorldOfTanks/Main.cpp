#include "stdafx.h"

#include <cmath>
#include <cstdio>
#include <algorithm>

#include "WorldOfTanks.h"

#include "PlayerApp.h"
#include "GameCore.h"

using namespace std;

struct Player    //竞赛Player结构定义.
{
	char pszName[32];    //名称.
	double score;        //rank分.
    int    change;       //变化.

    Player()
    {
        ZeroMemory(pszName,32);
        score = 1500.0f;
        change = 0;
    }
};
struct Match
{
	Player* pa;
	Player* pb;
	int ret[20];
};

//命令行解析.
void AnalyseCommandLine(int,TCHAR**,TCHAR*,TCHAR*,char*,char*);
//初始化比赛.
bool InitializeMatch(Player*,int&);
//生成对决名单.
void GenerateMatchList(Match*,Player*,int,int&);
//选手循环赛.
void StartMatch(int,Match*,Player*,int,int,TCHAR**);
//显示Rank分.
void ReportRank(Player*,int);
//写入CSV文件.
void WriteMatchCSV(Match*,int,int);
//比赛主函数.
int JudgeMain(int argc, TCHAR* argv[], TCHAR* envp[],int);
//拷贝视频文件.
void CopyFile(Player*);

void Main(int argc,TCHAR* argv[],TCHAR* envp[])
{
	int nPlayers = 0;
	int nMatch = 0;
	Player* plist;
	Match* mlist;
	plist = new Player[32];
	//Read Players.
	if(!InitializeMatch(plist,nPlayers))
	{
		cerr<<"Error in reading player list!"<<endl;
		return;
	}
	//Generate Match List.
	mlist = new Match[nPlayers * (nPlayers - 1)];
	GenerateMatchList(mlist,plist,nPlayers,nMatch);    //生成对决名单.
	
	//Match in randomization.
	if(nPlayers != 2)
	{
	for(int i = 0;i < 4;i++)
		StartMatch(i,mlist,plist,nPlayers,nMatch,envp);    //开始比赛.
	}
	else
		StartMatch(0,mlist,plist,nPlayers,nMatch,envp);
	ReportRank(plist,nPlayers);
	WriteMatchCSV(mlist,nMatch,5);

	cout<<"  ====================================End=====================================  "<<endl;
	CopyFile(plist);
	getchar();
	//system("pause");
	return;
}
//选手循环赛.
void StartMatch(int round,Match* mlist,Player* p,int n,int nMatch,TCHAR* envp[])
{
	TCHAR* argv[3];
	int nRet;
	bool* matched;
	double scoreA,scoreB;
	double scoreAold,scoreBold;
	double ea,eb;
	int seed = 2;
	for(int i = 0;i < 3;i++)
		argv[i] = new TCHAR[32];
	ZeroMemory(argv[0],32);
	matched = new bool[nMatch];
	ZeroMemory(matched,sizeof(bool)*nMatch);
	for(int m = 0;m < nMatch;m++)
	{
		int k = 0;
		srand(time(0)/seed);
		do
		{
			k = rand()%nMatch;
		}while(matched[k]);
		seed = ((seed+1) > 80 ? 2 : seed + 1);
		ZeroMemory(argv[1],32);
		ZeroMemory(argv[2],32);
		Player* a = mlist[k].pa;
		Player* b = mlist[k].pb;
		MultiByteToWideChar(CP_ACP,0,a->pszName,-1,argv[1],32);
		MultiByteToWideChar(CP_ACP,0,b->pszName,-1,argv[2],32);
		ReportRank(p,n);
		for(int i = 0;i < n;i++)
			p[i].change = 0;
		nRet = JudgeMain(3,argv,envp,n);
		switch(nRet)
		{
		case 0:
			break;
		case 1:
			scoreAold = scoreA = a->score;
			scoreBold = scoreB = b->score;
			ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
			eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
			scoreA = scoreA + 32.0 * (1.0 - ea);
			scoreB = scoreB + 32.0 * (0 - eb);
			a->score = scoreA;
			b->score = scoreB;
			//a->change = 1;
			//b->change = 2;
			a->change = (scoreAold < scoreA ? 1 : (scoreAold > scoreA ? 2 : 0));
			b->change = (scoreBold < scoreB ? 1 : (scoreBold > scoreB ? 2 : 0));
			break;
		case 2:
			scoreAold = scoreA = a->score;
			scoreBold = scoreB = b->score;
			ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
			eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
			scoreA = scoreA + 32.0 * (0 - ea);
			scoreB = scoreB + 32.0 * (1.0 - eb);
			a->score = scoreA;
			b->score = scoreB;
			//a->change = 2;
			//b->change = 1;
			a->change = (scoreAold < scoreA ? 1 : (scoreAold > scoreA ? 2 : 0));
			b->change = (scoreBold < scoreB ? 1 : (scoreBold > scoreB ? 2 : 0));
			break;
		case 3:
			scoreAold = scoreA = a->score;
			scoreBold = scoreB = b->score;
			ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
			eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
			scoreA = scoreA + 32.0 * (0.5 - ea);
			scoreB = scoreB + 32.0 * (0.5 - eb);
			a->score = scoreA;
			b->score = scoreB;
			//a->change = 0;
			//b->change = 0;
			a->change = (scoreAold < scoreA ? 1 : (scoreAold > scoreA ? 2 : 0));
			b->change = (scoreBold < scoreB ? 1 : (scoreBold > scoreB ? 2 : 0));
			break;
		}
		system("cls");
		mlist[k].ret[round] = nRet;
		matched[k] = true;
	}

	return;
	//for(int i = 0;i < n;i++)
	//	for(int j = 0;j < n;j++)
	for(int i = n-1;i >= 0;i--)
		for(int j = n-1;j >= 0;j--)
		{
			if(i == j)
				continue;
			ZeroMemory(argv[1],32);
			ZeroMemory(argv[2],32);
			MultiByteToWideChar(CP_ACP,0,p[i].pszName,-1,argv[1],32);
			MultiByteToWideChar(CP_ACP,0,p[j].pszName,-1,argv[2],32);
			ReportRank(p,n);
			nRet = JudgeMain(3,argv,envp,n);
			switch(nRet)
			{
			case 0:
				break;
			case 1:
				scoreA = p[i].score;
				scoreB = p[j].score;
				ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
				eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
				scoreA = scoreA + 32.0 * (1.0 - ea);
				scoreB = scoreB + 32.0 * (0 - eb);
				p[i].score = scoreA;
				p[j].score = scoreB;
				break;
			case 2:
				scoreA = p[i].score;
				scoreB = p[j].score;
				ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
				eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
				scoreA = scoreA + 32.0 * (0 - ea);
				scoreB = scoreB + 32.0 * (1.0 - eb);
				p[i].score = scoreA;
				p[j].score = scoreB;
				break;
			case 3:
				scoreA = p[i].score;
				scoreB = p[j].score;
				ea = (1/(1 + pow(10,(scoreB - scoreA) / 400)));
				eb = (1/(1 + pow(10,(scoreA - scoreB) / 400)));
				scoreA = scoreA + 32.0 * (0.5 - ea);
				scoreB = scoreB + 32.0 * (0.5 - eb);
				p[i].score = scoreA;
				p[j].score = scoreB;
				break;
			}
			system("cls");
		}
}
//显示Rank分.
bool ScoreComp(Player a,Player b)
{
	if(a.score > b.score)
		return true;
	else
		return false;
}
void ReportRank(Player* p,int n)
{
	cout<<"  =================================PlayerRank=================================  "<<endl;
	sort(p,p + n,ScoreComp);
	for(int i = 0;i < n;i++)
		cout<<setw(26)<<p[i].pszName<<' '
			<<setw(36)<<setprecision(2)<<std::fixed<<p[i].score
			<<(p[i].change == 0 ? " -" : (p[i].change == 1 ? "    ↑" : "  ↓"))<<endl;
}
//写入CSV文件.
void WriteMatchCSV(Match* mlist,int nMatch,int nRound)
{
	ofstream* fout;
	fout = new ofstream("MatchResult.csv");
	for(int i = 0;i < nMatch;i++)
	{
		(*fout)<<mlist[i].pa->pszName<<" vs "<<mlist[i].pb->pszName<<',';
		for(int j = 0;j < nRound;j++)
			(*fout)<<(mlist[i].ret[j] == 1 ? "A胜" : (mlist[i].ret[j] == 2 ? "B胜" : "平局"))<<',';
		(*fout)<<endl;
	}
	fout->close();
	delete fout;
}
//生成对决名单.
void GenerateMatchList(Match* mlist,Player* plist,int n,int& nMatch)
{
	for(int i = 0;i < n;i++)
		for(int j = 0;j < n;j++)
		{
			if(i == j)
				continue;
			(*mlist).pa = plist + i;
			(*mlist).pb = plist + j;
			ZeroMemory((*mlist).ret,sizeof(int)*20);
			mlist++;
			nMatch++;
		}
}
//读取选手清单.
bool InitializeMatch(Player* p,int& n)
{
	//Read Players.
	ifstream* flist = new ifstream("Players.list");
	if(flist->fail())
		return false;

	char tmp[64];
	n = 0;
	while(!flist->eof())
	{
		ZeroMemory(tmp,64);
		flist->getline(tmp,64);
		strcpy(p[n].pszName,tmp);
		p[n++].score = 1500;
	}
	n--;

	flist->close();
	delete flist;
	return true;
}
//拷贝视频文件.
void CopyFile(Player* p)
{
	char str1[1024] = "Copy ";
	char str2[1024] = "Copy ";

	strcat(str1,p[0].pszName);
	strcat(str1,"_vs_");
	strcat(str1,p[1].pszName);
	strcat(str1,"_video.txt .\\Player\\>nul");

	strcat(str2,p[1].pszName);
	strcat(str2,"_vs_");
	strcat(str2,p[0].pszName);
	strcat(str2,"_video.txt .\\Player\\>nul");
	//cout<<str1<<endl<<str2<<endl;
	//cout<<p[0].pszName<<endl<<p[1].pszName<<endl;
	//system("@echo off");
	system(str1);
	system(str2);
	ofstream fout("Players.list");
	fout<<p[0].pszName<<endl
		<<p[1].pszName<<endl;
	fout.close();
}
int JudgeMain(int argc, TCHAR* argv[], TCHAR* envp[],int n)
{
	PlayerApp playerA,playerB;
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
	coo.Y = 6 + n;

	AnalyseCommandLine(argc,argv,pszCommand1,pszCommand2,pszPlayerNameA,pszPlayerNameB);
	/*AfxMessageBox(pszCommand1);
	AfxMessageBox(pszCommand2);*/
	cout<<setw(38)<<pszPlayerNameA<<" vs "<<pszPlayerNameB<<'\n'<<endl;

	//return 0;

	cout<<"  Initializing...";
	if(!playerA.Initialize(pszCommand1) || !playerB.Initialize(pszCommand2))
	{
		cerr<<"  启动选手程序失败！(0x0001)"<<endl;
		goto ReleaseResource;
	}

	gameCore.SetPlayer(&playerA,&playerB);
	gameCore.SetPlayerName(pszPlayerNameA,pszPlayerNameB);

	if(!gameCore.Initialize())
	{
		cerr<<"  游戏状态初始化失败!(0x0002)"<<endl;
		goto ReleaseResource;
	}
	cout<<"OK!"<<endl;

	gameCore.GenerateInitialData();
	gameCore.DistributeData();

	cout<<"  The game is running....."<<endl;
	cout<<"  Round.01..."<<endl;
	int round = 1;

	while( true )
	{
		coo.X = 8;
		FillConsoleOutputCharacterA(hOut,round / 10 + 48,1,coo,&dwSize);
		coo.X = 9;
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
	cout<<"  The report:"<<endl;
	return gameCore.ReportResult();
	::Sleep(1000);
	//Cleaning.
ReleaseResource:
	delete []pszCommand1;
	delete []pszCommand2;
	delete []pszPlayerNameA;
	delete []pszPlayerNameB;
	return 0;
}
//分析命令行参数，解析生成选手程序的启动命令行.
void AnalyseCommandLine(int argc,TCHAR* argv[],TCHAR* psz1,TCHAR* psz2,char* name1,char* name2)
{
	int iLength;
	if(argc >= 3)
	{
		lstrcpy(psz1,argv[1]);
		iLength = WideCharToMultiByte(CP_ACP,0,psz1,-1,NULL,0,NULL,NULL);
		WideCharToMultiByte(CP_ACP,0,psz1,-1,name1,iLength,NULL,NULL);

		lstrcpy(psz2,argv[2]);
		iLength = WideCharToMultiByte(CP_ACP,0,psz2,-1,NULL,0,NULL,NULL);
		WideCharToMultiByte(CP_ACP,0,psz2,-1,name2,iLength,NULL,NULL);

		lstrcat(psz1,_T(" -m"));
		lstrcat(psz2,_T(" -m"));
	}
	else
	{
		lstrcpy(psz1,_T("a"));
		strcpy(name1,"a");
		lstrcpy(psz2,_T("b"));
		strcpy(name2,"b");
	}

	return;
}
