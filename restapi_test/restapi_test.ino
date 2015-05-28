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
 
#include <PgmSpace.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#include "root_html.h"

#define DEBUG_TO_SERIAL_PORT // When defined, the debug output is sent to serial at 74880 baud, otherwise the blue LED blinks on activity

#define SSIDNAME "ESP"
#define PASSWORD ""
#define SSID_PASS_SIZE 32
#define URL_SIZE 32
#define HASH_SIZE 20
#define DATA_HEADER 0xA5
#define DATA_VERSION 0x01
#define GPIO_SIZE 2

byte gpio_map[GPIO_SIZE] = {0, 2};

typedef struct SetupGPIOStruct {
  byte mode;
  byte value;
  char url_rising[URL_SIZE];
  char url_falling[URL_SIZE];
} setup_gpio_t;
  
typedef struct SetupStruct {
  char header;
  char version;
  byte start_counter;
  char ssid[SSID_PASS_SIZE];
  char password[SSID_PASS_SIZE];
  char dns_name[SSID_PASS_SIZE];
  byte ap_mode;
  setup_gpio_t gpio[GPIO_SIZE];
  byte hash[HASH_SIZE];
} setup_t;

setup_t setup_data;
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

/*
 * Web server
 */

void addContent( String *message, const char* content, ... ) {
  char buffer[256];
  va_list arglist;

  va_start( arglist, content );  
  snprintf_P( buffer, sizeof(buffer) - 1, content, arglist );
  //snprintf( buffer, sizeof(buffer) - 1, content, arglist );
  va_end( arglist );
  
  *message += buffer;

  DEBUGLN( *message );
  delay(1000);
}
 
