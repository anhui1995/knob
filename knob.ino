#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <AddrList.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>


#include <Wire.h>
#include "ClosedCube_HDC1080.h"
const char *hostdd = "http://ip/";
#include "M5_BM8563.h"

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;


BM8563 RTC;

rtc_time_type rtc_time_unit;
rtc_date_type rtc_date_unit;


ClosedCube_HDC1080 hdc1080;

#ifndef STASSID
#define STASSID "ssid"
#define STAPSK  "psk"
#endif


int batValue = 0;
int batNum = 0;
int sensorPin = A0;

int ledPin = 15;
int ledValue = 0;
int ledValueOld = 0;


int wrPin = 3;
int csPin = 0;
int dataPin = 1;



int keyPin = 14;
int ctrlRPin = 12;
int ctrlLPin = 13;

int ctrlAvail = 0;

#define uchar unsigned char
#define uint unsigned int

const char* host = "esp8266-webupdate";
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

uchar dispBuffer[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int isSocConnected = 0;
int isSocConnectedOld = 0;

uint16_t getHex(int n) {
  uint16_t tmpBuffer[2] = {0, 0};
  switch (n) {
    case 0: {
        tmpBuffer[0] = 15;
        tmpBuffer[1] = 10;
      } break;
    case 1: {
        tmpBuffer[0] = 0;
        tmpBuffer[1] = 10;
      } break;
    case 2: {
        tmpBuffer[0] = 13;
        tmpBuffer[1] = 6;
      } break;
    case 3: {
        tmpBuffer[0] = 9;
        tmpBuffer[1] = 14;
      } break;
    case 4: {
        tmpBuffer[0] = 2;
        tmpBuffer[1] = 14;
      } break;
    case 5: {
        tmpBuffer[0] = 11;
        tmpBuffer[1] = 12;
      } break;
    case 6: {
        tmpBuffer[0] = 15;
        tmpBuffer[1] = 12;
      } break;
    case 7: {
        tmpBuffer[0] = 1;
        tmpBuffer[1] = 10;
      } break;
    case 8: {
        tmpBuffer[0] = 15;
        tmpBuffer[1] = 14;
      } break;
    case 9: {
        tmpBuffer[0] = 11;
        tmpBuffer[1] = 14;
      } break;

  }
  return tmpBuffer[0] + tmpBuffer[1] * 100;
}


/*
  时间
  大数字
  小温度
  小湿度
*/

void setTimeNum(int h, int m, int s) {
  uint8_t h1 = h / 10;
  uint8_t h2 = h % 10;

  int flog = s % 2;

  uint16_t tmpBuffer = getHex(h1);

  dispBuffer[3] = tmpBuffer % 100;
  dispBuffer[4] = tmpBuffer / 100;
  WriteOneData(3 , dispBuffer[3]);
  WriteOneData(4 , dispBuffer[4]);

  tmpBuffer = getHex(h2);

  dispBuffer[5] = tmpBuffer % 100;
  dispBuffer[6] = tmpBuffer / 100;

  dispBuffer[6] = dispBuffer[6] + flog;
  WriteOneData(5 , dispBuffer[5]);
  WriteOneData(6 , dispBuffer[6]);


  uint8_t m1 = m / 10;
  uint8_t m2 = m % 10;

  tmpBuffer = getHex(m1);

  dispBuffer[7] = tmpBuffer % 100;
  dispBuffer[8] = tmpBuffer / 100;
  WriteOneData(7 , dispBuffer[7]);
  WriteOneData(8 , dispBuffer[8]);


  tmpBuffer = getHex(m2);

  dispBuffer[9] = tmpBuffer % 100;
  dispBuffer[10] = tmpBuffer / 100 + dispBuffer[10] % 2;
  WriteOneData(9 , dispBuffer[9]);
  WriteOneData(10 , dispBuffer[10]);
}


void setBigNum(int v) {
  uint8_t v1 = v / 10;
  uint8_t v2 = v % 10;
  uint16_t tmpBuffer;

  if (v1 == 0) {
    dispBuffer[11] = 0;
    dispBuffer[12] = dispBuffer[12] % 2;
  }
  else {
    tmpBuffer = getHex(v1);
    dispBuffer[11] = tmpBuffer % 100;
    dispBuffer[12] = tmpBuffer / 100 + dispBuffer[12] % 2;
  }


//   dispBuffer[14] = tmpBuffer + dispBuffer[14] % 2;
  WriteOneData(11 , dispBuffer[11]);
  WriteOneData(12 , dispBuffer[12]);


  tmpBuffer = getHex(v2);
  dispBuffer[13] = tmpBuffer % 100;
  dispBuffer[14] = tmpBuffer / 100 + 1;
  WriteOneData(13 , dispBuffer[13]);
  WriteOneData(14 , dispBuffer[14]);

}


void setLittleTemp(uint16_t v) {
  uint16_t v1 = v / 100;
  uint16_t v2 = (v % 100) / 10;
  uint16_t v3 = v % 10;
  uint16_t tmpBuffer;
  if (v1 == 0) {
    dispBuffer[15] = 0;
    dispBuffer[16] = 0;
  }
  else {
    tmpBuffer = getHex(v1);
    dispBuffer[15] = tmpBuffer % 100;
    dispBuffer[16] = tmpBuffer / 100;
  }
  dispBuffer[16] = dispBuffer[16] + 1;
  WriteOneData(15 , dispBuffer[15]);
  WriteOneData(16 , dispBuffer[16]);


  tmpBuffer = getHex(v2);

  dispBuffer[17] = tmpBuffer % 100;
  dispBuffer[18] = tmpBuffer / 100;
  dispBuffer[18] = dispBuffer[18] + 1;
  WriteOneData(17 , dispBuffer[17]);
  WriteOneData(18 , dispBuffer[18]);


  tmpBuffer = getHex(v3);

  dispBuffer[19] = tmpBuffer % 100;
  dispBuffer[20] = tmpBuffer / 100 + dispBuffer[20] % 2;
  WriteOneData(19 , dispBuffer[19]);
  WriteOneData(20 , dispBuffer[20]);

}


void setLittleHumi(uint16_t v) {
  uint16_t v1 = v / 100;
  uint16_t v2 = (v % 100) / 10;
  uint16_t v3 = v % 10;

  uint16_t tmpBuffer;
  if (v1 == 0) {
    dispBuffer[22] = 0;
    dispBuffer[23] = 0;
  }
  else {
    tmpBuffer = getHex(v1);
    dispBuffer[22] = tmpBuffer % 100;
    dispBuffer[23] = tmpBuffer / 100;
  }

  dispBuffer[22] = tmpBuffer % 100;
  dispBuffer[23] = tmpBuffer / 100;


  dispBuffer[23] = dispBuffer[23] + 1;
  WriteOneData(22 , dispBuffer[22]);
  WriteOneData(23 , dispBuffer[23]);


  tmpBuffer = getHex(v2);

  dispBuffer[24] = tmpBuffer % 100;
  dispBuffer[25] = tmpBuffer / 100;
  dispBuffer[25] = dispBuffer[25] + 1;
  WriteOneData(24 , dispBuffer[24]);
  WriteOneData(25 , dispBuffer[25]);


  tmpBuffer = getHex(v3);

  dispBuffer[26] = tmpBuffer % 100;
  dispBuffer[27] = tmpBuffer / 100;
  dispBuffer[27] = dispBuffer[27] + 1;
  WriteOneData(26 , dispBuffer[26]);
  WriteOneData(27 , dispBuffer[27]);

}

void wifiFlog(int v) {
  if (v == 0) {
    WriteOneData(28 , 0);
  }
  else {
    WriteOneData(28 , 15);
  }
}

void fanFlog(int v) {
  if (v == 0) {
    dispBuffer[12] = dispBuffer[12] & 0xFE;
  }
  else {
    dispBuffer[12] = dispBuffer[12] | 1;
  }
  WriteOneData(12 , dispBuffer[12]);
}

void tempFlog(int v) {
  if (v == 0) {
    dispBuffer[10] = dispBuffer[10] & 0xFE;
  }
  else {
    dispBuffer[10] = dispBuffer[10] | 1;
  }
  WriteOneData(10 , dispBuffer[10]);
}


void setBatValue(int v) {

  if (v == 0) {
    dispBuffer[20] = dispBuffer[20] & 0xFE;
    dispBuffer[21] = 8;
  }
  else if (v == 1) {
    dispBuffer[20] = dispBuffer[20] | 1;
    dispBuffer[21] = 8;
  }
  else if (v == 2) {
    dispBuffer[20] = dispBuffer[20] | 1;
    dispBuffer[21] = 9;
  }
  else if (v == 3) {
    dispBuffer[20] = dispBuffer[20] | 1;
    dispBuffer[21] = 11;
  }
  else if (v == 4) {
    dispBuffer[20] = dispBuffer[20] | 1;
    dispBuffer[21] = 15;
  }
  WriteOneData(20 , dispBuffer[20]);
  WriteOneData(21 , dispBuffer[21]);
}


/*
  tempFlog wifiFlog fanFlog
  0：边缘短线 * 4
  1：信号强度 * 3    摆风标志
  2：边缘短线 * 4
  3：时间数字，H十位部分
  4：时间数字，H十位部分  上方月亮标志
  5：时间数字，H个位部分
  6：时间数字，H个位部分  时间数字 冒号
  7：时间数字，M十位部分
  8：时间数字，M十位部分  开机标志
  9：时间数字，M个位部分
  10：时间数字，M个位部分 温度计标志
  11：大数字十位部分
  12：大数字十位部分    风扇标志
  13：大数字个位部分
  14：大数字个位部分    大数字 ° 符号
  15：小温度十位部分
  16：小温度十位部分    小温度 ° 符号
  17：小温度个位部分
  18：小温度个位部分    小温度 . 符号
  19：小温度零位部分
  20：小温度零位部分    电量一个电符号
  21：电量符号 部分
  22：小湿度十位部分
  23：小湿度十位部分
  24：小湿度个位部分
  25：小湿度个位部分    小湿度 . 符号
  26：小湿度零位部分
  27：小湿度零位部分    小湿度 % 符号
  28：wifi标志
  29：边缘短线 * 1   下方月亮标志    红蓝线
  30：边缘短线 * 4
  31：边缘短线 * 4


*/
#define DATA_1 digitalWrite(dataPin, HIGH);
#define DATA_0 digitalWrite(dataPin, LOW);

#define WRITE_1 digitalWrite(wrPin, HIGH);
#define WRITE_0 digitalWrite(wrPin, LOW);

#define CS_1 digitalWrite(csPin, HIGH);
#define CS_0 digitalWrite(csPin, LOW);



#define SYSDIS   0x00         //关系统振荡器和LCD偏压发生器
#define SYSEN    0x02    //打开系统振荡器

#define LCDOFF   0x04     //关LCD偏压
#define LCDON    0x06     //开LCD偏压

#define TONEON   0x12     //打开声音输出
#define TONEOFF  0x10     //关闭声音输出

#define XTAL     0x28     //外部接晶振
#define RC       0x30     //内部RC振荡

#define BIAS     0x52     //1/3偏压 4公共口,0X52=1/2偏压

#define WDTDIS   0x0a     //禁止看门狗
#define WDTEN    0x0e     //开启看门狗

#define nop delayMicroseconds(10) //宏定义


/********************延时函数*************************/
void delay_nms(uint n)
{
  delay(n);
}

/********************从高位写入数据*************************/
void Write_Data_H(uchar Data, uchar Cnt)           //Data的高cnt位写入TM1621，高位在前
{
  uchar i;
  for (i = 0; i < Cnt; i++)
  {
    WRITE_0;
    //从最高位发送
    if (Data & 0x80) {
      DATA_1;
    }
    else {
      DATA_0;
    }
    nop;
    nop;
    WRITE_1;
    Data <<= 1;
  }
  WRITE_0;
  DATA_0;
}

/********************从低位写入数据*************************/
void Write_Data_L(uchar Data, uchar Cnt)       //Data 的低cnt位写入TM1621，低位在前
{
  unsigned char i;
  for (i = 0; i < Cnt; i++)
  {
    WRITE_0;
    if (Data & 0x01) {
      DATA_1;
    }
    else {
      DATA_0;
    }
    nop;
    nop;
    WRITE_1;
    Data >>= 1;
  }
  WRITE_0;
  DATA_0;
}

/********************写入控制命令*************************/
void WriteCmd(uchar Cmd)
{
  CS_0;
  nop;
  Write_Data_H(0x80, 4);    //写入命令标志100
  Write_Data_H(Cmd, 8);     //写入命令数据
  CS_1;
  nop;
}

/*********指定地址写入数据，实际写入后4位************/
void WriteOneData(uchar Addr, uchar Data)
{
  CS_0;
  Write_Data_H(0xa0, 3);    //写入数据标志101
  Write_Data_H(Addr << 2, 6); //写入地址数据(A5-A4-A3-A2-A1-A0)从高位开始写数据
  Write_Data_L(Data, 4);    //写入数据
  CS_1;
  nop;
}

/*********连续写入方式，每次数据为8位，写入数据************
  void WriteAllData(uchar Addr,uchar *p,uchar cnt)
  {
  uchar i;
  CS_0;
  Write_Data_H(0xa0,3);      //写入数据标志101
  Write_Data_H(Addr<<2,6);   //写入地址数据
  for(i=0;i<cnt;i++)            //写入数据
  {
   Write_Data_L(*p,8);
   p++;
  }
  CS_1;
  nop;
  }
***************这个子程序暂时没用，注释掉******************/


/*******************TM1621初始化**********************/
void TM1621_init()
{
  CS_1;
  WRITE_1;
  DATA_1;
  nop;
  delay_nms(1);
  WriteCmd(BIAS);                 //1/3偏压 4公共口
  WriteCmd(RC);                         //内部RC振荡
  WriteCmd(SYSDIS);                 //关系统振荡器和LCD偏压发生器
  WriteCmd(WDTDIS);                 //禁止看门狗
  WriteCmd(SYSEN);                 //打开系统振荡器
  WriteCmd(LCDON);                 //开LCD偏压

}
/*************TM1621清屏函数*****************/
void Clear1621()
{
  uchar i;
  for (i = 9; i < 22; i++)
  {
    WriteOneData(i, 0x00);
  }
}
/*******************************************************************************
  函数名称                   ：1621显示函数显示
  函数功能                   :
  输    入         : 无
  输    出         : 无
*******************************************************************************/
void Disp1621(uchar Addr, uchar Data)
{
  WriteOneData(Addr, Data);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      isSocConnected = 0;
      break;
    case WStype_CONNECTED: {
        isSocConnected = 1;
        // send message to server when Connected
        //        webSocket.sendTXT("Connected");
      }
      break;
    case WStype_TEXT:
      ;
      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      ;
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_PING:
      // pong will be send automatically
      ;
      break;
    case WStype_PONG:
      // answer to a ping we send
      ;
      break;
  }

}

