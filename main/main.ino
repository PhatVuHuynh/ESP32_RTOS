#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// #include "Adafruit_MQTT.h"
// #include "Adafruit_MQTT_Client.h"

#define RELAY_MANUAL_AUTO_TIME 10

#define WLAN_SSID "RNM esp sida"
#define WLAN_PASS "whyarewestillhere"

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
JsonDocument doc_tx;


// #define AIO_SERVER      "io.adafruit.com"
// #define AIO_SERVERPORT  1883
// #define AIO_USERNAME    "NPNLab_BBC"
// #define AIO_KEY         ""

// #define OHS_SERVER      "mqtt.ohstem.vn"
// #define OHS_SERVERPORT  1883
// #define OHS_USERNAME    "abcd0386433465"
// #define OHS_KEY         "12345678"


// #define BKTK_SERVER      "mqttserver.tk"
// #define BKTK_SERVERPORT  1883
// #define BKTK_USERNAME    "innovation"
// #define BKTK_KEY         "Innovation_RgPQAZoA5N"

// WiFiClient client;
// //Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
// //Adafruit_MQTT_Client mqtt(&client, OHS_SERVER, OHS_SERVERPORT, OHS_USERNAME, OHS_USERNAME, OHS_KEY);
// Adafruit_MQTT_Client mqtt(&client, BKTK_SERVER, BKTK_SERVERPORT, BKTK_USERNAME, BKTK_USERNAME, BKTK_KEY);

// /****************************** Feeds ***************************************/

// // Setup a feed called 'time' for subscribing to current time
// Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

// // Setup a feed called 'slider' for subscribing to changes on the slider
// Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);

// // Setup a feed called 'onoff' for subscribing to changes to the button
// //Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);
// Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, OHS_USERNAME "/feeds/V1", MQTT_QOS_1);

// // Setup a feed called 'photocell' for publishing.
// //Adafruit_MQTT_Publish sensory = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sensory");
// Adafruit_MQTT_Publish sensory = Adafruit_MQTT_Publish(&mqtt, OHS_USERNAME "/feeds/V20");

// Define your tasks here
void TaskBlink(void *pvParameters);
void TaskTemperatureHumidity(void *pvParameters);
void TaskSoilMoistureAndRelay(void *pvParameters);
void TaskLightAndLED(void *pvParameters);

//Define your components here
Adafruit_NeoPixel pixels3(4, D7, NEO_GRB + NEO_KHZ800);
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

// int pubCount = 0;
float temp = 0.0;
float humid = 0.0;
uint16_t moist = 0;
uint16_t lightRes = 0;

bool LED_BUILTIN_STATUS = LOW;
bool RELAY_STATUS = LOW;
bool RGB_STATUS = LOW;
uint16_t RELAY_AUTO_COUNT = 0;

// String tempS;
// String humidS;
// String lightResS;
// String moistS;

// void slidercallback(double x) {
//   Serial.print("Hey we're in a slider callback, the slider value is: ");
//   Serial.println(x);
// }

// void onoffcallback(char *data, uint16_t len) {
//   Serial.print("Hey we're in a onoff callback, the button value is: ");
//   Serial.println(data);
// }

// void MQTT_connect() {
//   int8_t ret;

//   // Stop if already connected.
//   if (mqtt.connected()) {
//     return;
//   }

//   Serial.print("Connecting to MQTT... ");

//   uint8_t retries = 3;
//   while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
//     Serial.println(mqtt.connectErrorString(ret));
//     Serial.println("Retrying MQTT connection in 10 seconds...");
//     mqtt.disconnect();
//     delay(10000);  // wait 10 seconds
//     retries--;
//     if (retries == 0) {
//       // basically die and wait for WDT to reset me
//       while (1);
//     }
//   }
//   Serial.println("MQTT Connected!");
// }

