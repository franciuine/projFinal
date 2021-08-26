//include das libs que vai ser usado

// REFERENCIAS
// https://microcontrollerslab.com/arduino-freertos-structure-queue-receive-data-multiple-resources/
// https://www.arduino.cc/en/Reference.AnalogRead
// Biblioteca LCD
#include <LiquidCrystal.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//declaração do semaforo
SemaphoreHandle_t xTempSemaphore;

//              DECLARAÇÕES                //
LiquidCrystal LCD(12, 11, 5, 4, 3, 2);
float Temperatura = 0;
//Definindo LED VERMELHO no pino 13
int LED_VM = 7;
//Definindo LED VERDE no pino 8
int LED_VD = 8;
//Definindo o motor "ARCONDICIONADO" no pino 10
int ar_cond = 10;
//Definindo pino do sensor de temperatura no pino A0
int pino_lm = A0;
//definicao das funcoes periódicas
void task_leituraSensor (void *pvParameters);
void task_atuadores (void *pvParameters);
void task_display (void *pvParameters);

//liga desliga
void setup()
{
  pinMode(pino_lm, INPUT);
  pinMode(LED_VM, OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(ar_cond, OUTPUT);

  //Define a quantidade de colunas e linhas do LCD
  LCD.begin(16, 2);
  // put your setup code here, to run once:
  Serial.begin(9600);
  //checa criação semaforo
  if (xTempSemaphore == NULL)
  {
    //criando mutex
    xTempSemaphore = xSemaphoreCreateMutex();
    if ((xTempSemaphore) != NULL)
    {
      //libera semaforo
      xSemaphoreGive(xTempSemaphore);
    }
  }
  //cria task leitura dos sensores
  xTaskCreate(task_leituraSensor, "leituraSensor", 128, NULL, 2, NULL);
  //cria task dos atuadores
  xTaskCreate(task_atuadores, "atuadores", 128, NULL, 2, NULL);
  //cria task display
  xTaskCreate(task_display, "display", 128, NULL, 2, NULL);
}

//Não usa loop
void loop()
{
	
}

//task para leitura do sensor
void task_leituraSensor(void *pvParameters)
{
  while (1)
  {
    int SensorTempTensao = analogRead(pino_lm);
    // Converte a tensao lida
    float Tensao = SensorTempTensao * 5;
    Tensao /= 1023;
    // Converte a tensao lida em Graus Celsius
    float Temperatura_atual = (Tensao - 0.5) * 100;
	//adquiri o semaforo
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      //atualiza temperatura Global
      Temperatura = Temperatura_atual;
      //libera o samaforo
      xSemaphoreGive(xTempSemaphore);
    }
    //delay da leitura
    vTaskDelay(1);
  }
}

//funcao led
void task_atuadores(void *pvParameters)
{
  float temp = 0.0;
  while (1)
  {
    //adquiri semaforo
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      temp = Temperatura; //pego a temperatura global
      //libera semaforo
      xSemaphoreGive(xTempSemaphore);
    }
    //condicao pra ligar ou desliga refrigeração!
    if (temp >= 32.0)
    {
      digitalWrite(ar_cond, HIGH);
    }
    //DESLIGA ARCONDICIONADO
    if (temp < 20.0)
    {
      digitalWrite(ar_cond, LOW);
    }
    //Sinal de alerta!
    if (temp > 28.0)
    {
      digitalWrite(LED_VM, HIGH);
      digitalWrite(LED_VD, LOW);
    }
    //Sinal de ok!
    if (temp <= 24.0)
    {
      digitalWrite(LED_VM, LOW);
      digitalWrite(LED_VD, HIGH);
    }
  }
}

//funcao display
void task_display(void *pvParameters)
{
  float temp = 0.0;
  while (1)
  {
    //adquiri semaforo
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      temp = Temperatura;
      //libera semaforo
      xSemaphoreGive(xTempSemaphore);
    }
	
    LCD.setCursor(0, 1);
    LCD.print(temp);
    LCD.print(" C");

    //declara a flag que indica situacao do ar cond.
    int flag = digitalRead(ar_cond);

    if (flag == HIGH)
    {
      LCD.setCursor(0, 0);
      LCD.print("AC LIGADO");
    }
    if (flag == LOW)
    {
      LCD.setCursor(0, 0);
      LCD.print("AC DESLIGADO");
    }
  }
}
