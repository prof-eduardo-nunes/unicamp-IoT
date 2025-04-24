

// Estação Metereológica 
// FA099 - Internet das Coisas na Agricultura
// código de Claudio Umezu e Eduardo Nunes



// Inclui bibliotecas
#include <WiFi.h>
//#include <WiFiClient.h>
//#include <BlynkSimpleEsp32.h>
//#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <DHT.h>
#include <AS5600.h>
#include <SimpleTimer.h>


#include <AsyncMQTT_ESP32.h>

#define MQTT_HOST ""
#define MQTT_PORT 1883

static const char mqttUser[] = "";
static const char mqttPassword[] = "";

//#define MQTT_HOST         IPAddress(192, 168, 2, 110)
#define MQTT_HOST         "broker.emqx.io"        // Broker address
#define MQTT_PORT         1883


const char *Topico = "estacao/RA12345678/#";
const char *Topico_temp = "estacao/RA12345678/Temperatura";
const char *Topico_umid = "estacao/RA12345678/Umidade";
const char *Topico_press = "estacao/RA12345678/Pressao";
const char *Topico_lum = "estacao/RA12345678/Luminosidade";
const char *Topico_veloc = "estacao/RA12345678/Velocidade";
const char *Topico_dir = "estacao/RA12345678/Direcao";

AsyncMqttClient mqttClient;


//Configuracao do Wi-Fi

#define WIFI_SSID         "FA099"    
#define WIFI_PASS         "Fa099@123"

//Define do pino do ESP para o sensor DHT11 (Umidade)
#define DHT_DATA_PIN 32
#define DHTTYPE DHT11
DHT dht(DHT_DATA_PIN, DHTTYPE);

// Define o pino do encoder (Velocidade)
#define EncPIN 35

// Sensor de pressao e temperatura
Adafruit_BMP280 bmp;

// Sensor de luminosidade
BH1750 lightMeter;

// Sensor de direcao do vento (Efeito Hall)
AS5600 as5600; 

// Define os objetos timer para acionar as interrupções
SimpleTimer TimerLeituras; // Define o intervalo entre leituras
SimpleTimer TimerEncoder; // Define intervalo para ler contador

// Define a variável para armazenar o intervalo de amostragem (em milisegundos)
int IntLeituras = 5000; // Faz uma leitura dos sensores a cada 5 segundos
int IntEncoder = 2000; // Faz a totalizacao dos pulsos a cada 2 segundos

// Define as variáveis para armazenar os valores lidos
int Umidade = 0;
int Luminosidade = 0;
int Pressao = 0;
float Temperatura = 0.0;
int Velocidade = 0;
int Rotacao = 0;
float Direcao = 0;

// Strings que contém o texto indicativo de direção
char *myStrings[] = {"N", "NE", "L", "SE", "S", "SO", "O", "NO"
                    };

// Variável para ler o status do encoder
bool EncSt = LOW;
bool EncAnt = LOW;

// Variável para armazenar a contagem de pulsos
int Cont = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Configura o pino do encoder como entrada
  pinMode(EncPIN, INPUT);
  pinMode(2, OUTPUT); // led da placa

  // Configura a interrupção para ser acionada a cada intervalo de amostragem
  TimerLeituras.setInterval(IntLeituras, RunLeituras);  
  TimerEncoder.setInterval(IntEncoder, ContaPulsos);  

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);  

  // Tenta conectar ao Wi-Fi e repete tentativa a cada 5 segundos
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(".");
    delay(5000);
  }

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();

  // Inicia DHT11
  dht.begin();

  // Inicia BH1750
  lightMeter.begin();

  // Inicia BMP280
  bmp.begin(0x76);

  // Inicia AS5600
  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  // default, just be explicit.

  // Mensagem de conexão do Blynk
//  Blynk.begin(auth,WIFI_SSID,WIFI_PASS);
}

