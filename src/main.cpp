#include <ArduinoOTA.h>
#include <ESP8266WebServerSecure.h>
#include <LittleFS.h>

#include "credentials.h"

// Relay is connected to D2/GPIO4
#define RELAY_PIN 4

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;
const char* www_username = WWW_USERNAME;
const char* www_password = WWW_PASSWD;

const char* mdns_hostname = "doorbot";
const char* log_file_name = "/doorbot.log";

BearSSL::ESP8266WebServerSecure server(443);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICzzCCAbegAwIBAgIJAMPCB5ko3ZcPMA0GCSqGSIb3DQEBBQUAMBgxFjAUBgNV
BAMTDWRvb3Jib3QubG9jYWwwHhcNMjAwNzIzMTUyMTA3WhcNMzAwNzIxMTUyMTA3
WjAYMRYwFAYDVQQDEw1kb29yYm90LmxvY2FsMIIBIjANBgkqhkiG9w0BAQEFAAOC
AQ8AMIIBCgKCAQEAsSfegaV76NyNEbQUcIwtWs8JtTUCRV0+q3poyhO/FITZX197
+0eLAhsBQLKmdRGRph3ErReapNxszHvj/U3dPb/pFkSRUUyhNC80N66gKbySUwjn
dQ+Dgab5MjohNVS657MZ7W5bddh1ZycgmmViT5VFZYRP6KIAfowDnIA0/YqfB9Mr
9eFwWMqyOGYVpG7UaqSxMhp+4JsS20mCgMAWFr5QxFHI5z6hbjr1LIS2yRpSkG4y
SjCgJ290aRw7/Gsw4EPUo7f8xZdu3XM3qdIUDEYoOMXAYizdjt4LKMaHVjWvsffg
q3W0MboWYvROgzv/lKbjaeoRxF6s+4Hb4yBIdwIDAQABoxwwGjAYBgNVHREEETAP
gg1kb29yYm90LmxvY2FsMA0GCSqGSIb3DQEBBQUAA4IBAQA7D+7A3vfENjl38kAX
pOwh7JSk3+9py1u/TLdLlhuRLp/hZGbxD/KUWT3weCEdV4gHRqFE3D3xCLrUVirS
agQPwpqijWimH/qaihOejUv07QRThH8CkhgHD6cLF27U65xk4ROpI1t4kFQZj3FG
I9g3sq3z853HBCVDL1zwteWsGfTh/sexMmI6UmHghQYjgZYrUkFwMwLoY4hUFAiG
egYil1H1OcpNjjMgPaUgxEVPnuJm6My1kIALTdQNkd8yHNWUDYxTnLrda9fSCS8h
OuOtCAlXMfZmrowzYzl9RUz5SlSwLpyyxCHj+qiBbxk1MKKcR57iOWXddM9kW9jJ
+Qx5
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAsSfegaV76NyNEbQUcIwtWs8JtTUCRV0+q3poyhO/FITZX197
+0eLAhsBQLKmdRGRph3ErReapNxszHvj/U3dPb/pFkSRUUyhNC80N66gKbySUwjn
dQ+Dgab5MjohNVS657MZ7W5bddh1ZycgmmViT5VFZYRP6KIAfowDnIA0/YqfB9Mr
9eFwWMqyOGYVpG7UaqSxMhp+4JsS20mCgMAWFr5QxFHI5z6hbjr1LIS2yRpSkG4y
SjCgJ290aRw7/Gsw4EPUo7f8xZdu3XM3qdIUDEYoOMXAYizdjt4LKMaHVjWvsffg
q3W0MboWYvROgzv/lKbjaeoRxF6s+4Hb4yBIdwIDAQABAoIBAAcmWceXuzGBvTvK
rcyBC8+PbY/frNDJ256mGRaWx253kiJUQze4Anc6ScmnrCorZFp09xwZSMeAke+K
zH/LpbM5TJn2eHr0gckOYqggpxwoHSu2v0itMJyJ9aK+T9rMKaLySoiXnJ9b9hkM
rjctAlO97YgUV/lGmIiC9B5b2kelQGQwB6GAEfl7sxz9RlOmO3lXYJ9afrb5jJ/7
5I5AOI9xO9T4jG1tIQODgfLj8U5j3OG14UiVZ5YiDHWXnf9c7y1vso8wDWSbQ+mw
9wa7KVRNvp/ZSKYs98/tBElxukQCPy0dT3xurlL2nrhrxopamTD4NCkI+ux74glU
AfIWhhkCgYEA3Qt1kNs3rOOTlYptrp2222ezEZV+EiPJCOmIK0iLwVtUQ197sO9B
BtZmSqKvN4YiHiP6FZvbaMJPEGC59UO+cWtiSesNzJm65palkJmGyAaDzYsT9m1D
2TUaINE7OIiV1pJdjSgVQuefYDk6LnHiIlfO6NiRDVyIEMDyEFB08h0CgYEAzSun
RyOYruK2VBJM3cpd2kZSUQwY+3O4n21+sw5RSdPr0Pq4upKBMNcUHP+JZ2uBtkFo
HHFKdOYtl9klDNhUnrT3OPKtbDZALAsZE5cyOTNvakzqv3WTjkgR55ZbbTg7wMuL
jnsiO4K6063K33+skFWVLwC8OiyG55rcIhc4oKMCgYEA0LCq9HIjZFUP11V0LDG2
m/qRs35CInoqqQFikArT9190dI9HRkr8R6pPtRNW/cLjnzU0PQaC5ard3oaTyp2E
xawuhb8nrg1Nybpc4eIwv9R2x64Q0M1kC0P5QRaJJNy/Km/RZx16xUnMzJn69jKK
3wWr0WX1vHmp9LdVUlXDs3kCgYBA+ckxI1nQUOYzO9RyDQup+8lH94/V2nEmtFOC
u08NFXtCJJTqKUmWwRaSlG7cfNSIdrBVCZ/t45Oe2lr8dWpfFKqSs3AurAOorx8S
8Dgsm1h2jsNtPPws/DAHarurnDp4NT4OMnrF5AHs1cZA/7sTvrbPOulhdwaGRKng
dgAuBwKBgDNOqTh8Wt/v91A+bqMgZdSws5KNDFNsq3harpwwo1UaIgG70nQh5KNs
jrx1xX+WUoQ1/Y9LWm5LMZmGGtCeqTHO398di1lL+R129gox9oVWrh17Q7hWRwfI
h2Go3DPTvzFSFAZeD1ymvq8HXax71bEDsTei8iBrtGQAMGDJ0Y5v
-----END RSA PRIVATE KEY-----
)EOF";

