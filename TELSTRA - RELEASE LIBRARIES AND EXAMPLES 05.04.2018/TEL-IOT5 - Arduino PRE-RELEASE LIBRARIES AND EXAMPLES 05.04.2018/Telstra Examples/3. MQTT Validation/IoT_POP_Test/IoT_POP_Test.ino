/* This script requires the MQTT Compatible IoT Library */

#include <mqttpackets.h>

#define BUTTON 7
#define LED 13

TelstraM1Interface commsif;
MQTT_Packets MQTT_Packets_Object(&commsif);
mqtt_client_info ClientInfo;
mqtt_message LastWillMessage;
mqtt_message PublishMessage;
mqtt_topic SubscribeList[2];

bool connected;
char TOPIC[128];
char CONTENT[255];
char MainBuffer[1024];
#define HOST "[IoT Hub CNAME].azure-devices.net"
//#define HOST "test.mosquitto.org"
#define PORT 8883

void stream_fake_data();

void initialize_my_mqtt() {
//  ClientInfo.client_id = "random-client-cachilupi";
  ClientInfo.client_id = "[deviceID]";
  ClientInfo.keep_alive_sec = 60;
  ClientInfo.clean_session = true;
  ClientInfo.tls = true;
  ClientInfo.username_flag = true;
  ClientInfo.password_flag = true;
  ClientInfo.will_flag = false;
  ClientInfo.username = "[IoT Hub CNAME].azure-devices.net/[deviceID]/api-version=2016-11-14";
  ClientInfo.password = "SharedAccessSignature sr=xxxxxxxxxxxxxxxxxxxxxxxxxx";

  LastWillMessage.topic = "devices/[deviceID]/messages/events/LastWillTopc";
  LastWillMessage.content = "LastWillContent";
  LastWillMessage.qos = 0;
  LastWillMessage.retain = false;

  PublishMessage.topic = "devices/[deviceID]/messages/events/";
  PublishMessage.content = "{ \"hello\": \"world\"}";
  PublishMessage.qos = 0;
  PublishMessage.retain = false;

  SubscribeList[0].topic = "devices/[deviceID]/messages/devicebound/#";
  SubscribeList[0].qos = 1;
}

void setup() {

  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
	Serial.begin(115200);

  Serial.println("Initialising MQTT");	
  initialize_my_mqtt();
  connected = false;
   
}

void loop() {

  if (connected) {
    int msg = MQTT_Packets_Object.GetMessage(TOPIC, CONTENT, MAXTIMEOUT);
    //Serial.println(msg);

    if (msg > 0) {
      Serial.println(msg);
      Serial.println("Packet received");
      Serial.print("Topic: ");
      Serial.println(TOPIC);

      Serial.print("Message: ");
      Serial.println(CONTENT);
    }
  }

  if (Serial.available()) {
    if (connected) {
      switch (Serial.read()) {
        case 'd':
        {
          serialFlush();
          MQTT_Packets_Object.Disconnect();
          connected = false;
          Serial.println("Connected set to FALSE");
          break;
        }

        case 'p':
        {
          serialFlush();
          Serial.println("Sending Publish Command");
          Serial.println(MQTT_Packets_Object.Publish(&PublishMessage));
          Serial.println("Response received");
          break;
        }

        case 's':
        {
          serialFlush();
          Serial.println("Sending Subscribe Command");
          Serial.println(MQTT_Packets_Object.Subscribe(1, SubscribeList));
          Serial.println("Response received");
          break;
        }

        case 'u':
        {
          serialFlush();
          Serial.println("Sending Unsubscribe Command");
          Serial.println(MQTT_Packets_Object.Unsubscribe(1, SubscribeList));
          Serial.println("Response received");
          break;
        }

        case 'g':
        {
          serialFlush();
          Serial.println("Sending Ping Command");
          Serial.println(MQTT_Packets_Object.Ping());
          Serial.println("Response received");
          break;
        }
        case 'k':
        {
          stream_fake_data();
          break;
        }

        default:
          Serial.println("Invalid command");
      }

    } else {

      switch (Serial.read()) {
        case 'c':
        {
          serialFlush();

          Serial.println("Sending Connect Command");
          int res = MQTT_Packets_Object.Connect(HOST, PORT, &ClientInfo, &LastWillMessage);

          if (res < 0) {
            Serial.println("Connection error");
          } else {
            connected = true;
            Serial.println("We are connected!");
          }
          break;
        }

        case 'p':
        {
          char provCommand[2] = {98,98};
          serialFlush();
          Serial.println("Attempting to write to Config Zone");


          commsif.sendCommand(provCommand, 2);


          int bytesread = commsif.getResponse(MainBuffer, 1024, 10000);
          int i = 0;
          for (i=3; i<bytesread; i++){
            Serial.print(convert_nibble_to_ascii(MainBuffer[i]>>4));
            Serial.print(convert_nibble_to_ascii(MainBuffer[i]));
            Serial.print(", ");
          }
          Serial.print("\r\n");
          break;
        }
      
        default:
          Serial.println("No connection active!");
      }

    }
  }
}

void serialFlush(){
  while(Serial.available() > 0) {
    Serial.read();
  }
}

void stream_fake_data(){
  Serial.println("Sending Publish Command");
  String particulate_2_5;
  String publishmessage;
  String particulate_10;
  String temperature;
  String pressure;
  String spl;
  String motion;
  char pubmessage_c[200];
  char resp;
  char cmd='b';
  while(1){
    if(Serial.available()){
    cmd=Serial.read();
    }
  if( cmd == 's' ){
    serialFlush();
    Serial.println("Streaming stopped");
    break;
    }
  else{
    serialFlush();
    publishmessage="{ \"sensor_id\": \"telstra_iot_shield\", \"particulate_2_5\":\"";
    
    particulate_2_5 = random(6000, 10000);
    publishmessage+=particulate_2_5;
    
    publishmessage+= "\", \"particulate_10\":\"";
    particulate_10 = random(4000, 7000);
    publishmessage+=particulate_10;

    publishmessage+= "\", \"temperature\":\"";
    temperature = random(13, 15)+(random(1, 99)*0.01);
    publishmessage+=temperature;

    publishmessage+= "\", \"pressure\":\"";
    pressure = random(100000, 102000);
    publishmessage+=pressure;

    publishmessage+= "\", \"spl\":\"";
    spl = random(0, 90)+(random(1, 999)*0.001);
    publishmessage+=spl;
    
    publishmessage+= "\", \"motion\":\"";
    motion = random(0, 2);
    publishmessage+=motion;
    publishmessage+= "\"}\n";

    Serial.println(publishmessage);
    //Serial.println(publishmessage.length());
    publishmessage.toCharArray(pubmessage_c,publishmessage.length()) ;
    PublishMessage.content = pubmessage_c;
    resp=MQTT_Packets_Object.Publish(&PublishMessage);
    if (resp!=0){
      break;
      }
    delay(1000);
  }

  }
  serialFlush();
}
char convert_nibble_to_ascii(char nibble)
{
    nibble &= 0x0F;
    if (nibble <= 0x09 )
        nibble += '0';
    else
        nibble += ('A' - 10);
    return nibble;
}
