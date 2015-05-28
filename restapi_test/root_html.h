static const char root_html[] = "\
<html>\
  <head>\
    <title>ESP8266 PIO Bridge</title>\
    <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\
  </head>\
  <body>\
    <div style='background-color:lightgrey; padding:20px;'>\
    <h1>ESP8266 PIO Bridge</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    </div>\
\
    <div padding:20px><br></div>\
    <form method='post'>\
\
    <div style='background-color:lightgrey; padding:20px;'>\
    <h2>Current WiFi Settings</h2>\
    <p>IP  : %d.%d.%d.%d</p>\
    <p style='font-size:80%'>SSID:<br><input type='text' name='ssid' value='%s' size='31'></p>\
    <p style='font-size:80%'>Password:<br><input type='text' name='pass' value='%s' size='31'></p>\
    <p style='font-size:80%'>DNS Name:<br><input type='text' name='dns' value='%s' size='31'></p>\
    <p style='font-size:80%'>Start in AP mode\
    <input type='radio' name='ap_mode' value='yes' %s>Yes\
    <input type='radio' name='ap_mode' value='no' %s>No\
    </p>\
    </div>\
\
    <div padding:20px><br></div>\
\
    <div style='background-color:lightgrey; padding:20px;'>\
    <h2>Select function of GPIO<b>0</b></h2>\
    <p style='font-size:80%'>Mode\
    <input type='radio' name='gpio0_mode' value='%d' %s>Input\
    <input type='radio' name='gpio0_mode' value='%d' %s>Input Pull-up\
    <input type='radio' name='gpio0_mode' value='%d' %s>Input Pull-down\
    <input type='radio' name='gpio0_mode' value='%d' %s>Output\
    </p>\
    <p style='font-size:80%'>Value\
    <input type='radio' name='gpio0_value' value='0' %s>0\
    <input type='radio' name='gpio0_value' value='1' %s>1\
    </p>\
    <p style='font-size:80%'>URL on low/high transition<br><input type='text' name='gpio0_low2high' value='%s' size='63'></p>\
    <p style='font-size:80%'>URL on high/low transition<br><input type='text' name='gpio0_high2low' value='%s' size='63'></p>\
    </div>\
\
    <div padding:20px><br></div>\
\
    <div style='background-color:lightgrey; padding:20px;'>\
    <h2>Select function of GPIO<b>2</b></h2>\
    <p style='font-size:80%'>Mode\
    <input type='radio' name='gpio2_mode' value='%d' %s>Input\
    <input type='radio' name='gpio2_mode' value='%d' %s>Input Pull-up\
    <input type='radio' name='gpio2_mode' value='%d' %s>Input Pull-down\
    <input type='radio' name='gpio2_mode' value='%d' %s>Output\
    </p>\
    <p style='font-size:80%'>Value\
    <input type='radio' name='gpio2_value' value='0' %s>0\
    <input type='radio' name='gpio2_value' value='1' %s>1\
    </p>\
    <p style='font-size:80%'>URL on low/high transition<br><input type='text' name='gpio2_low2high' value='%s' size='63'></p>\
    <p style='font-size:80%'>URL on high/low transition<br><input type='text' name='gpio2_high2low' value='%s' size='63'></p>\
    </div>\
\
    <div padding:20px><br></div>\
\
    <div style='background-color:lightgrey; padding:20px;'>\
    <p style='font-size:80%'><input type='submit' value='Save'><input type='reset' value='Reset'><br>\
    <input type='checkbox' name='reset' value='yes'>Reset to defaults<br>\
    <input type='checkbox' name='reboot' value='yes'>Reboot after saving<br></p>\
    </div>\
\
  </form>\
\
</body>\
</html>\
";

