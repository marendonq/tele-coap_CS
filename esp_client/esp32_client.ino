#include <WiFi.h>
#include <WiFiUDP.h>
#include <DHT.h>
#include "message.h"     // tu CoAP minimal con extern "C"

// ====================== CONFIGURACIÓN ======================
const char* WIFI_SSID     = "RYQ";
const char* WIFI_PASSWORD = "70552339*";

const char* SERVER_IP     = "100.25.214.123";
const int   SERVER_PORT   = 5683;
const char* RESOURCE_PATH = "/sensors/temp";

const bool  USE_CONFIRMABLE = true;   // true = CON (ACK)
const uint32_t RECV_WINDOW_MS = 500;  // ventana de respuesta
const uint32_t MEASURE_INTERVAL_MS = 7000; // DHT11 → máx 1 Hz aprox.

// --- DHT11 físico ---
#define DHT_PIN   4
#define DHT_TYPE  DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ===========================================================
volatile unsigned g_seq = 0;

// --------- Utilidades ---------
static void add_uri_path_options(CoapMessage *msg, const char *path) {
  const char *p = path;
  while (*p == '/') p++;
  while (*p) {
    const char *start = p;
    while (*p && *p != '/') p++;
    size_t len = (size_t)(p - start);
    if (len) coap_add_option(msg, 11, (const unsigned char*)start, (unsigned short)len);
    while (*p == '/') p++;
  }
}
static unsigned short rand_u16() { return (unsigned short)(esp_random() & 0xFFFF); }

static String device_id() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return "esp32-" + mac;
}

// --------- Envío CoAP (idéntico al POSIX) ---------
static int send_one_udp_esp(const char *host,
                            int port,
                            const char *path,
                            const char *device_id_str,
                            unsigned seq,
                            double temp_c,
                            bool confirmable) {
  WiFiUDP udp;
  udp.begin(0);

  unsigned char buffer[1024];
  CoapMessage msg;
  coap_message_init(&msg);

  msg.type = confirmable ? 0 : 1;   // 0=CON,1=NON
  msg.code = 2;                     // POST
  msg.message_id = rand_u16();

  msg.tkl = 2;
  msg.token[0] = (unsigned char)(esp_random() & 0xFF);
  msg.token[1] = (unsigned char)(esp_random() & 0xFF);

  add_uri_path_options(&msg, path);
  unsigned char cf = 50;            // application/json
  coap_add_option(&msg, 12, &cf, 1);

  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"id\":\"%s\",\"seq\":%u,\"temp_c\":%.1f}",
           device_id_str, seq, temp_c);
  coap_set_payload(&msg, (const unsigned char*)payload, (int)strlen(payload));

  int len = coap_serialize(&msg, buffer, sizeof(buffer));
  if (len <= 0) {
    Serial.printf("[%s] Error al serializar CoAP\n", device_id_str);
    return -1;
  }

  Serial.printf("[SEND] -> %s:%d  %s  len=%d  (%s)\n",
                host, port, path, len, confirmable ? "CON" : "NON");

  if (!udp.beginPacket(host, port)) {
    Serial.println("[CoAP] beginPacket() falló");
    return -1;
  }
  udp.write(buffer, len);
  if (!udp.endPacket()) {
    Serial.println("[CoAP] endPacket() falló");
    return -1;
  }

  // Espera de respuesta (ACK / 2.xx)
  uint32_t t0 = millis();
  while ((millis() - t0) < RECV_WINDOW_MS) {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
      uint8_t rbuf[1024];
      int rlen = udp.read(rbuf, sizeof(rbuf));
      if (rlen > 0) {
        CoapMessage resp;
        if (coap_parse(&resp, rbuf, rlen) == 0) {
          Serial.printf("  <- resp code=%d len=%d\n", resp.code, resp.payload_len);
          for (int i = 0; i < resp.option_count; i++)
            if (resp.options[i].value) free(resp.options[i].value);
        } else {
          Serial.printf("  <- recibido %d bytes (no CoAP válido)\n", rlen);
        }
      }
      break;
    }
    delay(5);
  }
  return 0;
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== ESP32 -> CoAP (DHT11 físico) ===");

  // WiFi
  Serial.printf("Conectando a WiFi: %s ...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - t0 > 25000) {
      Serial.println("\n[WiFi] Timeout. Revisa credenciales.");
      while (true) delay(1000);
    }
  }
  Serial.printf("\n✓ WiFi conectado. IP: %s\n", WiFi.localIP().toString().c_str());

  // DHT11
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();
  delay(2000); // warm-up
  Serial.printf("DHT iniciado en GPIO %d (DHT11)\n", DHT_PIN);
  Serial.printf("DeviceID: %s\n", device_id().c_str());
}

// ====================== LOOP ======================
void loop() {
  static float last_ok_t = NAN;

  // Lectura con reintentos
  float t = NAN;
  for (int i = 0; i < 3; i++) {
    t = dht.readTemperature();
    if (!isnan(t)) break;
    delay(80);
  }

  if (isnan(t)) {
    Serial.println("[DHT] Lectura inválida (NaN).");
    if (!isnan(last_ok_t)) {
      t = last_ok_t;
      Serial.printf("[DHT] Usando último válido: %.1f°C\n", t);
    } else {
      delay(MEASURE_INTERVAL_MS);
      return;
    }
  } else {
    last_ok_t = t;
  }

  // Construir JSON y enviar (misma lógica que tu POSIX)
  unsigned seq = ++g_seq;
  String id = device_id();
  char json[160];
  snprintf(json, sizeof(json),
           "{\"id\":\"%s\",\"seq\":%u,\"temp_c\":%.1f}",
           id.c_str(), seq, t);

  Serial.printf("[DATA] %s\n", json);
  send_one_udp_esp(SERVER_IP, SERVER_PORT, RESOURCE_PATH, id.c_str(), seq, (double)t, USE_CONFIRMABLE);

  delay(MEASURE_INTERVAL_MS);
}
