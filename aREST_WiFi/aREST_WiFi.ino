// Relay control using the ESP8266 WiFi chip

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <aREST_UI.h>
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT21

// Create aREST instance
aREST_UI rest = aREST_UI();

// WiFi parameters
char APssid[] = "ESP_ABCDEF";
//const char* default_ssid = "wifi";
//const char* default_password = "haslo4wifi123";

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables
String ssid = "wifi";
String password = "haslo4wifi123";
int temperature = 23;

DHT dht( DHTPIN, DHTTYPE );

void setup(void)
{  
  // Start Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Setup variables
  rest.variable("temperature",&temperature);
  rest.variable("ssid", &ssid);
  
  // Setup functions
  rest.function("setup_gpio0_l2h",setup_gpio0_l2h);
  rest.function("set_ssid",set_ssid);
  
  // Create UI
  rest.title("Relay Control");
  rest.button(2);
  rest.slider(2);
  rest.label("temperature");
  rest.label("ssid");
    
  // Give name and ID to device
  byte mac[6];
  WiFi.macAddress( mac ); // Read MAC
  sprintf( APssid, "ESP_%X%X%X", mac[3], mac[4], mac[5] );
  rest.set_id( APssid );
  rest.set_name( "ESP8266" );
  
  // Connect to WiFi
  WiFi.mode( WIFI_STA );
  //WiFi.softAP( APssid );
  WiFi.begin( ssid.c_str(), password.c_str() );
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println( "Server started" );
  
  // Print the IP address
  printWifiStatus();
  
}

void loop() {
  
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

// WifiStatus
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Local IP Address: ");
  Serial.println(ip);

  ip = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(ip);
  
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Custom function accessible by the API
int set_ssid(String str ) {
  Serial.println( str );
  ssid = str;
  return 1;
}
int setup_gpio0_l2h(String url ) {
  Serial.println( url );
  return 1;
}
