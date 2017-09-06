
#define DEBUG        1              //Print to Serial


#include <ESP8266WiFi.h>
#include <Servo.h> 


//////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiAPPSK[] = "haveafun";
const int  serverPort  = 12345;

/////////////////////
// Pin Definitions //
/////////////////////
static const uint8_t PIN_CH1 = D1;
static const uint8_t PIN_CH2 = D2;
static const uint8_t PIN_CH3 = D5;
static const uint8_t PIN_CH4 = D6;

static const uint8_t LED_PIN = D7; // Thing's onboard, blue LED

WiFiServer server(serverPort);
WiFiClient client;

Servo servoCh1;
Servo servoCh2;
Servo servoCh3;
Servo servoCh4;

unsigned long lTime;
unsigned long cTime;

int    cutPos, iCh1, iCh2, iCh3, iCh4;

String req, channelValue, ch1Value, ch2Value, ch3Value, ch4Value, chStatus;

boolean alreadyConnected = false; 

char recvData[50];
int  recvDataLength = 0;

void setup() 
{
    initHardware();
    setupWiFi();
  
    server.begin();
    server.setNoDelay(true);

    Serial.setDebugOutput(true);

    servoCh1.attach(PIN_CH1);
    servoCh2.attach(PIN_CH2);
    servoCh3.attach(PIN_CH3);
    servoCh4.attach(PIN_CH4);

    servoCh1.write(90);
    servoCh2.write(90);
    servoCh3.write(90);
    servoCh4.write(90);
}

void loop() 
{
    // Check if a client has connected
    if  (!alreadyConnected)
    {
        client = server.available();
    }

  // when the client sends the first byte, say hello:
    if (client)
    {
        if (!alreadyConnected)
        {
            // clead out the input buffer:
            client.flush();
            Serial.println("We have a new client");
            client.println("Hello, client!");
      
            alreadyConnected = true;
            cTime = millis();
        }
    }
    
    while (client.available())
    {
        recvData[recvDataLength] = client.read();

//        Serial.printf("[%d]", recvDataLength);
//        Serial.printf(" -> [%c]\n", recvData[recvDataLength]);

        if (recvData[recvDataLength] == '\n')
        {
            cTime = millis();

            req = String(recvData).substring(0, recvDataLength);

//            Serial.printf("recvDataLength -> %d\n", recvDataLength);
//            Serial.printf("recvData -> [%s]\n", recvData);
//            Serial.println(req);

            controlServers(req);

            memset(recvData, 0, sizeof(recvData));
            recvDataLength = 0;

            //client.println("OK");
        }
        else
        {
           recvDataLength++;
        }
        
        yield();
    }

//    // Read the first line of the request
//    req = client.readStringUntil('\n');
        
    // NOW
    lTime = millis();

    recvDataLength = 0;
    memset(recvData, 0, sizeof(recvData));
    client.flush();

    // Call Fail Safe
    if ((lTime - cTime > 1000) && (cTime > 0))
    {
        failSafe();
    }

    if ((lTime - cTime > 10000) && (cTime > 0))
    {
        disconnectClient();
    }
}

void disconnectClient()
{
    if (DEBUG) Serial.printf("Client Disconnect\n");

    client.stop();
    cTime = 0;
    alreadyConnected = false;
}

void setupWiFi()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
  
    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);

    macID.toUpperCase();

    String AP_NameString = "WIFI RC RECEIVER " + macID;
  
    char AP_NameChar[AP_NameString.length() + 1];

    memset(AP_NameChar, 0, AP_NameString.length() + 1);
  
    for (unsigned int i = 0; i < AP_NameString.length(); i++)
    {
        AP_NameChar[i] = AP_NameString.charAt(i);
    }
    
    IPAddress ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1); 
    IPAddress subnet(255, 255, 255 ,0); 
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(AP_NameChar, WiFiAPPSK);
  
    delay(1000);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.println(" ");
    Serial.print("AP IP address : ");
    Serial.println(myIP);
}

