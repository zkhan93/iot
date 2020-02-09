#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "OpenWrt2GHz";
const char* password = "internett";
unsigned int serverPort = 80;
unsigned int reqUdpPort = 3311;
unsigned int resUdpPort = 3312;
char syncRes[] = "{\"id\":\"strand1\",\"model\":\"fakecandy\",\"hw_rev\":\"evt-1\",\"fw_rev\":\"v1-beta\",\"channels\":[1]}";
char syncReq[255]; 


WiFiServer server(serverPort);
WiFiUDP UdpSync;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected, Server started at: http://%s:%d\nListening to UDP packets on: %d", WiFi.localIP().toString().c_str(), serverPort, reqUdpPort);
  
  UdpSync.begin(reqUdpPort);
  server.begin();
}

void sync() {
  int packetSize = UdpSync.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, UdpSync.remoteIP().toString().c_str(), UdpSync.remotePort());
    Serial.print("UDP packet contents: ");
    int len = UdpSync.read(syncReq, 255);;
    if (len != -1){
      syncReq[len] = '\0';
      Serial.print(syncReq);
    }
    Serial.println();

    // send back a reply, to the IP address and port we got the packet from
    UdpSync.beginPacket(UdpSync.remoteIP(), resUdpPort);
    UdpSync.write(syncRes, 88);
    UdpSync.endPacket();
  }
}

void loop() {
  sync();
}
