#pragma once

#include "stdafx.h"

#include "Player.h"

class GameCore
{
public:
	GameCore();
	~GameCore();
	//传递选手Player对象指针.
	bool SetPlayer(Player*,Player*);
	//设置选手名称.
	bool SetPlayerName(const char*,const char*);
	//初始化选手程序状态.
	bool Initialize();
	//生成游戏初始信息.
	bool GenerateInitialData();
	//向选手程序分发初始信息.
	bool DistributeData();
	//选手A操作及逻辑反馈.
	bool PlayerAsTurn();
	//选手B操作及逻辑反馈.
	bool PlayerBsTurn();
	//判断游戏是否终止.
	bool IsGameOver();
	//输出游戏结果.
	bool ReportResult();

protected:
	//初始化坦克数据.
	void InitializeTank(int,Faction,TankType,Point,Tank&);
	//读入地图信息.
	void ReadInitialMap();
	//移动命令.
	int MoveTank(int,Move);
	//攻击命令.
	int AttackTank(int,Attack);
	//计算攻击伤害.
	int CalculateDamage(Tank&,Tank&,int&);
	//重坦碾压特效实现.
	void HeavyTankSkill();
	//写入视频文件.
	void WriteVideo_Move(int,Point);
	void WriteVideo_Attack(int,int,int,int,int);
	//写入文字战报.
	void WriteComment_Move(int,Point);
	void WriteComment_Attack(int,int,int,int,int);
	//写入警告.
	void WriteWarning(int,const char*);
	//判断坐标是否合法.
	inline bool IsInMap(int,int);
private:
	//游戏回合.
	int      round;
	//游戏地图.
	Landform map[8][8];
	//A方阵营.
	Faction  facA;
	//B方阵营.
	Faction  facB;
	//选手A方坦克信息.
	Tank     tankA[5];
	//选手B方坦克信息.
	Tank     tankB[5];
	//A方基地血量.
	int      baseHpA;
	//B方基地血量.
	int      baseHpB;
	//选手A得分.
	int      scoreA;
	float    valueA;
	//选手B得分.
	int      scoreB;
	float    valueB;
	//记录最后一条命令.
	Command  lastCommand;
	//最后一条命令执行结果.
	CmdRpt   lastCommandReport;
	//移动状态变量.
	bool     moved[10];
	//坦克状态变量.
	bool     controled[10];
	//选手对象.
	Player*  m_pPlayerA;
	Player*  m_pPlayerB;
	//选手名称.
	char     strNameA[128];
	char     strNameB[128];
	//视频输出文件对象.
	ofstream* fvideo;
	//文字战报输出文件对象.
	ofstream* fcomment;
	//文字警告输出文件对象.
	ofstream* fwarning;
	//错误代码.
	int m_nErrorCode;
	int m_nLastRound1;
	int m_nLastRound2;
};
