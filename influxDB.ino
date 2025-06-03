#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "FA099";
const char* password = "12345678";

const char* influxHost = "us-east-1-1.aws.cloud2.influxdata.com"; // ajuste conforme sua região

const int httpsPort = 443;

// InfluxDB configs
String org = "sua org";
String bucket = "seu bucket";
String token = "seu token";


// Exemplo: "sensor_umidade,sensor=esp32 umidade=45.3"
String getLineProtocol(float umidade) {
  return "sensor_umidade,sensor=esp32 umidade=" + String(umidade, 2);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
}

void loop() {
  float umidade = random(300, 600) / 10.0; // Simulação de umidade (30~60%)

  WiFiClientSecure client;
  client.setInsecure(); // para simplificar, sem verificação de certificado (não recomendado para produção)

  Serial.println("Conectando ao InfluxDB...");

  if (!client.connect(influxHost, httpsPort)) {
    Serial.println("Conexão falhou");
    return;
  }

  String data = getLineProtocol(umidade);
  String url = "/api/v2/write?org=" + org + "&bucket=" + bucket + "&precision=s";

  // Monta a requisição HTTP
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: " + String(influxHost));
  client.println("Authorization: Token " + token);
  client.println("Content-Type: text/plain");
  client.println("Content-Length: " + String(data.length()));
  client.println();
  client.println(data);

  // Lê resposta
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // fim do cabeçalho
  }

  String response = client.readString();
  Serial.println("Resposta:");
  Serial.println(response);

  client.stop();
  delay(10000); // envia a cada 10s
}
