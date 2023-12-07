import serial
import pywhatkit as kit
import datetime
import time

# Configuración del puerto serial
serialPort = serial.Serial(port='/dev/ttyUSB0', baudrate=9600, timeout=1)

# Número de teléfono y mensaje (ajústalo según tus necesidades)
telefono_destino = ""  # Número de teléfono

# Función para enviar mensaje de WhatsApp
def enviar_mensaje_whatsapp(mensaje):
    ahora = datetime.datetime.now()
    hora = ahora.hour
    minutos = ahora.minute + 2  # Ajustar para el próximo minuto disponible
    kit.sendwhatmsg(telefono_destino, mensaje, hora, minutos)
    time.sleep(20)  # Espera para evitar múltiples envíos

# Variables para controlar el envío de alertas
alertaTanqueVacioEnviada = False
alertaHumedadBajaEnviada = False

# Leer datos del puerto serial y actuar en consecuencia
try:
    while True:
        if serialPort.in_waiting > 0:
            linea = serialPort.readline().decode('utf-8').rstrip()
            print("Datos recibidos:", linea)

            # Ejemplo de lógica de alerta (ajustar según el formato de tus datos)
            if "El tanque está casi vacío" in linea and not alertaTanqueVacioEnviada:
                enviar_mensaje_whatsapp("Alerta: El tanque de agua está casi vacío.")
                alertaTanqueVacioEnviada = True
            elif "Motor activado debido a baja humedad del suelo" in linea and not alertaHumedadBajaEnviada:
                enviar_mensaje_whatsapp("Alerta: La humedad del suelo es baja, activando riego.")
                alertaHumedadBajaEnviada = True

except KeyboardInterrupt:
    print("Programa terminado por el usuario")
    serialPort.close()
