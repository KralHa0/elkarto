#include <WiFi.h>
#include <WebSocketsServer.h>

// UART bridge to STM32 (PA9 TX -> here RX, PA10 RX -> here TX), both 3.3V logic.
// GPIO20/21 are common on ESP32-C3 SuperMini clones for UART0 -- VERIFY against
// your specific board's silkscreen before flashing.
static const int STM32_RX_PIN = 20;
static const int STM32_TX_PIN = 21;
static const uint32_t STM32_BAUD = 115200;

static const char *AP_SSID = "gokart";
static const char *AP_PASSWORD = "gokart123"; // WPA2, min 8 chars -- change before use

WebSocketsServer webSocket(81);
static String lineBuf;

// STM32 telemetry lines are "timestamp,speed,gas,brake,servo_pos,motor_pos,target,output"
// (8 numeric fields, 7 commas). USART1 also carries stray debugPrint() boot text
// (e.g. "PCA: OK") -- only forward lines that actually look like telemetry so that
// text doesn't corrupt the WebSocket client's stream.
static bool isTelemetryLine(const String &line) {
    int commaCount = 0;
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == ',') { commaCount++; continue; }
        if (!isDigit(c) && c != '.' && c != '-') return false;
    }
    return commaCount == 7;
}

void webSocketEvent(uint8_t clientId, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        // Forward client commands (e.g. "K,1.5,0.02,0.01" gain updates) straight to the STM32
        Serial1.write(payload, length);
        Serial1.write('\n');
    }
}

void setup() {
    Serial1.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);

    WiFi.softAP(AP_SSID, AP_PASSWORD);

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

static unsigned long lastInfoMs = 0;

void loop() {
    webSocket.loop();

    while (Serial1.available()) {
        char c = Serial1.read();
        if (c == '\n') {
            lineBuf.trim();
            if (isTelemetryLine(lineBuf)) {
                webSocket.broadcastTXT(lineBuf);
            }
            lineBuf = "";
        } else if (c != '\r') {
            lineBuf += c;
            if (lineBuf.length() > 128) lineBuf = ""; // guard against noise/garbage
        }
    }

    unsigned long now = millis();
    if (now - lastInfoMs >= 2000) {
        lastInfoMs = now;
        String info = "I," + String(now) + "," + String(ESP.getFreeHeap()) + "," + String(WiFi.softAPgetStationNum());
        webSocket.broadcastTXT(info);
    }
}
