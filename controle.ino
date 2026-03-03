#include <esp_now.h>
#include <WiFi.h>

const int xPin = 32;
const int yPin = 33;
const int swPin = 4;

// MAC do ESP8266 receptor (coloque o endereço correto)
uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// Estrutura compatível
typedef struct {
  bool esquerda;
  bool direita;
  bool cima;
  bool baixo;
  bool centro;
  bool botao;
} JoystickState;

JoystickState js;

// Função para suavizar leitura do joystick
int suavizar(int pin, int samples = 10) {
  long soma = 0;
  for (int i = 0; i < samples; i++) {
    soma += analogRead(pin);
    delayMicroseconds(100);
  }
  return soma / samples;
}

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Falha ao adicionar peer");
    return;
  }

  pinMode(swPin, INPUT_PULLUP);
  Serial.println("Transmissor iniciado");
}

void loop() {
  int x = suavizar(xPin);
  int y = suavizar(yPin);
  int sw = digitalRead(swPin);

  // Zonas mortas e limiares
  int centroMin = 1800;
  int centroMax = 2100;

  js.esquerda = (x < 1600 );
  js.direita  = (x > 2200 );
  js.cima     = (y < 1600 );
  js.baixo    = (y > 2200 );

  Serial.println(y);
  Serial.println(x);
  Serial.println(sw);


  js.centro = (x > centroMin && x < centroMax && y > centroMin && y < centroMax);
  js.botao  = (sw == LOW);

  // Envia via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&js, sizeof(js));

  if (result == ESP_OK) Serial.println("Dados enviados!");
  else Serial.println("Erro no envio!");

  delay(100);
}
