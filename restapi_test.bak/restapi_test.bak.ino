/*
 * Copyright (c) 2015, Majenko Technologies
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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "ADB-LAB419-X-TEAM";
const char *password = "4melinium";
MDNSResponder mdns;

ESP8266WebServer server ( 80 );

const int led = 2;

const char *log_message = \
"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Log</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Log</h1>\
    <p>%s</p>\
  </body>\
</html>"; 

#define LOG_MSG_LENGTH 400
#define LOG_MAX_LENGTH (1024-LOG_MSG_LENGTH)
char log_buffer[LOG_MAX_LENGTH] = "log test\n";
int  log_length = 9;

void addLog( const char *msg ) {
  size_t len = strlen( msg );
  Serial.println ( "Log message: " );
  Serial.println ( msg );
  Serial.print ( "Msg length: " );
  Serial.println ( len );
  Serial.print ( "Log length: " );
  Serial.println ( log_length );
    
  if( len > LOG_MAX_LENGTH - 1 ) {
    Serial.println ( "Log message longer than the buffer" );
    return;
  }

  if( log_length + len > LOG_MAX_LENGTH - 1 ) {
    Serial.println ( "Trunkating:" );
    Serial.println ( log_buffer );
    strcpy( log_buffer, log_buffer + len );
    log_length -= len;
    Serial.println ( "After:" );
    Serial.println ( log_buffer );  
  }
  
  strcat( log_buffer, msg );
  strcat( log_buffer, "<br>" );
  log_length += len;
  Serial.print ( "New Log length: " );
  Serial.println ( log_length );
  Serial.println ( "New log:" );
  Serial.println ( log_buffer );
}

void displayLog() {
  digitalWrite ( led, 1 );

  char temp[LOG_MAX_LENGTH + LOG_MSG_LENGTH];
  snprintf ( temp, LOG_MAX_LENGTH + LOG_MSG_LENGTH, log_message, log_buffer );
  server.send ( 200, "text/html", temp );
  digitalWrite ( led, 0 );  
}

void handleRoot() {
	digitalWrite ( led, 1 );
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
    <img src=\"/test.svg\" />\
  </body>\
</html>",

		hr, min % 60, sec % 60
	);
	server.send ( 200, "text/html", temp );
	digitalWrite ( led, 0 );
}

void handleNotFound() {
	digitalWrite ( led, 1 );
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
	digitalWrite ( led, 0 );
}

void setup ( void ) {
  //pinMode ( 0, INPUT );
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );
	Serial.begin ( 74880 );
	WiFi.begin ( ssid, password );
	Serial.println ( "" );

  // Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
    digitalWrite ( led, 1 );
    delay ( 50 );
    digitalWrite ( led, 0 );
    delay ( 500 );
		Serial.print ( "." );
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
	server.on ( "/test.svg", drawGraph );
  server.on ( "/log", displayLog );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
}

int counter = 1;

void loop ( void ) {
  char buf[100];
  sprintf( buf, "This is a log message nr %d\n", counter );
	addLog( buf );
  counter += 1;
  
	mdns.update();
	server.handleClient();
}

void drawGraph() {
	String out = "";
	char temp[100];
	out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
 	out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
 	out += "<g stroke=\"black\">\n";
 	int y = rand() % 130;
 	for (int x = 10; x < 390; x+= 10) {
 		int y2 = rand() % 130;
 		sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
 		out += temp;
 		y = y2;
 	}
	out += "</g>\n</svg>\n";

	server.send ( 200, "image/svg+xml", out);
}
