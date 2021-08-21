//include das libs que vai ser usado

// REFERENCIAS
// https://microcontrollerslab.com/arduino-freertos-structure-queue-receive-data-multiple-resources/
// https://www.arduino.cc/en/Reference.AnalogRead


#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;


//struct ler dados do sensor
struct lerpino{
  int pino;
  float valor;
};

float bufferTemp[10];
const int pinLed = 11;

//This lines defines the hanlder for structQueue to access it for tasks reference.
QueueHandle_t structQueue;


//funcoes
void thread_analogic (void *pvParameters);
void thread_temperatura (void *pvParameters);
void thread_temperatura_new (void *pvParameters);
void thread_led (void *pvParameters);

//liga desliga
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial){
    ;
  }
  Serial.print("Conectando");

  pinMode(pinLed, OUTPUT);
  //semaforo
  if (xSerialSemaphore == NULL){
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ((xSerialSemaphore) != NULL){
      xSemaphoreGive ((xSerialSemaphore));
    }
  }

  //criar pilha das temperaturas do sensor
  structQueue = xQueueCreate(10, sizeof(struct pinRead));

  //testa criação da pilha
  if(structQueue != NULL){
  //cria threads com xTaskCreate
  //https://www.embarcados.com.br/rtos-para-iniciantes-com-arduino-e-freertos/
    xTaskCreate(thread_analogic, "analogic", 128, NULL, 2, NULL);
    xTaskCreate(thread_temperatura, "temperatura", 128, NULL, 2, NULL);
    xTaskCreate(thread_temperatura_new, "temperatura_new", 128, NULL, 2, NULL);
    xTaskCreate(thread_led, "led", 128, NULL, 2, NULL);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

/*THREADSSSSSSSSSSS*/
//definir funcao alogica
void thread_analogic(void *pvParameters __attribute__((unused)){
}

//definir funcao temperatura
void thread_temperatura(void *pvParameters __attribute__((unused)){
  
}


void thread_nova_temperatura(void *pvParameters __attribute__((unused)){
  
}

void thread_led(void *pvParameters __attribute__((unused)){
  
}

















//definir funcao nova temperatura

//definir funcao led
