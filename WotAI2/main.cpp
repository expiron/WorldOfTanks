#include <stdio.h>
#include "Wot.h"

int main()
{
	int round = 1;    //回合数：A操作一步、B操作一步算作一个回合.
	/*//TODO: 第一步：判断自己是先手还是后手.
	if(IsFirsthand())    //true:先手, false:后手.
	{
		//TODO: 第二步：选择自己的坦克阵营.
		ChooseFaction(Axis);    //Axis:轴心国, Allies:同盟国.
	}
	else
	{
		//TODO: 可能根据先后手选择不同的坦克阵营.
		ChooseFaction(Allies);
	}
	 TODO: 当然，如果无论先手、后手都只选择同一坦克阵营可以直接调用:*/
	ChooseFaction(Axis);

	//TODO: 第三步：读入游戏初始数据.
	GameData gdata;
	InputGameData(gdata);

	//TODO: 第四步：游戏开始.
	Command myCmd,enemyCmd;    //我方、敌方命令.
	CmdRpt  myRpt,enemyRpt;    //我方、敌方命令反馈.
	if(IsFirsthand())
	{
		myCmd = Command();
		//TODO: 先手先走一步.在此处添加自己的策略.
		if(round == 1)
		{
			myCmd.nMove=2;
			myCmd.moveCommand[0].nID=1;
			myCmd.moveCommand[0].pt=Point(1,3);
			myCmd.moveCommand[1].nID=3;
			myCmd.moveCommand[1].pt=Point(1,2);
			myCmd.ptc = Command::move; 
			myCmd.moveCommand[2].nID=3;
			myCmd.moveCommand[2].pt=Point(1,3);
		}

		OutputCommand(myCmd);    //输出我方命令.
		InputCommandReport(myRpt);    //读入我方命令的执行反馈.

		//更新游戏数据.
		UpdateGameData(true,gdata,myCmd,myRpt);    //更新己方数据.
	}
	else
		round--;    //后手先减1.(为适应程序结构)
	
	//TODO: 无限循环：读入对方命令、输出己方命令
	while( true )
	{
		//读入敌方命令及命令反馈.
		InputEnemyCommand(enemyCmd,enemyRpt);

		//更新游戏数据.
		UpdateGameData(false,gdata,enemyCmd,enemyRpt);    //更新敌方数据.
		round++;
		
		myCmd = Command();
		//TODO: 根据敌方命令，输出己方命令.在此处添加自己的策略.
		if(round==2)
		{
			myCmd.nMove=2;
			myCmd.moveCommand[0].nID=4;
			myCmd.moveCommand[0].pt=Point(1,2);
			myCmd.moveCommand[1].nID=3;
			myCmd.moveCommand[1].pt=Point(1,4);
			myCmd.ptc = Command::move; 
			myCmd.moveCommand[2].nID=4;
			myCmd.moveCommand[2].pt=Point(1,3);
		}
		if(round==3)
		{
			myCmd.nMove=2;
			myCmd.moveCommand[0].nID=0;
			myCmd.moveCommand[0].pt=Point(2,1);
			myCmd.moveCommand[1].nID=3;
			myCmd.moveCommand[1].pt=Point(1,5);
			myCmd.ptc = Command::move; 
			myCmd.moveCommand[2].nID=0;
			myCmd.moveCommand[2].pt=Point(3,1);
		}
		if(round==4)
		{
			myCmd.nMove=2;
			myCmd.moveCommand[0].nID=2;
			myCmd.moveCommand[0].pt=Point(2,1);
			myCmd.moveCommand[1].nID=3;
			myCmd.moveCommand[1].pt=Point(1,6);
			myCmd.ptc = Command::move; 
			myCmd.moveCommand[2].nID=2;
			myCmd.moveCommand[2].pt=Point(3,1);
			gdata.myTank[3].pt=Point(1,6);
		}
		if(round==5)
		{
			myCmd.nMove=1;
			myCmd.moveCommand[0].nID=3;
			myCmd.moveCommand[0].pt=Point(2,6);
			myCmd.ptc = Command::move; 
			myCmd.moveCommand[1].nID=3;
			myCmd.moveCommand[1].pt=Point(3,6);
		}
		if(round>=6)
		{
			myCmd.nMove=1;
			myCmd.moveCommand[0].nID=3;
			myCmd.moveCommand[0].pt=Point(3,6);
			if(gdata.enemyBaseHP>0)
			{
				myCmd.atkCommand[0].nID_Attacker = 3;
				myCmd.atkCommand[0].nID_Target = 10;
			}
			else
			{
				myCmd.atkCommand[0].nID_Attacker = -1;
				myCmd.atkCommand[0].nID_Target = -1;
			}
			if(gdata.enemyBaseHP>0)
			{
				myCmd.ptc = Command::attack;
				myCmd.atkCommand[1].nID_Attacker = 3;
				myCmd.atkCommand[1].nID_Target = 10;
			}
			else
			{
				myCmd.ptc = Command::attack;
				myCmd.atkCommand[1].nID_Attacker = -1;
				myCmd.atkCommand[1].nID_Target = -1;
			}
		}

		OutputCommand(myCmd);
		InputCommandReport(myRpt);

		//更新游戏数据.
		UpdateGameData(true,gdata,myCmd,myRpt);    //更新己方数据.
	}
	return 0;
}
