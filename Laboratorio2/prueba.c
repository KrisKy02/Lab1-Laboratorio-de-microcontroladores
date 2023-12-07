#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PASO_VEHICULOS    (1 << PB0) // LDPV
#define LED_PASO_PEATONES     (1 << PB1) // LDPP
#define LED_VEHICULOS_DETENIDOS (1 << PB2) // LDVD
#define LED_PEATONES_DETENIDOS (1 << PB3) // LDPD
#define BOTON_1               (1 << PB4)
#define BOTON_2               (1 << PB5)

typedef enum {
    PASS_VEHICLE,
    BLINK_VEHICLE,
    STOP_VEHICLE,
    PASS_PEDESTRIAN,
    BLINK_PEDESTRIAN,
    STOP_PEDESTRIAN
} State;

volatile uint8_t botonPresionado = 0;
volatile uint16_t tick_counter = 0;
State estado;
// Rutina de interrupción del timer
void setup() {
    // Configurar pines como entrada o salida
    DDRB = LED_PASO_VEHICULOS | LED_PASO_PEATONES | LED_VEHICULOS_DETENIDOS | LED_PEATONES_DETENIDOS;
    /*// Habilitar interrupciones de cambio de pin para el grupo 0
    GIMSK |= (1 << PCIE0);

    // Habilitar las interrupciones de cambio de pin para los pines 4 y 5
    PCMSK |= (1 << PCINT4) | (1 << PCINT5);*/
    // Configurar Timer
    TCCR1B |= (1 << WGM12); // Modo CTC
    TIMSK |= (1 << OCIE1A); // Habilitar interrupción de comparación
    OCR1A = 15624; // Para un intervalo de 1s con prescaler de 64 y reloj de 1MHz
    TCCR1B |= (1 << CS10) | (1 << CS11); // Prescaler 64
    sei(); // Habilitar interrupciones globales
    //estado = PASS_VEHICLE;
}

/*ISR(TIMER1_COMPA_vect) {
    tick_counter++;
}
ISR(PCINT0_vect) {
    if ((PINB & BOTON_1) || (PINB & BOTON_2)) {
        botonPresionado = 1;
    }
}*/
void FSM() {
    PORTB = LED_PASO_VEHICULOS | LED_PEATONES_DETENIDOS;
   /*static uint16_t last_tick = 0;
    switch (estado) {
        case PASS_VEHICLE:
            PORTB = LED_PASO_VEHICULOS | LED_PEATONES_DETENIDOS;
            if (botonPresionado && tick_counter - last_tick >= 10) {
                last_tick = tick_counter;
                estado = BLINK_VEHICLE;
                botonPresionado = 0;
            }
            break;
        case BLINK_VEHICLE:
            if (tick_counter - last_tick >= 5) {
                last_tick = tick_counter;
                estado = STOP_VEHICLE;
            }
            break;
        case STOP_VEHICLE:
            PORTB = LED_VEHICULOS_DETENIDOS | LED_PEATONES_DETENIDOS;
            if (tick_counter - last_tick >= 2) {
                last_tick = tick_counter;
                estado = PASS_PEDESTRIAN;
            }
            break;
        case PASS_PEDESTRIAN:
            PORTB = LED_PASO_PEATONES | LED_VEHICULOS_DETENIDOS;
            if (tick_counter - last_tick >= 10) {
                last_tick = tick_counter;
                estado = BLINK_PEDESTRIAN;
            }
            break;
        case BLINK_PEDESTRIAN:
            if (tick_counter - last_tick >= 5) {
                last_tick = tick_counter;
                estado = STOP_PEDESTRIAN;
            }
            break;
        case STOP_PEDESTRIAN:
            PORTB = LED_VEHICULOS_DETENIDOS | LED_PEATONES_DETENIDOS;
            if (tick_counter - last_tick >= 2) {
                last_tick = tick_counter;
                estado = PASS_VEHICLE;
            }
            break;
    }*/
}

/*void blinkRoutine() {
    static uint16_t last_blink_tick = 0;
    if (tick_counter - last_blink_tick >= 1) {  
        last_blink_tick = tick_counter;
        if (estado == BLINK_VEHICLE) {
            PORTB ^= LED_PASO_VEHICULOS;
        }
        if (estado == BLINK_PEDESTRIAN) {
            PORTB ^= LED_PASO_PEATONES;
        }
    }
}*/

int main(void) {
    setup();
    while (1) {
        FSM();
        //blinkRoutine();
    }
    return 0;
}