void setup(void) {
  //  Serial.begin(115200);
  //  Serial.println("\nasd\n");
  pinMode(ledPin, OUTPUT); //设置引脚模式
  pinMode(wrPin, OUTPUT); //设置引脚模式
  pinMode(csPin, OUTPUT); //设置引脚模式
  pinMode(dataPin, OUTPUT); //设置引脚模式

  pinMode(keyPin, INPUT);
  pinMode(ctrlRPin, INPUT);
  pinMode(ctrlLPin, INPUT);


  analogWrite(ledPin, ledValue);


  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    MDNS.begin(host);
    server.on("/", handleRoot);
    server.on("/updatebin", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        WiFiUDP::stopAll();
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          ;
          //          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          ;
          //          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
        } else {
          ;
          //          Update.printError(Serial);
        }
        //        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);


  } else {

  }
  //  Serial.println("server.argName(i)");
  delay(30);

  // server address, port and URL
  webSocket.begin("192.168.71.127", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //  webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

  delay(30);
  //  Serial.println("TM1621_init();");
  TM1621_init();
  delay(10);
  ledValue = 200;
  analogWrite(ledPin, ledValue);

  hdc1080.begin(0x40);
  delay(10);
  for (int aa = 0; aa < 32; aa++)
    WriteOneData(aa , 0);

  delay(100);
  RTC.begin();
  setBigNum(0);
  //  rtc_time_unit.Hours = 14;
  //  rtc_time_unit.Minutes = 40;
  //  rtc_time_unit.Seconds = 5;
  //
  //  rtc_date_unit.WeekDay = 5;
  //  rtc_date_unit.Month = 5;
  //  rtc_date_unit.Date = 27;
  //  rtc_date_unit.Year = 2022;
  //
  //  RTC.setTime(&rtc_time_unit);
  //  RTC.setDate(&rtc_date_unit);
}


