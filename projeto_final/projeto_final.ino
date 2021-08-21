//include das libs que vai ser usado

//REFERENCIAS
//https://microcontrollerslab.com/arduino-freertos-structure-queue-receive-data-multiple-resources/
//https://www.arduino.cc/en/Reference.AnalogRead


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

}

void loop() {
  // put your main code here, to run repeatedly:

}
