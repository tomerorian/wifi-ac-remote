// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include "wifi.h"
#include "ac-remote.h"
#include <WiFiClientSecure.h>

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
int set_fan_power(String power);
int set_mode(String ac_mode);
String get_state(String ignore); 

// Rest exposed variables
int v_temp = ac_remote.get_temp();
boolean v_on = ac_remote.get_on();
int v_fan_power = ac_remote.get_fan_power();
int v_mode = ac_remote.get_mode();

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
  rest.function("set-fan-power", set_fan_power);
  rest.function("set-mode", set_mode);

  rest.variable("Temperature", &v_temp);
  rest.variable("On", &v_on);
  rest.variable("Fan Power", &v_fan_power);
  rest.variable("Mode", &v_mode);
}

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

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
  v_temp = temp.toInt();
  
  return 0;
}

int set_on(String str_on)
{
  boolean is_on = str_on == "1";

  ac_remote.update_on(is_on);
  v_on = is_on;

  return 0;
}

int set_fan_power(String power)
{
  ac_remote.update_fan_power(power.toInt());
  v_fan_power = power.toInt();

  return 0;
}

int set_mode(String ac_mode)
{
  ac_remote.update_mode(ac_mode.toInt());
  v_mode = ac_mode.toInt();

  return 0;
}
