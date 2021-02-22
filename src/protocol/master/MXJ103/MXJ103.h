#ifndef _MXJ103_H
#define _MXJ103_H

#define XJ103_Slaver_MAX_BUF_LEN 255

//*********************************************************************************//
//    控制函数 dwCommand参数宏定义
//*********************************************************************************//
#define TP_YKSELECT    	1    //遥控预置
#define TP_YKOPERATE  	2    //遥控执行
#define TP_YKCANCEL  	3    //遥控撤消
#define TP_YKDIRECT  	4    //遥控直接执行

#define TP_OPEN      	2   //遥控、同期开关属性为分
#define TP_CLOSE      	1   //遥控、同期开关属性为合
#define TP_NULL      	0   //不区分开关属性

#define	PBI_ON			1
#define PBI_OFF			0

#define MAX_SUBDEVNO 32
#define maxDevNum		255/*连接最大装置数 设置为255*/


//==== 控制域的定义 ====

//启动报文位定义
#define PRM_MASTER     1        //启动站
#define PRM_SLAVE      0        //从动站

//帧计数有效位的宏定义
#define FCV_VALID      1        //帧计数位有效
#define FCV_INVALID    0        //帧计数位无效

//要求访问位定义
#define ACD_HAVECLASS1 1        //有1级用户数据
#define ACD_NOCLASS1   0        //无1级用户数据

//数据流控制位定义
#define DFC_OK         0        //正常
#define DFC_OVERFLOW   1        //缓冲区满

//功能码的定义
//FC_S 表示监视方向(从站)
#define FC_S_ACK          0     //确认
#define FC_S_BUSY         1     //链路忙
#define FC_S_DATA         8     //以数据响应
#define FC_S_NODATA       9     //无召唤数据
#define FC_S_LINKOK       11    //链路正常
#define FC_S_LINKERROR    14    //链路服务未工作
#define FC_S_LINKUNFIN    15    //链路服务未完成


//FC_M 表示控制方向(主站)
#define FC_M_RESETCU      0     //复位通信单元
#define FC_M_DATA         3     //传送数据,要求确认
#define FC_M_BROADCAST    4     //传送数据,无应答
#define FC_M_RESETFCB     7     //复位帧计数位
#define FC_M_CALLLINK     9     //召唤链路状态
#define FC_M_CALLCLASS1   10    //召唤1级数据
#define FC_M_CALLCLASS2   11    //召唤2级数据

//==== 应用服务数据单元定义 ====

//类型标识的定义,1-31为兼容范围,32-255为专用范围
//TYP_S 表示监视方向(从站)
#define TYP_S_TM       1        //带时标的报文
#define TYP_S_TMR      2        //带相对时标的报文
#define TYP_S_ME1      3        //被测值Ⅰ
#define TYP_S_TME      4        //带相对时标的被测值
#define TYP_S_IDENT    5        //标识
#define TYP_S_SST      6        //时间同步
#define TYP_S_CALLSTOP 8        //总召唤终止
#define TYP_S_ME2      9        //被测值Ⅱ
#define TYP_S_GDATA    10       //通用分类数据
#define TYP_S_GIDENT   11       //通用分类标识
///////wsnrrcs9612
#define TYP_S_ALLYC    50        //0x32全遥测类型
#define TYP_S_ALLYX    44        //0x2c全遥信类型
#define TYP_S_YKFX     64        //0x40遥控类型
#define TYP_S_BWYX_NOTIME     40        //0x28变位遥信不带时标
#define TYP_S_BWYX_TIME       41        //0x29变位遥信带时标
#define TYP_S_DISTURB_DATA    23        //0x17扰动数据
//////wsnrrcs9621
//ZYM
#define TYP_S_ALLDD		36			//电度量
#define TYP_S_YBBW_TIME 45			//压板变位

//TYP_M 表示控制方向(主站),
#define TYP_M_SST      6        //时间同步
#define TYP_M_CALL     7        //总召唤
#define TYP_M_GDATA    10       //通用分类数据
#define TYP_M_COMMAND  20       //一般命令
#define TYP_M_GCOMMAND 21       //通用命令
//////wsnr
#define TYP_M_YK       64       //0x40遥控类型
#define TYP_M_FUGUI    20       //0x14复归类型
/////wsnr

