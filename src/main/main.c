#include "sys.h"
#include "main.h"

extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern MONITORDATA_T monitorData;

 
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
int listenfd,connfd;
unsigned int StaCount = 0;
uint32_t sys_time_cnt = 0;//计时器，500ms一次

 
 

//局部函数
void *Pthread_Task(void *arg);
int ComConfig(int *arg);
int ComConfigTCPClient(int	*arg);
int ComConfigTCPServer(int *arg);
int ComConfigUDP(int *arg);
int ComConfigCK(int	DevNo);
int ReConnectClient(int* OldFd,int* DevNo);
int nonblockingClient(const char* ip, short port, int timeout, int DevNo);
int nonblockingServer(int *fd, const char* ip, short port, int timeout, int DevNo);
int nonblockingUDP(const char* ip, short port, int timeout, int DevNo);
void *AppTask(void *arg);
int TcpServerTask(int DevNo);
int TcpClientTask(int DevNo);
int CKTask(int DevNo);
int UDPTask(int DevNo);
void InitProtocol();
int OverTimeReConnectClient(int DevNo, uint32_t OverTime);

 
void InitDevice(void)
{
	InitQueue(&YxSoeQueue);
	InitQueue(&YxQueue);
	InitQueue(&YcSoeQueue);
	InitDbase();
	InitProtocol();
}


/*******************************************************************  
*名称：      		InitDeviceYKInfo  
*功能：			注册主站协议的遥控函数  
*入口参数：无        
 
*出口参数：无
*******************************************************************/ 
void InitProtocol()
{
	int i = 0;
	for(i = 0 ; i < gVars.dwDevNum ; i++){
		if(strcmp(gpDevice[i].Protocol, "GX104Master") == 0){
			Init_GX104Master(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "GX104Slaver") == 0){
			Init_GX104Slaver(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "GX101Master") == 0){
			Init_GX101Master(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "GX101Slaver") == 0){
			Init_GX101Slaver(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "miec104gx") == 0){
			Init_miec104gx(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "siec101gx") == 0){
//			Init_siec101gx(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "siec104_2002nr") == 0){
			Init_siec104_2002nr(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "XJ103Slaver") == 0){
			Init_XJ103Slaver(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "XJ103Master") == 0){
			Init_XJ103Master(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "Monitor") == 0){
			Init_Monitor(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "siec101_2002") == 0){
			Init_siec101_2002(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "ModbusRtuSlaver") == 0){
			Init_ModbusRtuSlaver(i);
		}
		else if(strcmp(gpDevice[i].Protocol, "ModbusRtuMaster") == 0){
			Init_ModbusRtuMaster(i);
		}
		
		

	}
}

/*******************************************************************  
*名称：      		Empty_Task  
*功能：			占位函数，若规约无循环任务，则占位  
*入口参数：无        
*出口参数：正确返回0，错误返回-1
*******************************************************************/ 
int Empty_Task(int DevNo)
{
	if(gpDevice[DevNo].flag_enpty_task == 0){
		perror("warming:Protocol(%s) Task is null\n", gpDevice[DevNo].Protocol);
		gpDevice[DevNo].flag_enpty_task = 1;
	}
	return 0;
}

/*******************************************************************  
*名称：      		Empty_OnTimeOut  
*功能：			占位函数，若规约无定时任务，则占位  
*入口参数：无        
*出口参数：正确返回0，错误返回-1
*******************************************************************/ 
int Empty_OnTimeOut(int DevNo)
{
	if(gpDevice[DevNo].flag_enpty_timeout == 0){
//		perror("warming:Protocol(%s) OnTimeOut is null\n", gpDevice[DevNo].Protocol);
		gpDevice[DevNo].flag_enpty_timeout = 1;
	}
	return 0;
}

/*******************************************************************  
*名称：      		Empty_Receive  
*功能：			占位函数，若规约无接收任务，则占位  
*入口参数：无        
*出口参数：正确返回0，错误返回-1
*******************************************************************/
int Empty_Receive(int DevNo, uint8_t *buf, uint16_t len)
{
	if(gpDevice[DevNo].flag_enpty_receive == 0){
		perror("warming:Protocol(%s) Receive is null\n", gpDevice[DevNo].Protocol);
		gpDevice[DevNo].flag_enpty_receive = 1;
	}
	return 0;
}

