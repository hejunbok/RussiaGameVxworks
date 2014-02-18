#include"RussiaBall.h"

void FlushQ ()
{
    UGL_MSG msg;
    UGL_STATUS status;
    do
    {
		status = uglInputMsgGet (inputServiceId, &msg, UGL_NO_WAIT);
	} while (status != UGL_STATUS_Q_EMPTY);
}

/**
功能:退出程序时清理内存空间
*/
void CleanUp()
{
    UGL_FREE (SPEED);
    UGL_FREE (ScreenArray);
    UGL_FREE (colorTable);    
    UGL_FREE (Block);
    uglFontDestroy (fontSystem);
    uglGcDestroy (gc);	
    uglDeinitialize();
}

/**
功能:图形变换
Param:
int blockID   待变换基本图样类型ID	取值范围:0--27
Retval:		  变换后的基本图样类型ID 取值范围:0--27
*/
int ChangeShape(int blockID)
{
	int newBlockID;
	if(blockID%4==0)
		newBlockID=blockID+3;
	else
		newBlockID=blockID-1;
	return newBlockID;
}
/**
功能:将Block所在单位块加到ScreenArray数组中
Param:
int row			单元格横坐标	取值范围:0--27
int column		单元格纵坐标	取值范围:0--19
int blockID		基本图样类型	取值范围:0--27
Retval:	NULL
*/
void UpdateArray(int row,int column,int blockID)
{
	int i,Position[4][2]={0};
	for(i=0;i<4;i++)
	{
		Position[i][0]=row+Block[blockID][i][0];
		Position[i][1]=column+Block[blockID][i][1];
		ScreenArray[Position[i][0]][Position[i][1]]=blockID/4+1;
	}	
}
/*
初始化游戏，形成方块逐行向上递增，然后逐行向下递减的效果
*/
void InitGameShow()
{
	int i,j,k;
	/*k=rand()%7+1;*//*建议刷屏颜色采用单色，如反对则打开此行及下面第五行的屏蔽*/
	
	for(i=0;i<28;i++)
	{
		for(j=0;j<20;j++)
			ScreenArray[i][j]=LIGHTGREEN;/*k;*/
		RefreshScreen();
		uglOSTaskDelay(10);
	}
	for(i=Row+3;i>=0;i--)
	{
		for(j=0;j<20;j++)			
			ScreenArray[i][j]=0;
		RefreshScreen();
		uglOSTaskDelay(10);
	}
}
/*=============复位游戏设置============*/
void ResetData()
{	
	russia.Nextscore = 500; /*设置升级初始值=============*/		
	russia.Speed = 0;
	russia.Level = 0;
	russia.Score = 0;
	
	russia.dTime = SPEED[0];
}


void GameSet()/*===========游戏设置==========*/
{
	UGL_MSG msg;
	UGL_STATUS status;
	char message_set[20]="Game Setting !";    
	
	russia.ResetFlag=1;
	ShowMessage(message_set,displayWidth/2-50,displayHeight/2-10);
	while(1)
	{
		uglOSTaskDelay(3);
		status = uglInputMsgGet (inputServiceId, &msg, UGL_NO_WAIT);
		if(status == UGL_STATUS_Q_EMPTY||msg.type != MSG_KEYBOARD)
			continue;
		if (msg.data.keyboard.modifiers & UGL_KBD_KEYDOWN)
			switch(msg.data.keyboard.key)
			{
			case UGL_UNI_LEFT_ARROW: 									
				russia.Speed --;
				if (russia.Speed < 0) 
					russia.Speed = 10;	
				RefreshCtrlPad();
				break;
				
			case UGL_UNI_RIGHT_ARROW:									
				russia.Speed ++;
				if (russia.Speed > 10) 
					russia.Speed = 0;
				RefreshCtrlPad();
				break;
				
			case UGL_UNI_DOWN_ARROW:									
				russia.Level --;
				if (russia.Level< 0) 
					russia.Level = 15;
				RefreshCtrlPad();
				break;
				
			case UGL_UNI_UP_ARROW:									
				russia.Level ++;
				if (russia.Level > 15) 
					russia.Level = 0;
				RefreshCtrlPad();
				break;
				
			case UGL_UNI_CARRIAGE_RETURN:									
				
				ClearScreen(displayWidth/2-50,displayWidth/2+50,displayHeight/2-10,displayHeight/2+30);
				RefreshCtrlPad();
				return ;
				
			default:
				break;
			}

	}
}