void handleRoot() {

  //  String message = "File Not Found\n\n";
  String req;
  int hh = 0;
  int mm = 0;
  int ss = 0;

  for (uint8_t i = 0; i < server.args(); i++) {


    int cmd = server.argName(i).toInt();
    int val = server.arg(i).toInt();
    if (cmd < 32) {
      WriteOneData(cmd , val);
    }
    else if (cmd == 32) {
      for (int aa = 0; aa < 32; aa++)
        WriteOneData(aa , val);

    } else if (cmd == 33) {
      ledValue = val;
    } else if (cmd == 97) {
      hh = val;
    } else if (cmd == 98) {
      mm = val;
    } else if (cmd == 99) {
      ss = val;
    }

    else if (cmd == 40) {
      tempFlog(val);
    } else if (cmd == 41) {
      fanFlog(val);
    } else if (cmd == 42) {
      wifiFlog(val);
    }



    // wifiFlog fanFlog


    if (hh + mm + ss != 0) {

      rtc_time_unit.Hours = hh;
      rtc_time_unit.Minutes = mm;
      rtc_time_unit.Seconds = ss;
      RTC.setTime(&rtc_time_unit);

    }



  }


  //  req = (String)"{codq:20,bat:" + batValue + "}";
  req = (String)"{\"codq\":20,\"batValue\":" + batValue + ",\"batNum\":" + batNum + ",\"millis\":" + millis() + "}";
  server.send(200, "text/html", req);

}


