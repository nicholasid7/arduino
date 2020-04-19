/*********
  EGR with Wi-Fi and  time
*********/

// подключаем библиотеку «ESP8266WiFi»:
#include <ESP8266WiFi.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <WiFiUdp.h>

// вписываем здесь SSID и пароль для вашей WiFi-сети:
const char* ssid = "ssid"; // введите название вашей Wi-Fi сети
const char* password = "pass"; // введите пароль Wi-Fi сети
unsigned int localPort = 2390; // local port to listen for UDP packets

unsigned long epoch = 0;

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

const int timeZone = 9; // GMT+9 for Yakutsk Time
const long timeZoneOffset = timeZone * 3600;

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;


// контакт для передачи данных подключен к D1 на ESP8266 12-E (GPIO5):
#define ONE_WIRE_BUS 5
int OutPin = A0;
int OutValue = 128;


// создаем экземпляр класса oneWire; с его помощью
// можно коммуницировать с любыми девайсами, работающими
// через интерфейс 1-Wire, а не только с температурными датчиками
// от компании Maxim/Dallas:
OneWire oneWire(ONE_WIRE_BUS);

// передаем объект oneWire объекту DS18B20:
DallasTemperature DS18B20(&oneWire);
char temperatureCString[6];
char temperatureFString[6];

// веб-сервер на порте 80:
WiFiServer server(80);

// блок setup() запускается только один раз – при загрузке:
void setup() {
  // инициализируем последовательный порт (для отладочных целей):

  Serial.begin(115200);
//  Serial.setDebugOutput(true);
  // put your setup code here, to run once:
  pinMode(D8, INPUT_PULLUP); //INPUT_PULLUP
  pinMode(D9, INPUT_PULLUP);
  analogWrite(OutPin, OutValue);
  delay(10);

  DS18B20.begin(); // по умолчанию разрешение датчика – 9-битное;
  // если у вас какие-то проблемы, его имеет смысл
  // поднять до 12 бит; если увеличить задержку,
  // это даст датчику больше времени на обработку
  // температурных данных

  // подключаемся к WiFi-сети:
  Serial.println();
  Serial.print("Connecting to "); // "Подключаемся к "
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); // "Подключение к WiFi выполнено"

  // запускаем веб-сервер:
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  // "Веб-сервер запущен. Ожидание IP-адреса ESP..."
  delay(10000);

  // печатаем IP-адрес ESP:
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

//void getTemperature() {
//  float tempC;
//  float tempF;
//  do {
//    DS18B20.requestTemperatures();
//    tempC = DS18B20.getTempCByIndex(0);
//    dtostrf(tempC, 2, 2, temperatureCString);
//    tempF = DS18B20.getTempFByIndex(0);
//    dtostrf(tempF, 3, 2, temperatureFString);
//    delay(100);
//  } while (tempC == 85.0 || tempC == (-127.0));
//}

void getOutValue() {
  if (!(digitalRead(D8)))
  {
    if (OutValue < 255) OutValue += 1;
  }
  if (!(digitalRead(D9)))
  {
    if (OutValue > 0) OutValue -= 1;
  }
  analogWrite(OutPin, OutValue);
//  digitalWrite(OutPin, OutValue);
  Serial.print("OutValue: ");
  Serial.println(OutValue);
  delay(500);
}

// блок loop() будет запускаться снова и снова:
void loop() {
  
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  //  delay(1000);
  unsigned long timeout = millis() + 1000;
  while ((udp.available() < NTP_PACKET_SIZE) && (millis() < timeout)) {}
  int cb = udp.parsePacket();
  if (! cb) {
    Serial.println("no packet yet");
  } else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    // unsigned long epoch = secsSince1900 - seventyYears;
    epoch = secsSince1900 - seventyYears;
  }
  // начинаем прослушку входящих клиентов:
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client");  //  "Новый клиент"
    // создаем переменную типа «boolean»,
    // чтобы определить конец HTTP-запроса:
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && blank_line) {
          getOutValue();
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          // веб-страница с данными о температуре:
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head></head><body><h1>EGR_motor_value</h1>");
          client.print("The UTC time is ");
          client.println((epoch % 86400L) / 3600);           //  часы
          client.println(" : ");
          if (((epoch % 3600) / 60) < 10) {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            client.print('0');
          }
          client.println((epoch  % 3600) / 60);                //  минуты
          client.println(" : ");
          if ((epoch % 60) < 10) {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            client.print('0');
          }
          client.println(epoch % 60);                             //  секунды
          client.print("<br><b> Chita +9 </b>");

          client.println("<h3>OutValue in bit: ");
          client.println(OutValue);
          client.println("*C</h3></body></html>");
          break;
        }
        if (c == '\n') {
          // если обнаружен переход на новую строку:
          blank_line = true;
        }
        else if (c != '\r') {
          // если в текущей строчке найден символ:
          blank_line = false;
        }
      }
    }
    // закрываем соединение с клиентом:
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
    //  "Клиент отключен."
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
