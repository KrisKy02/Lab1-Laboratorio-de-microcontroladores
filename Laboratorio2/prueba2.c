#include <avr/io.h>
#include <avr/interrupt.h>

/* MACROS */
// Definiciones para tiempos y ciclos de temporizador
#define TIMER_CYCLES_RESET 0
#define ONE_SECOND 1
#define THREE_SECONDS 3
#define TEN_SECONDS 10
#define HALF_SECOND_CYCLE 30
#define FULL_SECOND_CYCLE 60
#define FULL_SECOND_CYCLE_RESET 63

/* ESTADOS */
// Enumeración para representar los estados del semáforo
typedef enum {
    PASS_VEHICLE,
    BLINK_VEHICLE,
    STOP_VEHICLE,
    PASS_PEDESTRIAN,
    BLINK_PEDESTRIAN,
    STOP_PEDESTRIAN
} State;

// Variable de estado actual
State estado;

/* VARIABLES GLOBALES */
// Variables para mantener un registro del tiempo y el estado del botón
int pressed = 0;
int timerCycles = 0;
int seconds = 0;

/* DECLARACIÓN DE FUNCIONES */
// Prototipos de funciones
void setup();
void FSM();
inline void interruptionRoutine();  
inline void blinkingRoutine();     
inline void resetTimeAndCycles(); 

/* INTERRUPCIONES */
// Interrupción para el botón
ISR(INT0_vect){
  pressed = 1;
}

// Interrupción del temporizador
ISR (TIMER0_OVF_vect){
  interruptionRoutine();
}

/* MAIN */
// Bucle principal
int main(void) {
  setup();  // Inicializar
  sei();    // Habilitar interrupciones

  while (1) {
    FSM();  // Máquina de estados finitos para el semáforo
  }
}

/* FUNCIONES */

// Función de configuración inicial
void setup(){
  // Configurar los pines PB0, PB1, PB2 y PB3 como salidas
  DDRB |= (1 << PB3)|(1 << PB2)|(1 << PB1)|(1 << PB0);

  // Inicializar los estados de los pines: PB3 en LOW, PB2 en HIGH, PB1 en LOW, PB0 en HIGH
  PORTB = (0<<PB3)|(1<<PB2)|(0<<PB1)|(1<<PB0);

  // Establecer el estado inicial de la máquina de estados como PASS_VEHICLE
  estado = PASS_VEHICLE;

  // Inicializar la variable que registra si se ha pulsado un botón
  pressed = 0;

  // Inicializar la variable de segundos
  seconds = 0;

  // Habilitar la interrupción externa en el pin INT0
  GIMSK |= (1<<INT0);

  // Configurar la interrupción externa INT0 para que se active en el flanco de subida
  MCUCR |= (1 << ISC00) | (1 << ISC01);

  // Establecer el modo del Timer0 (modo normal)
  TCCR0A = 0x00;

  // Establecer el preescalador del Timer0 a 1024 (dividir la frecuencia del reloj entre 1024)
  TCCR0B = (1 << CS00) | (1 << CS02);

  // Inicializar el valor del contador del Timer0
  TCNT0 = 0;

  // Habilitar la interrupción por desbordamiento del Timer0
  TIMSK |= (1 << TOIE0);
}


