
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM
#endif

const PROGMEM prog_char root_html_header[] = "<html><head><title>ESP8266 PIO Bridge</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head>";

/*
 * %d H, %d M, %d s
 */
const PROGMEM prog_char root_html_body[] = "<body><div style='background-color:lightgrey; padding:20px;'><h1>ESP8266 PIO Bridge</h1><p>Uptime: %02d:%02d:%02d</p></div><form method='post'>";

const PROGMEM prog_char root_html_padding[] = "<div padding:20px><br></div>";

const PROGMEM prog_char root_html_wifi_header[] = "<div style='background-color:lightgrey; padding:20px;'><h2>Current WiFi Settings</h2>";
const PROGMEM prog_char root_html_wifi_footer[] = "</div>";

/*
 * IP display
 * %d.%d.%d.%d
 */
const PROGMEM prog_char root_html_wifi_ip[] = "<p>IP  : %d.%d.%d.%d</p>";

/*
 * Text input
 * %s label, %s name, %s value, %d size
 */
const PROGMEM prog_char root_html_text[] = "<p style='font-size:80%'>%s:<br><input type='text' name='%s' value='%s' size='%d'></p>";

/*
 * Radio header
 * %s label
 */
const PROGMEM prog_char root_html_radio_header[] = "<p style='font-size:80%'>%s";

/*
 * Radio input
 * %s name, %s value, %s checked, %s label
 */
const PROGMEM prog_char root_html_radio_input[] = "<input type='radio' name='%s' value='%s' %s>%s";

/*
 * Checkbox input
 * %s name, %s value, %s checked, %s label
 */
const PROGMEM prog_char root_html_checkbox_input[] = "<input type='checkbox' name='%s' value='%s' %s>%s";

/*
 * Radio footer
 */
const PROGMEM prog_char root_html_radio_footer[] = "</p>";

/*
 * GPIO header
 * %d gpio number
 */
const PROGMEM prog_char root_html_gpio_header[] = "<div style='background-color:lightgrey; padding:20px;'><h2>Select function of GPIO<b>%d</b></h2>";
const PROGMEM prog_char root_html_gpio_footer[] = "</div>";

const PROGMEM prog_char root_html_buttons_header[] = "<div style='background-color:lightgrey; padding:20px;'><p style='font-size:80%'><input type='submit' value='Save'><input type='reset' value='Reset'><br>";
const PROGMEM prog_char root_html_buttons_footer[] = "</div";

const PROGMEM prog_char root_html_footer[] = "</form></body></html>";

const PROGMEM prog_char root_html_rebooting[] = "<html><head><meta http-equiv='refresh' content='15'/><title>ESP8266 PIO Bridge</title>\
<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\
</head><body><h1>REBOOTING...</h1></body></html>"; 

