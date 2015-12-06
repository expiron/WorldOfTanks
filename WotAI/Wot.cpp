
#include "Wot.h"

//Global Variables.
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
	if((p1.x - p2.x)*(p1.y - p2.y) >=0 )
		return ((p1.x > p2.x) ? (p1.x - p2.x) : (p2.x - p1.x))
		+ ((p1.y > p2.y) ? (p1.y - p2.y) : (p2.y - p1.y));
	else
		return ((p1.x > p2.x) ? (p1.x - p2.x) : (p2.x - p1.x));
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
	cout<<fac<<endl;
}
//读入游戏初始数据.
void InputGameData(GameData& gdata)
{
	cin>>gdata;
}
//输出命令.
void OutputCommand(Command cmd)
{
	cout<<cmd;
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