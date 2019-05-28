#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>

//NEED TO CALCULATE: miliseconds = minutes * 60 * 1000
#define POST_DATA_PERIOD 5000

// The maximum number of sensors
#define SENSORCOUNT 3

// Led define
#define LED_BUILTIN 2


const char *ssid1 = "Bio-Mech Lab";
const char *password1 = "biomechlab2019";

const char *ssid2 = "Wifi Long Room";
const char *password2 = "0982407168";

String hostUrl = "http://shogunross:3000/hydro-hub"; // TODO: Change this url after production
String secretKey = "6MfzwnDGNNmcHat64fb4kq1qHxhvoGyU";
String requestBody;

float temperature, pH, nutrient;

unsigned long timeStamp = millis();

WiFiMulti wifiMulti;
HTTPClient http;

void setup()
{
	Serial.begin(115200);

	delay(10); //Delay needed before calling the WiFi.begin

	startWiFi();

	pinMode(LED_BUILTIN, OUTPUT);

	// if analog input pin 0 is unconnected, random analog
	// noise will cause the call to randomSeed() to generate
	// different seed numbers each time the sketch runs.
	// randomSeed() will then shuffle the random function.
	randomSeed(analogRead(0));
}

void loop()
{
	if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
	{
		postData(); // Call postData for every period
	}
	else
	{
		Serial.println("Error in WiFi connection");
	}
}

//_________________________________________ Start WiFi function _________________________________________
void startWiFi()
{
	wifiMulti.addAP(ssid1, password1);
	wifiMulti.addAP(ssid2, password2);

	Serial.println("Connecting");
	while (wifiMulti.run() != WL_CONNECTED) // Wait for the Wi-Fi to connect
	{
		delay(250);
		Serial.print('.');
	}
	Serial.println("\r\n");
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID()); // Tell us what network we're connected to
	Serial.print("IP address:\t");
	Serial.print(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
	Serial.println("\r\n");
}

//_________________________________________ Post Data function _________________________________________
/*
* This function use HTTP POST request to send sensor data to server. 
*/
void postData()
{
	if (millis() - timeStamp < 0)
	{ // Reset timeStamp because value return by millis() with this condition will rollover.
		timeStamp = millis();
	}
	else if (millis() - timeStamp > POST_DATA_PERIOD)
	{ // Execute post data when reach enough ms
		// random value for Temperature sensor

		http.begin(hostUrl); //Specify destination for HTTP request
		Serial.println("Host to post: " + hostUrl);

		http.addHeader("Content-Type", "application/json"); //Specify content-type header

		//TODO: If you get the real data form data, replace random function with your own data function
		nutrient = randomData(800, 1100, 1);
		pH = randomData(6, 7, 1);
		temperature = randomData(25, 30, 1);

		requestBody = "{\"query\":\"mutation{updateSensor(sensorInput:{secretKey:\"" + secretKey + "\",temperature:" + temperature + ",pH:" + pH + ",nutrient:" + nutrient + "){time}\"}";
		Serial.print(requestBody);

		int httpResponseCode = http.POST(requestBody); //Send the actual POST request

		if (httpResponseCode > 0)
		{
			String response = http.getString();

			Serial.println(httpResponseCode); //Print return code
			Serial.println(response);					//Print response from server
		}
		else
		{
			Serial.print("Error on sending request: ");
			Serial.println(httpResponseCode);
		}
		http.end(); //Free resources

		timeStamp = millis(); // If post doesn't success, new timeStamp shall not be set, so postData function will loop until the post success. BUG || FEATURE ??
	}
}

//_________________________________________ Get Random Sensor Data fuction _________________________________________
/*
*This function is used to get the value of Sensor by mean of sending command to main devices, then get the response and return the float value
* @param min lowest number 
* @param nax highest number
* @param precision number of index behind comma
* @return a random float number within a range of (mix, max) with given precision 
*/
float randomData(long min, long max, unsigned int precision){
    float value;
		int scale = (int) pow(10, precision);
		value = random(min * scale, max * scale);
		return value / scale;
};
