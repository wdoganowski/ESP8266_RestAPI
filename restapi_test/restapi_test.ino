/*
 * Copyright (c) 2015, Wojciech Doganowski
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SimpleTimer.h>
#include <Hash.h>

#include "root_html.h"

#define DEBUG_TO_SERIAL_PORT // When defined, the debug output is sent to serial at 74880 baud, otherwise the blue LED blinks on activity

#define SSIDNAME "ESP"
#define PASSWORD ""
#define SSID_PASS_SIZE 32
#define LOW_HIGH_SIZE 32
#define HASH_SIZE 20
#define DATA_HEADER 0xA5
#define DATA_VERSION 0x01

typedef struct SetupStruct {
  char header;
  char version;
  byte start_counter;
  char ssid[SSID_PASS_SIZE];
  char password[SSID_PASS_SIZE];
  char dns_name[SSID_PASS_SIZE];
  byte ap_mode;
  byte gpio0_mode;
  byte gpio0_value;
  char gpio0_low2high[LOW_HIGH_SIZE];
  char gpio0_high2low[LOW_HIGH_SIZE];
  byte gpio2_mode;
  byte gpio2_value;
  char gpio2_low2high[LOW_HIGH_SIZE];
  char gpio2_high2low[LOW_HIGH_SIZE];
  byte hash[HASH_SIZE];
} setup_t;

setup_t setup_data;
IPAddress myIP;
MDNSResponder mdns;

SimpleTimer timer;

ESP8266WebServer server ( 80 );

#define LED_ON  1 // For blue LED, ON is 0
#define LED_OFF 0

#ifdef DEBUG_TO_SERIAL_PORT
#define DEBUGLED( state, wait ) {digitalWrite ( 2, state ); delay ( wait );}
#define DEBUG__( message ) {Serial.print ( message );}
#define DEBUGLN( message ) {Serial.println ( message );}
#else
#define DEBUGLED( state, wait ) {digitalWrite ( 1, state?0:1 ); delay ( wait );}
#define DEBUG__( message ) {}
#define DEBUGLN( message ) {}
#endif

/*
 * Web server
 */
 
void handleRoot() {
	DEBUGLED( LED_ON, 0 );
	char temp[3000];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

  DEBUG__( "Size of page " );
  DEBUGLN( strlen( root_html ) );

  handleSetup();
  
	snprintf ( temp, 3000, root_html, 
	  hr, min % 60, sec % 60, 
	  myIP[0], myIP[1], myIP[2], myIP[3], 
	  setup_data.ssid, setup_data.password, setup_data.dns_name, 
	  setup_data.ap_mode?"checked":"", setup_data.ap_mode?"":"checked",
	  setup_data.gpio0_mode?"":"checked", setup_data.gpio0_mode?"checked":"",
	  setup_data.gpio0_value?"":"checked", setup_data.gpio0_value?"checked":"",
	  setup_data.gpio0_low2high, setup_data.gpio0_high2low,
    setup_data.gpio2_mode?"":"checked", setup_data.gpio2_mode?"checked":"",
    setup_data.gpio2_value?"":"checked", setup_data.gpio2_value?"checked":"",
    setup_data.gpio2_low2high, setup_data.gpio2_high2low
	  );
	server.send ( 200, "text/html", temp );

	DEBUGLED( LED_OFF, 0 );
}