void open_door() {
  // turn the RELAY on (HIGH is the voltage level)
  Serial.println("Activating door relay");
  digitalWrite(RELAY_PIN, HIGH);

  // wait for 10 seconds
  delay(10000);

  // turn the RELAY off by making the voltage LOW
  Serial.println("Releasing door relay");
  digitalWrite(RELAY_PIN, LOW);
}

String readFile(const char * path) {
  File file = LittleFS.open(path, "r");
  String contents = file.readString();
  file.close();
  return contents;
};

void appendFile(const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

const String rootPage = "<html>"
  "<head>"
    "<title>ESP8266 Web Server to open door</title>"
  "</head>"
  "<body>"
    "Click <a href=\"/open\">here</a> to open the door"
  "</body>\n"
"</html>\n";

void handleRoot() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", rootPage);
}

void handleOpen() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
  String log_line = server.arg("plain") + "\n";
  appendFile(log_file_name, log_line.c_str());
  open_door();
};

void handleLog() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  server.send(200, "text/plain", "contents of logfile\n" + readFile(log_file_name));
};

void setup() {
  Serial.begin(115200);

  LittleFS.begin();

  pinMode(RELAY_PIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  ArduinoOTA.setHostname(mdns_hostname);
  ArduinoOTA.begin();

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.on("/", handleRoot);
  server.on("/open", handleOpen);
  server.on("/log", handleLog);
  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
