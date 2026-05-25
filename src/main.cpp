#include "WiFiManager.h"
#include "Sensor.h"
#include "Motor.h"
#include "Display.h"
#include "Potentiometer.h"
#include "AWSShadow.h"

// ========== Credencials WIFI and AWS ==========
const char* ssid       = "HONORX7";
const char* password   = "71767607";

const char* awsEndpoint = "a2nswqoqjqeq5-ats.iot.us-east-1.amazonaws.com";
const int   awsPort     = 8883;
const char* thingName   = "Esp32Ventilador";

// ========== Certificates ==========
const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char* deviceCert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUVNkDbhl2BHfyLF6e58Vl9EpAwbowDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDUwMzIxMTQ1
NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAON/79OqxoPuWm4phP5K
OCJLb0nIMIDVnQ75MeW2d76DA8gFNa3GS6yHjO0Z/ia6u6Mx3IxDUE/RUBu/5Z6X
TOW+Z7K2BN5V7rpxLkyQdo7CeHHjpRSDM6r5PzKcaZHHI3bvG7OPZx4mlsO1yEA8
zZP+qLhGCwXu0x5PNFWu739sRLbXvfk34NzoXTM1rNNueIHpGvRInRn71uNcVlvC
3dwCboJlWLV/Rt101d8gZhF5kcYzZqXXwg+p1QD8zkJ7wY2LSJ21z9Eb3Q1QD2pt
2HqQcPSANay39fRySziHEigXyl3lpFgmZVjLKjCRz9paChyrUZZosAwPoQFCjsx0
/M8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUiGRKebkcG5MuUxeC5pN+jedL8C4wHQYD
VR0OBBYEFNldMLLA+1zmCr+0w7TjTUJqjZFiMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBSWzdaeI+Gr7fTkkUMZAn4+ICI
LkqT6kVbQKs1oP2KUHIv6UH9oZv/Mw1gWQd9D1YatRUtLhpw0Ciy72Hybv25keBG
w/vgjKaF14/KNvCLZ7yzMW2a4jvCH9AdPg/Po6AGrRhnYJICeqrnr1tCe7Z4A+2A
ZKbRHgheugOKIcaaAWaf3S/LkNS9H3M/cNi5tDcFlV8oYfmfGWfvXbD8/gdlLDz6
nIx0Eq3yKkTs3Zu1JNu4yI6zacshPXkGjJM6y+qHyzyFWLHeANGbC5dnrard3pIE
b42hanFAVN3TzGPEnUFLK+BIXq1AQTGsl9bcnW9C3wYSqsaZx2bptKwFT4UM
-----END CERTIFICATE-----

)EOF";