/**
功能:检测并执行消行
*/
void RemoveRow()
{

	int i,j;
	int row,column;
	int removeflag=1;   /*行消除标志，默认为可以消除*/
	int key=0;
	int temprow[4]={-1};
	
	for(i=0;i<Row;i++)
	{
		removeflag=1;   /*初始化行标志为可以消除*/
		for(j=0;j<Column;j++)
		{		
			if(ScreenArray[i][j]==0)/*此行有一个方块没有被填充*/
			{
				removeflag=0;
				break;
			}			
		}
		if(removeflag)			/*所有方块被填充，需要消除此行*/
		{	
			temprow[key]=i;		/*记录当前要消的行*/
			key++;
		}
	}
	russia.Lines=key;/*===================*/
	if(key<=0||key>4)
	{
		ClearScreen(GameRight+4,GameTop+366,GameRight+179,GameTop+366+195);
		return;
	} 
	RowColorChange(temprow,key);	
	for(i=key-1;i>=0 ; i--)					/*每次消除行标最大的行*/
	{
		for(row=temprow[i];row<Row-1;row++)	/*需要消除的行以上的行均下移一行*/
			for(column=0;column<Column;column++)
				ScreenArray[row][column]=ScreenArray[row+1][column];		
		for(column=0;column<Column;column++)
			ScreenArray[Row-1][column]=0;		
	}
	RefreshScreen();
	ShowEncourage(russia.Lines,russia.Level);		
	ScoreGet();/*===================*/	
}

/*初始化屏幕*/
void InitScreen()					
{	
	static UGL_CHAR * msg_change    = "  W/w     : Change Shape";
	static UGL_CHAR * msg_down 	= "  S/s       : Move Down";
	static UGL_CHAR * msg_left 	= "  A/a      : Move Left";
	static UGL_CHAR * msg_right	= "  D/d      : Move Right";



	static UGL_CHAR * msg_fastdown  	= "  L/l       : fast down";


	static UGL_CHAR * msg_set  	= "  R/r       : Game Reset";
	static UGL_CHAR * msg_quit 	= "  Esc       : Exit Game";
	static UGL_CHAR * msg_enter	= "  Enter    : set level/speed";
	static UGL_CHAR * msg_space	= "  Space   : Strat/Pause";
	static UGL_CHAR * msg_increaselevel	= "  ^         : increase level";
	static UGL_CHAR * msg_decreaselevel	= "  v          : decrease level";
	static UGL_CHAR * msg_speeddown    	= "  <         : speed down";
	static UGL_CHAR * msg_speedup      	= "  >         : speed up";
	static UGL_CHAR * msg_next = "[NEXT]";

	int textWidth, textHeight;
	int top,bottom;				/*游戏区的上下边界*/
	int left_left,left_right;	/*左侧方框的左右边界*/
	int mid_left,mid_right;		/*中间方框的左右边界*/
	int right_left,right_right;/*右侧方框的左右边界*/
	char info[40];	/*==为存放要显示的文本定义的数组===*/


	
	uglBackgroundColorSet(gc, colorTable [0].uglColor);	
	if (inputServiceId != UGL_NULL)
	{
		/*画游戏界面边框*/
		uglBatchStart(gc);
		
		/*左侧边框*/
		left_left = GameLeft-180;
		left_right = GameLeft-4;
		top = GameTop;
		bottom = GameBottom;
		if(left_left<0)
			left_left=0;
		uglForegroundColorSet(gc, colorTable[RED].uglColor);		
		uglLineWidthSet(gc, 2);
		uglRectangle(gc, left_left, top , left_right, bottom);
		
		/*游戏区边框*/
		mid_left = GameLeft-2;
		mid_right = GameRight;		
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);
		uglLineWidthSet(gc, 2);
		uglRectangle(gc, mid_left, top , mid_right, bottom);
		
		/*右侧边框*/
		right_left = GameRight+2;
		right_right = GameRight + 180;
		if(right_right>displayWidth-1)
		{
			right_right=displayWidth-1;
		}
		uglForegroundColorSet(gc, colorTable[RED].uglColor);
		uglLineWidthSet(gc, 2);
		uglRectangle(gc, right_left, top , right_right, bottom);		
		uglBatchEnd(gc);

		/*    A S D W 键的屏幕提示       */
	    /*    A S D W 图示说明                             */
		uglForegroundColorSet(gc, colorTable[BLACK].uglColor);
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+75, top+50 , left_left+105, top+80);		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+45, top+80 , left_left+75, top+110);		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+75, top+80 , left_left+105, top+110);		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+105, top+80 , left_left+135, top+110);
		
		/*ASDW 文字说明*/
		uglBackgroundColorSet(gc, colorTable[0].uglColor); /*sets a graphics context’s background color*/
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);/*sets a graphics context’s foreground color*/
		uglFontSet(gc, fontSystem);		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight,  -1, msg_change);
		uglTextDraw(gc,left_left+6, top+125, -1, msg_change);		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1,  msg_down);
		uglTextDraw(gc, left_left+6, top+textHeight+125, -1, msg_down);		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight,  -1, msg_left);
		uglTextDraw(gc, left_left+6, top+textHeight*2+125, -1, msg_left);		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_right);
		uglTextDraw(gc, left_left+6, top+textHeight*3+125, -1, msg_right);	
		

		/* 画一条黑色背景 宽5个像素的矩形框  */
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);
		uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);		 	
		uglRectangle(gc,left_left, top+220 , left_right, top+225);		
		uglForegroundColorSet(gc, colorTable[BLACK].uglColor);
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);

		uglFontSet (gc, fontSystem);
		uglTextDraw(gc, left_left+85, top+60, -1, "W");
		uglTextDraw(gc, left_left+55, top+90, -1, "A");
		uglTextDraw(gc, left_left+85, top+90, -1, "S");
		uglTextDraw(gc, left_left+115, top+90, -1, "D");	
		

		/*左边第二个方块的屏幕提示区 */
		uglForegroundColorSet(gc, colorTable[BLACK].uglColor);
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+75, top+240 , left_left+105, top+270);
		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+45, top+270 , left_left+75, top+300);
		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+75, top+270 , left_left+105, top+300);
		
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);		 	
		uglRectangle(gc,left_left+105, top+270 , left_left+135, top+300);
		
		
		/*  上下左右键的屏幕提示     */
		uglBackgroundColorSet(gc, colorTable[0].uglColor); /*sets a graphics context’s background color*/
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);/*sets a graphics context’s foreground color*/
		uglFontSet(gc, fontSystem);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight,  -1, msg_increaselevel);
		uglTextDraw(gc, left_left+6, top+315, -1, msg_increaselevel);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_decreaselevel);
		uglTextDraw(gc, left_left+6, top+textHeight*1+315, -1, msg_decreaselevel);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_speeddown);
		uglTextDraw(gc, left_left+6, top+textHeight*2+315, -1, msg_speeddown);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_speedup);
		uglTextDraw(gc, left_left+6, top+textHeight*3+315, -1, msg_speedup);
		
		/* 画一条黑色背景 宽5个像素的矩形框 */
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);
		uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);		 	
		uglRectangle(gc,left_left, top+410 , left_right, top+415);	
		
		uglForegroundColorSet(gc, colorTable[BLACK].uglColor);
		uglBackgroundColorSet(gc, colorTable[WHITE].uglColor);
		uglFontSet (gc, fontSystem);
		uglTextDraw(gc, left_left+85, top+250, -1, "^");	
		uglTextDraw(gc, left_left+85, top+280, -1, "v");
		uglTextDraw(gc, left_left+55, top+280, -1, "<");
		uglTextDraw(gc, left_left+115, top+280, -1, ">");		
		
		/*    ESC Enter Space R  L/l 键的屏幕提示*/

		uglBackgroundColorSet(gc, colorTable[0].uglColor); /*sets a graphics context’s background color*/
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);/*sets a graphics context’s foreground color*/
		uglFontSet(gc, fontSystem);


		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1,msg_fastdown);
		uglTextDraw(gc, left_left+6, top+450, -1, msg_fastdown);

		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_set);
		uglTextDraw(gc, left_left+6, top+textHeight*1+450, -1, msg_set);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_quit);
		uglTextDraw(gc, left_left+6, top+textHeight*2+450, -1, msg_quit);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight,  -1, msg_enter);
		uglTextDraw(gc,left_left+6, top+textHeight*3+450, -1, msg_enter);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_space);
		uglTextDraw(gc, left_left+6, top+textHeight*4+450, -1, msg_space);
		
		uglTextSizeGet(fontSystem, &textWidth, &textHeight, -1, msg_next);
		uglTextDraw(gc, right_left+20, top+20, -1, msg_next);/*show next brick*/
		
		/*===============游戏数据显示============================================*/
		uglForegroundColorSet(gc, colorTable[WHITE].uglColor);
		uglLineWidthSet(gc, 1);
		
		uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
		uglRectangle(gc, right_left+2, top+20*8 , right_right-2, top+20*9-15);/*两条线*/		
		uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
		uglRectangle(gc, right_left+2, top+20*18 , right_right-2, top+20*19-15);/**/
		
		ResetData();			/*初始化游戏数据*/
		RefreshCtrlPad();	/*========游戏数据显示===============*/	
	}
	FlushQ();
}

