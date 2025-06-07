#define BLYNK_TEMPLATE_ID "TMPL6wSHAqmzd"
#define BLYNK_TEMPLATE_NAME "DH22"
#define BLYNK_AUTH_TOKEN "EFvMDVrTAZJljyyELYSHWk56wEizmvlQ"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>  // Thư viện cho DS18B20

// Thông tin WiFi
char ssid[] = "RegG";
char pass[] = "zzzzzzzz";

// Màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Cảm biến siêu âm
#define TRIG_PIN 5  
#define ECHO_PIN 18 

// Cảm biến độ đục nước
#define TURBIDITY_PIN 34 

// Cảm biến DS18B20
#define ONE_WIRE_BUS 19  // chân kết nối với DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LED cảnh báo
#define LED_PIN 12  

// Mực nước tối đa (10 cm)
const float MAX_HEIGHT = 10.0;

void setup() {
    Serial.begin(115200);

    // Kết nối WiFi và Blynk
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Cấu hình chân
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(TURBIDITY_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Khởi động DS18B20
    sensors.begin();

    // Khởi động màn hình OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
}

// Đọc khoảng cách từ cảm biến siêu âm
float readUltrasonic() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * 0.034 / 2;
    return distance;
}

// Đọc độ đục nước
int readTurbidity() {
    return analogRead(TURBIDITY_PIN);
}

// Đọc nhiệt độ từ DS18B20
float readTemperature() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    return tempC == DEVICE_DISCONNECTED_C ? -99 : tempC;
}

void loop() {
    Blynk.run(); // Chạy Blynk

    // Đọc dữ liệu cảm biến
    float distance = readUltrasonic();
    int turbidity = readTurbidity();
    float temperature = readTemperature();

    // Tính mực nước
    float waterLevel = MAX_HEIGHT - distance;
    if (waterLevel < 0) waterLevel = 0; // Đảm bảo không âm

    // Tính độ đục (áp dụng công thức chuyển đổi mới)
    float turbidityMapped = 997.0 - ((turbidity * 1000.0) / 4095.0);
    if (turbidityMapped < 0) turbidityMapped = 0; // Giới hạn về 0
    if (turbidityMapped > 1000) turbidityMapped = 1000; // Giới hạn về 1000

    // Điều khiển LED cảnh báo