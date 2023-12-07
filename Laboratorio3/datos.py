#!/usr/bin/python3
import serial

# Configuración para el puerto serie.
# Asegúrate de tener permisos para acceder al puerto.
ser = serial.Serial(
    port='/tmp/ttyS1',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0
)

# Abre el archivo 'Tensiones.csv' en modo escritura.
f = open('datos.csv', 'w+')

print(f"Conectado a: {ser.portstr}")

try:
    # Ciclo infinito para leer datos del puerto serie.
    while True:
        for c in ser.read():
            c = chr(c)  # Convierte el byte a carácter.
            print(c, end="")  # Imprime el carácter en la consola.
            f.write(c)  # Escribe el carácter en el archivo.

except KeyboardInterrupt:
    # Manejo de la interrupción por teclado (Ctrl + C).
    print("\nEl usuario ha terminado el programa.")

except Exception as e:
    # Manejo de errores generales.
    print(f"Error: {e}")

finally:
    # Cierra las conexiones (puerto serie y archivo) al finalizar.
    ser.close()
    f.close()
