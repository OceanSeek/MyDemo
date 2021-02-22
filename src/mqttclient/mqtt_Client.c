#include "sys.h"

#define ADDRESS     "tcp://112.93.129.113:1883" //更改此处地址
#define ADDRESS_02     "tcp://192.168.1.135:1883" //更改此处地址
//#define ADDRESS     "tcp://127.0.0.1:1883" //更改此处地址
//#define ADDRESS     "tcp://mqtt.eclipse.org:1883" //更改此处地址
#define TOPIC       "topic01"  //更改发送的话题

#define PAYLOAD     "Hello Man, Can you see me ?!" //更改信息内容
#define QOS         1 	//0：至多一次；1：至少1次；2：确保只有一次
#define TIMEOUT     10000L

char *username= "E939DB405D8B44248B13C41C4A67AA1E"; //添加的用户名
char *password = "dece03aaaaaaaaaaaaaa"; //添加的密码


static MQTTClient client;
static MQTTClient client_02;
static int MQTTConnectStatus;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

int Mqtt_Client_public(char *topic, char *playloadstring)
{
    //#define MQTTClient_message_initializer { {'M', 'Q', 'T', 'M'}, 0, 0, NULL, 0, 0, 0, 0 }
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //声明消息token
    MQTTClient_deliveryToken token;
    int rc;
//	char *playloadstring;
//	playloadstring = Json_eSDKGetIPInfo();

	if(playloadstring == NULL) return RET_ERROR;
	
    pubmsg.payload = playloadstring;
    pubmsg.payloadlen = strlen(playloadstring);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n\n",
            (int)(TIMEOUT/1000), playloadstring, topic, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n\n", token);
    return rc;

}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
	Json_ParseData((char*)message->payload);
	
    MQTTClient_freeMessage(&message);
//    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
	
	MQTTConnectStatus = MQTT_DISCONNECT;
	MQTTClient_destroy(&client);
	
}

int Mqtt_Connect(void)
{
	if(Init_Mqtt_Client() != RET_ERROR){
		Mqtt_Client_Create_Thread(NULL);
		MQTTConnectStatus = MQTT_CONNECT;
	}
}


int Mqtt_Reconnect(void)
{
	if(MQTTConnectStatus == MQTT_DISCONNECT)Mqtt_Connect();
}

int Mqtt_Client_subscribe()
{
	int rc;
		
	char *topic_01 = "esdk/get/response/app1/ipAddr";
	char *topic_02 = "esdk/get/response/app1/deviceInfo";
	char *topic_03 = "#";
	char *topic_04 = "/v1/devices/device_03/commandResponse";
	char *topic_05 = "uDzAMyyb3491/out/device_03";
	char *topic_06 = "/v1/devices/device_03/command";
	
	
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "\n\n", topic_06, CLIENTID, QOS);
	
    if ((rc = MQTTClient_subscribe(client, topic_06, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
		MQTTClient_destroy(&client);
		return rc;
    }

//    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
//           "\n\n", topic_02, CLIENTID, QOS);
//    if ((rc = MQTTClient_subscribe(client, topic_02, QOS)) != MQTTCLIENT_SUCCESS)
//    {
//    	printf("Failed to subscribe TOPIC_02, return code %d\n", rc);
//    	rc = EXIT_FAILURE;
//		MQTTClient_destroy(&client);
//		return rc;
//    }
	    
    return rc;

}

void *Mqtt_Client_Thread(void *arg)
{
	int rc;
	
	Mqtt_Client_subscribe();

	while(1)
	{
		usleep(50000);
	}
	
	if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to unsubscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
	if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to disconnect, return code %d\n", rc);
		rc = EXIT_FAILURE;
	}
	MQTTClient_destroy(&client);
	
}

void Mqtt_Client_Create_Thread(void *arg)
{
	pthread_t tid1; 
	int err;
	
	err=pthread_create(&tid1, NULL, Mqtt_Client_Thread, arg);
	if(err!=0)	{	
		log("pthread_create error:%s\n",strerror(err)); 
		exit(-1);	
	} 
}

int Init_Mqtt_Client()
{
    //声明消息token
    MQTTClient_deliveryToken token;
    int rc;
    //使用参数创建一个client，并将其赋值给之前声明的client
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username; //将用户名写入连接选项中
    conn_opts.password = password;//将密码写入连接选项中

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
		MQTTClient_destroy(&client);
		return rc;
    }

	
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
		return RET_ERROR;
    }

    return rc;

}




