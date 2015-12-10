
#include "Wot.h"

//Global Variables.
const int cDirection[6][2] = {{0,-1},{0,1},{-1,0},{1,0},{-1,1},{1,-1}};
int g_firsthand = 0;

Point::Point()
{
	x = 0; y = 0;
}
Point::Point(int tx,int ty)
{
	x = tx; y = ty;
}
bool Point::operator == (const Point& pt)
{
	if(pt.x == x && pt.y == y)
		return true;
	return false;
}
Tank::Tank()
{
	nID = -1; type = heavy;  pt = Point(0,0);  value = 0.0f;
	atk = 0;  armour = 0;    hp = 0;           ap = 0;
}
GameData::GameData()
{
	nID = -1; fac = Axis;
	for(int i = 0;i < 8;i++)
		for(int j = 0;j < 8;j++)
			map[i][j] = border;
	myBaseHP = enemyBaseHP = 0;
}
Move::Move()
{
	nID = -1; pt = Point(0,0);
}
Attack::Attack()
{
	nID_Attacker = nID_Target = -1;
}
Command::Command()
{
	nMove = 0; ptc = Command::move;
}
CmdRpt::CmdRpt()
{
	period1 = period2 = period3 = 0;
}
//计算地图两坐标点的距离.
int dis(Point p1,Point p2)
{
	if(p1 == p2)
		return 0;
	Point q[40];
	int d[40] = {0};
	int u[8][8] = {0};
	int start,end;
	start = 0;
	end = 1;
	q[start] = p1;
	u[p1.x][p1.y] = 1;
	while(start < end)
	{
		for(int i = 0;i < 6;i++)
		{
			int tx = q[start].x + cDirection[i][0];
			int ty = q[start].y + cDirection[i][1];
			if(InMap(tx,ty))
			{
				if(!u[tx][ty])
				{
					q[end] = Point(tx,ty);
					d[end++] = d[start] + 1;
					u[tx][ty] = 1;
					if(Point(tx,ty) == p2)
						return d[end - 1];
				}
			}
		}
		start++;
	}
	return 0x7FFFFFFF;
}
//判断点坐标是否在地图内.
bool InMap(int x,int y)
{
	if(x < 1 || x > 6)
		return false;
	if(y < 1 || y > 6)
		return false;
	return true;
}
//判断先后手.
bool IsFirsthand()
{
	if(g_firsthand)
		return (g_firsthand == 1 ? true : false);
	cin>>g_firsthand;
	return (g_firsthand == 1 ? true : false);
}
//选择坦克阵营.
void ChooseFaction(Faction fac)
{
	if(!g_firsthand)
		IsFirsthand();
	cout<<'{'<<'\n'
		<<fac<<endl
		<<'}'<<'\n';
}
//读入游戏初始数据.
void InputGameData(GameData& gdata)
{
	cin>>gdata;
}
//输出命令.
void OutputCommand(Command cmd)
{
	cout<<'{'<<'\n'
		<<cmd
		<<'}'<<'\n';
}
//读入敌人命及命令反馈.
void InputCommandReport(CmdRpt& rpt)
{
	cin>>rpt;
}
//读入敌人命令.
void InputEnemyCommand(Command& cmd,CmdRpt& rpt)
{
	cin>>cmd>>rpt;
}
//更新游戏数据.
void UpdateGameData(bool myself,GameData& gdata,Command cmd,CmdRpt rpt)
{
	if(myself)    //更新我方游戏数据.
	{
		HeavyTankSkill(gdata);    //重坦伤害特效.
		switch(rpt.period1)    //第一阶段
		{
		case 0: break;    //命令失败.

		case 1:{          //移动1辆坦克成功.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			gdata.myTank[i].pt = cmd.moveCommand[0].pt;} break;

		case 2:{          //第1辆成功,第2辆失败.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			gdata.myTank[i].pt = cmd.moveCommand[0].pt;} break;

		case 4:{          //第2辆成功,第1辆失败.
			int i = IDToSubscript(cmd.moveCommand[1].nID);
			gdata.myTank[i].pt = cmd.moveCommand[1].pt;} break;

		case 6:{          //移动2辆坦克都成功.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			int j = IDToSubscript(cmd.moveCommand[1].nID);
			gdata.myTank[i].pt = cmd.moveCommand[0].pt;
			gdata.myTank[j].pt = cmd.moveCommand[1].pt;} break;
		}

		HeavyTankSkill(gdata);    //重坦伤害特效.
		switch (rpt.period2)    //第二阶段
		{
		case 0: break;    //命令失败.

		case 1:{    //未命中.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			if(j != 10)
			{
				if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
					gdata.enemyTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
				else
					gdata.enemyTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
			}
			else
				gdata.enemyBaseHP--;

			if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
			{
				gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			if(gdata.enemyBaseHP < 0)
				gdata.enemyBaseHP = 0;    //基地跪了.
			   } break;

		case 2:{    //命中.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
				gdata.enemyTank[IDToSubscript(j)].hp -= 1;    //特效伤害
			if(j != 10)
				gdata.enemyTank[IDToSubscript(j)].hp -= gdata.myTank[IDToSubscript(i)].atk;    //炮击伤害
			else
				gdata.enemyBaseHP -= gdata.myTank[IDToSubscript(i)].atk;    //攻击基地

			if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
			{
				gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			if(gdata.enemyBaseHP < 0)
				gdata.enemyBaseHP = 0;    //基地跪了.
			   } break;

		case 3:{    //触发瘫痪特效.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			/*if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
				gdata.enemyTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
			else*/
			gdata.enemyTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
			gdata.enemyTank[IDToSubscript(j)].paralyzed = true;    //瘫痪坦克.

			if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
			{
				gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			   } break;

		case 4:{    //命中且触发瘫痪特效.
			int j = cmd.atkCommand[0].nID_Target;
			gdata.enemyTank[IDToSubscript(j)].hp -= gdata.myTank[myAnti].atk;    //造成伤害.
			gdata.enemyTank[IDToSubscript(j)].paralyzed = true;

			if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
			{
				gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			   };break;
		}

		HeavyTankSkill(gdata);    //重坦伤害特效.
		if(cmd.ptc == Command::move)    //第三阶段.
		{
			switch(rpt.period3)
			{
			case 0: break;    //命令失败.

			case 1:{          //移动1辆坦克成功.
				int i = IDToSubscript(cmd.moveCommand[cmd.nMove].nID);
				gdata.myTank[i].pt = cmd.moveCommand[cmd.nMove].pt;} break;
			}
		}
		else
		{
			switch (rpt.period3)
			{
			case 0: break;    //命令失败.

			case 1:{    //未命中.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				if(j != 10)
				{
					if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
						gdata.enemyTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
					else
						gdata.enemyTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
				}
				else
					gdata.enemyBaseHP--;

				if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
				{
					gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				if(gdata.enemyBaseHP < 0)
					gdata.enemyBaseHP = 0;    //基地跪了.
				   } break;

			case 2:{    //命中.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
					gdata.enemyTank[IDToSubscript(j)].hp -= 1;    //特效伤害
				if(j != 10)
					gdata.enemyTank[IDToSubscript(j)].hp -= gdata.myTank[IDToSubscript(i)].atk;    //炮击伤害
				else
					gdata.enemyBaseHP -= gdata.myTank[IDToSubscript(i)].atk;    //攻击基地

				if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
				{
					gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				if(gdata.enemyBaseHP < 0)
					gdata.enemyBaseHP = 0;    //基地跪了.
				   } break;

			case 3:{    //触发瘫痪特效.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				/*if((i == myLight && j == enemySlfPro) || (i == myMiddle && j == enemyHeavy))
					gdata.enemyTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
				else*/
				gdata.enemyTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
				gdata.enemyTank[IDToSubscript(j)].paralyzed = true;    //瘫痪坦克.

				if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
				{
					gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				   } break;

			case 4:{    //命中且触发瘫痪特效.
				int j = cmd.atkCommand[1].nID_Target;
				gdata.enemyTank[IDToSubscript(j)].hp -= gdata.myTank[myAnti].atk;    //造成伤害.
				gdata.enemyTank[IDToSubscript(j)].paralyzed = true;

				if(gdata.enemyTank[IDToSubscript(j)].hp < 0)
				{
					gdata.enemyTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.enemyTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				   };break;
			}
		}

		for(int i = 0;i < 5;i++)    //恢复己方瘫痪坦克.
			gdata.myTank[i].paralyzed = false;
	}
	else    //更新敌方游戏数据.
	{
		HeavyTankSkill(gdata);    //重坦伤害特效.
		switch(rpt.period1)    //第一阶段
		{
		case 0: break;    //命令失败.

		case 1:{          //移动1辆坦克成功.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			gdata.enemyTank[i].pt = cmd.moveCommand[0].pt;} break;

		case 2:{          //第1辆成功,第2辆失败.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			gdata.enemyTank[i].pt = cmd.moveCommand[0].pt;} break;

		case 4:{          //第2辆成功,第1辆失败.
			int i = IDToSubscript(cmd.moveCommand[1].nID);
			gdata.enemyTank[i].pt = cmd.moveCommand[1].pt;} break;

		case 6:{          //移动2辆坦克都成功.
			int i = IDToSubscript(cmd.moveCommand[0].nID);
			int j = IDToSubscript(cmd.moveCommand[1].nID);
			gdata.enemyTank[i].pt = cmd.moveCommand[0].pt;
			gdata.enemyTank[j].pt = cmd.moveCommand[1].pt;} break;
		}

		HeavyTankSkill(gdata);    //重坦伤害特效.
		switch (rpt.period2)    //第二阶段
		{
		case 0: break;    //命令失败.

		case 1:{    //未命中.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			if(j != 10)
			{
				if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
					gdata.myTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
				else
					gdata.myTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
			}
			else
				gdata.myBaseHP--;

			if(gdata.myTank[IDToSubscript(j)].hp < 0)
			{
				gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			if(gdata.myBaseHP < 0)
				gdata.myBaseHP = 0;    //基地跪了.
			   } break;

		case 2:{    //命中.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
				gdata.myTank[IDToSubscript(j)].hp -= 1;    //特效伤害
			if(j != 10)
				gdata.myTank[IDToSubscript(j)].hp -= gdata.enemyTank[IDToSubscript(i)].atk;    //炮击伤害
			else
				gdata.myBaseHP -= gdata.enemyTank[IDToSubscript(i)].atk;    //攻击基地

			if(gdata.myTank[IDToSubscript(j)].hp < 0)
			{
				gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			if(gdata.myBaseHP < 0)
				gdata.myBaseHP = 0;    //基地跪了.
			   } break;

		case 3:{    //触发瘫痪特效.
			int i = cmd.atkCommand[0].nID_Attacker;
			int j = cmd.atkCommand[0].nID_Target;
			/*if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
				gdata.myTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
			else*/
			gdata.myTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
			gdata.myTank[IDToSubscript(j)].paralyzed = true;    //瘫痪坦克.

			if(gdata.myTank[IDToSubscript(j)].hp < 0)
			{
				gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			   } break;

		case 4:{    //命中且触发瘫痪特效.
			int j = cmd.atkCommand[0].nID_Target;
			gdata.myTank[IDToSubscript(j)].hp -= gdata.enemyTank[enemyAnti].atk;    //造成伤害.
			gdata.myTank[IDToSubscript(j)].paralyzed = true;

			if(gdata.myTank[IDToSubscript(j)].hp < 0)
			{
				gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
				gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
			}
			   } break;
		}

		HeavyTankSkill(gdata);    //重坦伤害特效.
		if(cmd.ptc == Command::move)    //第三阶段.
		{
			switch(rpt.period3)
			{
			case 0: break;    //命令失败.

			case 1:{          //移动1辆坦克成功.
				int i = IDToSubscript(cmd.moveCommand[cmd.nMove].nID);
				gdata.enemyTank[i].pt = cmd.moveCommand[cmd.nMove].pt;} break;
			}
		}
		else
		{
			switch (rpt.period3)
			{
			case 0: break;    //命令失败.

			case 1:{    //未命中.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				if(j != 10)
				{
					if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
						gdata.myTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
					else
						gdata.myTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
				}
				else
					gdata.myBaseHP--;

				if(gdata.myTank[IDToSubscript(j)].hp < 0)
				{
					gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				if(gdata.myBaseHP < 0)
					gdata.myBaseHP = 0;    //基地跪了.
				   } break;

			case 2:{    //命中.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
					gdata.myTank[IDToSubscript(j)].hp -= 1;    //特效伤害
				if(j != 10)
					gdata.myTank[IDToSubscript(j)].hp -= gdata.enemyTank[IDToSubscript(i)].atk;    //炮击伤害
				else
					gdata.myBaseHP -= gdata.enemyTank[IDToSubscript(i)].atk;    //攻击基地

				if(gdata.myTank[IDToSubscript(j)].hp < 0)
				{
					gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				if(gdata.myBaseHP < 0)
					gdata.myBaseHP = 0;    //基地跪了.
				   } break;

			case 3:{    //触发瘫痪特效.
				int i = cmd.atkCommand[1].nID_Attacker;
				int j = cmd.atkCommand[1].nID_Target;
				/*if((i == enemyLight && j == mySlfPro) || (i == enemyMiddle && j == myHeavy))
					gdata.myTank[IDToSubscript(j)].hp -= 2;    //特效伤害.
				else*/
				gdata.myTank[IDToSubscript(j)].hp--;    //普通未命中伤害.
				gdata.myTank[IDToSubscript(j)].paralyzed = true;    //瘫痪坦克.

				if(gdata.myTank[IDToSubscript(j)].hp < 0)
				{
					gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				   } break;

			case 4:{    //命中且触发瘫痪特效.
				int j = cmd.atkCommand[1].nID_Target;
				gdata.myTank[IDToSubscript(j)].hp -= gdata.enemyTank[enemyAnti].atk;    //造成伤害.
				gdata.myTank[IDToSubscript(j)].paralyzed = true;

				if(gdata.myTank[IDToSubscript(j)].hp < 0)
				{
					gdata.myTank[IDToSubscript(j)].hp = 0;    //坦克跪了.
					gdata.myTank[IDToSubscript(j)].pt = Point(-6,-6);    //位置归零.
				}
				   } break;
			}
		}

		for(int i = 0;i < 5;i++)    //恢复敌方瘫痪坦克.
			gdata.enemyTank[i].paralyzed = false;
	}
}
//重坦特效伤害.
void HeavyTankSkill(GameData& gdata)
{
	if(gdata.myTank[0].hp > 0 && !gdata.myTank[0].paralyzed)
	{
		Point pt = gdata.myTank[0].pt;
		for(int i = 0;i < 5;i++)
			if(gdata.enemyTank[i].hp > 0 && gdata.enemyTank[i].pt == pt)
				gdata.enemyTank[i].hp--;
	}
	if(gdata.enemyTank[0].hp > 0 && !gdata.enemyTank[0].paralyzed)
	{
		Point pt = gdata.enemyTank[0].pt;
		for(int i = 0;i < 5;i++)
			if(gdata.myTank[i].hp > 0 && gdata.myTank[i].pt == pt)
				gdata.myTank[i].hp--;
	}
}

istream& operator >> (istream& i,Faction& faction)
{
	int fac; i>>fac; faction = Axis;
	if(fac == 2)
		faction = Allies;
	return i;
}
istream& operator >> (istream& i,TankType& tanktype)
{
	int type; i>>type;
	switch(type)
	{
	case 2: tanktype = light;         break;
	case 3: tanktype = middle;        break;
	case 4: tanktype = selfPropelled; break;
	case 5: tanktype = antitank;      break;
	default: tanktype = heavy;        break;
	}
	return i;
}
istream& operator >> (istream& i,Landform& landform)
{
	int form; i>>form;
	switch(form)
	{
	case 1: landform = base;   break;
	case 2: landform = plain;  break;
	case 3: landform = hill;   break;
	case 4: landform = forest; break;
	default:landform = border; break;
	}
	return i;
}
istream& operator >> (istream& i,Point& pt)
{
	i>>pt.x>>pt.y; return i;
}
istream& operator >> (istream& i,Tank& t)
{
	i>>t.nID>>t.type>>t.pt>>t.value
	 >>t.atk>>t.armour>>t.hp>>t.ap;
	return i;
}
istream& operator >> (istream& i,GameData& d)
{
	i>>d.nID>>d.fac;
	for(int j = 0;j < 8;j++)
		for(int k = 0;k < 8;k++)
			i>>d.map[j][k];
	for(int j = 0;j < 5;j++)
		i>>d.myTank[j];
	for(int j = 0;j < 5;j++)
		i>>d.enemyTank[j];
	i>>d.myBaseHP>>d.enemyBaseHP;
	return i;
}
istream& operator >> (istream& i,Move& m)
{
	i>>m.nID>>m.pt;
	return i;
}
istream& operator >> (istream& i,Attack& a)
{
	i>>a.nID_Attacker>>a.nID_Target;
	return i;
}
istream& operator >> (istream& i,Command& c)
{
	int ptc;
	i>>c.nMove>>ptc
	 >>c.moveCommand[0]>>c.moveCommand[1]>>c.moveCommand[2]
	>>c.atkCommand[0]>>c.atkCommand[1];
	c.ptc = (ptc == 2 ? Command::attack : Command::move);
	return i;
}
istream& operator >> (istream& i,CmdRpt& r)
{
	i>>r.period1>>r.period2>>r.period3;
	return i;
}

ostream& operator << (ostream& o,Faction& f)
{
	if(f == Axis)
		o<<1<<' ';
	if(f == Allies)
		o<<2<<' ';
	return o;
}
ostream& operator << (ostream& o,TankType& type)
{
	switch(type)
	{
	case heavy:         o<<1<<' '; break;
	case light:         o<<2<<' '; break;
	case middle:        o<<3<<' '; break;
	case selfPropelled: o<<4<<' '; break;
	case antitank:      o<<5<<' '; break;
	};
	return o;
}
ostream& operator << (ostream& o,Landform& l)
{
	switch(l)
	{
	case border: o<<0<<' '; break;
	case base:   o<<1<<' '; break;
	case plain:  o<<2<<' '; break;
	case hill:   o<<3<<' '; break;
	case forest: o<<4<<' '; break;
	};
	return o;
}
ostream& operator << (ostream& o,Point& pt)
{
	o<<pt.x<<' '<<pt.y<<' ';
	return o;
}
ostream& operator << (ostream& o,Tank& t)
{
	o<<t.nID<<' '<<t.type<<t.pt<<t.value<<' '
	 <<t.atk<<' '<<t.armour<<' '<<t.hp<<' '<<t.ap<<endl;
	return o;
}
ostream& operator << (ostream& o,GameData& d)
{
	o<<d.nID<<' '<<d.fac<<endl;
	for(int j = 0;j < 8;j++)
	{
		for(int k = 0;k < 8;k++)
			o<<d.map[j][k];
		o<<endl;
	}
	for(int j = 0;j < 5;j++)
		o<<d.myTank[j];
	for(int j = 0;j < 5;j++)
		o<<d.enemyTank[j];
	o<<d.myBaseHP<<' '<<d.enemyBaseHP<<endl;
	return o;
}
ostream& operator << (ostream& o,Move& m)
{
	o<<m.nID<<' '<<m.pt<<endl;
	return o;
}
ostream& operator << (ostream& o,Attack& a)
{
	o<<a.nID_Attacker<<' '<<a.nID_Target<<endl;
	return o;
}
ostream& operator << (ostream& o,Command& c)
{
	o<<c.nMove<<' ';
	o<<(c.ptc == Command::attack ? 2 : 1)<<' ';
	o<<c.moveCommand[0]<<c.moveCommand[1]<<c.moveCommand[2]
	 <<c.atkCommand[0]<<c.atkCommand[1];
	return o;
}
ostream& operator << (ostream& o,CmdRpt& r)
{
	o<<r.period1<<' '<<r.period2<<' '<<r.period3<<endl;
	return o;
}
