#include "azureIoT.h"

AzureIoT::AzureIoT(const char* azureHost_, const char* deviceId_, const char* key_, int ttl_)
{
  azureHost = azureHost_;
  deviceId = deviceId_;
  key = key_;
  endPoint = azureHost + (String) "/devices/" + deviceId;

  if (ttl_ < 0)
  {
    ttl = 0;
  }
  else
  {
    ttl = ttl_;
  }
}

void AzureIoT::init(TelstraM1Device* IoTDevice)
{
  IoTDevice->waitUntilCellularSystemIsReady();
  char tmBuf[100];
  IoTDevice->getTime(tmBuf);
  currentTime(tmBuf);

  sasToken = (char*)getStringValue(generateSas((char*)key, endPoint));
}


const char* AzureIoT::getStringValue(String value)
{
  int len = value.length() + 1;
  char *temp = new char[len];
  value.toCharArray(temp, len);
  return temp;
}

String AzureIoT::urlEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg!='\0')
  {
    if( ('a' <= *msg && *msg <= 'z') \
      || ('A' <= *msg && *msg <= 'Z') \
      || ('0' <= *msg && *msg <= '9') )
        {
          encodedMsg += *msg;
        }
        else
        {
          encodedMsg += '%';
          encodedMsg += hex[*msg >> 4];
          encodedMsg += hex[*msg & 15];
        }
      msg++;
    }
  return encodedMsg;
}


String AzureIoT::generateSas(char *key, String endPointUri)
{
  if (ttl == 0)
  {
    sasExpiryTime = now();
  }
  else if (ttl > 31536000)                     // ttl > 365 days
  {
    sasExpiryTime = now() + 31536000;
  }
  else
  {
    sasExpiryTime = now() + ttl;
  }

 
  String stringToSign = endPointUri + "\n" + sasExpiryTime;

  // START: Create signature
  int keyLength = strlen(key);

  int decodedKeyLength = base64_dec_len(key, keyLength);
  char decodedKey[decodedKeyLength]; //allocate char array big enough for the base64 decoded key

  base64_decode(decodedKey, key, keyLength); //decode key

  Sha256.initHmac((const uint8_t *)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);
  char *sign = (char *)Sha256.resultHmac();
  // END: Create signature
  
  // START: Get base64 of signature
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH);

  // SharedAccessSignature
  return "Authorization: SharedAccessSignature sr=" + endPointUri + "&sig=" \
         + urlEncode(encodedSign) + "&se=" + sasExpiryTime;
  // END: create SAS
}

char* AzureIoT::printSas()
{
  return sasToken;
}

void AzureIoT::setPostContent(String content) //it does not check if JSON format is valid
{
  contentInfo = (String) "\r\nContent-Type: application/json\r\n" \
  + (String) "Content-Length: " + (String) content.length() \
  + (String) "\r\n\r\n" + content;
}

char* AzureIoT::getPostPacket()
{
  delete[] post;
  post = new char [0];
  post = (char*)getStringValue((String) sasToken + contentInfo);
  
  return post;
}

void AzureIoT::currentTime(char* timeBuffer)
{
  short int years = getNumber(timeBuffer, 4);
  uint8_t months = getNumber(timeBuffer + 5, 2);
  uint8_t days = getNumber(timeBuffer + 8, 2);
  uint8_t hours = getNumber(timeBuffer + 11, 2);
  uint8_t minutes = getNumber(timeBuffer + 14, 2);
  uint8_t seconds = getNumber(timeBuffer + 17, 2);

  setTime(hours, minutes, seconds, days, months, years);
}

int AzureIoT::getNumber(char *input, int len)
{
  char number[20];

  if (len > 20)
  {
    return 0;
  }

  strncpy(number, input, len);
  number[len] = '\0';
  int value = atoi(number);
  return value;
}

void AzureIoT::newSas(int newTime)
{
  delete[] sasToken;
  sasToken = new char [0];

  if (newTime < 0)
  {
    ttl = 0;
  }
  else
  {
    ttl = newTime;
  }
  sasToken = (char*)getStringValue(generateSas((char*)key, endPoint));
}
