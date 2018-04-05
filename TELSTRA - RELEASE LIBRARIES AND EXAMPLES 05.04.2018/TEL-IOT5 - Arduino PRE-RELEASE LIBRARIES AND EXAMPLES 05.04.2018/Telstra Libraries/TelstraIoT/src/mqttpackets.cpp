#include "mqttpackets.h"

  MQTT_Packets::MQTT_Packets(TelstraM1Interface *IoTDevice)
  {
  	this->IoTDevice = IoTDevice;
  }

  MQTT_Packets::~MQTT_Packets()
  {

  }


/* construct a byte to store bool variables */
char MQTT_Packets::options_byte(mqtt_client_info *client_info, mqtt_message *message_config) {
  char user_options = 0;
  
  if(client_info != NULL) {
    if(client_info->clean_session) {
      user_options |= CLEAN_SEESION;
    } 
    if(client_info->tls) {
      user_options |= TLS;
  	}
    if(client_info->username_flag) {
      user_options |= USER_FLAG;
    }
    if(client_info->password_flag) {
      user_options |= PASS_FLAG;
    }
    if(client_info->will_flag) {
      user_options |= WILL_FLAG;
    }
  }
  
  if(message_config !=NULL) {
    if(message_config->retain) {
      user_options |= RETAIN;
    }
  }

  user_options |= (message_config->qos);

  return user_options;
}


