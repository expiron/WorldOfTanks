#pragma once

#include <iostream>
#include <strstream>
using namespace std;

/*******************************结构体定义******************************/
enum Faction             //坦克阵营变量.
{
	Axis   = 1,          //轴心国.
	Allies = 2           //同盟国.
};

enum TankType            //坦克类型.
{
	heavy = 1,           //重型坦克.
	light,               //轻型坦克.
	middle,              //中型坦克.
	selfPropelled,       //自行火炮.
	antitank             //反坦克炮.
};

enum Landform            //地形定义.
{
	border = 0,          //边界.
	base,                //基地.
	plain,               //平原.
	hill,                //山地.
	forest               //树林.
};

struct Point             //坐标点.
{
	int x;
	int y;

	Point();
	Point(int,int);
	bool operator == (const Point&);
};

struct Tank              //坦克信息.
{
	int nID;             //坦克唯一标识符.
	TankType type;       //坦克类型.
	Point pt;            //坦克位置.
	float value;         //坦克价值.
	int atk;             //攻击力.
	int armour;          //护甲.
	int hp;              //耐久.
	int ap;              //穿甲值.

	Tank();
	//Tank(Tank&);
};

struct GameData          //游戏初始信息.
{
	int nID;             //nID == 1 先手,nID == 2 后手
	Faction fac;         //坦克阵营.
	Landform map[8][8];  //游戏地图地形信息.
	Tank myTank[5];      //我方坦克信息.
	Tank enemyTank[5];   //敌方坦克信息.
	int myBaseHP;        //我方基地血量.
	int enemyBaseHP;     //敌方基地血量.

	GameData();
};

struct Move              //坦克移动命令结构体.
{
	int nID;             //移动坦克ID.
	Point pt;            //目标位置.

	Move();
};

struct Attack            //坦克攻击命令结构体.
{
	int nID_Attacker;    //攻击坦克ID.
	int nID_Target;      //目标ID.(nID_Target == 10,攻击对方基地.)

	Attack();
};

struct Command           //选手命令.
{
	int nMove;           //移动坦克数量.
	enum {
		move = 1,
		attack = 2
	} ptc;               //机动命令.
	Move moveCommand[3]; //移动命令.
	Attack atkCommand[2];//攻击命令.

	Command();
};

struct CmdRpt            //命令反馈.
{
	int period1;
	int period2;
	int period3;

	CmdRpt();
};
/*******************************选手SDK********************************/
//计算地图两坐标点的距离.
int dis(Point,Point);
//判断先后手.
bool IsFirsthand();
//选择坦克阵营.
void ChooseFaction(Faction);
//读入游戏初始数据.
void InputGameData(GameData&);
//输出命令.
void OutputCommand(Command);
//读入命令反馈.
void InputCommandReport(CmdRpt&);
//读入敌人命及命令反馈.
void InputEnemyCommand(Command&,CmdRpt&);
//输出调试信息.
void OutputDebugInformation(const char*);

//IO函数重载.
istream& operator >> (istream&,Faction&);
istream& operator >> (istream&,TankType&);
istream& operator >> (istream&,Landform&);
istream& operator >> (istream&,Point&);
istream& operator >> (istream&,Tank&);
istream& operator >> (istream&,GameData&);
istream& operator >> (istream&,Move&);
istream& operator >> (istream&,Attack&);
istream& operator >> (istream&,Command&);
istream& operator >> (istream&,CmdRpt&);

ostream& operator << (ostream&,Faction&);
ostream& operator << (ostream&,TankType&);
ostream& operator << (ostream&,Landform&);
ostream& operator << (ostream&,Point&);
ostream& operator << (ostream&,Tank&);
ostream& operator << (ostream&,GameData&);
ostream& operator << (ostream&,Move&);
ostream& operator << (ostream&,Attack&);
ostream& operator << (ostream&,Command&);
ostream& operator << (ostream&,CmdRpt&);

/*
关于 Command:
选手每一回合会有三个阶段:

1.移动阶段 移动 1 辆或 2 辆坦克.
2.攻击阶段 攻击 1 辆坦克.
3.机动阶段 移动或攻击 1 辆坦克.

注意：变量 nMove 表示移动阶段要移动坦克的数量，故只能为 1 或 2;

     具体的移动命令存储在 moveCommand[0] 和 moveCommand[1] 结构体里;

	 攻击阶段的攻击命令存储在 atkCommand[0] 结构体里;

	 枚举变量 ptc 存储第三阶段（机动阶段）的具体动作;
	 相应的移动（攻击）命令存储在moveCommand 或 atkCommand;

	 程序在进行回合判定的时候依然是按照三个阶段来进行。

	 即当进行到攻击阶段时，移动阶段的命令已生效.
*/
/*
关于CmdRpt:
period1 = 0: 命令失败.（包括移动1辆失败和移动2辆都失败.）
		  1: 移动1辆坦克，成功。
		  2: 移动2辆坦克，第1辆成功，第2辆失败。
		  4: 移动2辆坦克，第2辆成功，第1辆失败。
		  6: 移动2辆坦克，都成功。
period2 = 0: 命令无效.（命令涉及到无法操作的坦克或逻辑上不允许的坦克;
                      这与坦克未命中不同，不会对目标坦克造成伤害.）
		  1: 命令有效，但攻击坦克目标未命中，造成伤害（若符合坦克特效发动条件，则自动
		     将特效伤害计算在内）
		  2: 命令有效，攻击坦克命中目标，造成伤害（若符合坦克特效发动条件，则自动将
		     特效伤害计算在内）
		  3: 当攻击坦克为反坦克炮时可能会返回此值，表示：命令有效，目标未命中，造成
		     伤害，但特效使目标坦克瘫痪触发。
		  4: 当攻击坦克为反坦克炮时可能会返回此值，表示：命令有效，目标命中，造成伤
		     害，但特效使目标坦克瘫痪触发。
注：1.period3 的返回值与period1和period2相应命令的返回值相同.
   2.攻击伤害的具体计算公式详见选手手册.
   3.反坦克炮攻击命中率与反坦克炮特效触发概率相互独立.也就是说反坦克炮可能未命中目标，
     但仍然有可能将目标坦克瘫痪.
*/
