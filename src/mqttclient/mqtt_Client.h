#ifndef _mqtt_Client_H
#define _mqtt_Client_H

// #define CLIENTID    "D3491110407lzFpy" //华为云生成的ID
#define CLIENTID    "device_04" //自定义ID
extern char *username; 
extern char *password; 

#define MQTT_DISCONNECT    -1
#define MQTT_CONNECT    	0


int Init_Mqtt_Client();
int Mqtt_Client_public(char *topic, char *playloadstring);
int Mqtt_Client_subscribe();
void Mqtt_Client_Create_Thread(void *arg);
int Mqtt_Connect(void);
int Mqtt_Reconnect(void);
char *mqtt_topic_joint(char *firstName, char *middleName, char *lastName);


#endif