/*
使需要消去的行在消去前闪烁
int RemoveRowID[]:需要消去的行号
int removeRowCount: 需要消去的总行数
*/
void RowColorChange(int RemoveRowID[],int removeRowCount)
{
	int i,j,k,color[4][Column];
	/*记录需要闪烁的行中所有方块的颜色*/
	for(i=0;i<removeRowCount;i++)
		for(j=0;j<Column;j++)
			color[i][j]=ScreenArray[RemoveRowID[i]][j];
	for(k=0;k<6;k++)/*闪烁次数*/
	{
		for(i=0;i<removeRowCount;i++)
		{	
			for(j=0;j<Column;j++)
				if(k%2)
					ScreenArray[RemoveRowID[i]][j]=0;	 
				else
					ScreenArray[RemoveRowID[i]][j]=color[i][j];
		}
		RefreshScreen();
		uglOSTaskDelay(75);
	}
}
/*===========游戏开始菜单==========*/
int GameStartMenu()
{
	UGL_MSG msg;
	UGL_STATUS status; 
	int i;
	char message[4][40]={
		"   Welcome to This Game !  ",			
			"Press [ Enter ] to Game Set",
			"Press [ Space] to Start Game",
			"       Press ESC to Quit        "			
	};

	for(i=0;i<4;i++)
	{
		ShowMessage(message[i],displayWidth/2-100,displayHeight/2-40+20*i);
	}
	while(1)
	{
		uglOSTaskDelay(3);
		status = uglInputMsgGet (inputServiceId, &msg, UGL_NO_WAIT);
		if(status != UGL_STATUS_Q_EMPTY && msg.type == MSG_KEYBOARD)
		{
			if (msg.data.keyboard.modifiers & UGL_KBD_KEYDOWN) 
			{
				if(msg.data.keyboard.key==UGL_UNI_SPACE)
				{
					ClearScreen(displayWidth/2-180,displayHeight/2-100,displayWidth/2+180,displayHeight/2+100);
					break;
				}
				else if (msg.data.keyboard.key==UGL_UNI_CARRIAGE_RETURN)
				{
					ClearScreen(displayWidth/2-180,displayHeight/2-100,displayWidth/2+180,displayHeight/2+100);
					GameSet();
					break;
				}
				else if (msg.data.keyboard.key==UGL_UNI_ESCAPE)
				{
					ClearScreen(displayWidth/2-180,displayHeight/2-100,displayWidth/2+180,displayHeight/2+100);
					return QUIT ;
				}
			}
		}
	}
}

