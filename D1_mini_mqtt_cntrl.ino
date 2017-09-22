//perkindo - Creative Commons License 
//
//Requires PubSubClient
//
//WeMos D1 Mini v2 - ESP8266 Simple MQTT light controller


#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//MQTT Server and Wifi Network Information
#define MQTT_SERVER "192.168.1.1"
const char* ssid="(wireless SSID)"; //Insert the SSID for your wireless network
const char* password="(password)"; //Add the password for your wireless network

//Assign Control Pin and Set Static IP Address
const int CntrlPin=16;
IPAddress ip(192, 168, 1, 51); // Static IP Address - Will need to be different for each D1 Mini
IPAddress gateway(192, 168, 1, 254); //Gateway for your network
IPAddress subnet(255, 255, 255, 0); //Subnet for your network

char* lightTopic = "/carport"; //Subject you want to Publish to - Used to control specific device assigned to this D1 mini

WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

  //turn the light on and publish confirmation to the MQTT server
  if(payload[0] == '1'){
    lightOn();
    Serial.println("carport light on"); //Change to reflect device being contolled
  }

  //turn the light off and publish confirmation to the MQTT server
  else if (payload[0] == '0'){
    lightOff();
    Serial.println("carpot light off"); //Change to reflect device being contolled
  }

}

PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void lightOn(){
    digitalWrite(CntrlPin, HIGH);
    client.publish("/mqtt-home/confirm", "Carport Light is On");
}

void lightOff(){
    digitalWrite(CntrlPin, LOW);
    client.publish("/mqtt-home/confirm", "Carport Light is Off");
}

void setup() {
  //initialize the light as an output and set to LOW (off)
  pinMode(CntrlPin, OUTPUT);
  digitalWrite(CntrlPin, LOW);

  //start the serial monitor
  Serial.begin(115200);
  delay(100);
 
  //Connect to and display information about Wifi Network and MQTT Server
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
  Serial.println("MQTT Server 192.168.1.1");
    

  //wait a bit before starting the main loop
      delay(1500);
}


void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
}


void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    //Serial.println("");
    //Serial.println("WiFi connected");  
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        Serial.print("\tMTQQ Connected");
        client.subscribe(lightTopic);
      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}
