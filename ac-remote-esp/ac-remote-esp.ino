// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include "wifi.h"
#include "storage.h"
#include "ac-remote.h"
#include <WiFiClientSecure.h>

Storage storage = Storage();

// Create aREST instance
aREST rest = aREST();

// Create AC Remote
ACRemote ac_remote = ACRemote(4);

// WiFi parameters
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Declare functions to be exposed to the API
int set_temp(String temp);
int set_on(String onOff);

void wifi_connect() 
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());  
}

void setup_rest() 
{
  // Set ID and name
  rest.set_id("123456");
  rest.set_name("esp-ac-remote");
  
  // Function to be exposed
  rest.function("set-temp", set_temp);
  rest.function("set-on", set_on);
}

void load_ir_code()
{
  storage.begin();
  storage.end();
//  EEPROM.begin(512);
//  bbb_username = load_string(BBB_USERNAME_ADDRESS);
//  bbb_password = load_string(BBB_PASSWORD_ADDRESS);
//  bis_username = load_string(BIS_USERNAME_ADDRESS);
//  bis_password = load_string(BIS_PASSWORD_ADDRESS);
//  EEPROM.end();
}

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  load_ir_code();

  setup_rest();

  wifi_connect();
}

void handle_rest() 
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  int client_timeout = 1000;
  while(!client.available() && client_timeout > 0) {
    delay(1);
    client_timeout -= 1;
  }

  if (client_timeout <= 0) {
    return;
  }

  rest.handle(client);  
}

void loop() 
{
  handle_rest();

  ac_remote.send_code_if_needed();
}

// Custom function accessible by the API
int set_temp(String temp)
{
  ac_remote.update_temp(temp.toInt());
  
  return 1;
}

int set_on(String strOn)
{
  boolean isOn = strOn == "1";

  ac_remote.update_on(isOn);
}

