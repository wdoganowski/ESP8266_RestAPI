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

#define FS(x) (__FlashStringHelper*)(x)

const PROGMEM prog_char root_html_header[] = {"<html><head><title>ESP8266 PIO Bridge</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head>"};

/*
 * %d H, %d M, %d s
 */
const PROGMEM prog_char root_html_body[] = {"<body><div style='background-color:lightgrey; padding:20px;'><h1>ESP8266 PIO Bridge</h1><p>Uptime: %02d:%02d:%02d</p></div><form method='post'>"};

const PROGMEM prog_char root_html_padding[] = {"<div padding:20px><br></div>"};

const PROGMEM prog_char root_html_wifi_header[] = {"<div style='background-color:lightgrey; padding:20px;'><h2>Current WiFi Settings</h2>"};
const PROGMEM prog_char root_html_wifi_footer[] = {"</div>"};

/*
 * IP display
 * %d.%d.%d.%d
 */
const PROGMEM prog_char root_html_wifi_ip[] = {"<p>IP  : %d.%d.%d.%d</p>"};

/*
 * Text input
 * %s label, %s name, %s value, %d size
 */
const PROGMEM prog_char root_html_text[] = {"<p style='font-size:80%'>%s:<br><input type='text' name='%s' value='%s' size='%d'></p>"};

/*
 * Radio header
 * %s label
 */
const PROGMEM prog_char root_html_radio_header[] = {"<p style='font-size:80%'>%s"};

/*
 * Radio input
 * %s name, %s value, %s checked, %s label
 */
const PROGMEM prog_char root_html_radio_input[] = {"<input type='radio' name='%s' value='%s' %s>%s"};

/*
 * Checkbox input
 * %s name, %s value, %s checked, %s label
 */
const PROGMEM prog_char root_html_checkbox_input[] = {"<input type='checkbox' name='%s' value='%s' %s>%s"};

/*
 * Radio footer
 */
const PROGMEM prog_char root_html_radio_footer[] = {"</p>"};

/*
 * GPIO header
 * %d gpio number
 */
const PROGMEM prog_char root_html_gpio_header[] = {"<div style='background-color:lightgrey; padding:20px;'><h2>Select function of GPIO<b>%d</b></h2>"};
const PROGMEM prog_char root_html_gpio_footer[] = {"</div>"};

const PROGMEM prog_char root_html_buttons_header[] = {"<div style='background-color:lightgrey; padding:20px;'><p style='font-size:80%'><input type='submit' value='Save'><input type='reset' value='Reset'><br>"};
const PROGMEM prog_char root_html_buttons_footer[] = {"</div"};

const PROGMEM prog_char root_html_footer[] = {"</form></body></html>"};

const PROGMEM prog_char root_html_rebooting[] = {"<html><head><meta http-equiv='refresh' content='15'/><title>ESP8266 PIO Bridge</title>\
<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\
</head><body><h1>REBOOTING...</h1></body></html>"}; 

