#include "sys.h"

// #define ADDRESS     "tcp://112.93.129.113:1883" //华为云地址
#define ADDRESS     "tcp://1.15.122.102:1883" //腾讯云地址
#define ADDRESS_02     "tcp://192.168.1.135:1883" //本地地址
//#define ADDRESS     "tcp://127.0.0.1:1883" //
//#define ADDRESS     "tcp://mqtt.eclipse.org:1883" //
#define TOPIC       "topic01"  

#define PAYLOAD     "Hello Man, Can you see me ?!" 
#define QOS         1 	//0:至多一次，1：至少一次，2：确保一次
#define TIMEOUT     10000L

char *username= "b784300a8310c7d5"; 
char *password = "9af083b5aee31a37"; 

// 订阅主题
char *topic_sub_command, *topic_sub_updataApp;


static MQTTClient client;
static MQTTClient client_02;
static int MQTTConnectStatus;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

int Mqtt_Client_public(char *topic, char *playloadstring)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

	if(playloadstring == NULL) return RET_ERROR;
	
    pubmsg.payload = playloadstring;
    pubmsg.payloadlen = strlen(playloadstring);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    // printf("Waiting for up to %d seconds for publication of %s\n"
    //         "on topic %s for client with ClientID: %s\n\n",
    //         (int)(TIMEOUT/1000), playloadstring, topic, CLIENTID);
    // rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    // printf("Message with delivery token %d delivered\n\n", token);
    // LogSysLocalTime();
    return RET_SUCESS;

}

volatile MQTTClient_deliveryToken deliveredtoken;

//拼接主题字符串
char *mqtt_topic_joint(char *firstName, char *middleName, char *lastName)
{
    char *topic;
    topic = (char *)malloc(strlen(firstName) + strlen(middleName) + strlen(lastName));
    strcpy(topic, firstName);
    strcat(topic, middleName);
    strcat(topic, lastName);
    return topic;
}

int8_t Check_Deivce_ID(cJSON *json)
{
    cJSON *node = NULL;
    node = cJSON_GetObjectItem(json,"deviceId");
    if(node == NULL){
		log("deviceId node == NULL\n");
        return RET_ERROR;
	}
    char *deviceid;
    deviceid = node->valuestring;
    if(strcmp(deviceid, CLIENTID) != 0){
		perror("deviceid(%s) is not this device \n", deviceid);
		return RET_ERROR;
    } 
    return RET_SUCESS;
}

int8_t Deal_topic_command(char* string)
{
    cJSON *json = cJSON_Parse(string);
    cJSON *node = NULL;
	int size;

	if(json == NULL){
		perror("playload is not json type\n");
		return RET_ERROR;
	}
	
	if(Check_Deivce_ID(json) == RET_ERROR) return RET_ERROR;
	
	node = cJSON_GetObjectItem(json,"cmd");
    if(node != NULL){
        log("cmd is %s\n", node->valuestring);
    }

	node = cJSON_GetObjectItem(json,"paras");
    if(node == NULL){
		log("paras node == NULL\n");
		return RET_ERROR;
    }

    cJSON *Tnode = NULL;
	Tnode = cJSON_GetObjectItem(node,"function");
    if(Tnode != NULL){
        log("function is %s\n", Tnode->valuestring);
        Modbus_Ask_T._function = atoi(Tnode->valuestring);
    }

	Tnode = cJSON_GetObjectItem(node,"modbusAsk");
    if(Tnode != NULL){
        log("modbusAsk is %s\n", Tnode->valuestring);
        if(strcmp(Tnode->valuestring, "true") == 0){
            Modbus_Ask_T._ask = true;
        } 
    }    

	Tnode = cJSON_GetObjectItem(node,"startAddr");
    if(Tnode != NULL){
        log("startAddr is %s\n", Tnode->valuestring);
        Modbus_Ask_T._startAddr = atoi(Tnode->valuestring);
    }

	Tnode = cJSON_GetObjectItem(node,"quantity");
    if(Tnode != NULL){
        log("quantity is %s\n", Tnode->valuestring);
        Modbus_Ask_T._quantity = atoi(Tnode->valuestring);
    }
    if(Modbus_Ask_T._ask){
		Json_DealModbusData(Modbus_Ask_T);
		Modbus_Ask_T._ask = false;
	}

	cJSON_Delete(json);
    return RET_SUCESS;

}