//      tempFlog(val);
//    } else if (cmd == 41) {
//      fanFlog(val);
//    } else if (cmd == 42) {
//      wifiFlog(val);

//int tempFlogNum = 0;
//int fanFlogNum = 0;
//int wifiFlogNum = 0;
//
//int tempFlogNumOld = 0;
//int fanFlogNumOld = 0;
//int wifiFlogNumOld = 0;


int ctrlROld = 0;
int ctrlLOld = 0;
long ctrlTime = 0;



void sendCommand(String cmd)
{
  if (ctrlAvail == 1) {
    webSocket.sendTXT(cmd);
  }
  //  WiFiClient client;
  //  HTTPClient http_client;
  //  String data = "nn";
  //  String req = (String)hostdd + "text?" + cmd;
  //  http_client.begin(client,req);
  //  int http_code = http_client.GET();
  //
  //  if (http_code > 0)
  //  {
  //    data = http_client.getString();
  //    Serial.println("WiFi连接失败，请用手机进行配网");
  //    Serial.println(data);
  //
  //  }
}



int keyOld = 0;
long keyTime = 0;

int nn = 300;
long timeNN = 0;
int ctrlNum = 0;
int ctrlNumOld = 0;
long ctrlNumOldTime = 0;
long vulemTime = 0;
//上一曲下一曲变动，两次之间要谈起按键
long nextUpTime = 0;
void loop(void) {
  webSocket.loop();
  server.handleClient();
  MDNS.update();
  delay(1);
  if (ledValueOld != ledValue) {
    analogWrite(ledPin, ledValue);
    ledValueOld = ledValue;
  }


  //
  //  tempFlogNum = digitalRead(ctrlRPin);
  //  fanFlogNum = digitalRead(ctrlLPin);
  //  wifiFlogNum = digitalRead(keyPin);


  if (ctrlROld != digitalRead(ctrlRPin)) {
    if (millis() - ctrlTime < 2000) {
      if (ctrlROld == ctrlLOld) {
        ctrlNum++;
        if (ctrlNum > 99)
          ctrlNum = 0;
      }
      else {
        ctrlNum--;
        if (ctrlNum < 0)
          ctrlNum = 99;
      }
      ledValue = ctrlNum * 2.5;
      //      setBigNum(ctrlNum);
      ctrlTime = 0;
    }
    ctrlROld = digitalRead(ctrlRPin);
  }
  if (ctrlLOld != digitalRead(ctrlLPin)) {
    ctrlTime = millis();
    //    fanFlog(ctrlAvail);
    ctrlLOld = digitalRead(ctrlLPin);
    //    fanFlog(ctrlLOld);
  }

  if (isSocConnected != isSocConnectedOld) {

    isSocConnectedOld = isSocConnected;
    wifiFlog(isSocConnectedOld);
  }
  int isSocConnected = 0;
  int isSocConnectedOld = 0;

  if (1 == digitalRead(keyPin)) {

    if (keyOld == 0) {

      ctrlNumOld = ctrlNum;
      keyTime = millis();

      keyOld = digitalRead(keyPin);

    }

    if (keyOld == 1 and millis() - keyTime > 3000) {
      ctrlAvail = 1 - ctrlAvail;


      fanFlog(ctrlAvail);
      keyOld = 6;

    }


  }

  if (0 == digitalRead(keyPin)) {

    keyTime = millis();
    if (keyOld == 1) {
      if (millis() - keyTime < 500) {
        //        开始暂停
        sendCommand("media_play_pause");
        keyOld = 0;
        ctrlNumOld = ctrlNum;
        vulemTime = millis();
      }

    }
    if (keyOld != 0) {

      ctrlNumOld = ctrlNum;

    }

    keyOld = digitalRead(keyPin);
  }


  if (ctrlNum != ctrlNumOld) {
    int tmp = ctrlNum - ctrlNumOld;
    //    上一曲下一曲
    if (keyOld == 1) {

      if (abs(tmp) > 50) {
        if (ctrlNumOld > 50) {
          ctrlNumOld = ctrlNumOld - 100;
        }
        else {
          ctrlNumOld = ctrlNumOld + 100;
        }
      }
      if (ctrlNum - ctrlNumOld > 2) {
        keyOld = 5;
        // 下一曲
        sendCommand("media_next");
        ctrlNumOld = ctrlNum;
        vulemTime = millis();
      }
      else if (ctrlNum - ctrlNumOld < -2) {
        keyOld = 5;
        // 上一曲
        sendCommand("media_prev");
        ctrlNumOld = ctrlNum;
        vulemTime = millis();
      }
    }
    else if (keyOld == 0) {
      if (millis() - vulemTime > 900) {
        if (abs(tmp) > 50) {
          if (ctrlNumOld > 50) {
            ctrlNumOld = ctrlNumOld - 100;
          }
          else {
            ctrlNumOld = ctrlNumOld + 100;
          }
        }
        if (ctrlNum - ctrlNumOld > 2) {
          //       音量加
          sendCommand("volume_up");
          ctrlNumOld = ctrlNum;
          vulemTime = millis();
        }
        else if (ctrlNum - ctrlNumOld < -2) {
          //        音量减
          sendCommand("volume_down");
          ctrlNumOld = ctrlNum;
          vulemTime = millis();
        }

      }
      else {
        ctrlNumOld = ctrlNum;
      }
    }


  }

  if (nn > 1000) {

    float t = hdc1080.readTemperature() - 4.4;
    float h = hdc1080.readHumidity();
    setLittleTemp((uint16_t)(t * 10));
    setLittleHumi((uint16_t)(h * 10));

    //    setBigNum((uint16_t)t);
    //    setLittleHumi setLittleTemp


    nn = 0;
  }



  if (nn % 2000 == 0) {
    batValue = analogRead(sensorPin);

    //    Serial.print("batValue -- ");
    //    Serial.println(batValue);
    //    webSocket.sendTXT("message here");
    batNum = (batValue - 890) / 24;
    if (batNum < 0)
      batNum = 0;
    if (batNum > 4)
      batNum = 4;

    setBatValue(batNum);

  }

  nn++;
  if (timeNN > 9) {

    RTC.getTime(&rtc_time_unit);
    setTimeNum(rtc_time_unit.Hours, rtc_time_unit.Minutes, rtc_time_unit.Seconds);
    setBigNum(rtc_time_unit.Seconds);
    timeNN = 0;
  }
  timeNN++;


  if (millis() < ctrlTime) {
    ctrlTime = 0;
  }
}
