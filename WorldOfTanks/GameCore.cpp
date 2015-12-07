#include "stdafx.h"

#include "GameCore.h"

using namespace std;

const int nDirVector[6][2] = {{0,-1},{0,1},{-1,0},{1,0},{-1,1},{1,-1}};

GameCore::GameCore()
{
	m_pPlayerA = NULL;
	m_pPlayerB = NULL;
	fvideo = NULL;
	fcomment = NULL;
	fwarning = NULL;
	ZeroMemory(strNameA,128);
	ZeroMemory(strNameB,128);
	m_nErrorCode = 0;
	nError = 0;
}
GameCore::~GameCore()
{
	if(fvideo)
	{
		fvideo->close();
		delete fvideo;
	}
	if(fcomment)
	{
		fcomment->close();
		delete fcomment;
	}
	if(fwarning)
	{
		fwarning->close();
		delete fwarning;
	}
}
bool GameCore::SetPlayer(Player* pA,Player* pB)
{
	if(pA != NULL && pB != NULL)
	{
		if(!m_pPlayerA && !m_pPlayerB)
		{
			m_pPlayerA = pA; m_pPlayerB = pB;
			return true;
		}
	}
	return false;
}
bool GameCore::SetPlayerName(const char* pstr1,const char* pstr2)
{
	if(!m_pPlayerA || !m_pPlayerB)
	{
		m_nErrorCode = 1;    //Player pointer is NULL!
		return false;
	}
	if(fvideo || fcomment || fwarning)
	{
		m_nErrorCode = 9;    //File pointer is exist!
		return false;
	}

	m_pPlayerA->SetName(pstr1);
	m_pPlayerB->SetName(pstr2);
	strcpy(strNameA,pstr1);
	strcpy(strNameB,pstr2);
	char tmp[256];

	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,pstr1);
	strcat(tmp,"_vs_");
	strcat(tmp,pstr2);
	strcat(tmp,"_video.txt");
	fvideo = new ofstream(tmp);
	if(!fvideo)
		return false;

	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,pstr1);
	strcat(tmp,"_vs_");
	strcat(tmp,pstr2);
	strcat(tmp,"_comment.txt");
	fcomment = new ofstream(tmp);
	if(!fcomment)
		return false;

	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,pstr1);
	strcat(tmp,"_vs_");
	strcat(tmp,pstr2);
	strcat(tmp,"_warning.txt");
	fwarning = new ofstream(tmp);
	if(!fwarning)
		return false;

	return true;
}
bool GameCore::Initialize()
{
	if(!m_pPlayerA || !m_pPlayerB)
	{
		m_nErrorCode = 1;    //Player pointer is NULL!
		return false;
	}
	if(!m_pPlayerA->WriteFirsthand(true) || !m_pPlayerB->WriteFirsthand(false))
	{
		cerr<<"先后手写入失败!"<<endl;
		m_nErrorCode = 2;    //Writing firsthand failed!
		return false;
	}
	if(!m_pPlayerA->ChooseFaction(facA) || !m_pPlayerB->ChooseFaction(facB))
	{
		cerr<<"选择阵营出错！"<<endl;
		m_nErrorCode = 3;    //Writing faction failed!
		return false;
	}
	return true;
}
bool GameCore::GenerateInitialData()
{
	//按阵营生成坦克数据.
	for(int i = 0;i < 5;i++)
		InitializeTank(i,facA,TankType(i + 1),Point(1,1),tankA[i]);

	for(int i = 0;i < 5;i++)
		InitializeTank(i + 5,facB,TankType(i + 1),Point(6,6),tankB[i]);

	//读入地图.
	ZeroMemory(map,sizeof(Landform)*8*8);
	ReadInitialMap();
	/*for(int i =0;i<8;i++)
	{
		for(int j = 0;j<8;j++)
			cout<<map[i][j];
		cout<<endl;
	}*/
	//双方基地血量.
	baseHpA = baseHpB = 6;
	//回合数置零.
	round = 0;
	m_nLastRound1 = 0;
	m_nLastRound2 = 0;
	//双方得分归零.
	scoreA = scoreB = 0;
	valueA = valueB = 12.0;
	lifeA = lifeB = 5;
	//重置坦克状态.
	memset(moved,0,sizeof(moved));
	memset(controled,0,sizeof(controled));
	//种子散列值初始化.
	hseed = 2;
	return true;
}
bool GameCore::DistributeData()
{
	GameData gdata;

	//playerA
	gdata.nID = 1;
	gdata.fac = facA;
	memcpy(gdata.map,map,sizeof(map));
	memcpy(gdata.myTank,tankA,sizeof(tankA));
	memcpy(gdata.enemyTank,tankB,sizeof(tankB));
	gdata.myBaseHP = gdata.enemyBaseHP = 6;
	if(!m_pPlayerA->WriteGameData(gdata))
	{
		m_nErrorCode = 4;    //Writing GameData failed!(A)
		return false;
	}
	
	//playerB
	gdata.nID = 2;
	gdata.fac = facB;
	memcpy(gdata.map,map,sizeof(map));
	memcpy(gdata.myTank,tankB,sizeof(tankA));
	memcpy(gdata.enemyTank,tankA,sizeof(tankB));
	gdata.myBaseHP = gdata.enemyBaseHP = 6;
	if(!m_pPlayerB->WriteGameData(gdata))
	{
		m_nErrorCode = 5;    //Writing GameData failed!(B)
		return false;
	}

	//Write Video File.
	(*fvideo)<<strNameA<<endl
		<<strNameB<<endl;
	(*fvideo)<<facA<<facB<<endl;
	for(int i = 0;i < 8;i++)
	{
		for(int j = 0;j < 8;j++)
			(*fvideo)<<map[i][j];
		(*fvideo)<<endl;
	}
	for(int i = 0;i < 5;i++)
		(*fvideo)<<tankA[i];
	for(int i = 0;i < 5;i++)
		(*fvideo)<<tankB[i];
	(*fvideo)<<baseHpA<<' '<<baseHpB<<endl;

	return true;
}
bool GameCore::PlayerAsTurn()
{
	round++;
	(*fvideo)<<1003<<' '<<1<<endl;
	//cout.fill('0');
	//cout<<"Round "<<setw(2)<<round<<'.'<<endl;
	Command cmd;
	CmdRpt  rpt;

	if(round != 1)
		m_pPlayerA->WriteLastCommand(lastCommand,lastCommandReport);
	
	if(!m_pPlayerA->ReadCommand(cmd))
	{
		//WriteVideo_End(round,2);
		WriteWarning(1,"AI程序异常结束！");
		SetError(1);
		return false;
	}
	
	//Game Logic Processing...

	//Period 1: Move Period.
	HeavyTankSkill();
	memset(moved,0,sizeof(moved));    //重置移动状态.
	for(int i = 0;i < cmd.nMove;i++)
		rpt.period1 += cmd.nMove * (i + 1) * MoveTank(1,cmd.moveCommand[i]);

	//Period 2: Attack Period.
	HeavyTankSkill();
	rpt.period2 = AttackTank(1,cmd.atkCommand[0]);

	//Period 3: Ptc Period.
	HeavyTankSkill();
	if(cmd.ptc == Command::move)
	{
		memset(moved,0,sizeof(moved));
		rpt.period3 = MoveTank(1,cmd.moveCommand[cmd.nMove]);
	}
	if(cmd.ptc == Command::attack)
		rpt.period3 = AttackTank(1,cmd.atkCommand[1]);
	
	for(int i = 0;i < 5;i++)    //恢复瘫痪坦克.
	{
		if(controled[i])
			(*fvideo)<<1007<<' '<<i<<endl;
		controled[i] = false;
	}
	//Game Logic Process End.

	m_pPlayerA->WriteCommandReport(rpt);

	lastCommand = cmd;
	lastCommandReport = rpt;

	return true;
}
bool GameCore::PlayerBsTurn()
{
	Command cmd;
	CmdRpt  rpt;
	
	m_pPlayerB->WriteLastCommand(lastCommand,lastCommandReport);
	
	if(!m_pPlayerB->ReadCommand(cmd))
	{
		//WriteVideo_End(round,1);
		WriteWarning(2,"AI程序异常结束！");
		SetError(2);
		return false;
	}
	//Game Logic Processing...

	//Period 1: Move Period.
	HeavyTankSkill();
	memset(moved,0,sizeof(moved));    //重置移动状态.
	for(int i = 0;i < cmd.nMove;i++)
		rpt.period1 += cmd.nMove * (i + 1) * MoveTank(2,cmd.moveCommand[i]);

	//Period 2: Attack Period.
	HeavyTankSkill();
	rpt.period2 = AttackTank(2,cmd.atkCommand[0]);

	//Period 3: Ptc Period.
	HeavyTankSkill();
	if(cmd.ptc == Command::move)
	{
		memset(moved,0,sizeof(moved));
		rpt.period3 = MoveTank(2,cmd.moveCommand[cmd.nMove]);
	}
	if(cmd.ptc == Command::attack)
		rpt.period3 = AttackTank(2,cmd.atkCommand[1]);
	
	for(int i = 5;i < 10;i++)    //恢复瘫痪坦克.
	{
		if(controled[i])
			(*fvideo)<<1007<<' '<<i<<endl;
		controled[i] = false;
	}
	//Game Logic Process End.

	m_pPlayerB->WriteCommandReport(rpt);

	lastCommand = cmd;
	lastCommandReport = rpt;

	return true;
}
int GameCore::MoveTank(int nPlayerID,Move m)
{
	int nID = m.nID;
	Point pt = m.pt;
	if(nID < 0)
		return 0;
	if(!IsInMap(pt.x,pt.y))    //非法坐标.
	{
		WriteWarning(nPlayerID,"移动坐标为非法坐标.");
		return 0;
	}
	if(nID < (nPlayerID - 1) * 5 || nID >= nPlayerID * 5)    //操作非法坦克.
	{
		WriteWarning(nPlayerID,"操作非法坦克.");
		return 0;
	}
	if(moved[nID])    //重复操作同一辆坦克.
	{
		WriteWarning(nPlayerID,"操作重复坦克.");
		return 0;
	}
	if(controled[nID])    //操作瘫痪坦克.
	{
		WriteWarning(nPlayerID,"操作瘫痪坦克.");
		return 0;
	}
	if(nPlayerID == 1)
	{
		if(tankA[nID].hp <= 0)    //操作阵亡坦克.
		{
			WriteWarning(nPlayerID,"操作阵亡坦克.");
			return 0;
		}
		int d = dis(tankA[nID].pt,pt);
		switch(d)
		{
		case 0:    //原地不动，无意义.
			WriteWarning(nPlayerID,"坦克移动距离为零.");
			return 0;
		case 1:    //不必分析坦克类型,直接移动.
			tankA[nID].pt = pt;
			WriteVideo_Move(nID,pt);
			WriteComment_Move(nID,pt);
			/*if(tankA[nID].type == heavy)    //重坦克造成碾压伤害.
				for(int i = 0;i < 5;i++)
					if(tankB[i].pt == pt)
						tankB[i].hp--;*/
			return 1;
		case 2:
			if(tankA[nID].type == light)
			{
				for(int i = 0;i < 6;i++)
				{
					int tx = tankA[nID].pt.x + nDirVector[i][0];
					int ty = tankA[nID].pt.y + nDirVector[i][1];
					if(IsInMap(tx,ty) && dis(pt,Point(tx,ty)) == 1)
					{
						if(map[tx][ty] == plain)    //附近有平原，可移动两格.
						{
							tankA[nID].pt = pt;
							WriteVideo_Move(nID,pt);
							WriteComment_Move(nID,pt);
							return 1;
						}
					}
				}
				WriteWarning(nPlayerID,"轻坦克移动2格非法：第1格不是平原.");
				return 0;    //未发现平原，操作非法.
			}
			else
			{
				WriteWarning(nPlayerID,"坦克移动距离太远.");
				return 0;    //非轻坦克.
			}
		default:    //移动的距离有点奇怪.
			WriteWarning(nPlayerID,"坦克移动距离太远.");
			return 0;
		}
	}
	if(nPlayerID == 2)
	{
		if(tankB[nID - 5].hp <= 0)    //操作阵亡坦克.
		{
			WriteWarning(nPlayerID,"操作阵亡坦克.");
			return 0;
		}
		int d = dis(tankB[nID - 5].pt,pt);
		switch(d)
		{
		case 0:    //原地不动，无意义.
			WriteWarning(nPlayerID,"坦克移动距离为零.");
			return 0;
		case 1:    //不必分析坦克类型,直接移动.
			tankB[nID - 5].pt = pt;
			WriteVideo_Move(nID,pt);
			WriteComment_Move(nID,pt);
			/*if(tankB[nID - 5].type == heavy)    //重坦克造成碾压伤害.
				for(int i = 0;i < 5;i++)
					if(tankA[i].pt == pt)
						tankA[i].hp--;*/
			return 1;
		case 2:
			if(tankB[nID - 5].type == light)
			{
				for(int i = 0;i < 6;i++)
				{
					int tx = tankB[nID - 5].pt.x + nDirVector[i][0];
					int ty = tankB[nID - 5].pt.y + nDirVector[i][1];
					if(IsInMap(tx,ty) && dis(pt,Point(tx,ty)) == 1)
					{
						if(map[tx][ty] == plain)    //附近有平原，可移动两格.
						{
							tankB[nID - 5].pt = pt;
							WriteVideo_Move(nID,pt);
							WriteComment_Move(nID,pt);
							return 1;
						}
					}
				}
				WriteWarning(nPlayerID,"轻坦克移动2格非法：第1格不是平原.");
				return 0;    //未发现平原，操作非法.
			}
			else
			{
				WriteWarning(nPlayerID,"坦克移动距离太远.");
				return 0;    //非轻坦克.
			}
		default:    //移动的距离有点奇怪.
			WriteWarning(nPlayerID,"坦克移动距离太远.");
			return 0;
		}
	}
	return 0;
}
int GameCore::AttackTank(int nPlayerID,Attack cmd)
{
	int nIDA = cmd.nID_Attacker;
	int nIDB = cmd.nID_Target;
	int damage = 0;
	int rpt = 0;
	if(nIDA < 0 || nIDB < 0)
		return 0;
	if(nIDA < (nPlayerID - 1) * 5 || nIDA >= nPlayerID * 5)    //操作非法坦克.
	{
		WriteWarning(nPlayerID,"操作非法坦克.");
		return 0;
	}
	if(nIDB == 10)    //攻击目标为基地.
	{
		if(nPlayerID == 1)
		{
			if(baseHpB <= 0)    //基地已爆炸.
			{
				WriteWarning(nPlayerID,"攻击目标无效.");
				return 0;
			}
			Tank t = tankA[nIDA];
			Tank base;
			base.type = middle; base.pt = Point(6,6); base.hp = baseHpB;
			rpt = CalculateDamage(t,base,damage);
			if(rpt == 3 || rpt == 4)
				rpt -= 2;

			if(rpt)
			{
				baseHpB -= damage;    //造成伤害.
				if(baseHpB <= 0)    //基地炸了.
				{
					scoreA += 3;
					valueB -= 3.0;
					WriteComment_Attack(nIDA,10,damage,0,1);
					WriteVideo_Attack(nIDA,10,damage,0,1);
				}
				else
				{
					WriteComment_Attack(nIDA,10,damage,0,0);
					WriteVideo_Attack(nIDA,10,damage,0,0);
				}
			}
		}
		if(nPlayerID == 2)
		{
			if(baseHpA <= 0)    //基地已爆炸.
			{
				WriteWarning(nPlayerID,"攻击目标无效.");
				return 0;
			}
			Tank t = tankB[nIDA - 5];
			Tank base;
			base.type = middle; base.pt = Point(1,1); base.hp = baseHpA;
			rpt = CalculateDamage(t,base,damage);
			if(rpt == 3 || rpt == 4)
				rpt -= 2;

			if(rpt)
			{
				baseHpA -= damage;
				if(baseHpA <= 0)
				{
					scoreB += 3;
					valueA -= 3.0;
					WriteComment_Attack(nIDA,10,damage,0,1);
					WriteVideo_Attack(nIDA,10,damage,0,1);
				}
				else
				{
					WriteComment_Attack(nIDA,10,damage,0,0);
					WriteVideo_Attack(nIDA,10,damage,0,0);
				}
			}
		}
		return rpt;
	}
	if(nIDB >= (nPlayerID - 1) * 5 && nIDB < nPlayerID * 5)    //关闭队友伤害.
	{
		WriteWarning(nPlayerID,"攻击己方坦克.");
		return 0;
	}
	if(controled[nIDA])    //操作瘫痪坦克.
	{
		WriteWarning(nPlayerID,"操作瘫痪坦克");
		return 0;
	}
	if(nPlayerID == 1)
	{
		if(tankB[nIDB - 5].hp <= 0)    //攻击目标已阵亡.
		{
			WriteWarning(nPlayerID,"攻击阵亡坦克.");
			return 0;
		}
		Tank t1 = tankA[nIDA];
		Tank t2 = tankB[nIDB - 5];
		rpt = CalculateDamage(t1,t2,damage);
		if(rpt == 3 || rpt == 4)
			controled[nIDB] = true;

		if(rpt)
		{
			tankB[nIDB - 5].hp -= damage;
			if(tankB[nIDB - 5].hp <= 0)
			{
				scoreA += 1;
				valueB -= tankB[nIDB - 5].value;
				lifeB--;
				WriteComment_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),1);
				WriteVideo_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),1);
			}
			else
			{
				WriteComment_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),0);
				WriteVideo_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),0);
			}
		}
	}
	if(nPlayerID == 2)
	{
		if(tankA[nIDB].hp <= 0)
		{
			WriteWarning(nPlayerID,"攻击阵亡坦克.");
			return 0;
		}
		Tank t1 = tankB[nIDA - 5];
		Tank t2 = tankA[nIDB];
		rpt = CalculateDamage(t1,t2,damage);
		if(rpt == 3 || rpt == 4)
			controled[nIDB] = true;

		if(rpt)
		{
			tankA[nIDB].hp -= damage;
			if(tankA[nIDB].hp <= 0)
			{
				scoreB += 1;
				valueA -= tankA[nIDB].value;
				lifeA--;
				WriteComment_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),1);
				WriteVideo_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),1);
			}
			else
			{
				WriteComment_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),0);
				WriteVideo_Attack(nIDA,nIDB,damage,((rpt == 3 || rpt == 4)? 1 : 0),0);
			}
		}
	}
	return rpt;
}
int GameCore::CalculateDamage(Tank& t1,Tank& t2,int& damage)
{
	double p;
	int k;
	damage = 0;
	if(t1.hp <= 0)
	{
		WriteWarning(((t1.nID < 5) ? 1 : 2),"攻击坦克已阵亡.");
		return 0;    //攻击方为阵亡坦克.
	}
	if(t2.hp <= 0)
	{
		WriteWarning(((t1.nID < 5) ? 1 : 2),"攻击阵亡坦克.");
		return 0;    //被攻击方为阵亡坦克.
	}
	srand(time(0) / hseed++);
	if(hseed >= 127)
		hseed = 2;
	switch(t1.type)
	{
	case heavy:           //重型坦克.
		if(dis(t1.pt,t2.pt) > 0 && dis(t1.pt,t2.pt) < 3)    //dis == 1,2
		{
			if(map[t1.pt.x][t1.pt.y] == hill)
				t1.ap += 2;
			if(map[t2.pt.x][t2.pt.y] == forest)
				t2.armour += 2;
			p = (t1.ap - t2.armour) / 6.0 * 100;    //概率.
			//srand(time(0));
			k = rand() % 100 + 1;    //随机化.
			if(k <= p)    //命中.
			{
				damage += t1.atk; return 2;
			}
			else          //未命中.
			{
				damage++; return 1;
			}
		}
		else
			return 0;     //不在攻击范围.
	case light:           //轻型坦克.
		if(dis(t1.pt,t2.pt) > 0 && dis(t1.pt,t2.pt) < 3)    //dis == 1,2
		{
			if(map[t1.pt.x][t1.pt.y] == hill)
				t1.ap += 2;
			if(map[t2.pt.x][t2.pt.y] == forest)
				t2.armour += 2;
			p = (t1.ap - t2.armour) / 6.0 * 100;
			//srand(time(0));
			k = rand() % 100 + 1;
			if(t2.type == selfPropelled)    //攻击特效.
				damage++;
			if(k <= p)    //命中.
			{
				damage += t1.atk; return 2;
			}
			else          //未命中.
			{
				damage++; return 1;
			}
		}
		else
			return 0;     //不在攻击范围.
	case middle:          //中型坦克.
		if(dis(t1.pt,t2.pt) == 2)    //dis == 2
		{
			if(map[t1.pt.x][t1.pt.y] == hill)
				t1.ap += 2;
			if(map[t2.pt.x][t2.pt.y] == forest)
				t2.armour += 2;
			p = (t1.ap - t2.armour) / 6.0 * 100;
			//srand(time(0));
			k = rand() % 100 + 1;
			if(t2.type == heavy)    //攻击特效.
				damage++;
			if(k <= p)    //命中.
			{
				damage += t1.atk; return 2;
			}
			else          //未命中.
			{
				damage++; return 1;
			}
		}
		else
			return 0;     //不在攻击范围.
	case selfPropelled:   //自行火炮.
		if(dis(t1.pt,t2.pt) == 3)    //dis == 3
		{
			if(map[t1.pt.x][t1.pt.y] == hill)
				t1.ap += 2;
			if(map[t2.pt.x][t2.pt.y] == forest)
				t2.armour += 2;
			p = (t1.ap - t2.armour) / 6.0 * 100;
			//srand(time(0));
			k = rand() % 100 + 1;
			if(k <= p)    //命中.
			{
				damage += t1.atk; return 2;
			}
			else          //未命中.
			{
				damage++; return 1;
			}
		}
		else
			return 0;     //不在攻击范围.
	case antitank:        //反坦克炮.
		if(dis(t1.pt,t2.pt) == 2)    //dis == 2
		{
			if(map[t1.pt.x][t1.pt.y] == hill)
				t1.ap += 2;
			if(map[t2.pt.x][t2.pt.y] == forest)
				t2.armour += 2;
			p = (t1.ap - t2.armour) / 6.0 * 100;
			//srand(time(0));
			k = rand() % 100;

			bool skill = false;
			if(k < 50)    //特效触发:瘫痪敌人.
				skill = true;

			k = rand() % 100 + 1;
			if(k <= p)    //命中.
			{
				damage += t1.atk; return (skill ? 4 : 2);
			}
			else          //未命中.
			{
				damage++; return (skill ? 3 : 1);
			}
		}
		else
			return 0;     //不在攻击范围.
	}
	return 0;
}
void GameCore::HeavyTankSkill()
{
	//PlayerA
	if(tankA[0].hp > 0 && !controled[0])
	{
		for(int i = 0;i < 5;i++)
			if(tankB[i].hp > 0 && tankB[i].pt == tankA[0].pt)
			{
				tankB[i].hp--;
				if(tankB[i].hp <= 0)
				{
					scoreA += 1;
					valueB -= tankB[i].value;
					WriteVideo_HeavyAttack(0,tankB[i].nID,1,1);
					WriteComment_HeavyAttack(0,tankB[i].nID,1,1);
				}
				else
				{
					WriteVideo_HeavyAttack(0,tankB[i].nID,1,0);
					WriteComment_HeavyAttack(0,tankB[i].nID,1,0);
				}
			}
	}
	//PlayerB
	if(tankB[0].hp > 0 && !controled[5])
	{
		for(int i = 0;i < 5;i++)
			if(tankA[i].hp > 0 && tankA[i].pt == tankB[0].pt)
			{
				tankA[i].hp--;
				if(tankA[i].hp <= 0)
				{
					scoreB += 1;
					valueA -= tankA[i].value;
					WriteVideo_HeavyAttack(5,tankA[i].nID,1,1);
					WriteComment_HeavyAttack(5,tankA[i].nID,1,1);
				}
				else
				{
					WriteVideo_HeavyAttack(5,tankA[i].nID,1,0);
					WriteComment_HeavyAttack(5,tankA[i].nID,1,0);
				}
			}
	}
	return;
}
bool GameCore::IsGameOver()
{
	if(round >= 40)    //40回合结束.
		return true;
	if(lifeA <= 0 || lifeB <= 0)
		return true;
	/*if(scoreA >= 5 || scoreB >= 5)    //分值.
		return true;*/
	return false;
}
bool GameCore::ReportResult()
{
	cout<<"Round = "<<round<<endl;
	cout<<"PlayerA.score = "<<scoreA<<"  PlayerA.value = "<<valueA<<endl;
	cout<<"PlayerB.score = "<<scoreB<<"  PlayerB.value = "<<valueB<<endl;

	char tmp[256];
	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,strNameA);
	strcat(tmp,"_vs_");
	strcat(tmp,strNameB);
	strcat(tmp,"_result.txt");
	ofstream* fresult = new ofstream(tmp);
	if(!fresult)
		return false;

	(*fresult)<<"Round = "<<round<<endl;
	(*fresult)<<"PlayerA.score = "<<scoreA<<"  PlayerA.value = "<<valueA<<endl;
	(*fresult)<<"PlayerB.score = "<<scoreB<<"  PlayerB.value = "<<valueB<<endl;

	if(round >= 40)
		if(valueA > valueB)
		{
			WriteVideo_End(round,1);
			cout<<"PlayerA wins the game!"<<endl;
			(*fresult)<<"PlayerA wins the game!"<<endl;
		}
		else
			if(valueB > valueA)
			{
				WriteVideo_End(round,2);
				cout<<"PlayerB wins the game!"<<endl;
				(*fresult)<<"PlayerB wins the game!"<<endl;
			}
			else
			{
				WriteVideo_End(round,0);

				cout<<"It ends in a draw."<<endl;
				(*fresult)<<"It ends in a draw."<<endl;
			}
		/*if(scoreA >= 5)
		{
			WriteVideo_End(round,1);
			cout<<"PlayerA wins the game!"<<endl;
			(*fresult)<<"PlayerA wins the game!"<<endl;
		}
		else
			if(scoreB >= 5)
			{
				WriteVideo_End(round,2);
				cout<<"PlayerB wins the game!"<<endl;
				(*fresult)<<"PlayerB wins the game!"<<endl;
			}
			else
			{
				WriteVideo_End(round,0);

				cout<<"It ends in a draw."<<endl;
				(*fresult)<<"It ends in a draw."<<endl;
			}*/
	else
	{
		if(nError == 1)
		{
			WriteVideo_End(round,2);
			cout<<"PlayerB wins the game!"<<endl;
			(*fresult)<<"PlayerB wins the game!"<<endl;
		}
		if(nError == 2)
		{
			WriteVideo_End(round,1);
			cout<<"PlayerA wins the game!"<<endl;
			(*fresult)<<"PlayerA wins the game!"<<endl;
		}
	}

	return true;
}
bool GameCore::IsInMap(int x,int y)
{
	if(0 < x && x < 7 && 0 < y && y < 7)
		return true;
	return false;
}
//写入移动命令.
void GameCore::WriteVideo_Move(int nID,Point pt)
{
	(*fvideo)<<1001<<' '<<nID<<' '<<pt<<endl;
}
//写入攻击命令.
void GameCore::WriteVideo_Attack(int atker,int target,int damage,int skill,int dead)
{
	(*fvideo)<<1002<<' '<<atker<<' '<<target<<' '
		<<damage<<' '<<skill<<' '<<dead<<endl;
}
//写入重坦碾压.
void GameCore::WriteVideo_HeavyAttack(int atker,int target,int damage,int dead)
{
	(*fvideo)<<1009<<' '<<atker<<' '<<target<<' '
		<<damage<<' '<<dead<<endl;
}
//写入结束命令.
void GameCore::WriteVideo_End(int r,int nID)
{
	(*fvideo).fill('0');
	(*fvideo)<<1005<<' '<<setw(2)<<r<<' '<<nID<<endl;
	/*if(r < 10)
		(*fvideo)<<0;
	(*fvideo)<<r<<' '<<nID<<endl;*/
}
//写入移动命令文字战报
void GameCore::WriteComment_Move(int nID,Point pt)
{
	if(m_nLastRound1 != round)
	{
		m_nLastRound1 = round;
		if(round != 1)
			(*fcomment)<<endl;
		(*fcomment).fill('0');
		(*fcomment)<<"Round "<<setw(3)<<m_nLastRound1<<':'
			<<"坦克 "<<nID<<" 移动至 ("<<pt.x<<','<<pt.y<<") 处."<<endl;
	}
	else
	{
		(*fcomment).fill(' ');
		(*fcomment)<<setw(10)<<' '
			<<"坦克 "<<nID<<" 移动至 ("<<pt.x<<','<<pt.y<<") 处."<<endl;
	}
	return;
}
//写入攻击命令文字战报
void GameCore::WriteComment_Attack(int atker,int target,int damage,int skill,int dead)
{
	if(m_nLastRound1 != round)
	{
		m_nLastRound1 = round;
		if(round != 1)
			(*fcomment)<<endl;
		(*fcomment).fill('0');
		(*fcomment)<<"Round "<<setw(3)<<m_nLastRound1<<':';
		if(skill)
		{
			(*fcomment)<<"坦克 "<<atker<<" 使坦克 "<<target<<" 瘫痪，并造成了 "
			<<damage<<" 点伤害.";
			if(dead)
				(*fcomment)<<"坦克 "<<target<<" 阵亡了."<<endl;
			else
				(*fcomment)<<endl;
		}
		else
		{
			(*fcomment)<<"坦克 "<<atker<<" 对"<<(target == 10 ? "敌方基地" : "坦克 ");
			if(target != 10)
				(*fcomment)<<target<<' ';
			(*fcomment)<<"造成了 "<<damage<<" 点伤害.";
			if(dead)
				(*fcomment)<<(target == 10 ? "敌方基地" : "坦克")<<(target == 10 ? "破坏" : " 阵亡")<<"了."<<endl;
			else
				(*fcomment)<<endl;
		}
	}
	else
	{
		(*fcomment).fill(' ');
		(*fcomment)<<setw(10)<<' ';
		if(skill)
		{
			(*fcomment)<<"坦克 "<<atker<<" 使坦克 "<<target<<" 瘫痪，并造成了 "
			<<damage<<" 点伤害.";
			if(dead)
				(*fcomment)<<"坦克 "<<target<<" 阵亡了."<<endl;
			else
				(*fcomment)<<endl;
		}
		else
		{
			(*fcomment)<<"坦克 "<<atker<<" 对"<<(target == 10 ? "敌方基地" : "坦克 ");
			if(target != 10)
				(*fcomment)<<target<<' ';
			(*fcomment)<<"造成了 "<<damage<<" 点伤害.";
			if(dead)
				(*fcomment)<<(target == 10 ? "敌方基地" : "坦克")<<(target == 10 ? "破坏" : "阵亡")<<"了."<<endl;
			else
				(*fcomment)<<endl;
		}
	}
	return;
}
void GameCore::WriteComment_HeavyAttack(int atker,int target,int damage,int dead)
{
	if(m_nLastRound1 != round)
	{
		m_nLastRound1 = round;
		if(round != 1)
			(*fcomment)<<endl;
		(*fcomment).fill('0');
		(*fcomment)<<"Round "<<setw(3)<<m_nLastRound1<<':';
		/*if(skill)
		{
		(*fcomment)<<"坦克 "<<atker<<" 使坦克 "<<target<<" 瘫痪，并造成了 "
		<<damage<<" 点伤害.";
		if(dead)
		(*fcomment)<<"坦克 "<<target<<" 阵亡了."<<endl;
		else
		(*fcomment)<<endl;
		}
		else
		{*/
		(*fcomment)<<"重坦 "<<atker<<" 对"<<(target == 10 ? "敌方基地" : "坦克 ");
		if(target != 10)
			(*fcomment)<<target<<' ';
		(*fcomment)<<"造成了 "<<damage<<" 点碾压伤害.";
		if(dead)
			(*fcomment)<<(target == 10 ? "敌方基地" : "坦克")<<(target == 10 ? "破坏" : " 阵亡")<<"了."<<endl;
		else
			(*fcomment)<<endl;
		//}
	}
	else
	{
		(*fcomment).fill(' ');
		(*fcomment)<<setw(10)<<' ';
		/*if(skill)
		{
		(*fcomment)<<"坦克 "<<atker<<" 使坦克 "<<target<<" 瘫痪，并造成了 "
		<<damage<<" 点伤害.";
		if(dead)
		(*fcomment)<<"坦克 "<<target<<" 阵亡了."<<endl;
		else
		(*fcomment)<<endl;
		}
		else
		{*/
		(*fcomment)<<"重坦 "<<atker<<" 对"<<(target == 10 ? "敌方基地" : "坦克 ");
		if(target != 10)
			(*fcomment)<<target<<' ';
		(*fcomment)<<"造成了 "<<damage<<" 点碾压伤害.";
		if(dead)
			(*fcomment)<<(target == 10 ? "敌方基地" : "坦克")<<(target == 10 ? "破坏" : "阵亡")<<"了."<<endl;
		else
			(*fcomment)<<endl;
		//}
	}
	return;
}
//写入警告.
void GameCore::WriteWarning(int nID,const char* strWarning)
{
	if(m_nLastRound2 != round)
	{
		m_nLastRound2 = round;
		if(round != 1)
			(*fwarning)<<endl;
		(*fwarning).fill('0');
		(*fwarning)<<"Round "<<setw(3)<<m_nLastRound2<<':'<<"选手"<<nID<<','<<strWarning<<endl;
	}
	else
	{
		(*fwarning).fill(' ');
		(*fwarning)<<setw(14)<<"选手"<<nID<<','<<strWarning<<endl;
	}
	return;
}
void GameCore::InitializeTank(int nID,Faction fac,TankType tt,Point pt,Tank& tank)
{
	if(fac == Axis)     //轴心国坦克.
	{
		switch(tt)
		{
		case heavy:                 //重型坦克.
			tank.nID = nID;         tank.type = heavy;
			tank.pt = pt;           tank.value = 3.5f;
			tank.atk = 3;           tank.armour = 3;
			tank.hp = 8;            tank.ap = 5;
			return;
		case light:                 //轻型坦克.
			tank.nID = nID;         tank.type = light;
			tank.pt = pt;           tank.value = 1.5f;
			tank.atk = 1;           tank.armour = 1;
			tank.hp = 4;            tank.ap = 4;
			return;
		case middle:                //中型坦克.
			tank.nID = nID;         tank.type = middle;
			tank.pt = pt;           tank.value = 2.0f;
			tank.atk = 2;           tank.armour = 2;
			tank.hp = 6;            tank.ap = 2;
			return;
		case selfPropelled:         //自行火炮.
			tank.nID = nID;         tank.type = selfPropelled;
			tank.pt = pt;           tank.value = 1.0f;
			tank.atk = 3;           tank.armour = 1;
			tank.hp = 4;            tank.ap = 5;
			return;
		case antitank:              //反坦克炮.
			tank.nID = nID;         tank.type = antitank;
			tank.pt = pt;           tank.value = 1.0f;
			tank.atk = 3;           tank.armour = 0;
			tank.hp = 4;            tank.ap = 4;
			return;
		};
	}
	if(fac == Allies)   //同盟国坦克.
	{
		switch(tt)
		{
		case heavy:                 //重型坦克.
			tank.nID = nID;         tank.type = heavy;
			tank.pt = pt;           tank.value = 3.5f;
			tank.atk = 3;           tank.armour = 3;
			tank.hp = 6;            tank.ap = 6;
			return;
		case light:                 //轻型坦克.
			tank.nID = nID;         tank.type = light;
			tank.pt = pt;           tank.value = 1.5f;
			tank.atk = 2;           tank.armour = 0;
			tank.hp = 4;            tank.ap = 3;
			return;
		case middle:                //中型坦克.
			tank.nID = nID;         tank.type = middle;
			tank.pt = pt;           tank.value = 2.0f;
			tank.atk = 2;           tank.armour = 2;
			tank.hp = 5;            tank.ap = 3;
			return;
		case selfPropelled:         //自行火炮.
			tank.nID = nID;         tank.type = selfPropelled;
			tank.pt = pt;           tank.value = 1.0f;
			tank.atk = 4;           tank.armour = 0;
			tank.hp = 5;            tank.ap = 4;
			return;
		case antitank:              //反坦克炮.
			tank.nID = nID;         tank.type = antitank;
			tank.pt = pt;           tank.value = 1.0f;
			tank.atk = 2;           tank.armour = 1;
			tank.hp = 4;            tank.ap = 5;
			return;
		};
	}
}
void GameCore::ReadInitialMap()
{
	ifstream* fin = new ifstream("map.txt");
	for(int i = 0;i < 8;i++)
		for(int j = 0;j < 8;j++)
			(*fin)>>map[i][j];
	fin->close();
	delete fin;
}
void GameCore::SetError(int nID)
{
	nError = nID;
}