/*刷新控制面板*/
void RefreshCtrlPad()	
{
    int top;
    char info[40];
    int right_left,right_right;/*右侧方框的左右边界*/
	
	GameRight=displayWidth/2+200;
	GameTop=displayHeight/2-282;
    top = GameTop;
    right_left = GameRight+2;
	right_right = GameRight + 180;
	
	uglForegroundColorSet(gc, colorTable[WHITE].uglColor);
	uglBackgroundColorSet(gc, colorTable[INBLUE].uglColor);
	uglFontSet (gc, fontSystem);
	
	sprintf(info, "[Score ]");			
	uglTextDraw(gc, right_left+20, top+20*9, -1, (UGL_CHAR *)info);			    
	sprintf(info, "[Next Score ]");
	uglTextDraw(gc, right_left+20, top+20*11, -1, (UGL_CHAR *)info);		    
	sprintf(info, "[Speed]");
	uglTextDraw(gc, right_left+20, top+20*13, -1, (UGL_CHAR *)info); 		
	sprintf(info, "[Level ]");
	uglTextDraw(gc, right_left+20, top+20*15, -1, (UGL_CHAR *)info);
	
	uglLineWidthSet(gc, 1);
	uglForegroundColorSet(gc, colorTable[RED].uglColor);
	uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
	uglRectangle(gc, right_left+20, top+20*10-2 , right_right-20, top+20*11-2);/**/
	
	uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
	uglRectangle(gc, right_left+20, top+20*12-2 , right_right-20, top+20*13-2);
	
	uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
	uglRectangle(gc, right_left+20, top+20*14-2 , right_right-20, top+20*15-2);
	
	uglBackgroundColorSet(gc, colorTable[BLACK].uglColor);
	uglRectangle(gc, right_left+20, top+20*16-2 , right_right-20, top+20*17-2);
	
	sprintf(info,"  %d",russia.Score*100);					/*==============*/
	uglTextDraw(gc, right_left+20+2, top+20*10+2, -1, (UGL_CHAR *)info);
	sprintf(info,"  %d",russia.Nextscore);
	uglTextDraw(gc, right_left+20+2, top+20*12+2, -1, (UGL_CHAR *)info);
	sprintf(info,"  %d",russia.Speed);
	uglTextDraw(gc, right_left+20+2, top+20*14+2, -1, (UGL_CHAR *)info);
	sprintf(info,"  %d",russia.Level);
	uglTextDraw(gc, right_left+20+2, top+20*16+2, -1, (UGL_CHAR *)info);		  
}

/**
功能
  int row		    单元格横坐标	取值范围:0--31
  int column	    单元格纵坐标	取值范围:0--19
  int nextBlockID	基本图样类型	取值范围:0--27
*/
void  RefreshPreview(int nextBlockID)
{
	int i,j,left,right,top,bottom;
	int position[4][2]={0};
	uglBatchStart(gc);				
	uglForegroundColorSet(gc, colorTable[0].uglColor);/*颜色和背景色一样*/
	uglBackgroundColorSet(gc, colorTable[0].uglColor);
	uglLineWidthSet(gc, 1);

	/*比实际绘图区域稍微大了一点点*/
	left=GameRight+5;
	top=GameTop+35;
	right=left+140;
	bottom=top+120;
	uglRectangle(gc, left, top , right, bottom);
	DisplayBlock(21,23,nextBlockID);
	uglBatchEnd(gc);
}

