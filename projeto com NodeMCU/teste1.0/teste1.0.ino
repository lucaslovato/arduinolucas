/*
 * @author: Lucas Lovato da Cunha Silveira
 * email: lucas.lovato@gmail.com
 */
//===============================================================================================
//                                     INCLUDE LIBRARIES
//===============================================================================================
#include <ESP8266WiFi.h>
#include <ConfigManager.h>
#include <PubSubClient.h>
#include "HX711.h"
//===============================================================================================
//                                     MQTT DEFINITIONS
//===============================================================================================
#define TOPICO_SUBSCRIBE "labtic"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "hardware_envia"    //tópico MQTT de envio de informações para Broker

//IMPORTANTE: recomendamos fortemente alterar os nomes
//            desses tópicos. Caso contrário, há grandes
//            chances de você controlar e monitorar o NodeMCU
//            de outra pessoa.
#define ID_MQTT  "HomeAut"     //ID DO MQTT (DEVE SER UNIQUE) ---- (para identificação de sessão)
//IMPORTANTE: este deve ser único no broker (ou seja,
//            se um client MQTT tentar entrar com o mesmo
//            id de outro já conectado ao broker, o broker
//            irá fechar a conexão de um deles).
//===============================================================================================
//                                      I/O PORTS (DIGITAL)
//===============================================================================================
#define D0    16  // internal LED
#define D1    5   // PIN_RESET
#define D2    4   // BUTTON_RESET
#define D3    0   // livre
#define D4    2   // used LED
#define D5    14  // HX711_DATA 
#define D6    12  // HX711_CLK  
#define D7    13  // livre
#define D8    15  // livre
#define D9    3   // livre
#define D10   1   // livre
//===============================================================================================
//                                   DEFINE CONSTANTtS
//===============================================================================================
/*

                       espaço reservado para definir possíveis constanstes

*/
//===============================================================================================
//                                   DEFINE FUNCTIONS
//===============================================================================================
void initScale(void);

void callback(char *, byte *, unsigned int length);

void reconnectMQTT(void);

void InitOutput(void);
//===============================================================================================
//                                      MOTOR INIT
//===============================================================================================
//const int motorPin = DX
//int Speed; // variable to store Speed, by default 0 PWM
//int flag;
//===============================================================================================
//                                      LOAD CELL INIT
//===============================================================================================
#define HX711_DATA D5
#define HX711_CLK  D6
HX711 scale(HX711_DATA, HX711_CLK, 128);
ADC_MODE(ADC_VCC);
float weight;

/*Função: Inicialização da balança
Parâmetros: nenhum
Retorno: nenhum
*/
void initScale() {
  scale.set_scale();
  scale.tare();
  scale.read();
}
//===============================================================================================
//                                      WIFI CONSTANTS
//===============================================================================================
namespace {
  const char *AP_NAME = "Teste LabTIC"; // Assigns your Access Point name
//MQTT
//const char* BROKER_MQTT = "iot.eclipse.org"; //URL do broker MQTT que se deseja utilizar
//int BROKER_PORT = 1883; // Porta do Broker MQTT
  const char *BROKER_MQTT = "192.168.0.180";
  int BROKER_PORT = 1883;
//  const char * DEVICE_LABEL = "my-device"; // Assigns your Device Label
//  const char * VARIABLE_LABEL = "my-variable"; // Assigns your Variable Label
  int SENSOR = A0;
}

char topic[150];
char payload[50];
String clientMac = "";
unsigned char mac[6];