void loop() {
  // Executa as tarefas programadas pelo timer
  TimerLeituras.run();
  TimerEncoder.run();
//  Blynk.run();  

  // Realiza a contagem de pulsos 
  EncSt = digitalRead(EncPIN);
  if (EncSt == HIGH && EncAnt == LOW) {
    Cont++;
    EncAnt = HIGH;
  }
  if (EncSt == LOW && EncAnt == HIGH) {
    EncAnt = LOW;
  }
}

void RunLeituras() {

  Umidade = dht.readHumidity();
  Luminosidade = lightMeter.readLightLevel();
  Pressao = bmp.readPressure()/100;
  Temperatura = bmp.readTemperature();
  // Conversao para graus e ajuste do zero
  Direcao = abs((as5600.rawAngle()*360.0/4095.0));

  if (Direcao > 337.5 || Direcao < 22.5){
//    Blynk.virtualWrite(V6, myStrings[0]);
  }
  if (Direcao > 22.5 && Direcao < 67.5){
//    Blynk.virtualWrite(V6, myStrings[1]);
  }
  if (Direcao > 67.5 && Direcao < 112.5){
//    Blynk.virtualWrite(V6, myStrings[2]);
  }
  if (Direcao > 112.5 && Direcao < 157.5){
//    Blynk.virtualWrite(V6, myStrings[3]);
  }
  if (Direcao > 157.5 && Direcao < 202.5){
//    Blynk.virtualWrite(V6, myStrings[4]);
  }
  if (Direcao > 202.5 && Direcao < 247.5){
//    Blynk.virtualWrite(V6, myStrings[5]);
  }
  if (Direcao > 247.5 && Direcao < 292.5){
//    Blynk.virtualWrite(V6, myStrings[6]);
  }
  if (Direcao > 292.5 && Direcao < 337.5){
//    Blynk.virtualWrite(V6, myStrings[7]);
  }
    Serial.print("Temperatura: ");
    Serial.print(Temperatura);
    Serial.println(" ºC"); 

    Serial.print("Umidade: ");
    Serial.print(Umidade);
    Serial.println(" % " );

    Serial.print("Pressão: ");
    Serial.print(Pressao);
    Serial.println(" hPa " );

    Serial.print("Luminosidade: ");
    Serial.print(Luminosidade);
    Serial.println(" lux  "); 

    Serial.print("Rotacao: ");
    Serial.print(Rotacao);
    Serial.println(" rpm  "); 

    Serial.print("Direcao: ");
    Serial.print(Direcao);
    Serial.println(" graus  "); 


   mqttClient.publish(Topico_temp, 0, false, (String(Temperatura)).c_str());
   mqttClient.publish(Topico_umid, 0, false, (String(Umidade)).c_str());
   mqttClient.publish(Topico_press, 0, false, (String(Pressao)).c_str());   
   mqttClient.publish(Topico_lum, 0, false, (String(Luminosidade)).c_str());      
   mqttClient.publish(Topico_veloc, 0, false, (String(Rotacao)).c_str()); 
   mqttClient.publish(Topico_dir, 0, false, (String(Direcao)).c_str());           
   

}  

// Rotina para integralizar os pulsos e converter para rpm
void ContaPulsos() {
  Rotacao = ((Cont * 6) ) * 1000.00 / IntEncoder;
  Cont = 0;    
}

void onMqttConnect(bool sessionPresent)
{
  Serial.print("Connected to MQTT broker: ");
  Serial.print(MQTT_HOST);
  Serial.print(", port: ");
  Serial.println(MQTT_PORT);

  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  Topico = "estacao/RA12345678/#";
  uint16_t packetIdSub = mqttClient.subscribe("estacao/RA12345678/#", 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);


}


void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties,
                   const size_t& len, const size_t& index, const size_t& total)
{
  

  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  coteudo: ");
  Serial.println(payload);

  String topico = topic;
  String conteudo = payload;  
  
  if (topico.indexOf("liga_led"))
  {
    if (conteudo == "1")
    {
      digitalWrite(2, HIGH);
    }
    else if (conteudo == "0")
    {
      digitalWrite(2, LOW);
    }
  }
}