void initHardware()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void controlServers(String req)
{
    // DISCONNECT
    if (req.startsWith(":EXIT:"))
    {
        disconnectClient();
    }

    if (req.length() > 0)
    {
          digitalWrite(LED_PIN, HIGH);

          if (req.startsWith(":CH:"))
          {
              ch1Value = req.substring(4, 8);
              ch2Value = req.substring(9, 13);
              ch3Value = req.substring(14, 18);
              ch4Value = req.substring(19, 23);
              chStatus = req.substring(24, 25);

              // DEFAULT DATA SETTING
              if (ch1Value.length() == 0) ch1Value = "0";
              if (ch2Value.length() == 0) ch2Value = "0";
              if (ch3Value.length() == 0) ch3Value = "0";
              if (ch4Value.length() == 0) ch4Value = "0";
              if (chStatus.length() == 0) chStatus = "0";
          }
          else if (req.startsWith(":CHK:"))
          {
              // DEFAULT DATA SETTING
              if (ch1Value.length() == 0) ch1Value = "0";
              if (ch2Value.length() == 0) ch2Value = "0";
              if (ch3Value.length() == 0) ch3Value = "0";
              if (ch4Value.length() == 0) ch4Value = "0";
              if (chStatus.length() == 0) chStatus = "0";

              //client.printf("OK:%d \n", lTime);
          }
          else
          {
              // DEFAULT DATA SETTING
              if (ch1Value.length() == 0) ch1Value = "0";
              if (ch2Value.length() == 0) ch2Value = "0";
              if (ch3Value.length() == 0) ch3Value = "0";
              if (ch4Value.length() == 0) ch4Value = "0";
              if (chStatus.length() == 0) chStatus = "0";
          }
    }
    else
    {
        // DEFAULT DATA SETTING
        ch1Value = "0";
        ch2Value = "0";
        ch3Value = "0";
        ch4Value = "0";
        chStatus = "0";
    }

    if (chStatus == "6")
    {
        disconnectClient();
    }

    if (alreadyConnected == false)
    {
        // DEFAULT DATA SETTING
        ch1Value = "0";
        ch2Value = "0";
        ch3Value = "0";
        ch4Value = "0";
        chStatus = "0";
    }
    
    // SERVO OUTPUT
    iCh1 = map(ch1Value.toInt(), -400, 400, 1000, 2000);
    iCh2 = map(ch2Value.toInt(), -400, 400, 1000, 2000);
    iCh3 = map(ch3Value.toInt(), -400, 400, 1000, 2000);
    iCh4 = map(ch4Value.toInt(), -400, 400, 1000, 2000);
          
    // SET SERVO ANGLE
    servoCh1.writeMicroseconds(iCh1);
    servoCh2.writeMicroseconds(iCh2);
    servoCh3.writeMicroseconds(iCh3);
    servoCh4.writeMicroseconds(iCh4);

    if (DEBUG)
    {
        Serial.printf(" Time  %d", lTime);
        Serial.printf(" cTime %d", lTime);
        Serial.printf(" %d", alreadyConnected);
        Serial.printf(" req -> '%s'", req.c_str());
        Serial.printf(" CH1 %s", ch1Value.c_str());
        Serial.printf(" -> %d", iCh1);
        Serial.printf(" CH2 %s", ch2Value.c_str());
        Serial.printf(" -> %d", iCh2);
        Serial.printf(" CH3 %s", ch3Value.c_str());
        Serial.printf(" -> %d", iCh3);
        Serial.printf(" CH4 %s", ch4Value.c_str());
        Serial.printf(" -> %d", iCh4);
        Serial.printf(" STATUS %s\n", chStatus.c_str());
        
    }

    digitalWrite(LED_PIN, LOW);
}

void failSafe()
{
    if (DEBUG)
    {
        //Serial.printf(" FAIL SAFE!! \n");
    }

    // SET SERVO ANGLE
    servoCh1.writeMicroseconds(1500);
    servoCh2.writeMicroseconds(1500);
    servoCh3.writeMicroseconds(1500);
    servoCh4.writeMicroseconds(1500);
}

