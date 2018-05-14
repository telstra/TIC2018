# Connecting to Azure IoT Hub

The Telstra Cat-M1 Development Board is capable of using the MQTT protocol to connect directly to an Azure IoT Hub.

* Please refer to this [article](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal)  to learn how to create a unit of Azure IoT Hub
* Please refer to this [article](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support#using-the-mqtt-protocol-directly) to learn about how to connect to connect to the public device endpoints of IoT Hub using the MQTT protocol on port 8883. Understand how to configure values to send in the CONNECT packet.

> Note: I have modified the [IoT_POP_Test.ino](https://github.com/faister/TIC2018/blob/master/TELSTRA%20-%20RELEASE%20LIBRARIES%20AND%20EXAMPLES%2005.04.2018/TEL-IOT5%20-%20Arduino%20PRE-RELEASE%20LIBRARIES%20AND%20EXAMPLES%2005.04.2018/Telstra%20Examples/3.%20MQTT%20Validation/IoT_POP_Test/IoT_POP_Test.ino) to make things easier for you to just enter your values in the respective placeholder.

## Configuring MQTT CONNECT packet values

1. Find out what is the CNAME of your Azure IoT Hub. You can find this out in the Azure Portal under your IoT Hub overview section.
2. [Download and install](https://github.com/Azure/azure-iot-sdk-csharp/releases/download/2018-3-13/SetupDeviceExplorer.msi) the Device Explorer dev tool under the Azure IoT Device Client SDK for .NET. 
3. Using the Device Explorer under the Management tab:

   i. Create a new device.
   
   ii. Click SAS Token. Select your newly created device ID. Specify your TTL (Days). You may specify a long expiry like 365 days just for dev/test. This is obviously not recommended for production. Copy the generated SAS token. Most important is what follows the sr attribute which you need to copy and paste into the IoT_POP_Test.ino configuration for ClientInfo.password.
   
4. Edit IoT_POP_Test.ino and in all placeholders which I have called out in square brackets '[]', replace with the actual values according to your IoT Hub name, device ID, and SAS token signature.

5. In your Arduino IDE, Compile and Upload the sketch to your Telstra Cat-M1 Development Board. The board is labelled as Adafruit Metro M0 Express. 

6. Under Tools, open the Serial Monitor. Enter the following commands:

- c : to connect to IoT Hub
- k : to stream fake telemetry to IoT Hub

7. Go to the Data tab in Device Explorer. Select your deviceID, and click Monitor. You will be able to see the messages sent by the board after ingestion in Azure IoT Hub.