/**
功能:在指定位置显示指定基本图样
  int row		单元格横坐标	取值范围:0--31
  int column	单元格纵坐标	取值范围:0--19
  int blockID   基本图样类型	取值范围:0--27
*/
void DisplayBlock(int row,int column,int blockID)
{
	/*left,right,top,bottom为像素坐标*/
	int i,left,right,top,bottom;
	int position[4][2]={0};
	
	uglBatchStart(gc);
	uglForegroundColorSet(gc, colorTable[LINECOLOR].uglColor);
	uglBackgroundColorSet(gc, colorTable[blockID/4+1].uglColor);
	uglLineWidthSet(gc, 1);
	
	for(i=0;i<4;i++)
	{
		position[i][0]=row+Block[blockID][i][0];
		position[i][1]=column+Block[blockID][i][1];
		/*将屏幕上单位格的数组坐标转换为像素坐标*/		
		left=GameLeft+20*position[i][1];
		top=GameBottom-20*position[i][0]-20;
		right=left+20;
		bottom=top+20;
		if(top>15)/**/
			uglRectangle(gc, left, top , right, bottom);
	}
	uglBatchEnd(gc);
}
/**
功能:检测基本图样是否越界或是否可以移动
Param:
int row			行标			取值范围:0--31
int column		列标			取值范围:0--19
int blockID		基本图样类型ID	取值范围:0--27
返回值定义:
CAN_MOVE: 能够移动
CANNOT_MOVE:不能移动
*/
int Check(int row,int column,int blockID)
{
	int i,sum=0,Position[4][2]={0};	
	for(i=0;i<4;i++)
	{
		Position[i][0]=row+Block[blockID][i][0];
		Position[i][1]=column+Block[blockID][i][1];
		if( Position[i][0] < 0 || Position[i][0] >= Row+4 )/*行越界*/
			return CANNOT_MOVE;
		if( Position[i][1] < 0 || Position[i][1] >= Column )/*列越界*/
			return CANNOT_MOVE;
	}	
	/*判断当前位置是否有效*/
	for(i=0;i<4;i++)
		sum+=ScreenArray[Position[i][0]][Position[i][1]];
	if(sum>0)
		return CANNOT_MOVE;
	else
		return CAN_MOVE;
}



void ScoreGet()/*===========得分计算==========*/
{
	switch (russia.Lines)/*加分 设奖励机制*/
	{
				case 1:	
					russia.Score += 1;
					
					break;
				case 2:	
					russia.Score += 3;
					
					break;
				case 3:	
					russia.Score += 5;
					
					break;
				case 4:	
					russia.Score += 10;
					
					break;
				default:
					break;
	}
	russia.Lines = 0;/*行计数重置*/
	
	if(russia.Score*100 >= russia.Nextscore)/*升级处理*/
	{
		russia.Speed++;
		russia.Speed %=10;
		russia.Level++;
		russia.Level %=16;
		russia.dTime = SPEED[russia.Speed];
		russia.Nextscore +=russia.Level*500;	
	}	
	RefreshCtrlPad();			
}
/*
功能:刷屏
*/
void RefreshScreen()
{
	int i,j;
	int left,right,top,bottom;
	
	uglBatchStart(gc);
	
	/*画出已有的方格*/
	for(i=0;i<Row;i++)
	{
		for(j=0;j<Column;j++)	
		{
			left=GameLeft+j*CheckWidth;
			top=GameBottom-(i+1)*CheckWidth;
			right=GameLeft+(j+1)*CheckWidth;				
			bottom=GameBottom-i*CheckWidth;
			if(ScreenArray[i][j]!=0)
			{
				uglForegroundColorSet(gc, colorTable[LINECOLOR].uglColor);
				uglBackgroundColorSet(gc, colorTable[ScreenArray[i][j]].uglColor);
				uglLineWidthSet(gc, 1);				
				uglRectangle(gc, left, top , right, bottom);			
			}
			else
			{
				uglForegroundColorSet(gc, colorTable[0].uglColor);/*颜色和虚线一样*/
				uglBackgroundColorSet(gc, colorTable[0].uglColor);
				uglLineWidthSet(gc, 2);	
				uglRectangle(gc, left+1, top -1, right-1, bottom-2);	
			}		
		}
	}
	uglBatchEnd(gc);	
}