/*******************************************************************  
*名称：      		CreatePthread  
*功能：			创建线程  
*入口参数：无        
 
*出口参数：正确返回0，错误返回-1
*******************************************************************/ 
static int threadID[MAXDEVNUM];
int CreatePthread(void){
	pthread_t tid1; 
	int i;
	int err;
	
	for(i=0; i<gVars.dwDevNum; i++){
		if(gVars.dwDevNum >= MAXDEVNUM){
			perror("gVars.dwDevNum >= MAXDEVNUM");
			write_log("gVars.dwDevNum >= MAXDEVNUM", LOGERROR, __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
		threadID[i] = i;
		err=pthread_create(&tid1, NULL, Pthread_Task, &threadID[i]);
		if(err!=0)	{	
			log("pthread_create error:%s\n",strerror(err));	
			exit(-1);	
		} 
	}

	
	return 0;
}
/*******************************************************************  
*名称：      		ClearDevicParamater  
*功能：			清零设备参数  
*入口参数：         
*	@DevNo 		设备ID
 
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int ClearDevicParamater(int DevNo)
{
	gpDevice[DevNo].ReadYXPtr = 0;
	gpDevice[DevNo].ReadYXSoePtr = 0;
	gpDevice[DevNo].ReadYCPtr = 0;
}

/*******************************************************************  
*名称：			Pthread_Task  
*功能：			创建线程  
*入口参数：		
*	arg:	设备ID
*出口参数：
*	PTableName: 返回表单指针
*修改日期：2020-3-19
*	阻塞模式下，"Pthread_Task_ID++"不能放在后面，否则会出错。创建线程的程序会不管阻塞是否返回都会创建，如果
*	“Pthread_Task_ID++”放在后面则不能执行，Pthread_Task_ID的值就会一直是0。
*修改日期：2020-6-8
*	将Pthread_Task_ID删除，采用传参方式传递DevNo
*修改日期：2020-7-9
*	将fd添加进gpdevice结构体中
*******************************************************************/ 
void *Pthread_Task(void *arg)
{	
	int i;
	int DevNo;
	int fd = -1;
	pthread_t tid1; 
	int err = -1;
	int ret;
    DevNo = *(int *)arg;
	ret = ComConfig((int *)arg);
	ClearDevicParamater(DevNo);
	if(ret == -1){
		perror("error:ComConfig  ");
		log("DevNo(%d)\n",DevNo);
		return NULL;
	}else if(ret >= 0){
		log("DevNo is (%d),fd (%d)\n",DevNo, gpDevice[DevNo].fd);
		err=pthread_create(&tid1,NULL,AppTask,arg);
		if(err!=0)	{	
			log("pthread_create error:\n");	
			return NULL;	
		} 
	}
	
}
/*******************************************************
*
* Function name ComConfig
* Description        : com config
* Parameter         ：
*        @arg            device id 

* Return          ：-1 error  ,  fd
* Others		:
**********************************************************/
int ComConfig(int *arg)
{
	int DevNo;
	int ret;
	DevNo = *arg;
	if(strcmp("Net", gpDevice[DevNo].Com) == 0){
		if(strcmp("Client", gpDevice[DevNo].TcpType) == 0){
			ret = ComConfigTCPClient(arg);
			if(ret == RET_ERROR){
				perror("error:ComConfigTCPClient\n");
				return RET_ERROR;
			}
			gpDevice[DevNo].Flag_Link_Ready = LinkReady;
			return RET_SUCESS;

		}
		else if(strcmp("Server", gpDevice[DevNo].TcpType) == 0){
			ret = ComConfigTCPServer(arg);
			if(ret == RET_ERROR){
				perror("error:ComConfigTCPServer\n");
				return RET_ERROR;
			}
			gpDevice[DevNo].Flag_Link_Ready = LinkReady;
			return RET_SUCESS;
		}
		else if(strcmp("UDP", gpDevice[DevNo].TcpType) == 0){
			ret = ComConfigUDP(arg);
			if(ret == RET_ERROR){
				perror("error:ComConfigUdp\n");
				return RET_ERROR;
			}
			gpDevice[DevNo].Flag_Link_Ready = LinkReady;
			return RET_SUCESS;
		}

		
	}else if(strcmp("Serial", gpDevice[DevNo].Com) == 0){

		ret = ComConfigCK(DevNo);
		if(ret == RET_ERROR){
			perror("error:ComConfigCK\n");
			return RET_ERROR;
		}
		return RET_SUCESS;
	}

}
/*******************************************************************  
*名称：			ComConfigTCPClient  
*功能：			配置TCP客户端  
*入口参数：		
*	arg:	设备ID
*出口参数：
*	
*修改日期：2020-3-19
*	阻塞模式下，"Pthread_Task_ID++"不能放在后面，否则会出错。创建线程的程序会不管阻塞是否返回都会创建，如果
*	“Pthread_Task_ID++”放在后面则不能执行，Pthread_Task_ID的值就会一直是0。
*修改日期：2020-6-8
*	将Pthread_Task_ID删除，采用传参方式传递DevNo
*	logbuf不可使用全局静态变量，多线程同时赋值会有冲突,在这使用全局变量，每个设备分配一个logbuf
*******************************************************************/ 
int ComConfigTCPClient(int *arg)
{
	int OverTime = 1;
	int FlagLog = 1;
	int ret = -1;
	int DevNo = *arg;

	while(nonblockingClient(gpDevice[DevNo].IP, gpDevice[DevNo].PORT, OverTime, DevNo)!=0){
		if(FlagLog){
			FlagLog = 0;
			log("DevNo(%d) can not connect to Server ip(%s) port(%d) \n",DevNo, gpDevice[DevNo].IP, gpDevice[DevNo].PORT);
			sprintf(gpDevice[DevNo].logbuf, "DevNo(%d) can not connect to Server ip(%s) port(%d) \n",DevNo, gpDevice[DevNo].IP, gpDevice[DevNo].PORT);
			write_log(gpDevice[DevNo].logbuf, LOGDEBUG, __FILE__, __FUNCTION__, __LINE__);
			
		}
	}
	return RET_SUCESS; 
}

/*******************************************************
*
* Function name 	：ComConfigTCPServer
* Description    	: tcp secver config. 
* Parameter         ：
* 		@DevNo		:device id 

* Return        	：fd
* Others			:
* 	
**********************************************************/
int ComConfigTCPServer(int *arg)
{
	int fd = -1;
	int OverTime = 1;
	int FlagLog = 1;
	int DevNo = *arg;
	nonblockingServer(&fd, gpDevice[DevNo].IP, gpDevice[DevNo].PORT, OverTime, DevNo);
	return fd;
}
/*******************************************************
*
* Function name 	ComConfigUDP
* Description    	: tcp secver config. 
* Parameter         ：
* 		@DevNo		:device id 

* Return        	：fd
* Others			:
* 	
**********************************************************/
int ComConfigUDP(int *arg)
{
	int OverTime = 1;
	int FlagLog = 1;
	int DevNo = *arg;
	nonblockingUDP(gpDevice[DevNo].IP, gpDevice[DevNo].PORT, OverTime, DevNo);
//	log("________________ desip(%s)udp fd is (%d)  DevNo:%d",gpDevice[DevNo].UDP_Dest_IP, gpDevice[DevNo].fd,DevNo);
	return gpDevice[DevNo].fd;

}


/*******************************************************
*
* Function name 	ComConfigCK
* Description    	: serial port config 
* Parameter         ：
* 		@DevNo		:device id 

* Return        	：fd , -1:error
* Others			:
* 	
**********************************************************/
int ComConfigCK(int DevNo)
{
	int err;			   //返回调用函数的状态	
	int len;							
	int i;
	char rcv_buf[256];			   
	int ttyfd = -1;
	pthread_t tid1,tid2;
	char *ttyPort;
	int ComID;
	uint32_t CKBault;
	ComID = gpDevice[DevNo].SerialID;
	CKBault = gpDevice[DevNo].SerialBault;
	switch(ComID){
		case 0:
			ttyPort = "/dev/ttymxc1";
			break;
		case 1:
			ttyPort = "/dev/ttymxc2";
			break;
		case 2: 
			ttyPort = "/dev/ttymxc3";
			break;
		case 3:
			ttyPort = "/dev/ttymxc4";
			break;
	}
#ifdef HUAWEI
	switch(ComID){
		case 0:
			ttyPort = "/dev/ttyRS1";
			break;
		case 1:
			ttyPort = "/dev/ttyRS2";
			break;
		default:
			perror("no such port");
			return -1;
	}
#endif
	
	ttyfd = UART0_Open(ttyPort); //打开串口，返回文件描述符	
	if(ttyfd < 0)
	{
		perror("Open %s fail!\n",ttyPort);
		return -1;
	}
	else{
		
		err = UART0_Init(ttyfd,CKBault,0,8,1,'N');
		if(err == FALSE)
		{
			perror("set port error!\n");
			
			return -1;
		}
		else 
		{
			log("open %s sucess!fd (%d)\n",ttyPort, ttyfd);
		}
	}

	gpDevice[DevNo].fd = ttyfd;
	log("____________gpDevice[%d].fd is %d \n", DevNo, gpDevice[DevNo].fd);
	return 0;

}

int OnTimeOut(struct timeval *lasttime, uint32_t TimeOut, long long *nowtime_ms, long long *lasttime_ms)
{
	struct timeval nowtime;
	int ProtocolID;
	gettimeofday(&nowtime,NULL);
	*nowtime_ms = (long long)nowtime.tv_sec*1000 + nowtime.tv_usec/1000;
	*lasttime_ms = (long long)(lasttime->tv_sec)*1000 + (lasttime->tv_usec)/1000;
	if((*nowtime_ms) - (*lasttime_ms) >= TimeOut){
		*lasttime = nowtime;
		return RET_SUCESS;
	}
	return RET_ERROR;

}

/*******************************************************
*
* Function name ：nonblockingClient
* Description        : TCPClient nonblock Connect
* Parameter         ：
*        @fd            returen fd 
*        @ip	        destination ip    
*        @port     		ip port
*        @timeout     	connect overtime
*        @DevNo       	device id
* Return          ：0 success  ,  other fail
**********************************************************/
int nonblockingClient(const char* ip, short port, int timeout, int DevNo)
{
	int FlagLog = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		log("_______LINE(%d)_______\n",__LINE__);
  	    log("create socket error: %s(errno: %d)\n", strerror(errno),errno);  
		exit(0);  
	}	
    //设置非阻塞
	int flag = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
	{
		log("fcntl failed\n");
		close(fd);
		return -1;
	}
	struct sockaddr_in sockaddr_in;
	memset(&sockaddr_in, 0, sizeof(struct sockaddr_in));
	sockaddr_in.sin_addr.s_addr = inet_addr(ip);
	sockaddr_in.sin_port = htons(port);
	sockaddr_in.sin_family = AF_INET;
    //为了处理EINTR,将connect放在循环内
	
    while (1)
    {
        int ret = connect(fd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in));
		
        if (ret == 0)
        {
			log("connect successfull\n");
            return 0;
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
            {
				log("signal interrupt \n");
                continue;
            }
            else if (errno != EINPROGRESS)
            {
                close(fd);
                return -1;
            }
            else
            {
                break;
            }
        }
 
    }
	fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
	struct timeval tv = { timeout, 0 };
	int ret = select(fd + 1, NULL, &wfds, NULL, &tv);
	if (ret <= 0)
	{
		close(fd);
		sleep(timeout*3);
		return -1;
	}
 
	if (FD_ISSET(fd, &wfds))
	{
		int error;
		socklen_t error_len = sizeof(int);
		
		ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
		
		if (ret == -1 || error != 0)
		{
			close(fd);
			sleep(timeout*3);
			return -1;
		}
 
        /**
        * 在linux下，select返回fd可写，有两种情况：1.连接成功，2.发生错误
        * getsockopt返回error为0则排除错误情况，连接已建立
        */
 		log("DevNo(%d) connect successfully  fd(%d) ip(%s) port(%d) ",DevNo,fd, ip, port);
		LogSysLocalTime();
		gpDevice[DevNo].fd = fd;
		Monitor_Enable(DevNo);
		
		sprintf(gpDevice[DevNo].logbuf, "DevNo(%d) tcp Client connect successfully  fd(%d)ip(%s)port(%d)\n",DevNo, fd, ip, port);
		write_log(gpDevice[DevNo].logbuf, LOGDEBUG, __FILE__, __FUNCTION__, __LINE__);
		
        return 0;
    }
	return -1;
}


