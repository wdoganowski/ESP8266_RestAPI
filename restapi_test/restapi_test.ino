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

#define DEBUG_TO_SERIAL_PORT // When defined, the debug output is sent to serial at 74880 baud, otherwise the blue LED blinks on activity

#define SSIDNAME "ESP8266xxx"
#define PASSWORD ""
#define SSID_PASS_SIZE 32

char ssid[SSID_PASS_SIZE];
char password[SSID_PASS_SIZE];
IPAddress myIP;
MDNSResponder mdns;

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

void handleRoot() {
	DEBUGLED( 1, 0 );
	char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

		hr, min % 60, sec % 60
	);
	server.send ( 200, "text/html", temp );
	DEBUGLED( 0, 0 );
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

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
  DEBUGLN( message );
	DEBUGLED( LED_OFF, 0 );
}

int ReadSetup( char* ssid, char* password ) {
  int addr = 0;
  byte value;

  value = EEPROM.read(addr); addr++;
  if( value = 0xA5 ) {
    // We have the setup
    // Read SSID
    for( int i=0; i<SSID_PASS_SIZE; i++ ) {
      ssid[i] = EEPROM.read(addr);
      addr++;
    }
    // Read PASS
    for( int i=0; i<SSID_PASS_SIZE; i++ ) {
      password[i] = EEPROM.read(addr);
      addr++;
    }
    return 1;
  } else {
    // Default values for AP  
    strcpy( ssid, SSIDNAME );
    strcpy( password, PASSWORD );
    return 0;
  }
}

void setup ( void ) {
  delay(1000);
  
  // Init PIO
#ifndef DEBUG_TO_SERIAL_PORT
  pinMode ( 1, OUTPUT ); // When debug on serial defined, this is TX pin, no need to init here
#endif
  pinMode ( 2, OUTPUT );
  DEBUGLED( LED_OFF, 0 );

#ifdef DEBUG_TO_SERIAL_PORT
  Serial.begin ( 74880 );
  // Init Serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
#endif

  DEBUGLN( "" );
  DEBUGLN( "ESP8266 REST API 1.0" );
  
  // Init EEPROM
  EEPROM.begin(512);
  if( ReadSetup( ssid, password ) ) {
    
    DEBUGLN( "Setup Exisists for AP:" ); 
    DEBUGLN( ssid );

    // Init WiFi
    DEBUGLN( "Connecting to WiFi" );
    WiFi.begin ( ssid, password );

    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
      DEBUGLED( LED_ON, 250 );
      DEBUGLED( LED_OFF, 0 );
      delay( 500);
      DEBUG__( "." );
    }

    myIP = WiFi.localIP();
    
    DEBUGLN( "" );
    DEBUG__( "Connected to " );
    DEBUGLN( ssid );
    DEBUG__( "IP address: " );
    DEBUGLN( myIP );
  
  } else {

    DEBUGLN( "Staring in AP mode:" );
    DEBUGLN( ssid );
    WiFi.softAP( ssid, password ); 
 
    // Wait for setup
    while ( WiFi.status() != WL_CONNECTED ) {
      DEBUGLED( LED_ON, 250 );
      DEBUGLED( LED_OFF, 0 );
      delay( 500);
      DEBUG__( "." );
    }  

    myIP = WiFi.softAPIP();

    DEBUGLN( "" );
    DEBUG__( "Created AP " );
    DEBUGLN( ssid );
    DEBUG__( "IP address: " );
    DEBUGLN( myIP );   
  }

  // Init mDNS
	if ( mdns.begin ( ssid, myIP ) ) {
    DEBUGLED( LED_ON, 1000 );
    DEBUGLED( LED_OFF, 1000 );		
    DEBUGLN( "MDNS responder started" );
	}

  // Init HTTP Server
	server.on ( "/", handleRoot );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
  DEBUGLED( LED_ON, 500 );
  DEBUGLED( LED_OFF, 500 );   
  DEBUGLED( LED_ON, 1000 );
  DEBUGLED( LED_OFF, 1000 );   
	DEBUGLN( "HTTP server started" );
}

void loop ( void ) {
	mdns.update();
	server.handleClient();
}