//传送原因,1-63为兼容范围,64-255为专用范围
//COT_S 表示监视方向(从站)
#define COT_S_BURST    1        //突发
#define COT_S_CYCLE    2        //循环
#define COT_S_RESETFCB 3        //复位帧计数位
#define COT_S_RESETCU  4        //复位通信单元
#define COT_S_TEST     7        //测试模式
#define COT_S_SST      8        //时间同步
#define COT_S_CALL     9        //总召唤
#define COT_S_CALLSTOP 10       //总召唤终止
#define COT_S_LOCAL    11       //当地操作
#define COT_S_REMOTE   12       //远方操作
#define COT_S_ACK      20       //命令的肯定认可
#define COT_S_NAK      21       //命令的否定认可
#define COT_S_GWACK    40       //通用分类写命令的肯定认可
#define COT_S_GWNAK    41       //通用分类写命令的否定认可
#define COT_S_GRACK    42       //通用分类读命令的有效数据响应
#define COT_S_GRNAK    43       //通用分类读命令的无效数据响应
#define COT_S_GWANS    44       //通用分类写确认

//COT_M 表示控制方向(主站)
#define COT_M_SST      8        //时间同步
#define COT_M_CALL     9        //总召唤的启动
#define COT_M_COMMAND  20       //一般命令
#define COT_M_GWRITE   40       //通用分类写命令
#define COT_M_GREAD    42       //通用分类读命令
/////wsnrrcs9621rcs9612
#define COT_M_REMOTE   12       //远方操作

//==== 任务标志定义 ====
#define SF_LINKSTATUS_ERROR 0   //链路状态标志

#define SF_CALLLINK         1   //召唤链路状态
#define SF_RESETCU          2   //复位通信单元
#define SF_RESETFCB         3   //复位帧计数位

#define SF_CALLYC           7
#define SF_CALLALL          8   //总召唤
#define SF_DATA             9   //传送数据,要求确认
#define SF_BRAODCAST        10  //传送数据,无应答
#define SF_CALLCLASS1       11  //召唤1级数据
#define SF_CALLCLASS2       12  //召唤2级数据
#define SF_SST              13  //对时

//通用分类数据的任务标志
#define SF_GEN_RGROUPITEM   16  //读所有被定义组的标题
#define SF_GEN_RGROUPVALUE  17  //读一个组的全部条目的值或属性
#define SF_GEN_RENTRYITEM   18  //读单个条目的目录
#define SF_GEN_RENTRYVALUE  19  //读单个条目的值或属性
#define SF_GEN_CALLALL      20  //对通用分类数据的总查询
#define SF_GEN_WENTRY       21  //写条目
#define SF_GEN_WENTRYACK    22  //带确认的写条目
#define SF_GEN_WENTRYEXEC   23  //带执行的写条目
#define SF_GEN_WENTRYCANCEL 24  //写条目终止

//帧同步字符的定义
#define SYN_START_10   0x10     //固定帧长启动字符
#define SYN_START_68   0x68     //可变帧长启动字符
#define SYN_STOP       0x16     //帧结束字符

//功能类型
#define FUN_BU         1        //间隔单元
#define FUN_TZ         128      //距离保护
#define FUN_I          160      //过流保护
#define FUN_IT         176      //变压器差动保护
#define FUN_IL         192      //线路差动保护
#define FUN_GEN        254      //通用分类
#define FUN_GLB        255      //全局


//lgh---
#define TP_DZ_CALL_QH        1    //定值区号召唤
#define TP_DZ_CALL_SEL      2    //定值召唤预置
#define TP_DZ_CALL_EXE      3    //定值召唤执行
#define TP_DZ_HAVE_DATA   4    //可以召唤定值一级数据
#define TP_DZ_MODI_QH       5     //定值区号下发
#define TP_DZ_MODI_SEL      6    //定值修改预置
#define TP_DZ_MODI_EXE      7    //定值修改执行


