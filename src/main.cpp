#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

#define RELAY_NO true
#define relayGPIO 14

const char* ssid = "SmartPlug";

const char* PARAM_INPUT_1 = "relay";
const char* PARAM_INPUT_2 = "state";

bool currentState = RELAY_NO;

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    h4 {padding-top: 120px;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px}
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Умная розетка&#128268;</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";

String relayState(int numRelay){
    if (RELAY_NO && currentState || !RELAY_NO && !currentState)
        return "";
    return "checked";
}

String processor(const String& var){
    if(var == "BUTTONPLACEHOLDER"){
        String relayStateValue = "checked";
        if (RELAY_NO && currentState || !RELAY_NO && !currentState)
            relayStateValue = "";
        String button = "<h4>Лампочка</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
        return button;
    }
    return String();
}

void setup(){
    Serial.begin(115200);

    pinMode(relayGPIO, OUTPUT);

    Serial.println();
    Serial.print("Setting soft-AP configuration ... ");
    Serial.println( WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!" );
    Serial.print("Setting soft-AP ... ");
    Serial.println( WiFi.softAP(ssid) ? "Ready" : "Failed!" );
    Serial.print("IP address = ");
    Serial.println( WiFi.softAPIP() );

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage;
        String inputParam;
        String inputMessage2;
        String inputParam2;

        if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
            inputMessage = request->getParam(PARAM_INPUT_1)->value();
            inputParam = PARAM_INPUT_1;
            inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
            inputParam2 = PARAM_INPUT_2;
            Serial.println(inputMessage + " | " + inputMessage2);

            currentState = inputMessage2.toInt();
            if(RELAY_NO)
                currentState = !currentState;

            digitalWrite(relayGPIO, currentState ? HIGH : LOW);

            Serial.println(String(RELAY_NO ? "(NO) " : "(NC) ") + "Update: " + String(currentState ? "ON" : "OFF"));

        }
        else
            Serial.println("No message sent");

        request->send(200, "text/plain", "OK");
    });

    server.begin();
}

void loop() {

}
