/*
 * 
 *  This sketch uses a ESP 8266 and a number of DS18B20 devices  to measure temperature 
 *  and sends messages to a SignalK server.
 *  
 *  Select board  ESP-12E , NodeMCU 1.0 ESP12-E module
 *
 * 
 *  SignalK uses UDP. The syntax is differ between ESP8622 and ESP32, this sketch 
 *  cannot be used for ESP32.
 *  
 *  
 *  Names of Signal K path/labels of the temperature sensors initialised in the setup function.
 *  
 */


// One wire bus setup
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D3      // Pin to which is attached a temperature sensor
#define ONE_WIRE_MAX_DEV 15  // The maximum number of devices

// Instanciate onewire and DS18B20 objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// Including WiFi stuff - this is UDP and differ from TCP and also from ESP32 cards-
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>

// Instanciate WiFi and UDP objects
ESP8266WiFiMulti WiFiMulti;
WiFiUDP Udp;


// Global variables 
int numberOfDevices;                      // Number of temperature devices found
DeviceAddress devAddr[ONE_WIRE_MAX_DEV];  // An array devicetemperature sensors
float tempDev[ONE_WIRE_MAX_DEV];          // Saving the last measurement of temperature

// WiFi network name and password 
const char * ssid = "TeamRocketHQ";
const char * pwd = "password";

// IP address to send UDP data to.
// Settings for SignalK port and SignalK server.
const char * udpAddress = "192.168.1.160";
const int udpPort = 55557;

/*
 * Signal K paths/names name of the temperature sensors, description of what they measure.
 */
const char * labels[ONE_WIRE_MAX_DEV];  // Labels assigned in setup function.

//  End declaration of global variables.



// Setup function

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
    Serial.begin(115200);         //Setup Serial port speed

  // Connect to the WiFi network
    WiFi.begin(ssid, pwd);
    Serial.println("");

  // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  // This initializes udp and transfer buffer
    Udp.begin(udpPort);    
    delay(10);


  // Set up the DS18B20 temperature sensors
    SetupDS18B20();   

/*
 * 
 * Set up the different SignalK names/paths :
 * 
 */
//**************************************************************** 

    labels[0]="temperature.environment.indoor";
    labels[1]="temperature.environment.computer";
    
//****************************************************************

}  // End setup 



// Convert device id to String
String GetAddressToString(DeviceAddress deviceAddress){
  String str = "";
  for (uint8_t i = 0; i < 8; i++){
    if( deviceAddress[i] < 16 ) str += String(0, HEX);
    str += String(deviceAddress[i], HEX);
  }
  return str;
} // End GetAddressToString



// Setting the temperature sensor
void SetupDS18B20(){
  DS18B20.begin();

  Serial.print("Parasite power is: "); 
  if( DS18B20.isParasitePowerMode() ){ 
    Serial.println("ON");
  }else{
    Serial.println("OFF");
  }
  
  numberOfDevices = DS18B20.getDeviceCount();
  Serial.print( "Device count: " );
  Serial.println( numberOfDevices );

  DS18B20.requestTemperatures();

  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if( DS18B20.getAddress(devAddr[i], i) ){
    // devAddr[i] = tempDeviceAddress;
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: " + GetAddressToString(devAddr[i]));
      Serial.println();
    }else{
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }

  // Get resolution of DS18B20
    Serial.print("Resolution: ");
    Serial.print(DS18B20.getResolution( devAddr[i] ));
    Serial.println();

  // Read temperature from DS18b20
    float tempC = DS18B20.getTempC( devAddr[i] );
    Serial.print("Temp C: ");
    Serial.println(tempC);
  }  // End for numberOfDevices
}  // End SetupDS18B20



/*
 * Function to convert strings and values and forward it using UDP to the Signal K server 
 * 
 */
 
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



/*
 * 
 * Loop measuring the temperature, collect all temperatures and forward to signal K sender.
 * 
 */

void TempLoop(){
    for(int i=0; i<numberOfDevices; i++){
      float tempC = DS18B20.getTempC( devAddr[i] ); //Measuring temperature in Celsius
      tempDev[i] = tempC; //Save the measured value to the array
      Serial.print("Device "); Serial.print(i); Serial.print(" Temp C: "); Serial.println(tempC);
      Send_to_SignalK(labels[i],tempDev[i]);
    }
    DS18B20.setWaitForConversion(false); // No waiting for measurement
    DS18B20.requestTemperatures();       // Initiate the temperature measurement
  }  // End TempLoop
 

// The infinite loop
void loop() {
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on while we transfer the data.
  TempLoop();                      // Loop through devices and measure 
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH.
  delay(5000);                     // Send frequancy 5s => 0.2 Hz.
}  // End looop
 