void handleRoot() {
	DEBUGLED( LED_ON, 0 );
	
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;
  
  handleSetup();

  // Setup GPIO
  setupGPIO();

  // Build response
  String message;

  addContent( &message, root_html_header );
  addContent( &message, root_html_body, hr, min % 60, sec % 60 );
  addContent( &message, root_html_padding );
  
  addContent( &message, root_html_wifi_header );/*
    addContent( &message, root_html_wifi_ip, myIP[0], myIP[1], myIP[2], myIP[3] );
    addContent( &message, root_html_text, "SSID", "ssid", setup_data.ssid, SSID_PASS_SIZE - 1 );
    addContent( &message, root_html_text, "Password", "pass", setup_data.password, SSID_PASS_SIZE - 1 );
    addContent( &message, root_html_text, "DNS name", "dns", setup_data.dns_name, SSID_PASS_SIZE - 1 ); */
    addContent( &message, root_html_radio_header, "Start in AP mode" );
      addContent( &message, root_html_radio_input, "ap_mode", "yes", setup_data.ap_mode?"checked":"", "Yes" );
      addContent( &message, root_html_radio_input, "ap_mode", "no", setup_data.ap_mode?"":"checked", "No" );
    addContent( &message, root_html_radio_footer );
  addContent( &message, root_html_wifi_footer );

  for( byte gpio = 0; gpio < GPIO_SIZE; gpio++ ) {
    addContent( &message, root_html_gpio_header, gpio_map[gpio] );
    
    addContent( &message, root_html_radio_header, "Mode" );
    {
      char buff[32];
      sprintf( buff, "gpio%d_mode", gpio_map[gpio] );
      
      addContent( &message, root_html_radio_input, buff, INPUT, (setup_data.gpio[gpio].mode==INPUT)?"checked":"", "Input" );
      addContent( &message, root_html_radio_input, buff, INPUT_PULLUP, (setup_data.gpio[gpio].mode==INPUT_PULLUP)?"checked":"", "Input Pull-up" );
      addContent( &message, root_html_radio_input, buff, INPUT_PULLDOWN, (setup_data.gpio[gpio].mode==INPUT_PULLDOWN)?"checked":"", "Input Pull-down" );
      addContent( &message, root_html_radio_input, buff, OUTPUT, (setup_data.gpio[gpio].mode==OUTPUT)?"checked":"", "Output" );
    }
    addContent( &message, root_html_radio_footer );
    
    addContent( &message, root_html_radio_header, "Value" );
    {
      char buff[32];
      sprintf( buff, "gpio%d_value", gpio_map[gpio] );

      addContent( &message, root_html_radio_input, buff, LOW, (setup_data.gpio[gpio].value==LOW)?"checked":"", "Low" );
      addContent( &message, root_html_radio_input, buff, HIGH, (setup_data.gpio[gpio].value==HIGH)?"checked":"", "High" );
    }
    addContent( &message, root_html_radio_footer );
/*
    {
      char buff[32];
      
      sprintf( buff, "gpio%d_rising", gpio_map[gpio] );            
      addContent( &message, root_html_text, "URL on rising edge ", buff, setup_data.gpio[gpio].url_rising, URL_SIZE - 1 );
      sprintf( buff, "gpio%d_falling", gpio_map[gpio] );            
      addContent( &message, root_html_text, "URL on falling edge", buff, setup_data.gpio[gpio].url_falling, URL_SIZE - 1 );
    }
*/
    addContent( &message, root_html_gpio_footer );
  }

  addContent( &message, root_html_buttons_footer );

  addContent( &message, root_html_checkbox_input, "reset", "yes", "", "Reset to defaults" );
  addContent( &message, root_html_checkbox_input, "reboot", "yes", "", "Reboot after saving" );
  
  addContent( &message, root_html_footer );
  
	server.send ( 200, "text/html", message );

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
        setup_data.gpio[0].mode = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio0_value" ) {
        setup_data.gpio[0].value = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio0_rising" ) {
        strncpy( setup_data.gpio[0].url_rising, server.arg(i).c_str(), URL_SIZE );
        setup_data.gpio[0].url_rising[URL_SIZE-1] = 0;
      } else if ( server.argName(i) == "gpio0_falling" ) {
        strncpy( setup_data.gpio[0].url_falling, server.arg(i).c_str(), URL_SIZE );
        setup_data.gpio[0].url_falling[URL_SIZE-1] = 0;
      } 

      // GPIO2
      if ( server.argName(i) == "gpio2_mode" ) {
        setup_data.gpio[1].mode = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio2_value" ) {
        setup_data.gpio[1].value = server.arg(i).toInt();
      } else if ( server.argName(i) == "gpio2_rising" ) {
        strncpy( setup_data.gpio[1].url_rising, server.arg(i).c_str(), URL_SIZE );
        setup_data.gpio[1].url_rising[URL_SIZE-1] = 0;
      } else if ( server.argName(i) == "gpio2_falling" ) {
        strncpy( setup_data.gpio[1].url_falling, server.arg(i).c_str(), URL_SIZE );
        setup_data.gpio[1].url_falling[URL_SIZE-1] = 0;
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
      server.send ( 200, "text/html", root_html_rebooting );
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

  for( byte gpio = 0; gpio<GPIO_SIZE; gpio++ ) {
    setup_data.gpio[gpio].mode = 0;
    setup_data.gpio[gpio].value = 0;
    strcpy( setup_data.gpio[gpio].url_rising, "" );
    strcpy( setup_data.gpio[gpio].url_falling, "" );
  }
    
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
 * GPIO Setup
 */

/*
#define DEBOUNCE_DELAY 50

void isr_GPIO( int gpio, byte *value_ptr, byte *last_ptr, int *time_ptr ) {
  int reading = digitalRead( gpio );
  
  // If the switch changed, due to noise or pressing:
  if( reading != *last_ptr ) {
    // reset the debouncing timer
    *time_ptr = millis();
  }

  if( ( millis() - *time_ptr ) > DEBOUNCE_DELAY ) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if( reading != *value_ptr ) {
      *value_ptr = reading;

      // action herer
    }
  }
  
  // save the reading.  Next time through the loop,
  // it'll be the last_GPIO_state:
  *last_ptr = reading;  
  
  *value_ptr = reading;

}
*/

byte last_value[GPIO_SIZE];
bool call_gpio_rising_callback[GPIO_SIZE] = {0};
bool call_gpio_falling_callback[GPIO_SIZE] = {0};

// GPIO0 ISR

void (*isr_GPIO[GPIO_SIZE])() = {isr_GPIO0, isr_GPIO1};

void isr_GPIO0( void ) {
  const byte gpio = 0;  
  //isr_GPIO( 0, &setup_data.gpio0_value, &last_value_0, &last_time_0 );
  setup_data.gpio[gpio].value = digitalRead( gpio_map[gpio] );
  if( setup_data.gpio[gpio].value != last_value[gpio] ) {
    last_value[gpio] = setup_data.gpio[gpio].value;
    if( setup_data.gpio[gpio].value ) call_gpio_rising_callback[gpio] = 1;
    else call_gpio_falling_callback[gpio] = 1;
  }
}

// GPIO2 ISR

void isr_GPIO1( void ) {
  const byte gpio = 1;  
  //isr_GPIO( 0, &setup_data.gpio0_value, &last_value_0, &last_time_0 );
  setup_data.gpio[gpio].value = digitalRead( gpio_map[gpio] );
  if( setup_data.gpio[gpio].value != last_value[gpio] ) {
    last_value[gpio] = setup_data.gpio[gpio].value;
    if( setup_data.gpio[gpio].value ) call_gpio_rising_callback[gpio] = 1;
    else call_gpio_falling_callback[gpio] = 1;
  }
}

// Helpers

bool checkIfCallbackSet( byte gpio, bool rising ) {
  bool result = 0;
  switch( rising ) {
    case 0: {
      call_gpio_rising_callback[gpio];
      break;
    }
    case 1: {
      call_gpio_falling_callback[gpio];
      break;
    }
  }
  return result;
}

// Setup function

void setupGPIO( void ) {
  for( byte gpio=0; gpio<GPIO_SIZE; gpio++ ) {
    DEBUG__( "GPIO" );
    DEBUG__( gpio_map[gpio] );
    DEBUG__( " mode " );
    DEBUGLN( setup_data.gpio[gpio].mode );

    pinMode( gpio_map[gpio], setup_data.gpio[gpio].mode );
    if( setup_data.gpio[gpio].mode & OUTPUT ) {
      detachInterrupt( gpio_map[gpio] ); 
      digitalWrite( gpio_map[gpio], setup_data.gpio[gpio].value );
      DEBUG__( "GPIO" );
      DEBUG__( gpio_map[gpio] );
      DEBUG__( " set to " );
      DEBUGLN( setup_data.gpio[gpio].value );
    } else {
      last_value[gpio] = setup_data.gpio[gpio].value;
      attachInterrupt( gpio_map[gpio], isr_GPIO[gpio], CHANGE );
      DEBUG__( "Interrupt attached to GPIO" );
      DEBUGLN( gpio_map[gpio] );
    }
  }
}


/*
 * Setup
 */
 
void setup ( void ) {
  
  // Init PIO
#ifndef DEBUG_TO_SERIAL_PORT
  pinMode ( 1, OUTPUT ); // When debug on serial defined, this is TX pin, no need to init here
#else
  pinMode ( 2, OUTPUT );
#endif
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

  setupGPIO();

  // Everything OK
  StartCounter( 0 );
}

void loop ( void ) {
	if( setup_data.dns_name[0] ) mdns.update();
	server.handleClient();

  for( byte gpio=0; gpio<GPIO_SIZE; gpio++ ) {
    if( call_gpio_rising_callback[gpio] ) { DEBUG__( "GPIO RISING " ); DEBUG__( gpio_map[gpio] ); DEBUGLN( setup_data.gpio[gpio].url_rising ); }
    if( call_gpio_falling_callback[gpio] ) { DEBUG__( "GPIO FALLING " ); DEBUG__( gpio_map[gpio] ); DEBUGLN( setup_data.gpio[gpio].url_falling ); }
  }

}


