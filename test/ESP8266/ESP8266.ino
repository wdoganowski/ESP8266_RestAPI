/* 
  This a simple example of the aREST Library for the ESP8266 WiFi chip. 
  See the README file for more details.
 
  Written in 2015 by Marco Schwartz under a GPL license. 
*/

//#define DEBUG_MODE 1
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial 

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

char auth[] = "01cc9c7f38d342e4bcb6e4273eae5e97";

SimpleTimer timer;

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "ADB-LAB419-X-TEAM";
const char* password = "4melinium";

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int temperature;
int humidity;

int input_pin = 0;
int output_pin = 2;

void setup(void)
{  
  // Start Serial
  Serial.begin( 115200 );

  pinMode( input_pin, INPUT_PULLUP );
  pinMode( output_pin, OUTPUT );

  Serial.println("Starting");
          
          // Init variables and expose them to REST API
          temperature = 24;
          humidity = 40;
          rest.variable("temperature",&temperature);
          rest.variable("humidity",&humidity);

          // Function to be exposed
          //rest.function("led",ledControl);
  
  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");
  
  // Connect to WiFi
  /*WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");*/

  // Setup Blynk
  Blynk.begin(auth, ssid, password);
 
  // Start the server
  server.begin();
  Serial.println("Server started");

  timer.setInterval(1000, sendUptime);
  
  // Print the IP address
  Serial.println(Blynk.localIP());
}

int v2 = 0;

void sendUptime()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  BLYNK_LOG("Push a value: %s", v2);
  Blynk.virtualWrite(5, v2);
}

BLYNK_WRITE(2)
{
  BLYNK_LOG("Got a value: %s", param.asStr());
  Blynk.push_notification("some text");
  v2 = param.asInt();
  Blynk.virtualWrite(5, v2);
  // You can also use: asInt() and asDouble()
}

void loop() {
  
  // Handle Blynk
  Blynk.run();
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

