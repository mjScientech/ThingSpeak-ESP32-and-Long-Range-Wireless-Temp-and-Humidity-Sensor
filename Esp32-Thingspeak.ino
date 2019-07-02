

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

const char* ssid     = "Yourssid"; // Your SSID (Name of your WiFi)
const char* password = "Wifipass"; //Your Wifi password

const char* host = "api.thingspeak.com";
String api_key = "APIKEY"; // Your API Key provied by thingspeak
int value;

int Temp;
int Rms_x;
int Rms_y;
int Rms_z;
void setup() {
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit​
  Serial.begin(9600);
  Serial.println("ncd.io IoT ESP32 Vibration Temperature sensor");


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
      for ( i = 1; i < 54; i++)
      {
        data[i] = Serial1.read();
        delay(1);
      }
      if (data[15] == 0x7F) /////// to check if the recive data is correct
      {
        if (data[22] == 0x08) //////// make sure the sensor type is correct
        {
          int16_t rms_x = ((uint16_t)(((data[24]) << 16) + ((data[25]) << 8) + (data[26])) / 100);
          int16_t rms_y = ((uint16_t)(((data[27]) << 16) + ((data[28]) << 8) + (data[29])) / 100);
          int16_t rms_z = ((uint16_t)(((data[30]) << 16) + ((data[31]) << 8) + (data[32])) / 100);

          int16_t max_x = ((uint16_t)(((data[33]) << 16) + ((data[34]) << 8) + (data[35])) / 100);
          int16_t max_y = ((uint16_t)(((data[36]) << 16) + ((data[37]) << 8) + (data[38])) / 100);
          int16_t max_z = ((uint16_t)(((data[39]) << 16) + ((data[40]) << 8) + (data[41])) / 100);

          int16_t min_x = ((uint16_t)(((data[42]) << 16) + ((data[43]) << 8) + (data[44])) / 100);
          int16_t min_y = ((uint16_t)(((data[45]) << 16) + ((data[46]) << 8) + (data[47])) / 100);
          int16_t min_z = ((uint16_t)(((data[48]) << 16) + ((data[49]) << 8) + (data[50])) / 100);


          int cTemp = ((((data[51]) * 256) + data[52]));
          float battery = ((data[18] * 256) + data[19]);
          float voltage = 0.00322 * battery;
          Serial.print("Sensor Number  ");
          Serial.println(data[16]);
          Serial.print("Sensor Type  ");
          Serial.println(data[22]);
          Serial.print("Firmware Version  ");
          Serial.println(data[17]);
          Serial.print("Temperature in Celsius :");
          Serial.print(cTemp);
          Serial.println(" C");

          Serial.print("RMS vibration in X-axis :");
          Serial.print(rms_x);
          Serial.println(" mg");
          Serial.print("RMS vibration in Y-axis :");
          Serial.print(rms_y);
          Serial.println(" mg");
          Serial.print("RMS vibration in Z-axis :");
          Serial.print(rms_z);
          Serial.println(" mg");

          Serial.print("Min vibration in X-axis :");
          Serial.print(min_x);
          Serial.println(" mg");
          Serial.print("Min vibration in Y-axis :");
          Serial.print(min_y);
          Serial.println(" mg");
          Serial.print("Min vibration in Z-axis :");
          Serial.print(min_z);
          Serial.println(" mg");

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
          Rms_x = rms_x;
          Rms_y = rms_y;
          Rms_z = rms_z;


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
    data_to_send += String(Rms_x);
    data_to_send += "&field2=";
    data_to_send += String(Temp);
    data_to_send += "&field3=";
    data_to_send += String(Rms_y);
    data_to_send += "&field4=";
    data_to_send += String(Rms_z);
    data_to_send += "\r\n\r\n";

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
