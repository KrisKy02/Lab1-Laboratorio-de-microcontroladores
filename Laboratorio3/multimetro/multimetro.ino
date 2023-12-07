
#include <PCD8544.h>
#include <math.h>

// Definiciones de los pines utilizados para leer las tensiones y controlar componentes
static const byte VOLT_PINS[] = {0, 1, 2, 3};
static const byte LED_PIN = 13;
static const byte SERIAL_ENABLE_PIN = 12;
static const byte ACDC_PIN = 4;

// Definiciones de las resistencias para el divisor de voltaje
const float RESISTOR_1 = 10000.0;  // Ohmios
const float RESISTOR_2 = 86000.0;  // Ohmios

static PCD8544 lcd;

int serialDataEnable;
int ACDCReading;

void setup() {
  lcd.begin();                       // Iniciar el LCD
  Serial.begin(9600);                // Iniciar comunicación serial a 9600 baudios
  pinMode(LED_PIN, OUTPUT);          // Configurar el pin del LED como salida
  pinMode(SERIAL_ENABLE_PIN, INPUT); // Configurar el pin para habilitar la lectura serial como entrada
}

void loop() {
  ACDCReading = analogRead(ACDC_PIN); // Leer el pin AC/DC

  // Determinar si mostrar valores en DC o AC
  if (ACDCReading == LOW) {
    displayAndLogVoltage(" Voltajes en DC: ", [](float value) { return value; });
  } else {
    displayAndLogVoltage(" Voltajes en AC: ", [](float value) { return value / sqrt(2); });
  }
}

/**
 * Función para leer, mostrar y registrar el voltaje en función del etiquetado proporcionado
 * @param label Etiqueta para mostrar en el LCD
 * @param convert Función para convertir el voltaje (por ejemplo, a Vrms)
 */
void displayAndLogVoltage(const char* label, float(*convert)(float)) {
  lcd.setCursor(0, 0);
  lcd.print(label);
  
  // Leer y mostrar los valores para cada uno de los pines de voltaje
  for (int i = 0; i < 4; i++) {
    float adcValue = analogRead(VOLT_PINS[i]);  // Leer valor ADC
    float voltage = (adcValue / 1023.0) * 5.0;  // Convertir ADC a voltaje (0-5V)
    // Escalar el voltaje usando las resistencias del divisor
    float scaledVoltage = voltage * (RESISTOR_1 + RESISTOR_2) / RESISTOR_1;
    float adjustedVoltage = (scaledVoltage - 24.0); // Ajustar el rango del voltaje

    float finalValue = convert(adjustedVoltage); // Convertir el voltaje si es necesario (ej: Vrms)
    
    // Mostrar el voltaje en el LCD
    lcd.setCursor(0, i + 1);
    lcd.print(" V");
    lcd.print(i + 1);
    lcd.print(": ");
    lcd.print(finalValue);
    lcd.print(" V ");
    
    // Encender el LED si el voltaje es mayor que 20V
    if (abs(finalValue) > 20) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
  
  delay(1000); // Esperar un segundo antes de la siguiente lectura
  
  // Registrar el voltaje en la consola serial si está habilitado
  serialDataEnable = digitalRead(SERIAL_ENABLE_PIN);
  if (serialDataEnable == HIGH) {
    Serial.print(label);
    Serial.println();
    for (int i = 0; i < 4; i++) {
      float adcValue = analogRead(VOLT_PINS[i]);
      float voltage = (adcValue / 1023.0) * 5.0;
      float scaledVoltage = voltage * (RESISTOR_1 + RESISTOR_2) / RESISTOR_1;
      float adjustedVoltage = (scaledVoltage - 24.0);
      float finalValue = convert(adjustedVoltage);
      
      Serial.print(" V");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(finalValue);
      Serial.print(" V ");
      Serial.println();
    }
  }
}