int8_t Deal_topic_updataApp(char* string)
{
    cJSON *json = cJSON_Parse(string);
    cJSON *node = NULL;
    char *url_app, *updata_app, *app_file_name;
	int size;

	if(json == NULL){
		perror("playload is not json type\n");
		return RET_ERROR;
	}
	
	if(Check_Deivce_ID(json) == RET_ERROR) return RET_ERROR;

	node = cJSON_GetObjectItem(json,"updata_app");
    if(node != NULL){
        log("updata_app is %s\n", node->valuestring);
        updata_app = node->valuestring;
    }

	node = cJSON_GetObjectItem(json,"url_app");
    if(node != NULL){
        log("app_file_name is %s\n", node->valuestring);
        url_app = node->valuestring;
    }
	node = cJSON_GetObjectItem(json,"app_file_name");
    if(node != NULL){
        log("app_file_name is %s\n", node->valuestring);
        app_file_name = node->valuestring;
    }
    if(strcmp(updata_app, "yes") == 0){
        log("start updata\n");
        char *updata_App_url;
        char *app_chmod;
        updata_App_url = (char *)malloc(strlen("curl ") + strlen(url_app) + strlen(" -o " ) + strlen(app_file_name));
        strcpy(updata_App_url, "curl ");
        strcat(updata_App_url, url_app);
        strcat(updata_App_url, " -o ");
        strcat(updata_App_url, app_file_name);
        system(updata_App_url);
        
        //升级前要释放文件描述符，否则重启时，不能打开
        int i = 0;
        for(i = 0; i < gVars.dwDevNum; i++){
            close(gpDevice[i].fd);
        }
        
        strcpy(app_chmod, "chmod a+x ");
        strcat(app_chmod, app_file_name);
        system(app_chmod);//设置可执行权限
        system("/mnt/internal_storage/remote_update.sh");

        free(updata_App_url);
        free(app_chmod);
    } 
    cJSON_Delete(json);
    return RET_SUCESS;

}

//消息发布成功确认回调函数
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

    if(strcmp(topicName, topic_sub_command) == 0){
        log("this is topic_sub_command\n");
        Deal_topic_command((char*)message->payload);
    }    

    if(strcmp(topicName, topic_sub_updataApp) == 0){
        log("this is topic_sub_updataApp\n");
        Deal_topic_updataApp((char*)message->payload);
    } 
	
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
    else {
        MQTTConnectStatus = MQTT_DISCONNECT;
    }
    
}

int Mqtt_Reconnect(void)
{
	if(MQTTConnectStatus == MQTT_DISCONNECT){
        log("mqtt reconnet\n");
        Mqtt_Connect();
    }
}

int Mqtt_Client_subscribe()
{
	int rc;
		
    char *firstName = "/v1/devices/";
    char *command = "/command";
    char *updateApp = "/CloudToPoint/updataApp";
    topic_sub_command = mqtt_topic_joint(firstName, CLIENTID, command);
    topic_sub_updataApp = mqtt_topic_joint(firstName, CLIENTID, updateApp);

    char *pTopics_sub[] = {topic_sub_command, topic_sub_updataApp};
    int *pQos;
    int count;
    count = sizeof(pTopics_sub)/sizeof(pTopics_sub[0]);
    pQos = (int *)malloc(count * sizeof(int));
    memset(pQos, 0, count);
	
    // 订阅单个主题
    // if ((rc = MQTTClient_subscribe(client, topic_06, QOS)) != MQTTCLIENT_SUCCESS)
    // {
    // 	printf("Failed to subscribe, return code %d\n", rc);
    // 	rc = EXIT_FAILURE;
	// 	MQTTClient_destroy(&client);
	// 	return rc;
    // }

    // 订阅多个主题
    if ((rc = MQTTClient_subscribeMany(client, count, pTopics_sub, pQos)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
		MQTTClient_destroy(&client);
		return rc;
    }

    int i;
    for(i = 0; i < count; i++){
        log("subscribe topic:%s\n", pTopics_sub[i]);
    }
    free(pQos);
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
    MQTTClient_deliveryToken token;
    int rc;
    //mqtt配置服务器参数
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username; 
    conn_opts.password = password;

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
		MQTTClient_destroy(&client);
		return rc;
    }

	
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect mqtt broker, return code %d\n", rc);
		return RET_ERROR;
    }

    return rc;

}




