#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>

// ==============================
// Wi-Fi 및 고정 IP 설정
// ==============================
const char* ssid = "SecondClass2.4G";
const char* password = "12345678";

IPAddress local_IP(10, 114, 189, 122);
IPAddress gateway(10, 114, 184, 1);
IPAddress subnet(255, 255, 252, 0);

// --- 로보1472 핀 설정 ---
const int PWMA = 19;
const int PWMB = 18;
const int AIN1 = 33;
const int AIN2 = 32;
const int BIN1 = 25;
const int BIN2 = 26;
const int STBY = 5;
const int LED_PIN = 27;
const int LED_COUNT = 2;
const int motorSpeed = 150;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void motor(int leftSpeed, int rightSpeed, int a1, int a2, int b1, int b2){
  analogWrite(PWMA, leftSpeed);
  analogWrite(PWMB, rightSpeed);
  digitalWrite(AIN1, a1);
  digitalWrite(AIN2, a2);
  digitalWrite(BIN1, b1);
  digitalWrite(BIN2, b2);
}

void handleCommand(char cmd) {
  int moveSpeed = 150; // 전진/후진 속도
  int turnSpeed = 90;  // 좌/우회전 속도를 낮춰서 조금만 움직이게 조절 (기존 150 -> 90)

  if (cmd == 'w') { // 전진 (초록)
    motor(moveSpeed, moveSpeed, LOW, HIGH, HIGH, LOW);
    strip.fill(strip.Color(0, 255, 0));
  } 
  else if (cmd == 's') { // 후진 (빨강)
    motor(moveSpeed, moveSpeed, HIGH, LOW, LOW, HIGH);
    strip.fill(strip.Color(255, 0, 0));
  } 
  else if (cmd == 'x') { // 정지 (흰색)
    motor(0, 0, LOW, LOW, LOW, LOW);
    strip.fill(strip.Color(255, 255, 255));
  } 
  else if (cmd == 'a') { // 좌회전 (노랑) - 속도를 낮춤
    motor(turnSpeed, turnSpeed, LOW, HIGH, LOW, HIGH);
    strip.fill(strip.Color(255, 255, 0));
  } 
  else if (cmd == 'd') { // 우회전 (파랑) - 속도를 낮춤
    motor(turnSpeed, turnSpeed, HIGH, LOW, HIGH, LOW);
    strip.fill(strip.Color(0, 0, 255));
  }
  strip.show();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    handleCommand((char)data[0]);
  }
}

void setup() {
  Serial.begin(115200);

  int pins[] = {PWMA, PWMB, AIN1, AIN2, BIN1, BIN2, STBY};
  for(int i = 0; i < 7; i++) pinMode(pins[i], OUTPUT);
  digitalWrite(STBY, HIGH);

  strip.begin(); 
  strip.show();

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("고정 IP 설정 실패!");
  }

  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("고정 IP로 연결 성공! 주소: http://");
  Serial.println(WiFi.localIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.begin();
}

void loop() {
  ws.cleanupClients();
}
