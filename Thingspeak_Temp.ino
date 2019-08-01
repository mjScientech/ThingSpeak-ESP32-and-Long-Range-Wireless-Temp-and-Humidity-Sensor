// Include library to connect the ESP32 to the internet
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <WiFiMulti.h>

WiFiMulti WiFiMulti;
uint8_t data[54];
int k = 10;
int i;

const char* ssid     = "xyz"; // Your SSID (Name of your WiFi)
const char* password = "xyz"; //Your Wifi password

const char* host = "api.thingspeak.com";
String api_key = "xyz"; // Your API Key provied by thingspeak
int value;

int Temp;
int Humidity;
void setup() {
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit​
  Serial.begin(9600);
  Serial.println("ncd.io IoT ESP32  Temperature sensor");


  Connect_to_Wifi();
}

void loop() {
  if (Serial1.available())
  {
    data[0] = Serial1.read();
    delay(k);
    if (data[0] == 0x7E)
  {
    while (!Serial1.available());
    for ( i = 1; i < 29; i++)
    {
      data[i] = Serial1.read();
      delay(1);
    }
    if (data[15] == 0x7F) /////// to check if the recive data is correct
    {
      if (data[22] == 1) //////// make sure the sensor type is correct
      {
        float humidity = ((((data[24]) * 256) + data[25]) / 100.0);
        int16_t cTempint = (((uint16_t)(data[26]) << 8) | data[27]);
        float cTemp = (float)cTempint / 100.0;
        float fTemp = cTemp * 1.8 + 32;
        float battery = ((data[18] * 256) + data[19]);
        float voltage = 0.00322 * battery;
        Serial.print("Sensor Number  ");
        Serial.println(data[16]);
        Serial.print("Sensor Type  ");
        Serial.println(data[22]);
        Serial.print("Firmware Version  ");
        Serial.println(data[17]);
        Serial.print("Relative Humidity :");
        Serial.print(humidity);
        Serial.println(" %RH");
        Serial.print("Temperature in Celsius :");
        Serial.print(cTemp);
        Serial.println(" C");
        Serial.print("Temperature in Fahrenheit :");
        Serial.print(fTemp);
        Serial.println(" F");
        Serial.print("ADC value:");
        Serial.println(battery);
        Serial.print("Battery Voltage:");
        Serial.print(voltage);
        Serial.println("\n");

          if (voltage < 1)
          {
            Serial.println("Time to Replace The Battery");
          }
          Temp = cTemp;
          Humidity=humidity;


        }
      }
      else
      {
        for ( i = 0; i < 54; i++)
        {
          Serial.print(data[i]);
          Serial.print(" , ");
          delay(1);
        }
      }
    }
  }
  // call function to send data to Thingspeak
  Send_Data();

  delay(5000);


}

void Connect_to_Wifi()
{

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void Send_Data()
{

  // map the moist to 0 and 100% for a nice overview in thingspeak.

  value = constrain(value, 0, 5000);
  value = map(value, 0, 5000, 100, 0);

  Serial.println("Prepare to send data");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  else
  {
    String data_to_send = api_key;
    data_to_send += "&field1=";
    data_to_send += String(Humidity);
    data_to_send += "&field2=";
    data_to_send += String(Temp);
   
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data_to_send.length());
    client.print("\n\n");
    client.print(data_to_send);

    delay(1000);
  }

  client.stop();

}
