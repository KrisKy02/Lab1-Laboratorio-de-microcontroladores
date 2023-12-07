/* Informacion de la cuenta de blink*/
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>



char auth[] = "";
char ssid[] = ""; //Nombre de red wifi
char pass[] = ""; //contraseña red

BlynkTimer timer;
bool Relay = 0;

//Definir pines
#define sensor A0
#define waterPump D3
// Definir humedad limite
#define SOIL_MOISTURE_THRESHOLD 5
// Volumen total del tanque y nivel actual en litros
const float volumenTotalTanque = 1.0; // Volumen total del tanque en litros
float nivelActualTanque = 1.0; // Nivel actual del agua en el tanque en litros
const float volumenPorCiclo = 0.139; // Volumen utilizado por ciclo (calculado previamente)
bool isTankFull = true; // Indicador de si el tanque está lleno


void setup() {
  Serial.begin(9600);
  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, HIGH);


  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  Serial.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    Serial.print(".");
    delay(500);
  }

  // Llamar a la función cada 5 segundos
  timer.setInterval(5000L, soilMoistureSensor);
}

void soilMoistureSensor() {
  int value = analogRead(sensor);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  Blynk.virtualWrite(V0, value);
  Serial.print("Humedad del suelo: ");
  Serial.println(value);

  float nivelEnPorcentaje = (nivelActualTanque / volumenTotalTanque) * 100;
  Blynk.virtualWrite(V2, nivelEnPorcentaje); // Enviar el nivel del tanque en porcentaje a Blynk

  if (nivelEnPorcentaje > 20 && value < SOIL_MOISTURE_THRESHOLD) {
    digitalWrite(waterPump, HIGH);
    nivelActualTanque -= volumenPorCiclo;
    if (nivelActualTanque <= 0) {
      nivelActualTanque = 0;
    }
    Blynk.virtualWrite(V1, 1);
    Serial.println("Motor activado debido a baja humedad del suelo");
    Blynk.virtualWrite(V2, nivelEnPorcentaje);
  } else {
    digitalWrite(waterPump, LOW);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, nivelEnPorcentaje);
    Blynk.virtualWrite(V3, 0);
    Serial.println("Motor desactivado");
  }

  // Detener todo y notificar si el nivel del tanque está en el 20% o menos
  if (nivelEnPorcentaje <= 20) {
    isTankFull = false;
    Serial.println("El tanque está casi vacío - 20% de capacidad");
  }
}



BLYNK_WRITE(V3) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    nivelActualTanque = volumenTotalTanque; // Restablecer el nivel del tanque
    isTankFull = true; // Indicar que el tanque está lleno
    Blynk.virtualWrite(V2, nivelActualTanque); // Actualizar el nivel del tanque en la app
    Serial.println("Tanque rellenado");
  }
}
void loop() {
  Blynk.run();
  timer.run();

}