/*generate and send packet for connect cmd*/
int16_t MQTT_Packets::Connect(char *hostname, uint16_t port, mqtt_client_info *client_info, mqtt_message *last_will_msg) {
  char c_checksum = 0;
  uint16_t c_size = 0;
  char port_char[2];
  char keep_alive_sec_char[2];
  int16_t res = 0;

  /* build a byte with user options */
  char user_options = options_byte(client_info, last_will_msg);

  if((hostname == NULL) || (client_info == NULL)) {
    return INVALIDINPUT;
  }

  /*convert uint16_t to 2 char */
  write_int16_to_char(port_char, port);
  write_int16_to_char(keep_alive_sec_char, (client_info->keep_alive_sec));

  packet_create_cmd(CMD_MQTT_CONNECT, this->commandBuffer, &c_checksum, &c_size);
  
  /* append hostname and port */
  packet_append_param(hostname, this->commandBuffer, &c_checksum, &c_size);
  packet_append_bin_param(port_char, SIZE_OF_INT16, this->commandBuffer, &c_checksum, &c_size);

  /* append client_id, keep_alive_sec, and the options byte */
  packet_append_param((client_info->client_id), this->commandBuffer, &c_checksum, &c_size);
  packet_append_bin_param(keep_alive_sec_char, SIZE_OF_INT16, this->commandBuffer, &c_checksum, &c_size);
  packet_append_bin_param(&user_options, SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);                /*clean_session, tls, qos, retain, etc are included*/

  /* append username and password if set */
  if ((user_options) & (USER_FLAG)) {
    packet_append_param((client_info->username), this->commandBuffer, &c_checksum, &c_size);
  }
  if ((user_options) & (PASS_FLAG)) {
    packet_append_param((client_info->password), this->commandBuffer, &c_checksum, &c_size);
  }

  if(last_will_msg != NULL) {
    /* append will options and payload if set */
    if((user_options) & (WILL_FLAG)) {
      /* append topic and content */
      packet_append_param((last_will_msg->topic), this->commandBuffer, &c_checksum, &c_size);
      packet_append_param((last_will_msg->content), this->commandBuffer, &c_checksum, &c_size);
    }
  }

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);

  /* send the packet */
  IoTDevice->sendCommand(this->commandBuffer, c_size);
  reportln(REPORT_L2, "<MQTT:Connect> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Connect> - Timeout");
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Connect> - Received!");
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*generate and send packet for publish cmd*/
int16_t MQTT_Packets::Publish(mqtt_message *publish_msg) {
  char c_checksum = 0;
  uint16_t c_size = 0;
  int16_t res = 0;

  /* build a byte with user options */
  char user_options = options_byte(NULL, publish_msg);

  if(publish_msg == NULL) {
    return INVALIDINPUT;
  }
  
  packet_create_cmd(CMD_MQTT_PUBLISH, this->commandBuffer, &c_checksum, &c_size);

  /* append the options byte */
  packet_append_bin_param(&user_options, SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);                /*qos, retain are included*/

  /* append topic and content */
  packet_append_param((publish_msg->topic), this->commandBuffer, &c_checksum, &c_size);
  packet_append_param((publish_msg->content), this->commandBuffer, &c_checksum, &c_size);

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);
  
  /* send the packet */
  //IoTDevice->sendCommand("TestingPublish", c_size);
  IoTDevice->sendCommand(this->commandBuffer, c_size);
  
  reportln(REPORT_L2, "<MQTT:Publish> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Publish> - Timeout");
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Publish> - Received!");
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*generate and send packet for subscribe cmd*/
int16_t MQTT_Packets::Subscribe(uint8_t topic_count, mqtt_topic *subscribe_list) {
  char c_checksum = 0;
  uint16_t c_size = 0;
  char topic_count_char;
  int16_t res = 0;

  if((topic_count == 0) || (subscribe_list == NULL)) {
    return Error;
  }

  /*convert int8 topic_count to char */
  topic_count_char = char(topic_count);

  packet_create_cmd(CMD_MQTT_SUBSCRIBE, this->commandBuffer, &c_checksum, &c_size);

  packet_append_bin_param(&topic_count_char, SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);

  for (uint8_t i = 0; i < topic_count; i++) {
    packet_append_param((subscribe_list[i].topic), this->commandBuffer, &c_checksum, &c_size);
    packet_append_bin_param(&(subscribe_list[i].qos), SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);
  }

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);
  
  /* send the packet */
  IoTDevice->sendCommand(this->commandBuffer, c_size);

  reportln(REPORT_L2, "<MQTT:Subscribe> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Subscribe> - Timeout");    
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Subscribe> - Received!");
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*generate and send packet for unsubscribe cmd*/
int16_t MQTT_Packets::Unsubscribe(uint8_t topic_count, mqtt_topic *unsubscribe_list) {
  char c_checksum = 0;
  uint16_t c_size = 0;
  char topic_count_char;
  int16_t res = 0;

  if((topic_count == 0) || (unsubscribe_list == NULL)) {
   return Error;
  }

  /*convert int8 topic_count to char */
  topic_count_char = char(topic_count);
  
  packet_create_cmd(CMD_MQTT_UNSUBSCRIBE, this->commandBuffer, &c_checksum, &c_size);

  packet_append_bin_param(&topic_count_char, SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);

  for (uint8_t i = 0; i < topic_count; i++) {
    packet_append_param((unsubscribe_list[i].topic), this->commandBuffer, &c_checksum, &c_size);
    packet_append_bin_param(&(unsubscribe_list[i].qos), SIZE_OF_BYTE, this->commandBuffer, &c_checksum, &c_size);
  }

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);

  /* send the packet */
  IoTDevice->sendCommand(this->commandBuffer, c_size);

  reportln(REPORT_L2, "<MQTT:Unsubscribe> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Unsubscribe> - Timeout");    
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Unsubscribe> - Done!");    
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*generate and send packet for ping cmd*/
int16_t MQTT_Packets::Ping() {
  char c_checksum = 0;
  uint16_t c_size = 0;
  int16_t res = 0;

  packet_create_cmd(CMD_MQTT_PING, this->commandBuffer, &c_checksum, &c_size);

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);
  
  /* send the packet */
  IoTDevice->sendCommand(this->commandBuffer, c_size);

  reportln(REPORT_L2, "<MQTT:Ping> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Ping> - Timeout");        
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Ping> - Done!");        
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*generate and send packet for disconnect cmd*/
int16_t MQTT_Packets::Disconnect() {
  char c_checksum = 0;
  uint16_t c_size = 0;
  int16_t res = 0;

  packet_create_cmd(CMD_MQTT_DISCONNECT, this->commandBuffer, &c_checksum, &c_size);

  /* end the append */
  packet_append_ending(this->commandBuffer, &c_checksum, &c_size);
  
  /* send the packet */
  IoTDevice->sendCommand(this->commandBuffer, c_size);

  reportln(REPORT_L2, "<MQTT:Disconnect> - Command sent, waiting for response...");

  /* wait for response and get response */
  this->time_out = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, MAXTIMEOUT);
  
  if(this->time_out == 0) {
    reportln(REPORT_L2, "<MQTT:Disconnect> - Timeout");        
  	return TIMEOUT;
  }
  else {
    reportln(REPORT_L2, "<MQTT:Disconnect> - Done!");        
  	return res = (int16_t) packet_get_numeric_param(this->commandBuffer, 0);
  }
}


/*get the packet from the read*/
int16_t MQTT_Packets::GetMessage(char *topicDest, char *contentDest, int timeout) {      //when time out or no message avalable this will return 0
	int16_t bytesRead = 0;

	if(IoTDevice->MessageAvailable()) {
    bytesRead = IoTDevice->getResponse(this->commandBuffer, BUFFER_SIZE, timeout);

    if (packet_get_command(this->commandBuffer) ==  CMD_MQTT_PUBLISH) {
      packet_get_param(topicDest, this->commandBuffer, 1);         //get the topic
      packet_get_param(contentDest, this->commandBuffer, 2);     //get the content
      return bytesRead;
    } else return -1;
  } else return 0;
}




