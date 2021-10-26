// PROGRAM TO SEND GPS LOCATION ON DEMAND AND SEND GPS DATA TO THINGSSPEAK SERVER //
///////////////////////////////////////////////////////////////////////////////////

#include <SoftwareSerial.h> // Library for using serial communication
#include <MicroNMEA.h> // Library for converting NEMA message 

/***********************************/
SoftwareSerial AIA9G(3, 4); // Pins 3, 4 are used as used as software serial pins RX ,TX
SoftwareSerial GPSData(10, 11); // Pins 10, 11 are used as used as software serial pins RX ,TX

const String api_key="XXXXXXXXXXXXXXXXX"; //API Key from thingsspeak.com
char buffer[85];
const unsigned long eventInterval = 20000;
unsigned long previousTime = 0;
MicroNMEA nmea(buffer, sizeof(buffer));

unsigned long lastLog = 0;
//int state = 0;
float latitude_mdeg ;
float longitude_mdeg ;

//Setup and initialize 
void setup()
{
   // Set Baud rate for all 3 channels
    Serial.begin(19200); // baudrate for serial monitor
    AIA9G.begin(115200); // baudrate for GSM shield
    GPSData.begin(9600); // baudrate for GPS data
    Serial.println("Starting...");
    
    // *************************************************************//
    // Start Initializing A9G board.
    // Initialize GPRS
    AIA9G.println("\r");
    AIA9G.println("AT+GPS=1\r");
    delay(100); 
    AIA9G.println("AT+CREG=2\r");
    delay(6000);
    //AIA9G.print("AT+CREG?\r");
    AIA9G.println("AT+CGATT=1\r");
    delay(6000);
    AIA9G.println("AT+CGDCONT=1,\"IP\",\"WWW\"\r");
    delay(6000);
   // AIA9G.println("AT+LOCATION=1\r");
    AIA9G.println("AT+CGACT=1,1\r");
    delay(6000);
    //Initialize ends  
    //Initialize GPS  
    AIA9G.println("\r");
    AIA9G.println("AT+GPS=1\r");
    delay(1000);
    //AIA9G.println("AT+GPSMD=1\r");   // Change to only GPS mode from GPS+BDS, set to 2 to revert to default.
    AIA9G.println("AT+GPSRD=10\r");
    delay(100);
    // set SMS mode to text mode
    AIA9G.println("AT+CMGF=1\r");  
    delay(1000);
    //AIA9G.println("AT+LOCATION=2\r");
    // set gsm module to tp show the output on serial out
    AIA9G.println("AT+CNMI=2,2,0,0,0\r"); 
    delay(1000);
     AIA9G.print("AT+CMGS=\"+XXXXXXXXXX\"\r"); //Replace this with your mobile number
     delay(1000);
     //The text of the message to be sent.
     AIA9G.print("Tracker active...");
     AIA9G.write(0x1A);
     delay(1000);
}
void loop()
{
  unsigned long currentTime = millis();
  GPSData.listen();
  delay(1000);
  while(GPSData.available()>0)
  {
    char inByte = GPSData.read(); // Get GPS data 
    Serial.print(inByte);
    nmea.process(inByte);
  }  
  Serial.println("");
  latitude_mdeg = nmea.getLatitude();
  longitude_mdeg = nmea.getLongitude();
  Serial.print("Latitude (deg): ");
  latitude_mdeg=latitude_mdeg / 1000000.;
  
  Serial.println(latitude_mdeg, 6);
  Serial.print("Longitude (deg): ");
  longitude_mdeg=longitude_mdeg / 1000000.;
  Serial.println(longitude_mdeg, 6); 
  Serial.println(nmea.isValid());   
  // sendGPSData();
  //send data after someinterval
  if (currentTime - previousTime >= eventInterval) {
    /* Send data */
    sendGPSData();
   /* Update the timing for the next time around */
    previousTime = currentTime;
  }
  AIA9G.listen();
  //AIA9G.println("AT+CNMI=2,2,0,0,0"); // AT Command to recieve a live SMS
  // delay(100);
     if (AIA9G.available() > 0) {
        String c = AIA9G.readString();
        c.trim();
        if (c.indexOf("GET-GPS") >= 0) {   //Send GPS data on request by SMS //
          AIA9G.print("\r");
          delay(1000);
          AIA9G.print("AT+CMGF=1\r");
          delay(1000);
          AIA9G.print("AT+CMGS=\"+XXXXXXXXXX\"\r");  // Replace this with your mobile number.
          delay(1000);
          //The text of the message to be sent.
          AIA9G.print("www.google.com/maps/?q=");
          AIA9G.println(latitude_mdeg, 6);
          AIA9G.print(",");
          AIA9G.println(latitude_mdeg, 6);
          delay(1000);
          AIA9G.write(0x1A);
          delay(1000);
         // state = 1;
      
        }  
        
     }
}
//Send GPSData to server.
void sendGPSData(){
  Serial.print("http://api.thingspeak.com/update?api_key="); // Display on Serial monitor
  Serial.print(api_key);
  Serial.print("&field1=");
  Serial.print(latitude_mdeg,6);
  Serial.print("&field2=");
  Serial.print(longitude_mdeg,6);
  Serial.println();
  if(latitude_mdeg != 0 && longitude_mdeg !=0){  //update only if valid values.
  //String url="AT+CIPSTART=\"TCP\",\"http://api.thingspeak.com/update?api_key=" + api_key + "&field1=" + (String)latitude_mdeg + "&field2=" + (String)longitude_mdeg + "\",80\r";
    AIA9G.print("AT+HTTPGET=\"https://api.thingspeak.com/update?api_key=");
    AIA9G.print(api_key);
    AIA9G.print("&field1=");
    AIA9G.print(latitude_mdeg,6);
    AIA9G.print("&field2=");
    AIA9G.println(longitude_mdeg,6);
 //   AIA9G.write(0x1A);
    delay(100);
//  Serial.println(url);
  }
}

///// Below codes not used.
/*
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";    
    AIA9G.println(command); 
    long int time = millis();   
    while( (time+timeout) > millis())
    {
      while(AIA9G.available())
      {       
        char c = AIA9G.read(); 
        response+=c;
      }  
    }    
    if(debug)
    {
      Serial.print(response);
    }    
    return response;
}
*/
