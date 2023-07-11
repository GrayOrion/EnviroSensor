/*
 * Display test & discovery progam for MakeItZone's Environmental Sensors group.
 * Target platform is the Heltec ESP32 LoRa v2
*/

// Bring in pre-created pieces code that does a bunch of work for us (libraries)
#include <Wire.h> // i2c
#include <Adafruit_GFX.h> // generic (abstract) drawing functions
#include <Adafruit_SSD1306.h> // interface for our specific screen

#include <Adafruit_Sensor.h> // abstract functions for sensors
#include <Adafruit_BME280.h> // specific code to access the BME280

// MQTT
#include <PubSubClient.h>

// From WifiManager
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

// Timer constant
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = LED_BUILTIN;

// MQTT server (Broker) address ( TODO: currently hardcoded, to be fixed)
const char* mqtt_server = "192.168.1.30";
// Other MQTT stuff - TODO: ugly, to be fixed 
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int counterValue = 0; //TODO: ?!?! Fix this uglyness


// OLED Screen Constants
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define SCREEN_I2C_SDA 4
#define SCREEN_I2C_SCL 15
#define SCREEN_RESET 16
#define VEXT 21
#define SCREEN_ADDRESS 0x3C

// create instances of i2c interface and SSD1306 objects
TwoWire SCREENI2C = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SCREENI2C, SCREEN_RESET);

// Constaants for the BME280 Sensor
#define SENSOR_I2C_SDA 17
#define SENSOR_I2C_SCL 22
TwoWire SENSORI2C = TwoWire(1); // second, seperate, I2C bus

// create a global variable to hold our BME280 interface object
Adafruit_BME280 bme;



// From WifiManager
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;

// Stores LED state
String ledState;

/*
 * ===============================================================
 * End of global declarations\
*/


void setup() 
{ // put your setup code here, to run once:
  // Setup a serial port so we can send (and recieve) text to a monitoring computer.
  // This is a "virtual" serial port that is sent via USB.
  Serial.begin(9600);
  Serial.println("Starting up...");
  
  // Set an OUTPUT led
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Setup the screen
  setupScreen();

  // Setup the sensor
  setupSensor();

  // initialize file system (for retrival of wifi info)
  initSPIFFS();

  // initialize wifi connection, or get connection values if fails
  connectWifi();

  // Initialize random number generator for MQTT id
  randomSeed(micros());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() 
{
  // put your main code here, to run repeatedly:
  
  //display.
  // Output a message to the programmers computer so we know the code is running
  // Serial.println("run...");

  // get and display the current temperature and humidity
  tempAndHumidityReading();

  // wait a second before doing it all again
  delay(1000);

  // MQTT portion TODO: refactor
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) 
  {
    lastMsg = now;
    ++counterValue;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", counterValue);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}


void setupScreen()
{
  // Turn on the screen
  // On the Heltec ESP32 LoRa v2 the power for the display can be turned off.
  pinMode(VEXT,OUTPUT);
	digitalWrite(VEXT, LOW);
  // and give a slight delay for it to initialize
  delay(100);

  // start communications with the screen
  SCREENI2C.begin(SCREEN_I2C_SDA, SCREEN_I2C_SCL, 100000);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setupSensor()
{
  // setup hardware pins for use to communicate to the sensor
  pinMode(SENSOR_I2C_SCL, OUTPUT);
  digitalWrite(SENSOR_I2C_SCL, HIGH);
  pinMode(SENSOR_I2C_SDA, OUTPUT);
  digitalWrite(SENSOR_I2C_SDA, HIGH);

  // initialize the sensor
  Serial.println("Starting BME280...");
  display.println("Starting BME280...");
  display.display();
  SENSORI2C.begin(SENSOR_I2C_SDA, SENSOR_I2C_SCL, 400000);
  bool status = bme.begin(0x76, &SENSORI2C);  

  // Check that we can communicate with the sensor.
  // If we can't, output a message and go no further.
  // This kind of error handling is important.
  // Can you find a similar example elsewhere in the code?
  if (!status) 
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    display.println("No BME- check wiring!");
    display.display();
    while (1); // same as "while (true)" - i.e. do nothing more, forever
  }
}


// and here is the code for the tempAndHumidityReading function
void tempAndHumidityReading()
{
  display.clearDisplay();
  // display temperature
  display.setTextSize(2);
  // the x (horizontal) and y (vertical) location for the next
  // thing to be put on the screen
  display.setCursor(0,0);
  display.print("Temp:");
  display.setTextSize(2);
  display.setCursor(10,17);
  display.print(String(bme.readTemperature()));
  display.print(" ");
  display.setTextSize(1);
  // the next two lines display a degree symbol
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 37);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.print(String(bme.readHumidity()));
  display.print(" %"); 
  
  display.display();  
}


////////////////// SPIFFS File System ////////////////////////////

// Initialize SPIFFS
void initSPIFFS() 
{
  if (!SPIFFS.begin(true)) 
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message))
  {
    Serial.println("- file written");
  } 
  else 
  {
    Serial.println("- write failed");
  }
}


//////////////////// WIFI ////////////////////

void connectWifi()
{
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile (SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi())
  {
    // wifi works fine, now serve web server:
    serveLedChangerPage();
  }
  else
  {
    // wifi does not work, create a web page to ask for wifi login:
    serveNetworkCredentialForm();
  }
    
}

// Initialize WiFi
bool initWiFi() 
{
  if(ssid=="" || ip=="")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
  {
    Serial.println("STA Failed to configure");
    return false;
  }
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) 
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) 
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

////////////////////// Web Forms ////////////////////////////

void serveLedChangerPage()
{ 
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  server.serveStatic("/", SPIFFS, "/");
  
  // Route to set GPIO state to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    digitalWrite(ledPin, HIGH);
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  // Route to set GPIO state to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    digitalWrite(ledPin, LOW);
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  server.begin();
}

void serveNetworkCredentialForm()
{ // wifi does not work, create a web page to ask for wifi login:
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP); 

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/wifimanager.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");
  
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) 
  {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == PARAM_INPUT_1) {
          ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          // Write file to save value
          writeFile(SPIFFS, ssidPath, ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == PARAM_INPUT_2) {
          pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          // Write file to save value
          writeFile(SPIFFS, passPath, pass.c_str());
        }
        // HTTP POST ip value
        if (p->name() == PARAM_INPUT_3) {
          ip = p->value().c_str();
          Serial.print("IP Address set to: ");
          Serial.println(ip);
          // Write file to save value
          writeFile(SPIFFS, ipPath, ip.c_str());
        }
        // HTTP POST gateway value
        if (p->name() == PARAM_INPUT_4) {
          gateway = p->value().c_str();
          Serial.print("Gateway set to: ");
          Serial.println(gateway);
          // Write file to save value
          writeFile(SPIFFS, gatewayPath, gateway.c_str());
        }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    delay(3000);
    ESP.restart();
  });
  server.begin();
}


// Replaces placeholder with LED state value
String processor(const String& var) 
{
  if(var == "STATE") 
  {
    if(digitalRead(ledPin)) 
    {
      ledState = "ON";
    }
    else 
    {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}


////////////////////////////////  MQTT Stuff  /////////////////////////////////////


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    //digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  //while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
 // }
}
