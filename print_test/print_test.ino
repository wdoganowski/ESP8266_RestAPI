void setup() {
  char buf[2];
    Serial.begin ( 74880 );
    delay(5000);
  // put your setup code here, to run once:
  for(byte i=0; i<256;i++) {
    sprintf(buf, "%c", i);      
    Serial.print ( buf[0], HEX );
    Serial.print ( buf[0] );
    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