void setup() {

  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200); 


  dht20.begin();
  lcd.begin();
  pixels3.begin();
  // pinMode(LED_BUILTIN, OUTPUT);
  // Serial.println("123");

  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(WLAN_SSID, WLAN_PASS);
  
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(WLAN_SSID, WLAN_PASS);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println();
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  delay(100);

  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  // Serial.println("wifi");

  server.on("/", handle_Onconnect);
  // server.on("/t", handle_temp_humid);
  server.on("/relayOn", handle_relayOn);
  server.on("/relayOff", handle_relayOff);
  server.on("/RGBOn", handle_RGBOn);
  server.on("/RGBOff", handle_RGBOff);
  // server.on("/l", handle_light);
  // server.on("/ledon", handle_ledOn);
  // server.on("/ledoff", handle_ledOff);
  server.onNotFound(handle_NotFound);
  server.begin();
  webSocket.begin();
  Serial.println("Server Started.");

  // slider.setCallback(slidercallback);
  // onoffbutton.setCallback(onoffcallback);
  // mqtt.subscribe(&slider);
  // mqtt.subscribe(&onoffbutton);

  
  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskTemperatureHumidity, "Task Temperature" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);
  
  
  //Now the task scheduler is automatically started.
  Serial.printf("Basic Multi Threading Arduino Example\n"); 
}

void loop() {
  webSocket.loop();
  webSocket.onEvent(webSocketEvent);

  String jsonString = "";
  doc_tx["temp"] = String(temp);
  doc_tx["humid"] = String(humid);
  doc_tx["moist"] = String(moist * 100 / 4096);
  doc_tx["light"] = String(lightRes * 100 / 4096);
  doc_tx["relay"] = RELAY_STATUS;
  doc_tx["RGB"] = RGB_STATUS;
  serializeJson(doc_tx, jsonString);
    
  webSocket.broadcastTXT(jsonString);
  delay(2000);

  // uint8_t test[2];
  // test[0] = moist;
  // test[1] = lightRes;

  // // webSocket.broadcastTXT(moistS);
  // webSocket.broadcastTXT(test, 16);

  // if(LED_BUILTIN_STATUS){
  //   digitalWrite(LED_BUILTIN, HIGH);
  // }
  // else{
  //   digitalWrite(LED_BUILTIN, LOW);
  // }
  // MQTT_connect();
  // mqtt.processPackets(10000);
  // if(! mqtt.ping()) {
  //   mqtt.disconnect();
  // }
}

void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length){
  JsonDocument doc_rx;
  switch (type){
    case WStype_DISCONNECTED:
      Serial.println("Client Disconnected.");
      break;
    case WStype_CONNECTED:
      Serial.println("Client Connected.");
      break;
    case WStype_TEXT:
      String text;
      for(int i =0; i < length; ++i){
        // Serial.print((char)payload[i]);
        text += (char)payload[i];
      }
      deserializeJson(doc_rx, text);

      RGB_STATUS = doc_rx["RGB"];
      RELAY_STATUS = doc_rx["relay"];
      
      Serial.println(text);
      serializeJson(doc_rx, Serial);
      Serial.println();
      break;
  }
}

