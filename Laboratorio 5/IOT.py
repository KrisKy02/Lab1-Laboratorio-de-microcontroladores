import time
import json
import serial
import paho.mqtt.client as mqtt

class IOTClient:

    def __init__(self, broker, port, topic, token, serial_port, baudrate):
        self.broker = broker
        self.port = port
        self.topic = topic
        self.token = token
        self.serial_port = serial_port
        self.baudrate = baudrate

        self.client = mqtt.Client("SensorClient")
        self.data = self.setup_serial()
        self.setup_mqtt()

    def setup_serial(self):
        try:
            data = serial.Serial(self.serial_port, self.baudrate, timeout=1)
            print("¡Conexión serial establecida con éxito!")
            return data
        except serial.SerialException as e:
            print("Error de conexión serial:", str(e))
            return None

    def setup_mqtt(self):
        self.client.on_connect = self.on_connect
        self.client.username_pw_set(self.token)
        self.client.connect(self.broker, self.port)

    def on_connect(self, client, userdata, flags, rc):
        status = "¡Conexion con el servidor establecida!" if rc == 0 else f"Ups, hubo un problema al conectar. Codigo: {rc}"
        print(status)

    def read_sensor_data(self):
        
        raw_data = self.data.readline().decode('utf-8').rstrip()

        if "musica" in raw_data:
            return {"Comando": "musica"}
        elif "abrir" in raw_data:
            return {"Comando": "abrir"}
        elif "encender" in raw_data:
            return {"Comando": "encender"}
        elif "anomaly score:" in raw_data:
            score = raw_data.split(":")[1].strip()
            return {"Anomalía": score}
        else:
            return {"Mensaje": "Datos no reconocidos."}


    def run(self):
        self.client.loop_start()
        while True:
            sensor_data = self.read_sensor_data()
            if sensor_data:
                message = json.dumps(sensor_data)
                print(f"Datos del sensor: {message}")
                self.client.publish(self.topic, message, qos=1)
                time.sleep(0.8)

if __name__ == "__main__":
    client = IOTClient(
        broker="iot.eie.ucr.ac.cr",
        port=1883,
        topic="v1/devices/me/telemetry",
        token="s1b5ysozrstmuhtjua7b",
        serial_port="/dev/ttyACM0",
        baudrate=115200
    )
    client.run()

