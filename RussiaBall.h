#if !defined(WINDML_NATIVE)
#include <vxWorks.h>
#elif defined(__unix__)
#include <unistd.h>
#endif

#include <ugl/ugl.h>
#include <ugl/uglos.h>
#include <ugl/uglMsg.h>
#include <ugl/uglfont.h>
#include <ugl/uglinput.h>
#include <ugl/ugldib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugl/uglucode.h>/*方向键定义函数*/

UGL_DEVICE_ID devId;
UGL_GC_ID gc;
UGL_INPUT_SERVICE_ID inputServiceId;
UGL_FONT_ID fontSystem;
UGL_FONT_DRIVER_ID fontDrvId;

/*得分计算与控制数据结构定义===============*/

/*速度定义-数组表示*/
static int SPEED[11] = {60, 50, 40, 35, 30, 25, 20, 15, 10, 7, 5};

typedef struct RUSSIA_SCORE_DATA
{
	int		ResetFlag;
	int		Lines;/*消行时获得行数*/
	int		Score;
	int		Speed;
	int 	Level;
	int		dTime;
	int 	Nextscore;
}Russia_Data;
Russia_Data russia;

int displayHeight, displayWidth;
/*硬件相关的数据,目的是为了做到平台无关*/
int GameLeft;
int GameRight;
int GameTop;
int GameBottom;

#define Row			28									/*游戏区的行数*/
#define Column		20									/*游戏区的列数*/
#define CheckWidth  20									/*每个小格子的像素数*/
int 	moveStep=20;									/*每次小图形移动的像素数*/
int 	ScreenArray[Row+4][Column]={0};					/*屏幕对应的数组,屏幕显示的依据*/

void FlushQ ();											/*清除消息队列*/
void Russia();											/*游戏开始*/
void Init();
void InitScreen();										/*初始化屏幕*/
int  GameStartMenu();									/*游戏开始菜单*/
int  MessageHandle(int *row,int *column,int *blockID);	/*消息处理函数*/


void GameSet();											/*进入游戏设置*/
void ScoreGet();										/**/
void ResetData();										/*数据重置*/
void RefreshCtrlPad();									/*刷新游戏控制区域*/
void RefreshScreen();									/*刷新游戏区*//*Over*/
void RefreshPreview(int nextBlockID);

void DisplayBlock(int row,int column,int blockID);
int  ChangeShape(int blockId);							/*图形变换，成功返回1，否则返回0，但本程序不使用*/
int  Check(int row,int column,int blockID);
void RemoveRow();										/*消除一行*/
void RowColorChange(int a[],int count);
void UpdateArray(int row,int column,int blockID);

void ClearScreen(int left,int top,int right,int bottom);	/*清除屏幕指定区域，参数为左上角和右下角的坐标*/
void ShowMessage(char msg[],int left,int top);				/*msg:待显示的字符串，left和top：字符串左上角像素的坐标*/
void ShowEncourage(int rows,int level);						/*显示右下角提示*/

struct _colorStruct
{
    UGL_RGB rgbColor;
    UGL_COLOR uglColor;
}colorTable[] =
{
    { UGL_MAKE_RGB(0x1E, 0x90, 0xff), 0},
    { UGL_MAKE_RGB(0xff, 0x6E, 0xB4), 0},
    { UGL_MAKE_RGB(0xff, 0x30, 0x30), 0},
    { UGL_MAKE_RGB(0xff, 0xff, 0x00), 0},
    { UGL_MAKE_RGB(0xff, 0x8c, 0x00), 0},
    { UGL_MAKE_RGB(0x7c, 0xfc, 0x00), 0},
    { UGL_MAKE_RGB(0xfc, 0xfc, 0xfc), 0},
    { UGL_MAKE_RGB(0x7a, 0x67, 0xee), 0},
	{ UGL_MAKE_RGB(0x99, 0xd1, 0xd3), 0},
	{ UGL_MAKE_RGB(0, 0, 0), 0}
};

#define INBLUE			(0)  
#define LIGHTPURPLE		(1)
#define RED				(2)
#define YELLOW			(3)
#define ORANGE			(4)
#define LIGHTGREEN		(5)
#define WHITE			(6)
#define LIGHTBLUE		(7)
#define LINECOLOR       (8)
#define BLACK			(9)

#define CANNOT_MOVE		(0)
#define CAN_MOVE		(1)
#define NORMAL			(0)
#define RESET			(1)
#define GAMESET			(2)
#define QUIT			(3)

int Block[28][4][2]=
{
	{/*0-0竖条*/
		{0,1},
		{1,1},
		{2,1},
		{3,1}
	},
	{/*0-1竖条*/
		{1,0},
		{1,1},
		{1,2},
		{1,3}
	},
	{/*0-2竖条*/
		{0,1},
		{1,1},
		{2,1},
		{3,1}
	},
	{/*0-3竖条*/
		{1,0},
		{1,1},
		{1,2},
		{1,3}
	},
	{/*1-0田字*/
		{2,1},
		{2,2},
		{3,1},
		{3,2}
	},
	{/*1-1田字*/
		{2,1},
		{2,2},
		{3,1},
		{3,2}
	},
	{/*1-2田字*/
		{2,1},
		{2,2},
		{3,1},
		{3,2}
	},
	{/*1-3田字*/
		{2,1},
		{2,2},
		{3,1},
		{3,2}
	},
	{/*2-0Z形*/
		{2,0},
		{2,1},
		{3,1},
		{3,2}
	},
	{/*2-1Z形*/
		{1,1},
		{2,0},
		{2,1},
		{3,0}
	},
	{/*2-2Z形*/
		{1,0},
		{1,1},
		{2,1},
		{2,2}
	},
	{/*2-3Z形*/
		{1,2},
		{2,2},
		{2,1},
		{3,1}
	},
	{/*3-0反Z形*/
		{2,1},
		{2,2},
		{3,0},
		{3,1}
	},
	{/*3-1反Z形*/
		{1,0},
		{2,0},
		{2,1},
		{3,1}		
	},
	{/*3-2反Z形*/	
		{1,1},
		{1,2},
		{2,0},
		{2,1}
	},
	{/*3-3反Z形*/
		{1,1},
		{2,1},
		{2,2},
		{3,2}	
	},
	{/*4-0L形*/
		{1,1},
		{2,1},
		{3,1},
		{3,2}
	},
	{/*4-1L形*/
		{2,0},
		{2,1},
		{2,2},
		{3,0}
	},
	{/*4-2L形*/
		{1,0},
		{1,1},
		{2,1},
		{3,1}
	},
	{/*4-3L形*/
		{1,2},
		{2,0},
		{2,1},
		{2,2}
	},
	{/*5-0反L形*/
		{1,2},
		{2,2},
		{3,2},
		{3,1}
	},
	{/*5-1反L形*/
		{1,1},
		{2,1},
		{2,2},
		{2,3}
	},
	{/*5-2反L形*/
		{1,2},
		{2,2},
		{3,2},
		{1,3}
	},
	{/*5-3反L形*/
		{2,1},
		{2,2},
		{2,3},
		{3,3}
	},
	{/*6-0T形*/
		{2,0},
		{2,1},
		{2,2},
		{3,1}
	},
	{/*6-1T形*/
		{1,1},
		{2,0},
		{2,1},
		{3,1}
	},
	{/*6-2T形*/
		{2,0},
		{2,1},
		{2,2},
		{1,1}
	},
	{/*6-3T形*/
		{1,1},
		{2,1},
		{3,1},
		{2,2}
	}
};