int nonblockingServer(int *fd, const char* ip, short port, int timeout, int DevNo)
{
	int sfp, nfp, on = 1, num = 0;
	struct sockaddr_in s_add,c_add;
	int sin_size;
	int ret;  

	log("Server ip is (%s) port(%d)\n",ip,port);

	sfp = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sfp)
	{
			perror("socket fail ! \r\n");
			return -1;
	}

	log("socket ok fd(%d)!\r\n",sfp);
	gpDevice[DevNo].sfd = sfp;
	setsockopt(sfp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr=inet_addr(ip);
	s_add.sin_port=htons(port);

	if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
			log("bind fail !\r\n");
			return -1;
	}
	

	log("bind ok !\r\n");

	if(-1 == listen(sfp,5))
	{
			log("listen fail !\r\n");
			return -1;
	}

	log("listen ok\r\n");

	sin_size = sizeof(struct sockaddr_in);
	
	*fd = sfp;
	return 0;

}

int nonblockingUDP(const char* ip, short port, int timeout, int DevNo)
{
	int fd = -1;
	fd = socket(AF_INET,SOCK_DGRAM,0);
	if (0 > fd)
	{
		perror("sockfd");
		return -1;
	}

	//准备地址
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;//ipv4
	addr.sin_port = htons(port);//端口号
	addr.sin_addr.s_addr = inet_addr(ip);//我的ip地址
	//绑定
	int ret = bind(fd,(struct sockaddr *)&addr,sizeof(addr));
	if (0 > ret)
	{
		 perror("bind udp ip:%s\n",ip);
		 return -1;
	}

	gpDevice[DevNo].fd = fd;
	return 0;

}