const char* privateKey = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA43/v06rGg+5abimE/ko4IktvScgwgNWdDvkx5bZ3voMDyAU1
rcZLrIeM7Rn+Jrq7ozHcjENQT9FQG7/lnpdM5b5nsrYE3lXuunEuTJB2jsJ4ceOl
FIMzqvk/Mpxpkccjdu8bs49nHiaWw7XIQDzNk/6ouEYLBe7THk80Va7vf2xEtte9
+Tfg3OhdMzWs0254geka9EidGfvW41xWW8Ld3AJugmVYtX9G3XTV3yBmEXmRxjNm
pdfCD6nVAPzOQnvBjYtInbXP0RvdDVAPam3YepBw9IA1rLf19HJLOIcSKBfKXeWk
WCZlWMsqMJHP2loKHKtRlmiwDA+hAUKOzHT8zwIDAQABAoIBACsmAsi0pT40oPYv
7ceFMCxg/lSSIZDt8wH6BZq6BvAdTURMqKey6ntoojP+MBQMarZDKL95rOvWmUkK
FnUD4VPZzzU4DKhWyBaYjN18sN/Wxp1jPdYsJEqTR4dD30oE6RcPtsNRuY8gZovs
GHUGkz80l6+LqV+Qsn4XK6fsrAQQmzX2FHg6LtLGR18RBQS3d/74YwvuJhE2SqIG
R6lMp0jYrY3dIHuhevudRdsqDm5f04B5iHt9b2nHt9k2lApdAOMDLbG9lRB8mYNF
blXQCpcCVwKorkK+gPKXfXS3654DTNh8xXaBansGdOgInGeZghDv4qnYNW/S5cXs
1sc/5bECgYEA/fQx6brdciqEngWugLEU+9AWdZeg1izoWXVW+zwWEsEvZizjbska
y2lQC5hv9lR0LMN7HONFm9brahzBpFpITSxxjqhTvIGxZzDAef+VbctAMekCd16E
RNsA/I5RZHn8c7X2iQVex5IABSWC6Xvc842yjNXG1BB5cU+rLBmWSFMCgYEA5VUt
dgjAMWKpugrg7NRdl2sXL9ajNHcZQoVGXH3fHcjlme6O1yqCQB3YEBXWwIurWrTu
OmqBxyt1Bf8UIVx186h7iiiOi3ZD/QLEMTdO33BQVAgUJxs7uvZr6QsP0TqhZX0v
nMJja5jjSKm8DnF6tLiy6twBj14REP1enD2A+hUCgYBUwdiaypw4C0rBtuG7395C
APxN+PJcKRbPLKOALFitn75jGp45jnMb6qUliZXsn9k6S2/ef1fvIDbn7nsOkHdL
Eoc/9pwj8SE1cIb88Ll9sHVXcw44qtn4ZSOaJyGxKCmCsPt8VceBUhAhp5hettFS
lDtAV+UJSApQMCfQCyYmMwKBgQC8lkf1KYsK0YyrPejrOFPigaEmqMW7DjmKd1h+
FNBWiO8un3t9OyqzGRelWqAn5HM7+09wYEdqhrUMAlAGgydIAIPfAC0ASHfuZsj5
g/brTUvxXldcNUvwGlZw5LTUJPe2IrT1nvDs+WPmTBYmdnEmjkArXFI4GG3GX+Te
bMFUZQKBgAx3vb8/ah97/PzcfCGa3LRVSvmaeACP/CMJKnYaO/8WGDWk1iSL3kCe
PDuIQvk3DWqmY03ERIpKFAmaCVwWvKav2i+q0gf4k7Dq++5M8DodHnrWhxNN6Lyh
dpn12fSlyav85985YQRsudAtvFNqlzDp4T+y0eunKmoDW7dpHPDR
-----END RSA PRIVATE KEY-----
)EOF";


// ========== Pins and objects ==========
#define POT_PIN 35

WiFiManager wifi(ssid, password);
Display display(0x3C, 21, 22);
Sensor sensor(4, DHT11);
Motor motor(18, 5, 0, 5000, 8);
Potentiometer pot(POT_PIN);
AWSShadow aws(thingName, awsEndpoint, awsPort,
              rootCA, deviceCert, privateKey,
              motor, sensor, display);

// ========== variable of state ==========
float lastReportedTemp = -100.0;
float lastReportedHum  = -1.0;
int   lastReportedSpeed = -1;

// Potentiometer debouncing
int   lastStableRange = -1;
int   candidateRange = -1;
unsigned long lastPotChangeTime = 0;
const unsigned long potDebounceDelay = 500;

// Auto‑mode control
unsigned long lastAutoUpdate = 0;
const unsigned long autoUpdateInterval = 2000;

// NEW: Periodic reporting (every 10 minutes)
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 600000;   // 10 minutes in milliseconds

int rangeToSpeed(int range) {
  switch(range) {
    case 0: return 0;
    case 1: return 25;
    case 2: return 50;
    case 3: return 75;
    case 4: return 100;
    default: return 0;
  }
}