void handleNotFound() {
	DEBUGLED( LED_ON, 0 );
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( byte i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
  DEBUGLN( message );
	DEBUGLED( LED_OFF, 0 );
}

/*
 * Handling setup parameters from URI
 */

void handleSetup() {
  DEBUGLED( LED_ON, 0 );
  int reboot = 0;
  int reset_data = 0;

  if( server.args() ) {

    DEBUG__( "Handle Setup with # of args " );
    DEBUGLN( server.args() );

    for ( byte i = 0; i < server.args(); i++ ) {
      DEBUGLN( server.argName(i) + ": " + server.arg(i) );

      // WiFi  
      if ( server.argName(i) == "ssid" ) {
        strncpy( setup_data.ssid, server.arg(i).c_str(), SSID_PASS_SIZE );
        setup_data.ssid[SSID_PASS_SIZE-1] = 0;
      } else if ( server.argName(i) == "pass" ) {
        strncpy( setup_data.password,  server.arg(i).c_str(), SSID_PASS_SIZE );
        setup_data.password[SSID_PASS_SIZE-1] = 0;
      } else if ( server.argName(i) == "dns" ) {
        strncpy( setup_data.dns_name,  server.arg(i).c_str(), SSID_PASS_SIZE );
        setup_data.dns_name[SSID_PASS_SIZE-1] = 0;
      } else if ( server.argName(i) == "ap_mode" ) {
        if ( server.arg(i) == "yes" ) {
          setup_data.ap_mode = 1;
        } else {
          setup_data.ap_mode = 0;
        }
      } 

      // GPIO0
      if ( server.argName(i) == "gpio0_mode" ) {
        setup_data.gpio0_mode = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio0_value" ) {
        setup_data.gpio0_value = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio0_low2high" ) {
        strncpy( setup_data.gpio0_low2high, server.arg(i).c_str(), LOW_HIGH_SIZE );
        setup_data.gpio0_low2high[LOW_HIGH_SIZE-1] = 0;
      } else if ( server.argName(i) == "gpio0_high2low" ) {
        strncpy( setup_data.gpio0_high2low, server.arg(i).c_str(), LOW_HIGH_SIZE );
        setup_data.gpio0_high2low[LOW_HIGH_SIZE-1] = 0;
      } 

      // GPIO2
      if ( server.argName(i) == "gpio2_mode" ) {
        setup_data.gpio2_mode = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio2_value" ) {
        setup_data.gpio2_value = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio2_low2high" ) {
        strncpy( setup_data.gpio2_low2high, server.arg(i).c_str(), LOW_HIGH_SIZE );
        setup_data.gpio2_low2high[LOW_HIGH_SIZE-1] = 0;
      } else if ( server.argName(i) == "gpio2_high2low" ) {
        strncpy( setup_data.gpio2_high2low, server.arg(i).c_str(), LOW_HIGH_SIZE );
        setup_data.gpio2_high2low[LOW_HIGH_SIZE-1] = 0;
      }   

      // Reboot & Reset
      if ( server.argName(i) == "reset" ) {
        if ( server.arg(i) == "yes" ) {
          reset_data = 1;
        }
      } else if ( server.argName(i) == "reboot" ) {
        if ( server.arg(i) == "yes" ) {
          reboot = 1;
        }
      } 
    } 

    if( reset_data ) ResetSetup();
    else WriteSetup();
  
    if (reboot) {
      server.send ( 200, "text/html", 
       "<html><head><meta http-equiv='refresh' content='15'/><title>ESP8266 PIO Bridge</title>\
        <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\
        </head>\
        <body><h1>REBOOTING...</h1></body>\
        </html>" 
      );
      // reset here
      ESP.restart();
    }
  
  }
  
  DEBUGLED( LED_OFF, 0 );
}

/*
 * EEPROM Reset/Reading/Writing
 */

void ResetSetup() {
  DEBUGLED( LED_ON, 0 );

  DEBUGLN( "Reset EEPROM " );

  // Default values for AP
  setup_data.header = DATA_HEADER;
  setup_data.version = DATA_VERSION;
  setup_data.start_counter = 0; 
  byte mac[6];
  WiFi.macAddress(mac); // Read MAC
  sprintf( setup_data.ssid, "%s_%X%X%X", SSIDNAME, mac[3], mac[4], mac[5] );
  strcpy( setup_data.password, PASSWORD );
  strcpy( setup_data.dns_name, setup_data.ssid );
  setup_data.ap_mode = 1;
  
  setup_data.gpio0_mode = 0;
  setup_data.gpio0_value = 0;
  strcpy( setup_data.gpio0_low2high, "" );
  strcpy( setup_data.gpio0_high2low, "" );
  
  setup_data.gpio2_mode = 0;
  setup_data.gpio2_value = 0;
  strcpy( setup_data.gpio2_low2high, "" );
  strcpy( setup_data.gpio2_high2low, "" );
  
  WriteSetup();
  
  DEBUGLED( LED_OFF, 0 );
}
 
void ReadSetup() {
  DEBUGLED( LED_ON, 0 );

  DEBUG__( "Reading EEPROM (" );
  DEBUG__( sizeof(setup_t) );
  DEBUGLN( " bytes) " );
 
  memcpy( &setup_data, EEPROM.getDataPtr(), sizeof(setup_t) );
  DEBUGLN( "" );

  byte hash[HASH_SIZE];
  sha1( (byte*)&setup_data, sizeof(setup_t) - HASH_SIZE, (byte*)&hash[0] );
  
  if( setup_data.header == DATA_HEADER && setup_data.version == DATA_VERSION 
      && memcmp( hash, setup_data.hash, HASH_SIZE ) == 0 ) {
    // We have the setup
  } else {
    // Default values for AP  
    ResetSetup();
  }
  
  DEBUGLED( LED_OFF, 0 );
}

void EEPROM_Update( int address, uint8_t value ) {
  if ( EEPROM.read(address) != value ) {
    EEPROM.write(address, value);
  }
}

void WriteSetup() {
  DEBUGLED( LED_ON, 0 );
  
  char *ptr = (char *) &setup_data;

  DEBUG__( "Writing EEPROM (" );
  DEBUG__( sizeof(setup_t) );
  DEBUGLN( " bytes) " );

  sha1( ptr, sizeof(setup_t) - HASH_SIZE, &setup_data.hash[0] );
  
  for( int addr=0; addr<sizeof(setup_t); addr++ ) {
    EEPROM_Update( addr, *(ptr+addr) );
  }
  DEBUGLN( "" );
  DEBUG__( "EEPROM Commit " );  
  while( !EEPROM.commit() ) {
    DEBUG__( "." );
    DEBUGLED( LED_OFF, 100 );    
    DEBUGLED( LED_ON, 100 ); 
    DEBUGLED( LED_OFF, 0 );   
    delay( 500 );
    DEBUGLED( LED_ON, 0 );
  } 
  DEBUGLN( " Done" );  
  delay(100);
  
  DEBUGLED( LED_OFF, 0 );
}

void StartCounter( byte counter ) {
  setup_data.start_counter = counter;
  EEPROM.write(1, counter);
  EEPROM.commit();
}

/*
 * Setup
 */
 
void setup ( void ) {
  
  // Init PIO
#ifndef DEBUG_TO_SERIAL_PORT
  pinMode ( 1, OUTPUT ); // When debug on serial defined, this is TX pin, no need to init here
#endif
  pinMode ( 2, OUTPUT );
  DEBUGLED( LED_OFF, 0 );

#ifdef DEBUG_TO_SERIAL_PORT
  delay(5000);
  Serial.begin ( 74880 );
  // Init Serial
  while (!Serial) {
    delay( 1 ); // wait for serial port to connect. Needed for Leonardo only
  }
#endif

  DEBUGLN( "" );
  DEBUGLN( "ESP8266 REST API 1.0" );
  
  // Init EEPROM
  EEPROM.begin( sizeof( setup_t ) );
  ReadSetup();
  if( setup_data.start_counter > 3 ) {
    DEBUGLN( "*** RESET SETUP ***" );
    for( int i=100; i<1000; i+=100 ) {
      DEBUGLED( LED_ON, i );
      DEBUGLED( LED_OFF, i );
    }
    ResetSetup();
  } else {
    StartCounter( setup_data.start_counter + 1 );
  }
  
  if( setup_data.ap_mode )
  {    
   
    DEBUGLN( "Staring in AP mode:" );
    DEBUGLN( setup_data.ssid );
    WiFi.softAP( setup_data.ssid ); 
 
    // Wait for setup
    /*while ( WiFi.status() != WL_CONNECTED ) {
      DEBUGLED( LED_ON, 250 );
      DEBUGLED( LED_OFF, 0 );
      delay( 500 );
      DEBUG__( "." );
    } */ 

    myIP = WiFi.softAPIP();

    DEBUGLN( "" );
    DEBUG__( "Created AP " );
    DEBUGLN( setup_data.ssid );
    DEBUG__( "IP address: " );
    DEBUGLN( myIP );   

  } else {

    DEBUG__( "Connection to SSID: " ); 
    DEBUGLN( setup_data.ssid );

    // Init WiFi
    WiFi.begin ( setup_data.ssid, setup_data.password );

    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
      DEBUGLED( LED_ON, 250 );
      DEBUGLED( LED_OFF, 0 );
      delay( 500 );
      DEBUG__( "." );
    }

    myIP = WiFi.localIP();
    
    DEBUGLN( "" );
    DEBUG__( "Connected to " );
    DEBUGLN( setup_data.ssid );
    DEBUG__( "IP address: " );
    DEBUGLN( myIP );
  
  }

#ifdef DEBUG_TO_SERIAL_PORT
  DEBUGLN( "==========" );
  WiFi.printDiag(Serial);
  DEBUGLN( "==========" );
#endif

  // Init mDNS
	if ( setup_data.dns_name[0] && mdns.begin( setup_data.dns_name, myIP ) ) {
    DEBUGLED( LED_ON, 1000 );
    DEBUGLED( LED_OFF, 1000 );		
    DEBUGLN( "MDNS responder started" );
	}

  // Init HTTP Server
	server.on ( "/", handleRoot );
	server.onNotFound ( handleNotFound );
	server.begin();
  DEBUGLED( LED_ON, 500 );
  DEBUGLED( LED_OFF, 500 );   
  DEBUGLED( LED_ON, 1000 );
  DEBUGLED( LED_OFF, 1000 );   
	DEBUGLN( "HTTP server started" );

  pinMode( 0, setup_data.gpio0_mode?INPUT:OUTPUT );
  pinMode( 2, setup_data.gpio2_mode?INPUT:OUTPUT );
  if( setup_data.gpio0_mode == OUTPUT ) digitalWrite( 0, setup_data.gpio0_value );
  if( setup_data.gpio2_mode == OUTPUT ) digitalWrite( 0, setup_data.gpio2_value );

  // Everything OK
  StartCounter( 0 );

  timer.setInterval(1000, repeatMe);
}

void loop ( void ) {
	if( setup_data.dns_name[0] ) mdns.update();
	server.handleClient();
/*
  if( setup_data.gpio0_mode == 0 ) {
    // Input
    setup_data.gpio0_value = digitalRead( 0 );
    DEBUG__( "GPIO0 read " );
    DEBUGLN( setup_data.gpio0_value );
  } else {
    // Output
    digitalWrite( 0, setup_data.gpio0_value );
    DEBUG__( "GPIO0 write " );
    DEBUGLN( setup_data.gpio0_value );
  }

  if( setup_data.gpio2_mode == 0 ) {
    // Input
    setup_data.gpio2_value = digitalRead( 2 );
    DEBUG__( "GPIO2 read " );
    DEBUGLN( setup_data.gpio2_value );
  } else {
    // Output
    digitalWrite( 2, setup_data.gpio2_value );
    DEBUG__( "GPIO2 write " );
    DEBUGLN( setup_data.gpio2_value );
  }
*/

  timer.run();
}

void repeatMe() {
    Serial.print("Uptime (s): ");
    Serial.print(millis() / 1000);
    Serial.print(" Heap free: ");
    Serial.println(ESP.getFreeHeap());
}