/**
功能:接收键盘消息
Param:
int *row		单元格横坐标	取值范围:0--31
int *column		单元格纵坐标	取值范围:0--19
int *blockID	基本图样类型	取值范围:0--27
Retval:	消息
*/
int MessageHandle(int *row,int *column,int *blockID)
{
	UGL_MSG msg;
	UGL_STATUS status;
	int retVal=0,i;
	char message[]="Press [Space] to Resume !";

	FlushQ();
	for(i=0;i<russia.dTime;i++)/*======控制速度变化========*/
	{
		uglOSTaskDelay(3);
		status = uglInputMsgGet (inputServiceId, &msg, UGL_NO_WAIT);
		if(status != UGL_STATUS_Q_EMPTY)
		{
			if (msg.type == MSG_KEYBOARD)
			{
				if (msg.data.keyboard.modifiers & UGL_KBD_KEYDOWN) 
				{
					switch(msg.data.keyboard.key)
					{
					case 'a':
					case 'A':if(Check(*row,*column-1,*blockID)==CAN_MOVE)
								 (*column)--;
						break;
					case 's':
					case 'S':if(Check(*row-1,*column,*blockID)==CAN_MOVE)
								 (*row)--;
								else
									return NORMAL;
						break;
					case 'd':
					case 'D':if(Check(*row,*column+1,*blockID)==CAN_MOVE)
								 (*column)++;
						break;
					case 'w':
					case 'W':if(Check(*row,*column,ChangeShape(*blockID))==CAN_MOVE)
								 *blockID=ChangeShape(*blockID);
						break;
					case ' ':
						ShowMessage(message,displayWidth/2-100,displayHeight/2-10);
						while(1){
							uglOSTaskDelay(3);
							status = uglInputMsgGet (inputServiceId, &msg, UGL_NO_WAIT);
							if(status != UGL_STATUS_Q_EMPTY && msg.type == MSG_KEYBOARD)
								if (msg.data.keyboard.modifiers & UGL_KBD_KEYDOWN) 
									if(msg.data.keyboard.key==' ')
									{
										ClearScreen(displayWidth/2-50,displayWidth/2+50,displayHeight/2-10,displayHeight/2+10);
										break;
									}
						}
						break;
					case 'r':
					case 'R':
						return RESET;
					case 'l':
					case 'L':while(Check(*row-1,*column,*blockID)==CAN_MOVE)
							 {
								(*row)--;
								RefreshScreen();
								DisplayBlock(*row,*column,*blockID);
								uglOSTaskDelay(15);
							 }
						return NORMAL;
						
					case UGL_UNI_CARRIAGE_RETURN:/*按ENTER键进入游戏设置状态*/
						InitGameShow();
						
						return GAMESET;
						
					case UGL_UNI_ESCAPE:
						return QUIT;	
						
					default: break;
					}
				}
				RefreshScreen();
				DisplayBlock(*row,*column,*blockID);
			}
		}
	}
	return NORMAL;	
}

/*初始化程序中的全局变量*/
void Init()
{	
	UGL_REG_DATA *pRegistryData;
	UGL_FONT_DEF systemFontDef;
	UGL_FONT_DEF fixedFontDef;
	UGL_ORD textOrigin = UGL_FONT_TEXT_UPPER_LEFT;	
	UGL_FB_INFO fbInfo;

	if (uglInitialize() == UGL_STATUS_ERROR)
		return;
	
	/* 获取显示器ID*/
	pRegistryData = uglRegistryFind (UGL_DISPLAY_TYPE, 0, 0, 0);
	if (pRegistryData == UGL_NULL)
	{
		printf("Display not found. Exiting.\n");
		uglDeinitialize();
		return;
	}
	devId = (UGL_DEVICE_ID)pRegistryData->id;
	/* 获得输入设备ID */	
	pRegistryData = uglRegistryFind (UGL_INPUT_SERVICE_TYPE, 0, 0, 0);
	if (pRegistryData == UGL_NULL)
	{
		printf("Input service not found. Exiting.\n");
		uglDeinitialize();
		return;
	}
	inputServiceId = (UGL_INPUT_SERVICE_ID)pRegistryData->id;	
	/* 创建graphics context */	
	gc = uglGcCreate(devId);	
	/*创建字体*/
	pRegistryData = uglRegistryFind (UGL_FONT_ENGINE_TYPE, 0, 0, 0);
	if (pRegistryData == UGL_NULL)
	{
		printf("Font engine not found. Exiting.\n");
		uglDeinitialize();
		return;
	}
	fontDrvId = (UGL_FONT_DRIVER_ID)pRegistryData->id;
	uglFontDriverInfo(fontDrvId, UGL_FONT_TEXT_ORIGIN, &textOrigin);
	uglFontFindString(fontDrvId, "familyName=Lucida; pixelSize = 12", &systemFontDef);
	if ((fontSystem = uglFontCreate(fontDrvId, &systemFontDef)) == UGL_NULL)
	{
		printf("Font not found. Exiting.\n");
		return;        
	}
	/* 获取显示器信息 */
	uglInfo(devId, UGL_FB_INFO_REQ, &fbInfo);	
	displayWidth = fbInfo.width;
	displayHeight = fbInfo.height;	

	/*为设备分配颜色*/
	uglColorAlloc (devId, &colorTable[INBLUE].rgbColor, UGL_NULL, 
		&colorTable[INBLUE].uglColor, 1);
	uglColorAlloc(devId, &colorTable[LIGHTPURPLE].rgbColor, UGL_NULL,
		&colorTable[LIGHTPURPLE].uglColor, 1);
	uglColorAlloc(devId, &colorTable[ORANGE].rgbColor, UGL_NULL,
		&colorTable[ORANGE].uglColor, 1);
	uglColorAlloc(devId, &colorTable[YELLOW].rgbColor,  UGL_NULL,
		&colorTable[YELLOW].uglColor, 1);
	uglColorAlloc(devId, &colorTable[WHITE].rgbColor,  UGL_NULL,
		&colorTable[WHITE].uglColor, 1);
	uglColorAlloc(devId, &colorTable[LIGHTGREEN].rgbColor, UGL_NULL,
		&colorTable[LIGHTGREEN].uglColor, 1);
	uglColorAlloc(devId, &colorTable[RED].rgbColor, UGL_NULL,
		&colorTable[RED].uglColor, 1);
	uglColorAlloc(devId, &colorTable[LIGHTBLUE].rgbColor, UGL_NULL,
		&colorTable[LIGHTBLUE].uglColor, 1);
	uglColorAlloc(devId, &colorTable[LINECOLOR].rgbColor, UGL_NULL,
		&colorTable[LINECOLOR].uglColor, 1);
	uglColorAlloc(devId, &colorTable[BLACK].rgbColor, UGL_NULL,
		&colorTable[BLACK].uglColor, 1);
	
	/*定义游戏显示区*/
	GameLeft=displayWidth/2-202;
	GameRight=displayWidth/2+200;
	GameTop=displayHeight/2-282;
	GameBottom=displayHeight/2+280;
	if(GameLeft<0)
		GameLeft=0;
	if(GameRight>displayWidth-1)
		GameRight=displayWidth-1;
	if(GameTop<0)
		GameTop=0;
	if(GameBottom>displayWidth-1)
		GameBottom=displayWidth-1;
}