String getHTML(){
  String htmlcode = "<!DOCTYPE html> <html>\n";
  htmlcode += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>\n";
  htmlcode += "<tittle>Test</tittle>\n";
  htmlcode += "</head>\n";

  htmlcode += "<body>\n";
  htmlcode += "<h1>Yolo Uno Mini Server</h1>\n";
  htmlcode += "<h3>Simple demo using AP mode</h3>\n";

  htmlcode += "<p>Temperature: ";
  // htmlcode += tempS;
  htmlcode += "<span id = 'temp'></span> *C";
  htmlcode += "</p>\n";

  htmlcode += "<p>Humid: ";
  // htmlcode += humidS;
  htmlcode += "<span id = 'humid'></span>%";
  htmlcode += "</p>\n";

  htmlcode += "<p>Moist: ";
  // htmlcode += moistS;
  htmlcode += "<span id = 'moist'></span>%";
  htmlcode += "</p>\n";

  // if(RELAY_STATUS){
  //   htmlcode += "<p>Relay status: ON</p><a href='/relayOff'>Please turn off relay</a>\n";
  // }
  // else{
  htmlcode += "<p>Relay status: ";
  htmlcode += "<span id = 'relay'></span> </p>";
  htmlcode += "<button type = 'button' id = 'Relay_send'></button>";
  // htmlcode += "<a href='/relayOn'>Please turn on relay</a>\n";
  // }

  htmlcode += "<p>Light: ";
  // htmlcode += moistS;
  htmlcode += "<span id = 'light'></span>%";
  htmlcode += "</p>\n";

  // htmlcode += "<p>Light: ";
  // htmlcode += lightResS;
  // htmlcode += "%</p>\n";

  // if(RGB_STATUS){
  //   htmlcode += "<p>RGB LED status: ON</p><a href='/RBGOff'>Please turn off RGB LED</a>\n";
  // }
  // else{
  htmlcode += "<p>RGB status: ";
  htmlcode += "<span id = 'RGB'></span> </p>";
  htmlcode += "<button type = 'button' id = 'RGB_send'></button>";
  // htmlcode += "<p>RGB LED status: OFF</p><a href='/RBGOn'>Please turn on RGB LED</a>\n";
  // }

  htmlcode += "</body>\n";

  htmlcode += "<script>\n var Socket; var obj;\n";
  htmlcode += "document.getElementById('Relay_send').addEventListener('click', relaySend);\n";
  htmlcode += "document.getElementById('RGB_send').addEventListener('click', rgbSend);\n";
  htmlcode += "function init(){\n";
  htmlcode += "Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n";
  htmlcode += "Socket.onmessage = function(event){\n";
  htmlcode += "processCommand(event);};}\n";

  htmlcode += "function processCommand(event){\n";
  // htmlcode += "console.log(event);\n";
  htmlcode += "obj = JSON.parse(event.data);\n";
  htmlcode += "console.log(obj);\n";
  htmlcode += "document.getElementById('temp').innerHTML = obj.temp;\n";
  htmlcode += "document.getElementById('humid').innerHTML = obj.humid;\n";
  htmlcode += "document.getElementById('moist').innerHTML = obj.moist;\n";
  htmlcode += "document.getElementById('relay').innerHTML = obj.relay == true ? 'ON' : 'OFF';\n";
  htmlcode += "document.getElementById('Relay_send').innerHTML = obj.relay == true ? 'Turn off relay' : 'Turn on relay';\n";
  htmlcode += "document.getElementById('light').innerHTML = obj.light;\n";
  htmlcode += "document.getElementById('RGB').innerHTML = obj.RGB == true ? 'ON' : 'OFF';\n";
  htmlcode += "document.getElementById('RGB_send').innerHTML = obj.RGB == true ? 'Turn off RGB' : 'Turn on RGB';}\n";
  // htmlcode += "console.log(obj);}\n";

  htmlcode += "function relaySend(){\n";
  htmlcode += "let sendRelay = JSON.stringify({relay: !obj.relay});\n";
  htmlcode += "console.log(sendRelay);\n";
  htmlcode += "Socket.send(sendRelay);}\n";

  htmlcode += "function rgbSend(){\n";
  htmlcode += "let sendRGB = JSON.stringify({RGB: !obj.RGB});\n";
  htmlcode += "console.log(sendRGB);\n";
  htmlcode += "Socket.send(sendRGB);}\n";

  htmlcode += "window.onload = function(event){\n";
  htmlcode += "init();}\n";
  htmlcode += "</script>";

  htmlcode += "</html>\n";

  return htmlcode;
}

void handle_Onconnect(){
  // LED_BUILTIN_STATUS = LOW;
  // Serial.println("LED status: OFF");
  server.send(200, "text/html", getHTML());
}

// void handle_temp_humid(){
//   // LED_BUILTIN_STATUS = LOW;
//   // Serial.println("LED status: OFF");
//   server.send(200, "text/html", getHTML());
// }

