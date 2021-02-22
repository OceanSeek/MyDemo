#define Slaver_Sta_Addr	1
#define TAILLEN	2

#define DZ_Clear	0
#define DZ_YU_ZHI	1
#define DZ_JI_HUO	2
#define DZ_STOP		3
#define DZ_TU_FA	1 
#define DZ_YZ_CONFIRM	2 //定值预置确认
#define TP_DZ_CALL_SEL      2    //定值召唤预置
#define TP_DZ_CALL_EXE      3    //定值召唤执行

#define Dz_DevID_3      3    //设备ID号3的设备用作定值缓存


#define     GX101Slaver_STABLE_BEGING             0X10
#define     GX101Slaver_STABLE_END                0X16
#define     GX101Slaver_STABLE_LEN                0X06

#define 	PBI_ON						1
#define 	PBI_OFF						0


/*
 * DIR, PRM, FCB, FCV, ACD, DFC
 * */
#define     GX101Slaver_CTRL_DIR_DOWN            0x01
#define     GX101Slaver_CTRL_DIR_UP              0x00
#define     GX101Slaver_CTRL_FCB_OPPO_BIT        0x01
#define     GX101Slaver_CTRL_FCB_OPPO_NONE       0x00
#define     GX101Slaver_CTRL_PRM_MASTER          0x01
#define     GX101Slaver_CTRL_PRM_SLAVE           0x00
#define     GX101Slaver_CTRL_FCV_ENABLE          0x01
#define     GX101Slaver_CTRL_FCV_DISABLE         0x00
#define     GX101Slaver_CTRL_ACD_WITH_DATA       0x01
#define     GX101Slaver_CTRL_ACD_NONE_DATA       0x00
#define     GX101Slaver_CTRL_DFC_CAN_REC         0x00
#define     GX101Slaver_CTRL_DFC_CANNOT_REC      0x01

#define     GX101Slaver_CTRL_RES_CONFIRM        		 0x00
#define     GX101Slaver_CTRL_RESPONSE_LINK_STATUS     0x0B


#define TYPE_101	1
#define TYPE_104	2

#define GX101Slaver_MAX_BUF_LEN  256


//帧控制域定义
#define BIT_DIR    0x80    //传输方向位
#define BIT_PRM    0x40    //启动报文位
#define BIT_FCB    0x20    //帧计数位
#define BIT_FCV    0x10    //帧计数有效位
#define BIT_CODE   0x0F    //功能码所占位

#define Dev1_DZ_Start		10	//定值缓冲区起始地址
#define Dev2_DZ_Start		70	//定值缓冲区起始地址
#define Dev3_DZ_Start		130	//定值缓冲区起始地址


#define DZ_CALL_ALL	3 //定值总召唤
#define DZ_CALL_GROUP	4 //定值组总召唤


#define Type_101		1	
#define Type_104		2	
#define Type_clear		0	


#pragma pack(1)  //按1字节对齐

//typedef union{
//	float  _float;
//	uint32_t _int32;
//}Data_Value_T, *PData_Value_T;


//typedef struct{   //规约io 
//    unsigned short BufSize;		//缓冲区大小
//    unsigned short ReadPtr;		//读指针
//    unsigned short OldReadPtr;  //读指针
//    unsigned short WritePtr;	//写指针
//    WORD	Address;			//地址码(主要用于发送)
//    DWORD	Flag;				//控制码(主要用于发送)
//    uint8_t  Buf[255];		//缓冲区
//}TCommIO;


typedef struct 
{
	BYTE Dev1;
	BYTE Dev2;
	BYTE Dev3;
	
}DZRETURNADDR_T;

//typedef  struct {
//
//	uint8_t _func:4;			   /*function*/
//	uint8_t _dfc:1;
//	uint8_t _acd:1;
//	uint8_t _prm:1;
//	uint8_t _dir:1;
//
//}CTRL_UP_T;
//typedef  struct{
//
//	uint8_t _func:4;			   /* function */
//	uint8_t _fcv:1;
//	uint8_t _fcb:1; 			   /* Frame calculate bit */
//	uint8_t _prm:1; 			   /* 1:from start station, 0:from end station */
//	uint8_t _dir:1;
//
//}CTRL_DOWN_T;


//typedef  union
//{
//	CTRL_UP_T up;
//	CTRL_DOWN_T down;
//	uint8_t val;
//} CTRL_T,*PCTRL_T;

typedef  struct
{
    uint8_t _begin;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _cs;
    uint8_t _end;
}GX101Slaver_10_T,*PGX101Slaver_10_T;

typedef struct	//I104S规约的可变帧长结构  
{
    unsigned char Start; 		//启动字符
    unsigned char Length; 		//长度
    unsigned char Control1;		 //控制域1
    unsigned char Control2; 	//控制域2
    unsigned char Control3; 	//控制域3
    unsigned char Control4; 	//控制域4
    unsigned char Style;   		//类型标识，7
    unsigned char Definitive; 	//结构限定词
    unsigned char Reason_L; 	//传送原因
    unsigned char Reason_H; 	//传送原因
    unsigned char Address_L; 	//公共地址（子站站址）
    unsigned char Address_H; 	//公共地址（子站站址）
    unsigned char AddInfom1;  	//信息体地址，第13字节
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
    
}APDU104_T;


typedef struct
{
	DWORD DZValue;
	BYTE Flag;//1:定值预置     2:定值激活
}S101DZDATA_T;

typedef struct
{
	WORD addr;
	DWORD DZValue;
}S101DZVALUE_T;

int Init_siec101gx(int DevID);

#pragma pack()	//解除按1字节对齐


