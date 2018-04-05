#ifndef MQTTPACKETS_H
#define MQTTPACKETS_H

#include <TelstraM1Interface.h>
#include <packet_utils.h>
#include <str_utils.h>
#include <g55commands.h>


#define INVALIDINPUT -1
#define TIMEOUT -2
#define Error -3

#define dataPin 2
#define chipSelect 3

#define STATUS_READ 1
#define STATUS_TIMEOUT -6

#define MAXTIMEOUT 10000
#define BUFFER_SIZE 1024
#define SIZE_OF_BYTE 1
#define SIZE_OF_INT16 2
#define SIZE_OF_INT32 4
#define SIZE_OF_TOPICS 1024

#define QOS_MSB 1               
#define QOS_LSB 2               
#define CLEAN_SEESION 4        
#define TLS 8                     
#define USER_FLAG 16               
#define PASS_FLAG 32               
#define WILL_FLAG 64              
#define RETAIN 128   

struct mqtt_client_info {
  char *client_id;
  uint16_t keep_alive_sec;
  bool clean_session;     /*this will be stored in a byte[0]*/
  bool tls;               /*this will be stored in a byte[1]*/
  bool username_flag;     /*this will be stored in a byte[2]*/
  bool password_flag;     /*this will be stored in a byte[3]*/
  bool will_flag;         /*this will be stored in a byte[4]*/

  char *username;
  char *password;
};

struct mqtt_message {
  char *topic;
  char *content;

  char qos;               /*this will be stored in a byte[6]&[7]*/
  bool retain;            /*this will be stored in a byte[5]*/
};

struct mqtt_topic {
  char *topic;
  char qos;
};

class MQTT_Packets {
public:

  /**
    * Create MQTT_Packets instance
    */
  MQTT_Packets(TelstraM1Interface *IoTDevice);

  ~MQTT_Packets();

  /**
    *Create connect packet
    *    @param hostname - hostname of the broker in format: domain.com
    *    @param port - port of the broker
    *    @param client_info - pointer to mqtt_client_info struct for authorization
    *    @param last_will_msg - pointer to mqtt_message struct for last will message
    *    @return -1 for invalid input
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Connect(char *hostname, uint16_t port, mqtt_client_info *client_info, mqtt_message *last_will_msg);

  /**
    *Create publish packet
    *    @param publish_msg - pointer to mqtt_message struct for publish message
    *    @return -1 for invalid input
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Publish(mqtt_message *publish_msg);

  /**
    *Create subscribe packet
    *    @param topic_count - number of topics to subscribe
    *    @param subscribe_list - pointer to the first mqtt_topic struct in the list for subscribe
    *    @return -1 for invalid input
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Subscribe(uint8_t topic_count, mqtt_topic *subscribe_list);

  /**
    *Create unsubscribe packet
    *    @param topic_count - number of topics to unsubscribe
    *    @param unsubscribe_list - pointer to the first mqtt_topic struct in the list for unsubscribe
    *    @return -1 for invalid input
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Unsubscribe(uint8_t topic_count, mqtt_topic *unsubscribe_list);

  /**
    *Create ping packet
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Ping();

  /**
    *Create Disconnect packet
    *    @return -2 in case of timeout reading response
    *    @return others in case of getting responses
    */
  int16_t Disconnect();

  /**
    *Get publish packet
    *    @param topicDest - pointer given by users to store the topic
    *    @param contentDest - pointer given by users to store the content
    *    @return 0 in case there is no message available for reading
    *    @return -2 in case of timeout reading response
    *    @return the number of bytes read in case of getting responses
    */
  int16_t GetMessage(char *topicDest, char *contentDest, int timeout);

private:
  char commandBuffer[BUFFER_SIZE];
  uint16_t time_out = 0;

  TelstraM1Interface *IoTDevice;

  char options_byte(mqtt_client_info *client_info, mqtt_message *trans_config);
};

#endif