struct Config {
  char name[20];
  bool enabled;
  int8 hour;
} config;
//===============================================================================================
//                                      GLOBAL INSTANCES INIT
//===============================================================================================
WiFiClient espClient;
PubSubClient client(espClient);
ConfigManager configManager;
//===============================================================================================
//                                      AUX FUNCTIONS INIT
//===============================================================================================
/*Função: Funcão de callback
Parâmetros: topico, payload
Retorno: nenhum
*/
void callback(char *topic, byte *payload, unsigned int length) {
  String msg;
  //teste se esta entrando no callback
  //Serial.println("chegou aqui");

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char) payload[i];
    msg += c;
  }
  //printa a msg q recebeu
  //Serial.println(msg);

  //toma ação dependendo da string recebida:
  //verifica se deve colocar nivel alto de tensão na saída D0:
  //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
  //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
  if (msg.equals("H")) {
    digitalWrite(D4, LOW);
    Serial.println("era para o led ligar aqui");
    Serial.println(D4);
  }

  //verifica se deve colocar nivel alto de tensão na saída D0:
  if (msg.equals("L")) {
    digitalWrite(D4, HIGH);
    Serial.println("era para o led desligar aqui");
  }
  if (msg.equals("thanos")) {
    Serial.println("OSVALDO FOI MORTO POR THANOS PELO BEM DO UNIVERSO");
  }
  if (msg.equals("teste")) {
    //todo: testar chamada de função vindo do outro arquivo!
  }
}
//===============================================================================================
//                                  função anterior de reconect
//===============================================================================================
//void reconnect() {
//  while (!client.connected()) {
//    Serial.print("Attempting MQTT connection...");
//    // Attempt to connect
//    if (client.connect(clientMac.c_str(), TOKEN, NULL)) {
//      Serial.println("connected");
//      break;
//      } else {
//        configManager.reset();
//        Serial.print("failed, rc=");
//        Serial.print(client.state());
//        Serial.println(" try again in 3 seconds");
//        for(uint8_t Blink=0; Blink<=3; Blink++){
//          digitalWrite(LED, LOW);
//          delay(500);
//          digitalWrite(LED, HIGH);
//          delay(500);
//        }
//      }
//  }
//}
//
//String macToStr(const uint8_t* mac) {
//  String result;
//  for (int i = 0; i < 6; ++i) {
//    result += String(mac[i], 16);
//    if (i < 5)result += ':';
//  }
//  return result;
//}
//===============================================================================================
/*Função: Conectar ao MQTT quando não conectado fica tentando conexão
Parâmetros: nenhum
Retorno: nenhum
*/
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (client.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      client.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentativa de conexao em 1s");
      delay(1000);
    }
  }
}

//===============================================================================================
//                                      MAIN FUNCTIONS
//===============================================================================================
void setup() {
  Serial.begin(115200);
  /* Declare PINs as input/outpu */
  InitOutput();
  initScale();
  /* Access Point configuration */
  //reconnect()

  configManager.setAPName(AP_NAME);
  configManager.addParameter("name", config.name, 20);
  configManager.addParameter("enabled", &config.enabled);
  configManager.addParameter("hour", &config.hour);
  configManager.begin(config);

  /* Set Sets the server details */
  client.setServer(BROKER_MQTT, BROKER_PORT);
  client.setCallback(callback);
  /* Build the topic request */
  //  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
}
//===============================================================================================

//void EnviaEstadoOutputMQTT(void)
//{
//    if (EstadoSaida == '0')
//      client.publish(TOPICO_PUBLISH, "L");
//
//    if (EstadoSaida == '1')
//      client.publish(TOPICO_PUBLISH, "H");
//
//    Serial.println("- Estado da saida D0 enviado ao broker!");
//    Serial.println(EstadoSaida);
//    delay(1000);
//}
//===============================================================================================
/*Função: inicializa o output em nível lógico baixo
Parâmetros: nenhum
Retorno: nenhum
*/
void InitOutput(void) {
  //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
  //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)(led interno do nodeMCU(ESP8266))
  pinMode(SENSOR, INPUT);
  pinMode(PIN_RESET, INPUT);
  pinMode(LED, OUTPUT);
  //pinMode(motorPin, OUTPUT);

  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);
  delay(500);
  pinMode(D2, OUTPUT);
}

//===============================================================================================
//                                      MAIN LOOP
//===============================================================================================
void loop() {
  configManager.reset();
  configManager.loop();
  /* MQTT client reconnection */
  if (!client.connected()) {
    reconnectMQTT();
  }
  /* Build the payload request */
  //sprintf(payload, "{\"%s\": %d}", VARIABLE_LABEL, value);
  /* Publish sensor value to Ubidots */
  //client.publish(topic, payload);

  client.loop();
}
//===============================================================================================
//                                    SCALE TEST
//===============================================================================================
//   Serial.println("5s para posicionar o celular");
//     for(int i = 5; i >= 0; i--){
//      Serial.println(i);
//      delay(1000);
//    }
//     Serial.println("lendo....");
//     delay(1000);
//para descobrir o peso preciso pegar esse valor que ira printar e dividir pelo peso conhecido
//(precisa mais precisao)
//     Serial.println("\n\n");
//     Serial.println(scale.get_units(10));
//     Serial.println("\n\n");
//===============================================================================================

/*
 * obs, o NODE controlará as funcoes de quando e como o nodeMCU irá abrir, o hardware basicamente sabe
 * quando abrir e quando parar de abrir somemte
 */