void *AppTask(void *arg)
{
	int DevNo = *(int *)arg;
	if(strcmp("Net", gpDevice[DevNo].Com) == 0){
		if(strcmp("Client", gpDevice[DevNo].TcpType) == 0){
			TcpClientTask(DevNo);
			PRINT_FUNLINE;
			return NULL;
		}
		else if(strcmp("Server", gpDevice[DevNo].TcpType) == 0){
			TcpServerTask(DevNo);
			return NULL;
		}
		else if(strcmp("UDP", gpDevice[DevNo].TcpType) == 0){
			UDPTask(DevNo);
			return NULL;
		}
		
	}else if(strcmp("Serial", gpDevice[DevNo].Com) == 0){
		CKTask(DevNo);
		return NULL;
	}
}

/*******************************************************************  
*名称：      		ReAcceptServer  
*功能：			读取字符长度异常时，远方客户端断开链接，重新侦听  
*入口参数：         
*	@DevNo		设备ID号
*	@len		读取到的字符长度
                    port    串口号(ttyS0,ttyS1,ttyS2)  
*出口参数：正确返回为0，错误返回为-1 
*修改日期：2020-7-9
*	tcp服务器端口文件描述符存放为sfd，存放在gpdevice中
*	tcp服务器侦听端口文件描述符存放为fd，存放在gpdevice中
*******************************************************************/ 
int ReAcceptServer(int DevNo, int len)
{
	struct sockaddr_in c_add;
	int sin_size;
	int nfd;
	
	sin_size = sizeof(struct sockaddr_in);
	
	if(len	<= 0 || len > 1500){//远方关闭链接
		perror("tcpserver: the other side has been closed (%d)\n",len);
		sprintf(gpDevice[DevNo].logbuf, "tcpserver: the other side has been closed (%d)\n",len);
		write_log(gpDevice[DevNo].logbuf, LOGMARNING, __FILE__, __FUNCTION__, __LINE__);
		gpDevice[DevNo].Flag_Link_Ready = LinkDisconect;
		gpDevice[DevNo].Flag_Brust_Send_Enable = DISABLE;
		Monitor_Disable(DevNo);
		close(gpDevice[DevNo].fd);//释放socket
		
		while(1){
			nfd = accept(gpDevice[DevNo].sfd, (struct sockaddr *)(&c_add), &sin_size);
			gpDevice[DevNo].fd = nfd;
			if(RET_ERROR == nfd){
				perror("accept fail fd(%d)!\r\n",gpDevice[DevNo].fd);
				return RET_ERROR;
			}
			else{
				gpDevice[DevNo].Flag_Link_Ready = LinkReady;
				
				Monitor_Enable(DevNo);
			
				LOG("tcpserver:socket aceept ok ,fd(%d)\n",gpDevice[DevNo].fd);
				LogSysLocalTime();
				sprintf(gpDevice[DevNo].logbuf, "tcpserver:socket aceept ok\n");
				write_log(gpDevice[DevNo].logbuf, LOGMARNING, __FILE__, __FUNCTION__, __LINE__);
				return RET_SUCESS;
			}	
		}
	}


}


