#include <GyverWDT.h>

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#define IR_SEND_PIN 3
#if !defined(FLASHEND)
#define FLASHEND 0xFFFF  // Dummy value for platforms where FLASHEND is not defined
#endif
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
#include <IRremote.hpp>
#define DELAY_AFTER_SEND 300
#define DELAY_AFTER_LOOP 5000

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 25, 20);
EthernetServer server(80);
EthernetClient client;

bool trigger = false;
bool reloadtrigger = true;
uint16_t sAddress;
uint16_t sCommand;
short intProtocol;
int resetPin = 12;

void setup() {
  digitalWrite(resetPin, HIGH);
  delay(200);
  pinMode(resetPin, OUTPUT);
  delay(200);
  Serial.begin(115200);
  Serial.println("initialization");
  IrSender.begin();  // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
  Ethernet.begin(mac, ip);
  // start the server
  server.begin();
}

void loop() {
  //if (millis() > 86400000)
  if (millis() > 43200000) {
    //Serial.println(millis());
    Serial.println("Reseting....");
    delay(100);
    Watchdog.enable(RESET_MODE, WDT_PRESCALER_1024);
    while (1) {                                              // Бесконечный цикл , эмуляция "зависания"
      if (!(millis() % 1000)) {                              // Каждую секунду
        Serial.println((uint16_t)((millis() / 1000) - 10));  // Вывести время после включения watchdog в секундах
        delay(10);
      }
    }
    //asm("JMP 0");
  }

  client = server.available();
  if (client) {                                     // client is true only if it is connected and has data to read
    String getData = client.readStringUntil('\n');  // read the message incoming from one of the clients
    getData.trim();                                 // trim eventual \r
    Serial.println(getData);                        // print the message to Serial Monitor
    if (getData.lastIndexOf("GET") != -1) {
      getData = getData.substring(getData.indexOf("?"), getData.lastIndexOf(" "));
      // Serial.println(getData);
    }
    if (getData[1] == '1') {
      Serial.println("getData[1] == '1'");
      //client.print("echo: ");
      //client.println(getData);  // this is only for the sending client
      if (getData.lastIndexOf("?") != -1) {
        intProtocol = (getData.substring(getData.lastIndexOf("?") + 1)).toInt();
        getData = getData.substring(0, getData.lastIndexOf("?"));
        if (getData.lastIndexOf("?") != -1) {
          sCommand = (uint16_t)(getData.substring(getData.lastIndexOf("?") + 1)).toInt();
          getData = getData.substring(0, getData.lastIndexOf("?"));
          if (getData.lastIndexOf("?") != -1) {
            //String myTemp = getData.substring(getData.lastIndexOf("?") + 1);
            sAddress = (uint16_t)(getData.substring(getData.lastIndexOf("?") + 1)).toInt();
            getData = getData.substring(0, getData.lastIndexOf("?"));
            //intRecnum = getData.toInt();
            getData = "";
          }
        }
      }
      trigger = true;
    } else {
      if (getData.length() == 0) {
        Serial.println("getData == ''");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<head>");
        client.println("<meta charset='UTF-8'>");
        client.println("</head>");
        client.print("<h3>Для отключения рекламы перейдите по ссылкам:</h3>");
        client.print("<a href='?2'>Маленький пульт. Exit.</a></br>");
        client.print("<a href='?3'>Средний пульт(старый). Exit.</a></br>");
        client.print("<a href='?4'>Большой пульт. Exit.</a></br></br>");
        client.print("<a href='?5'>Средний пульт(старый). Прибавить громкость.</a></br>");    
        client.print("<a href='?6'>Средний пульт(старый). Уменьшить громкость.</a></br>");              
        client.println("<input type='submit' id='submit' style='visibility:hidden;' value='Refresh'>");
        client.println("</body>");
        client.println("</html>");
        // задержка для получения клиентом данных
        delay(100);
        // закрыть соединение
        client.stop();
        reloadtrigger = false;
        trigger = false;
      }
      if ((getData[1] == '2') || (getData[1] == '3') || (getData[1] == '4') || (getData[1] == '5') || (getData[1] == '6')) {
        Serial.println("getData[1] == '2,3,4,5,6'");
        if (getData[1] == '2') { //малый пульт. Кнопка Exit. IrSender.sendNEC(0x9948, 0xD, <numberOfRepeats>);
          intProtocol = 7;
          sCommand = 0xD;
          sAddress = 0x9948;
        }
        if (getData[1] == '3') { //средний пульт (старый). Кнопка Exit. IrSender.sendNEC(0xFD01, 0xC5, <numberOfRepeats>);
          intProtocol = 7;
          sCommand = 0xC5;
          sAddress = 0xFD01;
        }
        if (getData[1] == '4') { //большой пульт. Exit. IrSender.sendNEC(0x202, 0x67, <numberOfRepeats>); Protocol=NEC Address=0x202 Command=0x67
          intProtocol = 7;
          sCommand = 0x67;
          sAddress = 0x202;
        }
        if (getData[1] == '5') { //средний пульт (старый). Кнопка Vol+. Protocol=NEC Address=0xFD01 Command=x85
          intProtocol = 7;
          sCommand = 0x85;
          sAddress = 0xFD01;
        }
        if (getData[1] == '6') { //средний пульт (старый). Кнопка Vol-. Protocol=NEC Address=0xFD01 Command=0x86
          intProtocol = 7;
          sCommand = 0x86;
          sAddress = 0xFD01;
        }
        if (reloadtrigger == false) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<meta charset='UTF-8'>");
          client.println("</head>");
          client.println("<body>");
          client.println("<form method='get'>");
          client.print("<div>");
          client.print("<h3>Команда выполнена</h3>");
          client.print("<script>");
          client.print("alert('Команда выполнена!');");
          client.print("setTimeout(function() {  window.history.back(); }, 2000);");
          client.print("</script>");
          client.println("</div>");
          client.println("</form>");
          client.println("<br />");
          client.println("</body>");
          client.println("</html>");
          // задержка для получения клиентом данных
          delay(100);
          // закрыть соединение
          client.stop();
          reloadtrigger = true;
          trigger = true;
        }
      }
    }
  }
  if (trigger) {
    IRData IRSendData;
    //Serial.print(F("Recieve address & command : "));
    //Serial.print(sAddress);
    //Serial.print(F(" "));
    //Serial.println(sCommand);
    IRSendData.address = sAddress;
    IRSendData.command = sCommand;
    IRSendData.flags = IRDATA_FLAGS_EMPTY;
    IRSendData.protocol = intProtocol;
    //Serial.print(F("Send "));
    //Serial.println(getProtocolString(IRSendData.protocol));
    //Serial.flush();
    IrSender.write(&IRSendData, 1);
    trigger = false;
    delay(DELAY_AFTER_SEND);
  }
}