void setup() {
  Serial.begin(115200);
  
  if (!display.begin()) { while(1); }
  display.showMessage("Iniciando...", "", 0);

  if (!wifi.connect()) {
    display.showMessage("WiFi FAIL", "", 0);
    while(1);
  }
  display.showMessage("WiFi OK", wifi.localIP().c_str(), 0);
  delay(1000);

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) { delay(500); }

  motor.begin();
  sensor.begin();
  pot.begin();

  if (!aws.connect()) {
    display.showMessage("AWS FAIL", "", 0);
    while(1);
  }
  aws.requestShadowDocument();
  delay(1000);   // espera respuesta inicial
}

void loop() {
  aws.loop();

  // ----- 1. Potenciometro con debounce (solo si NO estamos en auto) -----
  if (!aws.getAutoMode()) {
    int rawPercent = pot.readPercent();
    int currentRange;
    if (rawPercent < 20)      currentRange = 0;
    else if (rawPercent < 40) currentRange = 1;
    else if (rawPercent < 60) currentRange = 2;
    else if (rawPercent < 80) currentRange = 3;
    else                      currentRange = 4;

    // Detectar cambio y empezar a estabilizar
    if (currentRange != candidateRange) {
      candidateRange = currentRange;
      lastPotChangeTime = millis();
    }

    // Si el candidato se mantiene estable durante el tiempo de debounce
    if (candidateRange != -1 && (millis() - lastPotChangeTime) >= potDebounceDelay) {
      if (candidateRange != lastStableRange) {
        lastStableRange = candidateRange;
        int desiredSpeed = rangeToSpeed(candidateRange);
        aws.publishDesiredSpeed(desiredSpeed);
        Serial.printf("Pot estabilizado en rango %d → velocidad %d%%\n", candidateRange, desiredSpeed);
      }
      candidateRange = -1;   // ya publicado, reiniciar
    }
  } else {
    // Si estamos en modo auto, no usamos el potenciómetro; reseteamos estado del pot
    candidateRange = -1;
    lastStableRange = -1;
  }

  // ----- 2. Lógica de auto‑mode: control de velocidad por temperatura -----
  if (aws.getAutoMode()) {
    float temp = sensor.readTemperature();
    int threshold = aws.getTempThreshold();
    int desiredSpeed = 0;

    if (!isnan(temp) && temp >= threshold) {
      desiredSpeed = 50;   // motor arranca al 50%
    } else {
      desiredSpeed = 0;
    }

    // Solo publicar si ha cambiado y no lo hemos hecho muy recientemente
    if (desiredSpeed != motor.getSpeedPercent() &&
        (millis() - lastAutoUpdate) >= autoUpdateInterval) {
      aws.publishDesiredSpeed(desiredSpeed);
      lastAutoUpdate = millis();
      Serial.printf("Auto‑mode: temp=%.1f°C, threshold=%d°C → velocidad %d%%\n",
                    temp, threshold, desiredSpeed);
    }
  }

  // ----- 3. Actualización periódica de pantalla y publicación de reported -----
  const unsigned long displayInterval = 2000;
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = millis();

    float temp = sensor.readTemperature();
    float hum  = sensor.readHumidity();
    int speed = motor.getSpeedPercent();

    // Show on display
    display.showNormal(temp, speed, hum, aws.getAutoMode(), aws.getTempThreshold());

    // Check for significant changes
    bool tempChanged = !isnan(temp) && abs(temp - lastReportedTemp) >= 1.0;
    bool humChanged  = !isnan(hum)  && abs(hum  - lastReportedHum)  >= 3.0;
    bool shadowChanged = motor.isShadowChanged();

    // Also check if 10 minutes have passed since last publish
    bool timeToPublish = (millis() - lastPublishTime) >= publishInterval;

    if (tempChanged || humChanged || shadowChanged || timeToPublish) {
      aws.publishReported(temp, hum, speed);
      lastPublishTime = millis();   // update timestamp

      if (!isnan(temp)) lastReportedTemp = temp;
      if (!isnan(hum))  lastReportedHum  = hum;
      lastReportedSpeed = speed;
      motor.clearShadowChanged();

      if (timeToPublish && !(tempChanged || humChanged || shadowChanged)) {
        Serial.println("Periodic publish (10 min timeout)");
      }
    }
  }
}
