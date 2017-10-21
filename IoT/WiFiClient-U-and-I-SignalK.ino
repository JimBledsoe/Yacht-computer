/*
 *  This sketch uses a ESP 8266 and ADS1115 to measure voltage and current and 
 *  sends messages to a SignalK server
 *
 */
// Including I2C and ADC board ADS1115 
#include <Wire.h>
#include <Adafruit_ADS1015.h>

// Including WiFi stuff
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>

// Instanciate an ads object
Adafruit_ADS1115 ads;

// Instanciate WiFi object
ESP8266WiFiMulti WiFiMulti;
WiFiUDP Udp;


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    Serial.begin(115200);
    delay(10);

    // Set up the ADS 1115 gain
    ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 0.0625mV
    ads.begin();  
    Serial.println("ADS 1115 set up");

    // We start by connecting to a WiFi network
    WiFiMulti.addAP("TeamRocketHQ", "password");

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);
} /* End setup */



void Send_to_SignalK(String path, double value){
   
    // Settings for SignalK port and SignalK server.
    const uint16_t port = 55557;   // SignalK uses this port.
    const char * host = "192.168.1.160"; // ip number of the SignalK server.
    String cmd;
    char valuestring[6];

    cmd = "{\"updates\": [{\"$source\": \"ESP8266\",\"values\":[ {\"path\":\"";
    cmd += path; cmd += "\","; cmd += "\"value\":";
    dtostrf(value,3,2,valuestring); // Convert double to a string
    cmd += valuestring;
    cmd += "}]}]}\0";
    /*
    Serial.println(cmd);
    Serial.println(cmd.length());
    */

    char cmdc[cmd.length()+1];        // Convert the String to an array of characters.
    Udp.beginPacket(host,port);       // Connect to to server and prepare for UDP transfer.
    strncpy(cmdc,cmd.c_str(),sizeof(cmdc));  // Convert from String to array of characters. 
    Serial.println(cmdc); Serial.print(" Message har length: "); Serial.println(sizeof(cmdc));
    Udp.write(cmdc);                  // Send the message to the SignalK server. 
    Udp.endPacket();                  // End the connection.
    delay(10);                        // Short delay to recover. 
} /* End Send_to_SignalK */



void Send_U(int inputno) {
    double U; 
    int val,offset;

    /*
     * Voltage is measured using the single ADC input on the ESP8266 chip.
     * However, this routine is generic and it can easily change to ADS1115. 
     */
    offset=-1;    // Differ from board to board, make your own calibration.
    val=analogRead(A0);  // Read the analog input on the ESP8266.
    val+=offset;
    U=((double)val/(double)1023.0)*19.576;  // Calibrated using a multimeter, check this value carefully. 

    Send_to_SignalK("electrical.service.voltage",U);
}

     
void Send_I(int inputno) {
    double I;
    int val, offset;

    if (inputno==1) {      
      // Read ADC and calculate the equivalt current in Amps.
      offset=680;    // Differ from sensor to sensor, calibration is needed.
      val=ads.readADC_Differential_0_1();   // There are two possibilities in differential mode, 0-1.
      val+=offset;
      I = (double)val/(double)32766*18.0; // Calibrated with a multimeter.
      Send_to_SignalK("electrical.service.current1",I);
    }
    if (inputno==2) {
      offset=0;
      val=ads.readADC_Differential_2_3();   // There are two possibilities in differential mode, 2-3.
      val+=offset;
      I = (double)val/(double)32766*30.0; // Calibrated with a multimeter.
      Send_to_SignalK("electrical.service.current1",I);
    }
}


void loop() {   
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on while we transfer the data.
    Send_U(1);  // Get the voltage and send it to SignalK.
    Send_I(1);  // Get the current and send it to SignalK.
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH.
    
    Serial.println("wait 2 sec...");   // This high refrech rate put a load in the SignalK server.
    delay(2000);
}  /* End infinite loop */