//ZYM B600 定值召唤
#define CALL_DZ_FLAG0    0//空闲
#define CALL_DZ_FLAG1    1//请求
#define CALL_DZ_FLAG2    2//召唤完成
#define CALL_DZ_ERROR    3//召唤错误

#define DZ_NULL         0
#define DZ_SELECT	1
#define DZ_OPERATE      2

#define DZ_CHG_NULL     0
#define DZ_CHG_PREPARE  1
#define DZ_CHG_OPERATE  2

//240-255,通用分类功能
#define INF_M_GEN_RGROUPITEM   240   //读所有被定义组的标题
#define INF_M_GEN_RGROUPVALUE  241   //读一个组的全部条目的值或属性
#define INF_M_GEN_RENTRYITEM   243   //读单个条目的目录
#define INF_M_GEN_RENTRYVALUE  244   //读单个条目的值或属性
#define INF_M_GEN_CALLALL      245   //对通用分类数据的总查询
#define INF_M_GEN_WENTRY       248   //写条目
#define INF_M_GEN_WENTRYACK    249   //带确认的写条目
#define INF_M_GEN_WENTRYEXEC   250   //带执行的写条目
#define INF_M_GEN_WENTRYCANCEL 251   //写条目终止



//广播地址的定义
#define MASTER_BROADCAST_ADDRESS  0xFF      //主站对从站的广播



//应用服务数据单元结构
typedef struct
{
	BYTE byTYP;                 //类型标识
	BYTE byVSQ;                 //可变结构限定词
	BYTE byCOT;                 //传送原因
	BYTE byADDRESS;             //应用服务单元地址
	BYTE byFUN;                 //功能类型
	BYTE byINF;                 //信息序号
	BYTE byData;                //数据区
}TAsdu;


typedef struct
{
	BYTE RESERVED:1;
	BYTE PRM:1;
	BYTE FCB:1;
	BYTE FCV:1;
	BYTE FC:4;
}TTxdControl;


//定义处理时需要存储的信息结构
typedef struct
{
	BYTE byCurrentFCB;          //每次正确接收后保存的FCB位
	BYTE byScanNumber;          //扫描序号
	BYTE byCOL;                 //兼容级别
}TInfo;

//固定长度数据帧头结构(发送)
typedef struct
{
	BYTE byStart;               //启动字符
	TTxdControl Control;        //控制域
	BYTE byAddress;             //地址域
	BYTE byChecksum;            //帧校验和
	BYTE byStop;                //结束字符
}TTxdFrameFix;

//可变长度数据帧头结构(发送)
typedef struct
{
	BYTE byStart1;              //启动字符
	BYTE byLength1;             //长度
	BYTE byLength2;             //长度(重复)
	BYTE byStart2;              //启动字符(重复)
	TTxdControl Control;        //控制域
	BYTE byAddress;             //地址域
	TAsdu Asdu;                 //应用服务数据单元
}TTxdFrameVar;

//发送帧头结构
typedef union
{
	TTxdFrameFix FrameFix_10;
	TTxdFrameVar FrameVar_68;
}TTxdFrameHead;

//接收帧头结构
typedef union
{
	TTxdFrameFix FrameFix_10;
	TTxdFrameVar FrameVar_68;
}TRxdFrameHead;


//后续数据结构
typedef struct
{
	BYTE Number:7;              //数目
	BYTE Cont:1;                //后续状态位
}TNext;


//通用分类标识序号结构定义
typedef struct
{
	BYTE byGroup;               //组
	BYTE byEntry;               //条目
}TGin;

//通用分类数据描述结构
typedef struct
{
	BYTE byDataType;            //数据类型
	BYTE byDataSize;            //数据宽度
	TNext Next;                 //后续数据
}TGdd;


//通用分类数据结构
typedef struct
{
	TGin Gin;                   //通用分类标识序号
	BYTE byKod;                 //描述的类别
	TGdd Gdd;                   //通用分类数据描述
	BYTE byGdi;                 //通用分类标识数据
}TGenData;



int Init_XJ103Master(int DevID);

#endif

