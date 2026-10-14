#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN WIFI Y MQTT
// ======================================================
const char* WIFI_SSID = "iPhone de Isolina";
const char* WIFI_PASSWORD = "rafa123456";
const char* MQTT_SERVER = "172.20.10.10";
const int MQTT_PORT = 1883;

const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";

// ======================================================
// CONFIGURACIÓN DE SENSORES (Resuelto)
// ======================================================
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LDRPIN 1 // Pin ADC para la fotoresistencia

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned long INTERVALO_ENVIO = 2000;
unsigned long ultimoEnvio = 0;

void conectarWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando a Mosquitto...");
    String clientId = "ESP32-" + String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" conectado");
    } else {
      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");
      delay(2000);
    }
  }
}

// ======================================================
// LEER Y PUBLICAR SENSORES (Resuelto)
// ======================================================
void publicarSensores() {
  // 1 y 2. Obtener temperatura y humedad
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Validar si la lectura del DHT falló
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error al leer el sensor DHT!");
    return;
  }

  // 3. Obtener lectura LDR (El ADC del ESP32 va de 0 a 4095)
  int ldrRaw = analogRead(LDRPIN);
  // Convertir lectura ADC a un porcentaje (0% a 100%)
  int luzPorcentaje = map(ldrRaw, 0, 4095, 0, 100);

  // 4. Preparar valores (Convertir numéricos a String para MQTT)
  String tempStr = String(temp, 2);
  String humStr = String(hum, 2);
  String luzStr = String(luzPorcentaje);

  // 5. Publicar en MQTT
  mqttClient.publish(TOPIC_TEMPERATURA, tempStr.c_str());
  mqttClient.publish(TOPIC_HUMEDAD, humStr.c_str());
  mqttClient.publish(TOPIC_LUZ, luzStr.c_str());

  // 6. Mostrar en Monitor Serial
  Serial.println("------------------------------");
  Serial.print("Temperatura: "); Serial.print(tempStr); Serial.println(" °C");
  Serial.print("Humedad: "); Serial.print(humStr); Serial.println(" %");
  Serial.print("LDR ADC: "); Serial.print(ldrRaw); 
  Serial.print(" | Luz: "); Serial.print(luzStr); Serial.println(" %");
}

void setup() {
  Serial.begin(115200);

  // Inicializar sensores
  dht.begin();
  pinMode(LDRPIN, INPUT); // Buena práctica aunque analogRead lo maneja

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { conectarWiFi(); }
  if (!mqttClient.connected()) { conectarMQTT(); }
  
  mqttClient.loop();

  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = millis();
    publicarSensores();
  }
}