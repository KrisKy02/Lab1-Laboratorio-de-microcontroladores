#include <pic14/pic12f683.h>

typedef unsigned int word;
word __at 0x2007 __CONFIG = (_WDT_OFF);
// Función para retraso
void delay(unsigned int tiempo);

void main(void)
{
    // Configurar pines GP0, GP1, GP2, GP4 como salidas y GP5 como entrada
    TRISIO = 0b00100000;

    unsigned char counter = 0;
    unsigned char random_number;

    // Bucle infinito
    while(1)
    {
        // Incrementar el contador de forma continua
        counter++;

        // Un pequeño retraso para la desincronización
        delay(10);

        if (GP5 == 0)
        {
            //Manejo del rebote del boton
            while(GP5 == 0);
            delay(10);
            // Generar un número aleatorio con el valor actual del contador
            random_number = counter % 6 + 1;

            // Configurar LEDs según el número aleatorio generado
            switch(random_number)
            {
                case 1:
                    GP2 = 1;
                    break;
                case 2:
                    GP1 = 1;
                    break;
                case 3:
                    GP2 = 1; GP1 = 1;
                    break;
                case 4:
                    GP0 = 1; GP1 = 1;
                    break;
                case 5:
                    GP0 = 1; GP1 = 1; GP2 = 1;
                    break;
                case 6:
                    GP0 = 1; GP1 = 1; GP4 = 1;
                    break;
            }

            // Un pequeño retraso para mostrar el número
            delay(500);

            // Limpiar los LEDs
            GPIO = 0x00;
        }
    }
}

void delay(unsigned int tiempo)
{
    for(unsigned int i = 0; i < tiempo; i++)
        for(unsigned int j = 0; j < 1275; j++);
}
