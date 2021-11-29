/*******************************************************************
  based of the code of Brian Lough
  https://github.com/witnessmenow
 *******************************************************************/

#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Wifi network station credentials
#define WIFI_SSID "WIFINAME"
#define WIFI_PASSWORD "YOURPASSWORD"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1234567890:XXXXX-XXXXXXXXXXXXXXXXXXX"
#define CHAT_ID "YOURCHATID"

WebServer server(80);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

const int led = 13;
int counter = 0;
bool messageSent = false;

void handleRoot() {
  counter = 0;

  if(messageSent == true){
     bot.sendMessage(CHAT_ID, "Bot restarted", "");
     Serial.println("Bot restarted");
  }
  else{
     Serial.println("Bot is alive");
    }
  messageSent = false;
  digitalWrite(led, 1);
  server.send(200, "text/plain", "you are alive!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup()
{
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Does not work. See issues in UniversalTelegram library
  secured_client.setInsecure();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop()
{
  server.handleClient();

  //Waits two minutes before sending a notification in telegram
  if(counter >= 122 && messageSent == false){
    messageSent = true;
    bot.sendMessage(CHAT_ID, "Bot is dead", "");
    Serial.println("Bot is dead");
  }

  counter++;
  
  delay(1000);
}
