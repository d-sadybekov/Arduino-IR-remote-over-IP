#include <UIPEthernet.h>
#include <Arduino.h>
#if RAMEND <= 0x4FF || (defined(RAMSIZE) && RAMSIZE < 0x4FF)
#define RAW_BUFFER_LENGTH 150     // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#define EXCLUDE_EXOTIC_PROTOCOLS  // saves around 650 bytes program memory if all other protocols are active
#elif RAMEND <= 0x8FF || (defined(RAMSIZE) && RAMSIZE < 0x8FF)
//#define RAW_BUFFER_LENGTH 600  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#else
//#define RAW_BUFFER_LENGTH 750  // 750 (600 if we have only 2k RAM) is the value for air condition remotes. Default is 112 if DECODE_MAGIQUEST is enabled, otherwise 100.
#endif
#define NO_LED_FEEDBACK_CODE  // saves 92 bytes program memory
#define IR_RECEIVE_PIN 2      // To be compatible with interrupt example, pin 2 is chosen here.
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
#include <IRremote.hpp>
byte mac[] = { 0xAE, 0xB2, 0x26, 0xE4, 0x4A, 0x5C };  // MAC-адрес
byte ip[] = { 192, 168, 1, 190 };                     // IP-адрес клиента
byte ipServ[] = { 192, 168, 1, 177 };                 // IP-адрес сервера
String myGetString = "";
uint16_t oldAddress;
uint16_t oldCommand;
String strAddress = "";
String strCommand = "";
int numRec = 1;
unsigned long inMuteMs = 0;
int clearCount = 0;
int LED=8;
bool needCon = true;
EthernetClient client;  // создаем клиента

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Ethernet.begin(mac, ip);  // инициализация контроллера
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void loop() {
  if (!client.connected()) {
    Serial.println(F("Установка соединения с сервером, подождите!"));
    client.connect(ipServ, 10885);
    delay(3000);
    if (client.connected()) {
      Serial.println("Connected!");
      digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    } else {
      Serial.println("Connection failed!");
      digitalWrite(LED, LOW);
    }
  }

  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (IrReceiver.decode()) {
    //if (millis() - inMuteMs > 200) {
    Serial.println(millis() - inMuteMs);
    // Print a minimal summary of received data
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    //IrReceiver.printIRResultMinimal(&Serial);
    //IrReceiver.printIRResultRawFormatted(&Serial, true);
    clearCount++;
    if ((millis() - inMuteMs < 500) && (IrReceiver.decodedIRData.address == oldAddress) && (IrReceiver.decodedIRData.command == oldCommand)) {
      Serial.println("repeat, ignoring");
    } else {
    oldAddress = IrReceiver.decodedIRData.address;
    oldCommand = IrReceiver.decodedIRData.command;
      /*
         * Finally check the received data and perform actions according to the received address and commands
         */
      if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
        //Serial.println(IrReceiver.decodedIRData.repeat);
        myGetString = "?" + String(numRec) + "?" + String(IrReceiver.decodedIRData.address) + "?" + String(IrReceiver.decodedIRData.command) + "?" + String(IrReceiver.decodedIRData.protocol) + "\n";
        //Serial.println("Считано с IR: ");
        Serial.println(myGetString);
        if (client.connected()) {
          for (int i = 0; i <= myGetString.length() - 1; i++)
            client.print(myGetString[i]);
        } else {
          Serial.println("Потеря связи");
        }
        myGetString = "";
      }
      inMuteMs = millis();
    }
    IrReceiver.resume();
    /*
    if (IrReceiver.decodedIRData.address == 0x4040) {
      Serial.println();
      if (IrReceiver.decodedIRData.command == 0x1) {
        // do something
        Serial.println();
      } else if (IrReceiver.decodedIRData.command == 0x11) {
        // do something else
      }
    }*/
    if (clearCount == 10) {
      clearCount = 0;
      Serial.end();
      delay(200);
      Serial.begin(115200);
    }  //очищаем буфер
  }
  /*
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (client.connected()) {
      client.print(inChar);
    }
  }*/
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    digitalWrite(LED, LOW);
  }
}