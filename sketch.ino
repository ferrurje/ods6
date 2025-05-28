#include <WiFi.h>
#include <PubSubClient.h>

// Definições de pinos
#define TRIGGER_PIN 5
#define ECHO_PIN 18
#define RELAY_PIN 19

// Configurações Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações do broker MQTT
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_client_id = "esp32-nivel-agua";
const char* mqtt_topic_pub = "sensor/agua/nivel";
const char* mqtt_topic_sub = "atuador/agua/valvula";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

// Variáveis para estatísticas
int sensorCount = 0;
int relayCount = 0;
unsigned long sensorTimes[4];  // Para 4 medições
unsigned long relayTimes[4];   // Para 4 medições

//Conectando ao WiFi de acesso aberto
void setup_wifi() {
  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Conectado!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // INÍCIO DA MEDIÇÃO DO RELÉ
  unsigned long startTime = micros();
  
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  payload[length] = '\0';
  String message = String((char*)payload);
  message.trim();

  Serial.print("Comando recebido: ");
  Serial.println(message);

  if (String(topic) == mqtt_topic_sub) {
    if (message == "1") {
      digitalWrite(RELAY_PIN, HIGH);
      
      // FIM DA MEDIÇÃO DO RELÉ
      unsigned long endTime = micros();
      unsigned long responseTime = endTime - startTime;
      
      // Armazenar para estatísticas (se ainda não coletamos 4 medições)
      if (relayCount < 4) {
        relayTimes[relayCount] = responseTime;
        relayCount++;
        
        Serial.print("Relé ligado. Tempo de resposta: ");
        Serial.print(responseTime);
        Serial.println(" μs");
        Serial.print("Medição RELÉ #");
        Serial.print(relayCount);
        Serial.print(": ");
        Serial.print(responseTime / 1000.0, 2);
        Serial.println(" ms");
        Serial.println("---");
        
        // Se completou 4 medições do relé, mostrar estatísticas
        if (relayCount == 4) {
          printRelayStatistics();
        }
      }
      
    } else if (message == "0") {
      digitalWrite(RELAY_PIN, LOW);
      
      // FIM DA MEDIÇÃO DO RELÉ
      unsigned long endTime = micros();
      unsigned long responseTime = endTime - startTime;
      
      Serial.print("Relé desligado. Tempo de resposta: ");
      Serial.print(responseTime);
      Serial.println(" μs");
      Serial.print("Tempo em ms: ");
      Serial.print(responseTime / 1000.0, 2);
      Serial.println(" ms");
      Serial.println("---");
      
    } else {
      Serial.println("Comando inválido (use 1 ou 0)");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("conectado!");
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== SISTEMA DE MEDIÇÃO DE PERFORMANCE ===");
  Serial.println("Coletando 4 medições de cada componente...");
  Serial.println("==========================================");
  
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;

    // INÍCIO DA MEDIÇÃO DO SENSOR
    unsigned long sensorStartTime = micros();
    
    // Disparo do pulso ultrassônico
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    
    // FIM DA MEDIÇÃO DO SENSOR
    unsigned long sensorEndTime = micros();
    unsigned long sensorTime = sensorEndTime - sensorStartTime;
    
    float distance = duration * 0.034 / 2.0;

    // Armazenar medição do sensor (se ainda não coletamos 4)
    if (sensorCount < 4) {
      sensorTimes[sensorCount] = sensorTime;
      sensorCount++;
      
      Serial.print("SENSOR - Distância: ");
      Serial.print(distance, 1);
      Serial.print(" cm | Tempo: ");
      Serial.print(sensorTime);
      Serial.print(" μs | ");
      Serial.print(sensorTime / 1000.0, 2);
      Serial.println(" ms");
      Serial.print("Medição SENSOR #");
      Serial.println(sensorCount);
      Serial.println("---");
      
      // Se completou 4 medições do sensor, mostrar estatísticas
      if (sensorCount == 4) {
        printSensorStatistics();
      }
    } else {
      // Apenas mostrar leitura normal após coletar dados
      Serial.print("Distância: ");
      Serial.print(distance, 1);
      Serial.println(" cm");
    }

    char msg[16];
    dtostrf(distance, 6, 2, msg);
    client.publish(mqtt_topic_pub, msg);

    // Aciona relé automaticamente com base na distância
    if (distance > 10.0) {
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      digitalWrite(RELAY_PIN, LOW);
    }
  }
}

void printSensorStatistics() {
  Serial.println("\n========== ESTATÍSTICAS DO SENSOR HC-SR04 ==========");
  Serial.println("Medições coletadas:");
  
  unsigned long total = 0;
  for (int i = 0; i < 4; i++) {
    Serial.print("Medição ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(sensorTimes[i]);
    Serial.print(" μs (");
    Serial.print(sensorTimes[i] / 1000.0, 2);
    Serial.println(" ms)");
    total += sensorTimes[i];
  }
  
  unsigned long average = total / 4;
  Serial.print("\nTempo médio: ");
  Serial.print(average);
  Serial.print(" μs (");
  Serial.print(average / 1000.0, 2);
  Serial.println(" ms)");
  Serial.println("==================================================\n");
}

void printRelayStatistics() {
  Serial.println("\n========== ESTATÍSTICAS DO RELÉ (MQTT) ==========");
  Serial.println("Medições coletadas:");
  
  unsigned long total = 0;
  for (int i = 0; i < 4; i++) {
    Serial.print("Medição ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(relayTimes[i]);
    Serial.print(" μs (");
    Serial.print(relayTimes[i] / 1000.0, 2);
    Serial.println(" ms)");
    total += relayTimes[i];
  }
  
  unsigned long average = total / 4;
  Serial.print("\nTempo médio: ");
  Serial.print(average);
  Serial.print(" μs (");
  Serial.print(average / 1000.0, 2);
  Serial.println(" ms)");
  Serial.println("===============================================\n");
}