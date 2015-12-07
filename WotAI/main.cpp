#include <stdio.h>
#include "Wot.h"

int main()
{
	int round = 1;    //回合数：A操作一步、B操作一步算作一个回合.
	//TODO: 第一步：判断自己是先手还是后手.
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
	/* TODO: 当然，如果无论先手、后手都只选择同一坦克阵营可以直接调用:
	ChooseFaction(...); */

	//TODO: 第三步：读入游戏初始数据.
	GameData gdata;
	InputGameData(gdata);

	//TODO: 第四步：游戏开始.
	Command myCmd,enemyCmd;    //我方、敌方命令.
	CmdRpt  myRpt,enemyRpt;    //我方、敌方命令反馈.
	if(IsFirsthand())
	{
		myCmd = Command();    //将命令置为空命令.
		//TODO: 先手先走一步.在此处添加自己的策略.
		//...
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
		
		myCmd = Command();    //将命令置为空命令.
		//TODO: 根据敌方命令，输出己方命令.在此处添加自己的策略.
		//...
		OutputCommand(myCmd);
		InputCommandReport(myRpt);

		//更新游戏数据.
		UpdateGameData(true,gdata,myCmd,myRpt);    //更新己方数据.
	}
	return 0;
}