#if defined(WINDML_NATIVE) && defined(__unix__)
int main (int argc, char *argv [])
{
    int mode = 0;
    if (argc > 1) 
		mode = atoi (argv [1]);
	
    Russia (mode);
    return 0;
}
#elif defined(WINDML_NATIVE) && defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd)
{
    uglWin32Parameters(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    Russia(0);
    return (0);
}
#else

void StartGame(int mode)
{
    uglOSTaskCreate("tRussia", (UGL_FPTR)Russia, 110, 0, 10000,mode,0,0,0,0);
}
#endif

void Russia()
{ 
	int i,j,row,column,temp,retVal,BlockID,nextBlockID;
	Init();	
	InitScreen(); 			/*初始化屏幕*/
	retVal=GameStartMenu();
	if(retVal ==QUIT)
	{
		CleanUp();
		return;
	}

	BlockID=(rand()+russia.Score+russia.Speed) % 28;	/*生成一个新的基本图形*/
	while(1)
	{
		row=Row-1;			/*新图形置顶*/
		column=Column/2-1;	/*设置方块下落的起始横坐标为屏幕中心偏左1格*/
		nextBlockID=(rand()+russia.Score+russia.Speed) % 28;	/*生成一个新的基本图形*/
		/*======游戏设置判断有设置则为1 然后重置为0=======*/
		if(russia.ResetFlag==1)
		{
			russia.ResetFlag=0;
			russia.dTime = SPEED[russia.Speed];
			/*根据设置的不同的等级 设置不同级别初始屏幕*/
			temp=rand()%7+1;
			for (i = 0; i < Column; i++)
				for (j=0;j < russia.Level; j++)
					if(ScreenArray[j][i] = (rand()) % 2)
						ScreenArray[j][i]=temp;
		}
		RefreshCtrlPad();									/*刷新游戏设置区域*/
		RefreshPreview(nextBlockID);						/*刷新预览区域,显示下一个图形*/	
		if(Check(row-1,column,BlockID)==CANNOT_MOVE)
		{
			InitGameShow();
			/*根据设置的不同的等级 设置不同级别初始屏幕*/
			temp=rand()%7+1;
			for (i = 0; i < Column; i++)
				for (j=0;j < russia.Level; j++)
					if(ScreenArray[j][i] = (rand()) % 2)
						ScreenArray[j][i]=temp;
			continue;
		}
		
		/*如果Block不能通过Check函数，说明Block到达最底端*/
		while(Check(row-1,column,BlockID)==CAN_MOVE)
		{
			RefreshScreen();
			row=row-1;/*图形下降1行*/
			DisplayBlock(row,column,BlockID);/*重新显示*/	
			retVal=MessageHandle(&row,&column,&BlockID);	/*检测键盘输入*/			
			if(retVal!=NORMAL)
				break;
		}
		if(retVal==QUIT)
		{
			InitGameShow();
			break;
		}
		if(retVal==RESET)			
		{
			ResetData();			
			InitGameShow();
		}
		if(retVal==GAMESET)
		{
			GameSet();
			InitGameShow();
		}
		if(retVal==NORMAL)
		{
			UpdateArray(row,column,BlockID);
			RemoveRow();
		}
		BlockID=nextBlockID;
		RefreshScreen();
	}
	CleanUp();
	return;
}
/*
函数功能：在指定的地方显示字符串
参数：
	message:待显示的字符串
	left和top：字符串左上角像素的坐标
*/
void ShowMessage(char message[],int left,int top)
{ 
	UGL_REG_DATA *pRegistryData;
	UGL_FONT_DRIVER_ID fontDriveId;
    UGL_FONT_DEF systemFontDef;
    UGL_FONT_DEF fixedFontDef;
    UGL_ORD textOrigin = UGL_FONT_TEXT_UPPER_LEFT;
    int numRandomPoints;
    int i, index, textpage, tmp;
    int textWidth, textHeight;
    UGL_FB_INFO fbInfo;

    pRegistryData = uglRegistryFind (UGL_FONT_ENGINE_TYPE, 0, 0, 0);
    if (pRegistryData == UGL_NULL)
    {
		printf("Font engine not found. Exiting.\n");
		uglDeinitialize();
		return;
	}
    fontDriveId = (UGL_FONT_DRIVER_ID)pRegistryData->id;
    uglFontDriverInfo(fontDriveId, UGL_FONT_TEXT_ORIGIN, &textOrigin);
    uglFontFindString(fontDriveId, "familyName=Lucida; pixelSize = 20", &systemFontDef);
	
    if ((fontSystem = uglFontCreate(fontDriveId, &systemFontDef)) == UGL_NULL)
    {
		printf("Font not found. Exiting.\n");
		return;        
    }
    uglBatchStart(gc);   
	uglForegroundColorSet(gc, colorTable[RED].uglColor);
	uglBackgroundColorSet(gc, colorTable[INBLUE].uglColor);
	uglFontSet(gc, fontSystem);
	uglTextSizeGet(fontSystem, UGL_NULL, &tmp, -1, message);
	uglTextDraw(gc, left,top, -1, message);
    uglBatchEnd(gc);
	fontDrvId = (UGL_FONT_DRIVER_ID)pRegistryData->id;
	uglFontDriverInfo(fontDrvId, UGL_FONT_TEXT_ORIGIN, &textOrigin);
	uglFontFindString(fontDrvId, "familyName=Lucida; pixelSize = 12", &systemFontDef);
	if ((fontSystem = uglFontCreate(fontDrvId, &systemFontDef)) == UGL_NULL)
	{
		printf("Font not found. Exiting.\n");
		return;        
	}	
}

/*
函数功能：清除屏幕指定区域的图像
参数：
	left和top：左上角像素坐标
	right和bottom：右下角像素坐标
*/
void ClearScreen(int left,int top,int right,int bottom)/*待清除的屏幕区域，参数为左上角和右下角的坐标*/
{ 
    uglBatchStart(gc);
	uglForegroundColorSet(gc, colorTable[0].uglColor);
	uglBackgroundColorSet(gc, colorTable[0].uglColor);
	uglLineWidthSet(gc, 1);	
	uglRectangle(gc, left,top,right,bottom);
    uglBatchEnd(gc);	
}

/*
函数功能：消行后显示鼓励信息
参数：
	int lines：一次消去的行数
	int level：当前关数
*/
void ShowEncourage(int lines,int level)
{
	/* 字体大小显示*/
    UGL_REG_DATA *pRegistryData;
    UGL_FONT_DRIVER_ID fontDriveId;
    UGL_FONT_DEF systemFontDef;
    UGL_ORD textOrigin = UGL_FONT_TEXT_UPPER_LEFT;
    /*边界线*/
	int left=GameRight+4;
	int top=GameTop+366;
	int right=left+175;
	int bottom=top+195;
	char info[4][40]={"Come On !","Well Done !","Very Good !","Great !!!!"};	
	char fontstr[40];
	int i;
	
	pRegistryData = uglRegistryFind (UGL_FONT_ENGINE_TYPE, 0, 0, 0);
    if (pRegistryData == UGL_NULL)
    {
		printf("Font engine not found. Exiting.\n");
		uglDeinitialize();
		return;
    }
    fontDriveId = (UGL_FONT_DRIVER_ID)pRegistryData->id;
    uglFontDriverInfo(fontDriveId, UGL_FONT_TEXT_ORIGIN, &textOrigin);

	/*  刷新显示区域*/ 
	uglFontSet (gc, fontSystem);
    
	uglBackgroundColorSet(gc, colorTable[0].uglColor);
	uglLineWidthSet( gc, 8);	
	
	for(i=0;i<4;i++)
	{
		uglForegroundColorSet(gc, colorTable[rand()%7+1].uglColor);
		uglRectangle(gc,left+4, top+4, right-4, bottom-4);
		
		sprintf(fontstr,"familyName=Lucida; pixelSize = %d",(i+1)*5);
		uglFontFindString(fontDriveId, fontstr, &systemFontDef);
		fontSystem = uglFontCreate(fontDriveId, &systemFontDef);
		uglFontSet (gc, fontSystem);
		
		uglForegroundColorSet(gc, colorTable[rand()%7+1].uglColor);
		uglTextDraw(gc, left+45-i*2, top+80, -1, (UGL_CHAR *)info[lines-1]);
		uglOSTaskDelay (200);
	}
	
    uglFontFindString(fontDrvId, "familyName=Lucida; pixelSize = 12", &systemFontDef);	
    if ((fontSystem = uglFontCreate(fontDrvId, &systemFontDef)) == UGL_NULL)
    {
		printf("Font not found. Exiting.\n");
		return;        
    }
}