int OverTimeReConnectServer(int DevNo, uint32_t OverTime)
{
	int fd;
	int sin_size;
	struct sockaddr_in c_add;
	sin_size = sizeof(struct sockaddr_in);
	
	if(strcmp("Monitor", gpDevice[DevNo].Protocol) == 0)
		return RET_SUCESS;
	
	if((sys_time_cnt - gpDevice[DevNo].ReConnectOldTime) > OverTime){
		gpDevice[DevNo].ReConnectOldTime = sys_time_cnt;
		close(gpDevice[DevNo].fd);
		fd = accept(gpDevice[DevNo].sfd, (struct sockaddr *)(&c_add), &sin_size);
		if(RET_ERROR == fd)
		{
				perror("accept fail !\r\n");
				return RET_ERROR;
		}
		gpDevice[DevNo].fd = fd;
		log("Server reaccept succes ");
		LogSysLocalTime();
	}
	return RET_SUCESS;

}
int OverTimeReConnectClient(int DevNo, uint32_t OverTime)
{
	int fd;
	int sin_size;
	struct sockaddr_in c_add;
	
	if(strcmp("Monitor", gpDevice[DevNo].Protocol) == 0)
		return RET_SUCESS;
	
	sin_size = sizeof(struct sockaddr_in);
	if((sys_time_cnt - gpDevice[DevNo].ReConnectOldTime) > OverTime){
		gpDevice[DevNo].ReConnectOldTime = sys_time_cnt;
		close(gpDevice[DevNo].fd);
		log("close DevNo(%d) fd(%d) ", DevNo, gpDevice[DevNo].fd);
		LogSysLocalTime();
		while(nonblockingClient(gpDevice[DevNo].IP, gpDevice[DevNo].PORT, 1, DevNo) != 0){
			gpDevice[DevNo].Flag_Link_Ready = LinkReady;
			GX104Master_Start_Link(DevNo);
		}
		
	}
	return RET_SUCESS;

}