// Máquina de estados finitos para controlar el semáforo y el paso de peatones
void FSM(){
  switch (estado){
    // Estado donde los vehículos pueden pasar
    case (PASS_VEHICLE):
      // Configurar las luces del semáforo para el paso de vehículos
      PORTB = (0<<PB3)|(1<<PB2)|(0<<PB1)|(1<<PB0);
      // Si se presiona el botón y han pasado 10 segundos, cambie al estado BLINK_VEHICLE
      if((pressed == 1) && (seconds >= TEN_SECONDS)){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Cambiar estado a BLINK_VEHICLE
        estado = BLINK_VEHICLE;
      }
      break;

    // Estado donde la luz para vehículos parpadea
    case (BLINK_VEHICLE):
      // Si han pasado 3 segundos y estamos en un ciclo de medio segundo o un segundo completo,
      // cambiar al estado STOP_VEHICLE
      if((seconds >= THREE_SECONDS) && (timerCycles == HALF_SECOND_CYCLE || timerCycles == FULL_SECOND_CYCLE)){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Cambiar estado a STOP_VEHICLE
        estado = STOP_VEHICLE;
      }
      break;

    // Estado donde los vehículos deben detenerse
    case (STOP_VEHICLE):
      // Configurar las luces del semáforo para detener vehículos
      PORTB = (0<<PB3)|(1<<PB2)|(1<<PB1)|(0<<PB0);
      // Si ha pasado un segundo, cambiar al estado PASS_PEDESTRIAN
      if (seconds >= ONE_SECOND){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Cambiar estado a PASS_PEDESTRIAN
        estado = PASS_PEDESTRIAN;
      }
      break;

    // Estado donde los peatones pueden pasar
    case (PASS_PEDESTRIAN):
      // Configurar las luces del semáforo para el paso de peatones
      PORTB = (1<<PB3)|(0<<PB2)|(1<<PB1)|(0<<PB0);
      // Si han pasado 10 segundos, cambiar al estado BLINK_PEDESTRIAN
      if(seconds >= TEN_SECONDS){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Cambiar estado a BLINK_PEDESTRIAN
        estado = BLINK_PEDESTRIAN;
      }
      break;

    // Estado donde la luz para peatones parpadea
    case (BLINK_PEDESTRIAN):
      // Si han pasado 3 segundos y estamos en un ciclo de medio segundo o un segundo completo,
      // cambiar al estado STOP_PEDESTRIAN
      if((seconds >= THREE_SECONDS) && (timerCycles == HALF_SECOND_CYCLE || timerCycles == FULL_SECOND_CYCLE)){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Cambiar estado a STOP_PEDESTRIAN
        estado = STOP_PEDESTRIAN;
      }
      break;

    // Estado donde los peatones deben detenerse
    case (STOP_PEDESTRIAN):
      // Configurar las luces del semáforo para detener peatones
      PORTB = (0<<PB3)|(1<<PB2)|(1<<PB1)|(0<<PB0);
      // Si ha pasado un segundo, cambiar al estado PASS_VEHICLE
      if(seconds >= ONE_SECOND){
        // Llamar a la función para reiniciar el tiempo y los ciclos
        resetTimeAndCycles();
        // Resetear el estado de "pressed" a 0
        pressed = 0;
        // Cambiar estado a PASS_VEHICLE
        estado = PASS_VEHICLE;
      }
      break;

    // Estado predeterminado
    default:
      estado = PASS_VEHICLE;
      break;
  }
}

// Función para manejar el parpadeo de las luces del semáforo en los estados BLINK_VEHICLE y BLINK_PEDESTRIAN
inline void blinkingRoutine(){
  switch (estado){
    // Cuando el semáforo para vehículos debe parpadear
    case BLINK_VEHICLE:
      // Parpadea la luz del semáforo para vehículos (PB0) cada medio segundo o segundo completo
      if(timerCycles == HALF_SECOND_CYCLE || timerCycles == FULL_SECOND_CYCLE){
        PORTB ^= (1<<PB0);
      }
      break;

    // Cuando el semáforo para peatones debe parpadear
    case BLINK_PEDESTRIAN:
      // Parpadea la luz del semáforo para peatones (PB3) cada medio segundo o segundo completo
      if(timerCycles == HALF_SECOND_CYCLE || timerCycles == FULL_SECOND_CYCLE){
        PORTB ^= (1<<PB3);
      }
      break;

    // Caso por defecto
    default:
      // No hacer nada si el estado no es ninguno de los anteriores
      break;
  }
}

// Función que se ejecuta en cada interrupción del Timer
inline void interruptionRoutine(){
  // Llamar a la función para manejar el parpadeo de las luces
  blinkingRoutine();
  
  // Si se ha alcanzado un ciclo completo de un segundo
  if(timerCycles == FULL_SECOND_CYCLE_RESET){
    // Incrementa el contador de segundos
    seconds++;
    // Reiniciar el contador de ciclos del temporizador
    timerCycles = TIMER_CYCLES_RESET;
  } else {
    // Incrementar el contador de ciclos del temporizador
    timerCycles++;
  }
}

// Función para reiniciar el contador de tiempo y ciclos
inline void resetTimeAndCycles() {
  // Reiniciar el contador de ciclos del temporizador
  timerCycles = TIMER_CYCLES_RESET;
  // Reiniciar el contador de segundos
  seconds = TIMER_CYCLES_RESET;
}
