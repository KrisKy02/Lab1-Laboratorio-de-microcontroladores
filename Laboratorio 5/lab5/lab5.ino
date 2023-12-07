/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 4

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

#include <PDM.h>  // Incluye la biblioteca PDM para manejar el micrófono
#include <Universidad_de_Costa_Rica_-project-1_inferencing.h>  // Incluye los encabezados para el modelo de inferencia

/** Estructura para almacenar buffers de audio, contadores y selectores */
typedef struct {
    signed short *buffers[2];  // Dos buffers para almacenar muestras de audio
    unsigned char buf_select;  // Selector para alternar entre buffers
    unsigned char buf_ready;   // Indicador de buffer listo para procesamiento
    unsigned int buf_count;    // Contador de muestras en el buffer actual
    unsigned int n_samples;    // Número total de muestras a almacenar en un buffer
} inference_t;

// Variables globales
static inference_t inference;  // Instancia de la estructura para manejo de audio
static bool record_ready = false;  // Indicador de si el sistema está listo para grabar
static signed short *sampleBuffer;  // Buffer para almacenar muestras temporales
static bool debug_nn = false;  // Habilitar para ver información de depuración del modelo
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);  // Contador para imprimir resultados

/**
 * @brief      Función de configuración de Arduino
 */
void setup()
{
    Serial.begin(115200);  // Inicia la comunicación en serie
    while (!Serial);       // Espera a que la conexión en serie esté lista
    Serial.println("Edge Impulse Inferencing Demo");  // Imprime mensaje inicial

    // Imprime un resumen de los ajustes de inferencia
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();  // Inicializa el clasificador

    // Comienza la grabación de audio para inferencia
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d)\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;  // Termina si no se puede inicializar la grabación
    }
}

/**
 * @brief      Función principal de Arduino. Ejecuta el bucle de inferencia.
 */
void loop()
{
    // Graba audio para inferencia
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;  // Sale si falla la grabación
    }

    // Prepara la señal para el clasificador
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;

    ei_impulse_result_t result = {0};  // Almacena el resultado de la inferencia

    // Ejecuta el clasificador en modo continuo
    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;  // Sale si falla la clasificación
    }

    // Umbral de confianza para clasificación
    const float confidence_threshold = 0.9; 

    // Itera a través de los resultados de clasificación
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > confidence_threshold) {
            // Compara y actúa según la etiqueta clasificada
            if (strcmp(result.classification[ix].label, "musica") == 0) {
                ei_printf("musica fue detectado con un porcentaje de confianza del %.5f\n", result.classification[ix].value);
                Serial.println("musica"); // Envía el comando reconocido a través del puerto serie
                break; // Salir del bucle después de encontrar 'musica'
            }
            else if (strcmp(result.classification[ix].label, "abrir") == 0) {
                ei_printf("abrir fue detectado con un porcentaje de confianza del %.5f\n", result.classification[ix].value);
                Serial.println("abrir"); // Envía el comando reconocido a través del puerto serie
                break; // Salir del bucle después de encontrar 'abrir'
            }
            else if (strcmp(result.classification[ix].label, "encender") == 0) {
                ei_printf("encender fue detectado con un porcentaje de confianza del %.5f\n", result.classification[ix].value);
                Serial.println("encender"); // Envía el comando reconocido a través del puerto serie
                break; // Salir del bucle después de encontrar 'encender'
            }
        }
    }

    // Controla la impresión de resultados después de cada ventana de clasificación
    if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)) {
#if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %.3f\n", result.anomaly); // Imprime la puntuación de anomalía si está disponible
#endif
        print_results = 0;
    }
}

/**
 * @brief      Callback para cuando los datos del PDM están listos. Maneja la lectura de datos de audio.
 */
static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();  // Obtiene el número de bytes disponibles

    // Lee los datos en el buffer de muestra
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    // Procesa los datos leídos si la grabación está lista
    if (record_ready) {
        for (int i = 0; i < bytesRead >> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            // Cambia de buffer cuando el actual está lleno
            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;  // Cambia el buffer activo
                inference.buf_count = 0;   // Reinicia el contador de buffer
                inference.buf_ready = 1;   // Marca el buffer como listo
            }
        }
    }
}

/**
 * @brief      Inicia la estructura de inferencia y configura/inicia PDM para grabación de audio.
 * @param[in]  n_samples  Número de muestras por buffer
 * @return     Verdadero si la inicialización es exitosa, falso en caso contrario.
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    // Asigna memoria para los buffers de audio
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) return false;

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));
    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    // Configura las variables de la estructura
    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // Configura la callback para recibir datos de PDM
    PDM.onReceive(&pdm_data_ready_inference_callback);

    // Establece el tamaño del buffer de PDM
    PDM.setBufferSize((n_samples >> 1) * sizeof(int16_t));

    // Inicializa PDM con un canal (modo mono) y una tasa de muestreo de 16 kHz
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
        return false;
    }

    // Establece la ganancia del PDM
    PDM.setGain(127);

    record_ready = true;  // Indica que la grabación está lista
    return true;
}

/**
 * @brief      Espera a que haya nuevos datos de audio disponibles.
 * @return     Verdadero (true) si la grabación ha finalizado correctamente.
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    // Verifica si el buffer está listo para ser procesado
    if (inference.buf_ready == 1) {
        // Imprime un mensaje de error si hay un desbordamiento del buffer
        // Sugiere disminuir el número de segmentos por ventana de modelo
        ei_printf(
            "Error sample buffer overrun. Decrease the number of slices per model window "
            "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;  // Establece el retorno como falso debido al error
    }

    // Espera activamente hasta que el buffer esté listo
    while (inference.buf_ready == 0) {
        delay(1);  // Pequeña pausa para evitar saturar el CPU
    }

    // Reinicia el indicador de buffer listo para el siguiente lote de datos
    inference.buf_ready = 0;

    return ret;  // Retorna verdadero o falso dependiendo del estado del buffer
}

/**
 * @brief      Obtiene los datos de la señal de audio cruda.
 * @param[in]  offset   El desplazamiento inicial en el buffer de audio.
 * @param[in]  length   La cantidad de datos a obtener.
 * @param[out] out_ptr  Puntero para almacenar los datos convertidos.
 * @return     Cero, indicando finalización exitosa de la función.
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    // Convierte los datos de audio de entero corto a flotante
    // Utiliza el buffer actualmente no seleccionado para la lectura
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;  // Retorna 0, indicando que la operación fue exitosa
}

/**
 * @brief      Detiene la grabación de PDM y libera los buffers.
 */
static void microphone_inference_end(void)
{
    PDM.end();  // Finaliza la grabación de PDM
    // Libera la memoria asignada para los buffers de audio
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);  // Libera el buffer de muestras temporal
}

// Verifica si el sensor configurado es compatible con el modelo de inferencia
#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."  // Genera un error de compilación si hay incompatibilidad
#endif