int TcpServerTask(int DevNo)
{

	char buff[1499];
	struct timeval tv;  
	struct sockaddr_in c_add;
	int sin_size;
	int ret;
	int nfp;
	int i = 0;
	sin_size = sizeof(struct sockaddr_in);
	nfp = accept(gpDevice[DevNo].sfd, (struct sockaddr *)(&c_add), &sin_size);
	if(-1 == nfp)
	{
			perror("accept fail !\r\n");
			return -1;
	}
	gpDevice[DevNo].fd = nfp;
	log("accept ok sfd(%d)fd(%d) !\r\nServer start get connect from %#x : %#x ", gpDevice[DevNo].sfd,gpDevice[DevNo].fd,
									ntohl(c_add.sin_addr.s_addr), ntohs(c_add.sin_port));
	LogSysLocalTime();
	struct timeval lasttime;
	long long nowtime_ms,lasttime_ms;
	gettimeofday(&lasttime,NULL);
	
	while(1)
	{
		if(OnTimeOut(&lasttime, Timer_1000ms, &nowtime_ms, &lasttime_ms) == RET_SUCESS){
			if(gpDevice[DevNo].OnTimeOut == NULL){
				gpDevice[DevNo].OnTimeOut = Empty_OnTimeOut;
			}
			gpDevice[DevNo].OnTimeOut(DevNo);
		}
		//超时无应答则关闭fd，重新侦听
		OverTimeReConnectServer(DevNo, Timer_60s);

		fd_set rfds,wfds;	
		FD_ZERO(&rfds); 			
		FD_SET(gpDevice[DevNo].fd, &rfds);  
		FD_ZERO(&wfds); 		   
		FD_SET(gpDevice[DevNo].fd, &wfds); 
		
		
		tv.tv_sec = 1;	
		tv.tv_usec = 0; 
		
		ret = select(gpDevice[DevNo].fd + 1, &rfds, &wfds, NULL, &tv);

		if(ret <= 0){
			
		}else{
			if(FD_ISSET(gpDevice[DevNo].fd, &rfds)){
				gpDevice[DevNo].ReConnectOldTime = sys_time_cnt;
				int len = read(gpDevice[DevNo].fd, buff, MAXLINE);
				MonitorRx(monitorData._RX_ID, DevNo, monitorData._fd, buff, len);
				if(gpDevice[DevNo].Receive == NULL){
					gpDevice[DevNo].Receive = Empty_Receive;
				}
				gpDevice[DevNo].Receive(DevNo, buff, len);
				ReAcceptServer(DevNo, len);
			}
		}
		if(gpDevice[DevNo].Task == NULL){
			gpDevice[DevNo].Task = Empty_Task;
		}
		gpDevice[DevNo].Task(DevNo);
		usleep(50000);
	}
	close(gpDevice[DevNo].fd);
	close(gpDevice[DevNo].sfd);
	pthread_exit(NULL);
	exit(0); 
}
int TcpClientTask(int DevNo)
{
	char buff[1499];
	struct timeval lasttime;
	long long nowtime_ms,lasttime_ms;
	gettimeofday(&lasttime,NULL);

	while(1){
		pthread_mutex_lock(&mutex);
		if(OnTimeOut(&lasttime, Timer_1000ms, &nowtime_ms, &lasttime_ms) == RET_SUCESS){
			if(gpDevice[DevNo].OnTimeOut == NULL){
				gpDevice[DevNo].OnTimeOut = Empty_OnTimeOut;
			}
			gpDevice[DevNo].OnTimeOut(DevNo);
		}
		pthread_mutex_unlock(&mutex);
		
		OverTimeReConnectClient(DevNo, Timer_60s);
		
		int len = recv(gpDevice[DevNo].fd, buff, MAXLINE, 0);	
		if(len == 0){//Client disconnect
			log("DevNo(%d) tcp-Client disconnect ",DevNo);
			LogSysLocalTime();
			Monitor_Disable(DevNo);
		
			sprintf(gpDevice[DevNo].logbuf, "DevNo(%d) Client disconnect \n",DevNo);
			write_log(gpDevice[DevNo].logbuf, LOGMARNING, __FILE__, __FUNCTION__, __LINE__);
			gpDevice[DevNo].Flag_Link_Ready = LinkDisconect;
			gpDevice[DevNo].Flag_start_link = LINK_COLSE;
			GX104Master_Start_Link(DevNo);
			while(nonblockingClient(gpDevice[DevNo].IP, gpDevice[DevNo].PORT, 1, DevNo) != 0){
				gpDevice[DevNo].Flag_Link_Ready = LinkReady;
				GX104Master_Start_Link(DevNo);
			}
			
		}
		pthread_mutex_lock(&mutex);
		if(len > 0){
			gpDevice[DevNo].ReConnectOldTime = sys_time_cnt;
			MonitorRx(monitorData._RX_ID, DevNo, monitorData._fd, buff, len);
			if(gpDevice[DevNo].Receive == NULL){
				gpDevice[DevNo].Receive = Empty_Receive;
			}
			gpDevice[DevNo].Receive(DevNo, buff, len);
		}
		if(gpDevice[DevNo].Task == NULL){
			gpDevice[DevNo].Task = Empty_Task;
		}
		gpDevice[DevNo].Task(DevNo);
		pthread_mutex_unlock(&mutex);
		usleep(50000);
	}
	close(gpDevice[DevNo].fd);
	pthread_exit(NULL);
	exit(0); 

}
int UDPTask(int DevNo)
{

	char buff[1499];
	struct timeval lasttime;
	long long nowtime_ms,lasttime_ms;
	gettimeofday(&lasttime,NULL);

	struct sockaddr_in src_addr ={};
    socklen_t addr_len = sizeof(struct sockaddr_in);

	while(1){
		if(OnTimeOut(&lasttime, Timer_1000ms, &nowtime_ms, &lasttime_ms) == RET_SUCESS){
			if(gpDevice[DevNo].OnTimeOut == NULL){
				gpDevice[DevNo].OnTimeOut = Empty_OnTimeOut;
			}
			gpDevice[DevNo].OnTimeOut(DevNo);
		}
	
		int len = recvfrom(gpDevice[DevNo].fd, buff, MAXLINE, MSG_DONTWAIT,(struct sockaddr *)&src_addr,&addr_len);
		
		pthread_mutex_lock(&mutex);
		if(len > 0){
			gpDevice[DevNo].ReConnectOldTime = sys_time_cnt;
			MonitorRx(monitorData._RX_ID, DevNo, monitorData._fd, buff, len);
//			DumpHEX(buff, 5);
//			log("ip:%s  port:%d\n",inet_ntoa(src_addr.sin_addr),ntohs(src_addr.sin_port));
			if(gpDevice[DevNo].Receive == NULL){
				gpDevice[DevNo].Receive = Empty_Receive;
			}
			gpDevice[DevNo].Receive(DevNo, buff, len);
		}
		if(gpDevice[DevNo].Task == NULL){
			gpDevice[DevNo].Task = Empty_Task;
		}
		gpDevice[DevNo].Task(DevNo);
		pthread_mutex_unlock(&mutex);
		usleep(50000);
	}
	close(gpDevice[DevNo].fd);
	pthread_exit(NULL);
	exit(0); 

}


