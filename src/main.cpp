#define BLYNK_DEVICE_NAME "Esp 32 KONTROL LED"
#define BLYNK_PRINT Serial

#define BLYNK_AUTH_TOKEN "y8E-GzIoQWj537tvayHjMZa2ndnykEpC"
#define BLYNK_TEMPLATE_ID "TMPL6wEcO7Hpi"
#define BLYNK_TEMPLATE_NAME "Anandas Template"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// 🔹 Koneksi ke Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

// 🔹 Inisialisasi LCD & Sensor
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define LDR_PIN 34

// 🔹 Output LED
#define LED_R 26

// 🔹 Variabel global
bool tampilanSuhu = true;
bool wifiConnected = false;

// --- 🔥 Fungsi untuk mengirim data ke Blynk (Semua dalam INT) ---
void sendSensor() {
  int suhu = (int) round(dht.readTemperature()); // Konversi ke int
  int kelembaban = (int) round(dht.readHumidity()); // Konversi ke int
  int cahaya = analogRead(LDR_PIN);
  int cahayaPersen = map(cahaya, 0, 4095, 0, 100);
  if (cahayaPersen < 0) cahayaPersen = 0;

  // 🔹 Cek apakah data valid
  if (isnan(suhu) || isnan(kelembaban)) {
    Serial.println("❌ Gagal membaca DHT22! Coba lagi...");
    return;
  }

  // 🔹 Pastikan suhu dalam rentang normal sebelum dikirim
  if (suhu < -10 || suhu > 50) {
    Serial.println("⚠️ Suhu tidak valid, cek sensor!");
    return;
  }

  // 🔹 Kirim data ke Blynk
  Blynk.virtualWrite(V4, suhu);
  Blynk.virtualWrite(V1, kelembaban);
  Blynk.virtualWrite(V0, cahayaPersen);

  // 🔹 Debug di Serial Monitor (Semua tanpa desimal)
  Serial.print("🌡 Suhu: "); Serial.print(suhu); Serial.print(" C, ");
  Serial.print("💧 Kelembaban: "); Serial.print(kelembaban); Serial.print(" %, ");
  Serial.print("☀️ Cahaya: "); Serial.print(cahayaPersen); Serial.println(" %");
}

// --- 💡 Mengontrol LED dari Blynk ---
BLYNK_WRITE(V2) {
  int nilaiBacaIO = param.asInt();
  digitalWrite(LED_R, nilaiBacaIO);
  Blynk.virtualWrite(V3, nilaiBacaIO);
}

// 🔄 Cek & Reconnect WiFi
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🔴 WiFi Terputus! Mencoba Reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();
    wifiConnected = false;
  } else if (!wifiConnected) {
    Serial.println("✅ WiFi Kembali Tersambung!");
    wifiConnected = true;
  }
}

void setup() {
  Serial.begin(115200);

  // 🔹 Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan...");

  // 🔹 Inisialisasi DHT
  dht.begin();
  delay(2000);

  // 🔹 Koneksi ke WiFi & Blynk
  WiFi.begin(ssid, pass);
  int waktuTunggu = 0;
  while (WiFi.status() != WL_CONNECTED && waktuTunggu < 20) {
    delay(500);
    Serial.print(".");
    waktuTunggu++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Tersambung!");
    wifiConnected = true;
  } else {
    Serial.println("\n❌ WiFi Gagal Tersambung!");
  }

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, sendSensor);
  timer.setInterval(5000L, checkWiFi);  // Cek WiFi setiap 5 detik

  pinMode(LED_R, OUTPUT);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Siap!");
}

void loop() {
  Blynk.run();
  timer.run();

  // 🔹 Baca data sensor (Semua diubah ke int)
  int suhu = (int) round(dht.readTemperature());
  int kelembaban = (int) round(dht.readHumidity());
  int cahaya = analogRead(LDR_PIN);
  int cahayaPersen = map(cahaya, 0, 4095, 0, 100);
  if (cahayaPersen < 0) cahayaPersen = 0;

  // 🔹 Cek apakah sensor gagal
  if (isnan(suhu) || isnan(kelembaban)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT22 Error!");
    delay(2000);
    return;
  }

  // 🔹 Ganti tampilan setiap 3 detik
  lcd.clear();
  if (tampilanSuhu) {
    lcd.setCursor(0, 0);
    lcd.print("Suhu: ");
    lcd.print(suhu);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Kelembaban: ");
    lcd.print(kelembaban);
    lcd.print(" %");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Cahaya: ");
    lcd.print(cahayaPersen);
    lcd.print(" %");
  }

  delay(3000);
  tampilanSuhu = !tampilanSuhu;
}