void handle_relayOn(){
  RELAY_STATUS = HIGH;
  RELAY_AUTO_COUNT = RELAY_MANUAL_AUTO_TIME;
  Serial.println("Relay status: ON");
  server.send(200, "text/html", getHTML());
}

void handle_relayOff(){
  RELAY_STATUS = LOW;
  RELAY_AUTO_COUNT = RELAY_MANUAL_AUTO_TIME;
  Serial.println("Relay status: OFF");
  server.send(200, "text/html", getHTML());
}

void handle_RGBOn(){
  RGB_STATUS = HIGH;
  Serial.println("RGB LED status: ON");
  server.send(200, "text/html", getHTML());
}

void handle_RGBOff(){
  RGB_STATUS = LOW;
  Serial.println("RGB LED status: OFF");
  server.send(200, "text/html", getHTML());
}

// void handle_light(){
//   // LED_BUILTIN_STATUS = LOW;
//   // Serial.println("LED status: OFF");
//   server.send(200, "text/html", getHTML());
// }

// void handle_ledOn(){
//   // LED_BUILTIN_STATUS = HIGH;
//   Serial.println("LED status: ON");
//   server.send(200, "text/html", getHTML());
// }

// void handle_ledOff(){
//   // LED_BUILTIN_STATUS = LOW;
//   Serial.println("LED status: OFF");
//   server.send(200, "text/html", getHTML());
// }

void handle_NotFound(){
  server.send(404, "text/plain", "NOT FOUND");
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/


uint32_t x=0;
void TaskBlink(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(LED_BUILTIN, OUTPUT);
  

  while(1) {    
    server.handleClient();

    digitalWrite(LED_BUILTIN, LED_BUILTIN_STATUS);  // turn the LED ON
    LED_BUILTIN_STATUS = !LED_BUILTIN_STATUS;

    if(RELAY_AUTO_COUNT != 0) --RELAY_AUTO_COUNT;
    delay(1000);
    // if (sensory.publish(x++)) {
    //   Serial.println(F("Published successfully!!"));
    // }
  }
}


void TaskTemperatureHumidity(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    // Serial.println("Task Temperature and Humidity");
    dht20.read();
    // Serial.println(dht20.getTemperature());
    // Serial.println(dht20.getHumidity());
    temp = dht20.getTemperature();
    humid = dht20.getHumidity();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(temp);
    lcd.setCursor(0, 1);
    lcd.print("Humid:");
    lcd.print(humid);

    delay(5000);
  }
}

void TaskSoilMoistureAndRelay(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(D9, OUTPUT);

  while(1) {                          
    // Serial.println("Task Soild and Relay");
    
    moist = analogRead(A3);
    // Serial.println(moist);
    
    if(RELAY_AUTO_COUNT == 0){
      if(moist > 500){
        RELAY_STATUS = LOW;
      }
      if(moist < 50){
        RELAY_STATUS = HIGH;
      }
    }
    digitalWrite(D9, RELAY_STATUS);
    delay(1000);
  }
}

void TaskLightAndLED(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    // Serial.println("Task Light and LED");

    lightRes = analogRead(A2);
    // Serial.println(lightRes);

    if(lightRes < 350){
      RGB_STATUS = HIGH;
      pixels3.setPixelColor(0, pixels3.Color(255,0,0));
      pixels3.setPixelColor(1, pixels3.Color(255,0,0));
      pixels3.setPixelColor(2, pixels3.Color(255,0,0));
      pixels3.setPixelColor(3, pixels3.Color(255,0,0));
      pixels3.show();
    }
    if(lightRes > 550){
      RGB_STATUS = LOW;
      pixels3.setPixelColor(0, pixels3.Color(0,0,0));
      pixels3.setPixelColor(1, pixels3.Color(0,0,0));
      pixels3.setPixelColor(2, pixels3.Color(0,0,0));
      pixels3.setPixelColor(3, pixels3.Color(0,0,0));
      pixels3.show();
    }
    delay(1000);
  }
}