int CKTask(int DevNo)
{
	char buff[255];
	char test_buf[16] = {0x10,0x0b,1,0,0x0c,0x16};
	int len;
	
	struct timeval lasttime;
	long long nowtime_ms,lasttime_ms;
	gettimeofday(&lasttime,NULL);
	
	
	while(1){
		if(OnTimeOut(&lasttime, Timer_1000ms, &nowtime_ms, &lasttime_ms) == RET_SUCESS){
			if(gpDevice[DevNo].OnTimeOut == NULL){
				gpDevice[DevNo].OnTimeOut = Empty_OnTimeOut;
			}
			gpDevice[DevNo].OnTimeOut(DevNo);
		}
		len = UART0_Recv(gpDevice[DevNo].fd, buff,sizeof(buff));
		if(len > 0)    
		{	 
			MonitorRx(monitorData._RX_ID, DevNo, monitorData._fd, buff, len);
			log("DevNo %d Serial receive:", DevNo);
			DumpHEX(buff, len);
			if(gpDevice[DevNo].Receive == NULL){
				gpDevice[DevNo].Receive = Empty_Receive;
			}
			gpDevice[DevNo].Receive(DevNo, buff, len);
		}	 
		else	
		{	 
		}
		
		pthread_mutex_lock(&mutex);
		if(gpDevice[DevNo].Task == NULL){
			gpDevice[DevNo].Task = Empty_Task;
		}
		gpDevice[DevNo].Task(DevNo);
		pthread_mutex_unlock(&mutex);
		usleep(50000);
	}
	close(gpDevice[DevNo].fd);
	pthread_exit(NULL);
	exit(0); 

}


//char* sysLocalTime()  
//{  
//    time_t             timesec;  
//    struct tm         *p;  
//	char *timestamp;
//
//    time(&timesec);
//    p = localtime(&timesec);  
//
//	timestamp = malloc(21);
//	sprintf(timestamp, "%d:%02d:%02dT%02d:%02d:%02dZ\n", 1900+p->tm_year, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
////	log(timestamp);
//	return timestamp;
//
//}  
void sysUsecTime()  
{  
    struct timeval    tv;  
    struct timezone tz;  

    struct tm         *p;  

    gettimeofday(&tv, &tz);  
    log("tv_sec:%ld\n",tv.tv_sec);  
    log("tv_usec:%ld\n",tv.tv_usec);  
    log("tz_minuteswest:%d\n",tz.tz_minuteswest);  
    log("tz_dsttime:%d\n",tz.tz_dsttime);  

    p = localtime(&tv.tv_sec);  
    log("time_now:%d /%d /%d %d :%d :%d.%3ld\n", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);  
} 
/*******************************************************************  
*名称：      		led_run_switch  
*功能：			led点灯  
*入口参数：         
*	@gpioID 	LED的编号
*	@status 	状态：0亮灯，1灭灯
*******************************************************************/ 
void led_run_switch(int gpioID, int status)
{
	FILE *p=NULL;
	char S_direction[50];
	char S_value[50];
	sprintf(S_direction,"/sys/class/gpio/gpio%d/direction",gpioID);
	sprintf(S_value,"/sys/class/gpio/gpio%d/value",gpioID);
	
	p = fopen("/sys/class/gpio/export","w");
	fprintf(p,"%d",gpioID);
	fclose(p);
	p = fopen(S_direction,"w");
	fprintf(p,"out");
	fclose(p);
	p = fopen(S_value,"w");
	fprintf(p,"%d",status);
	fclose(p);
}
/*******************************************************************  
*名称：      		IntWatchDog  
*功能：			初始化开门狗  
*入口参数：         
*返回值：
*	返回看门狗文件描述符
*******************************************************************/ 
int IntWatchDog()
{
	int fd_watchdog = open("/dev/watchdog", O_WRONLY);  
    if(fd_watchdog == -1)  
    {  
        int err = errno;  
        printf("failed to open /dev/watchdog, errno: %d, %s\n", err, strerror(err));  
        return RET_ERROR;  
    }  
    else  
    {  
        printf("open watchdog success!\n");  
    }  

	return fd_watchdog;

}
/*******************************************************************  
*名称：      		FeedWatchDog  
*功能：			喂狗  
*入口参数：
*	wd_fd 看门狗文件描述符
*******************************************************************/ 
void FeedWatchDog(int wd_fd){
	static unsigned char food = 0;	
	// 每个一段时间向/dev/watchdog 设备写入数据（“定期喂狗”）  
	if(wd_fd >= 0)  
	{  
		ssize_t eaten = write(wd_fd, &food, 1);  
		if(eaten != 1)	
		{  
			log("failed feeding watchdog\n");  
		}  
		else  
		{  
//			printf("success feeded watchdog\n");  
		}  
	}  
}


int main()
{
	struct timeval nowtime;
	struct timeval lasttime;
	struct timeval testtime;
	int wd_fd = -1;
	char ch;
	long long nowtime_ms,lasttime_ms,tets_ms;
	char *buf;
	char *timestamp;
	buf = malloc(200);
	sprintf(buf, "______________SWQ-Protocol-Conver HardVersion(V1.03) SofeVersion(1.0) EditTime:2021.01.05______________ \n");
	log(buf);
	write_log(buf, STARTINFO, __FILE__, __FUNCTION__, __LINE__);
	free(buf);
	gettimeofday(&lasttime,NULL);
	Stm32f103RegisterIec10x();
	InitDevice();
//	wd_fd = IntWatchDog();
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&mutex2,NULL);
	CreatePthread();

//	Mqtt_Connect();
	Init_Json_Test();
	char *topic_01 = "app1/get/request/esdk/ipAddr";
	char *topic_02 = "app1/get/request/esdk/deviceInfo";
	char *topic_03 = "app1/get/request/esdk/resourceInfo";
	char *topic_04 = "app1/get/request/esdk/apn";
	char *topic_05 = "app1/get/request/esdk/ServiceAgentTempletList";
	char *topic_06 = "app1/get/request/esdk/ipRoute";
	char *topic_07 = "/v1/devices/device_03/commandResponse";
	char *topic[] = {topic_01,topic_02,topic_03,topic_04,topic_05,topic_06,topic_07};
	
	while(1){
		gettimeofday(&nowtime,NULL);
		nowtime_ms = (long long)nowtime.tv_sec*1000 + nowtime.tv_usec/1000;
		lasttime_ms = (long long)lasttime.tv_sec*1000 + lasttime.tv_usec/1000;
		if(nowtime_ms-lasttime_ms >=Timer_1000ms){
			sys_time_cnt++;
//			if(sys_time_cnt %5 == 0)Mqtt_Client_public(topic[(cnt++)%6]);
//			if(sys_time_cnt %15 == 0)Mqtt_Client_public(topic_07);
//			log("time cnt %d\n",sys_time_cnt);

//			if(sys_time_cnt %5 == 0)Mqtt_Reconnect();
			
			lasttime = nowtime;
//			FeedWatchDog(wd_fd);
		}
		CheckBrustYx();
		CheckBrustYxSoe();
		CheckBrustDZ();
		usleep(50000);//usleep线程休眠，sleep进程休眠
		
	}
		
 	close(wd_fd);
	return 0;
}


 
 
