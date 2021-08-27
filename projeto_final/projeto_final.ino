/**
* @file projeto_final.ino
* @author by Franciuíne Barbosa da Silva de Almeida, Victor Eugenio Mainardi Fritz
* @link https://github.com/franciuine/projFinal
* @date 08-2021
*/


#include <LiquidCrystal.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

<<<<<<< HEAD
//declaração do semaforo
=======
/**
  SEMÁFORO QUE RECEBE O REGISTRO DE TEMPERETURA ATUAL
*/

>>>>>>> a706020c69300a740bdd4026940c37ebad13500f
SemaphoreHandle_t xTempSemaphore;

/**
  DECLARA AS VARIÁVEIS E PINOS DE ENTRADA E SAÍDA
*/

LiquidCrystal LCD(12, 11, 5, 4, 3, 2);

float Temperatura = 0;
int LED_VM = 7;
int LED_VD = 8;
int ar_cond = 10;
int pino_lm = A0;

/// DECLARACAO DAS TAREFAS
void task_leituraSensor (void *pvParameters);
void task_atuadores (void *pvParameters);
void task_display (void *pvParameters);

void setup()
{
  pinMode(pino_lm, INPUT);
  pinMode(LED_VM, OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(ar_cond, OUTPUT);
/// DEFINE A QUANTIDADE DE LINHAS E COLUNAS DO LCD
  LCD.begin(16, 2);
  Serial.begin(9600);

/// VERIFICA SE O SEMÁFORO JÁ EXISTE
  if (xTempSemaphore == NULL)
  {
/// SE AINDA NÃO EXISTE, CRIA NOVO MUTEX
    xTempSemaphore = xSemaphoreCreateMutex();
    if ((xTempSemaphore) != NULL)
    {
      o
      xSemaphoreGive(xTempSemaphore);
    }
  }
/// CRIA TAREFA PARA PER O SENSOR
  xTaskCreate(task_leituraSensor, "leituraSensor", 128, NULL, 2, NULL);
/// CRIA TAREFA DOS ATUADORES
  xTaskCreate(task_atuadores, "atuadores", 128, NULL, 2, NULL);
/// CRIA TAREFA DI DISPLAY
  xTaskCreate(task_display, "display", 128, NULL, 2, NULL);
}

void loop()
{
/// O CÓDIGO SERÁ EXECUTADO APENAS NAS TAREFAS
}

/// TASK LEITURA DO SENSOR
void task_leituraSensor(void *pvParameters)
{
  while (1)
  {
/// INPUT ANALOGICO  
    int SensorTempTensao = analogRead(pino_lm);
/// CODIFICACAO DO SINAL
    float Tensao = SensorTempTensao * 5;
    Tensao /= 1023;
/// CONVERSAO PRA GRAUS CELCIUS
    float Temperatura_atual = (Tensao - 0.5) * 100;
/// TAKE SEMAPHORE
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {

/// TENTA ADQUIRIR SEMAFORO
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      Temperatura = Temperatura_atual;
      xSemaphoreGive(xTempSemaphore);
    }
/// DELAY NA LEITURA
    vTaskDelay(1);
  }
}
/// TAREFA PARA CONTROLE DOS LED'S
void task_atuadores(void *pvParameters)
{
  float temp = 0.0;
  while (1)
  {
/// ADQUIRI SEMAFORO
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      temp = Temperatura;
      xSemaphoreGive(xTempSemaphore);
    }
/// CONTROLE DO AR CONDICIONADO (LIGA/DESLICA)
    if (temp >= 32.0)
    {
      digitalWrite(ar_cond, HIGH);
    }
    if (temp < 20.0)
    {
      digitalWrite(ar_cond, LOW);
    }
/// SINAIS DE ALERTA
    if (temp > 28.0)
    {
      digitalWrite(LED_VM, HIGH);
      digitalWrite(LED_VD, LOW);
    }
    if (temp <= 24.0)
    {
      digitalWrite(LED_VM, LOW);
      digitalWrite(LED_VD, HIGH);
    }
  }
}
/// TAREFA QUE CONTROLA A SAÍDA DE DADOS NO DISPLAY
void task_display(void *pvParameters)
{
  float temp = 0.0;
  while (1)
  {
/// ADQUIRI SEMÁFORO
    if (xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE)
    {
      temp = Temperatura;
      xSemaphoreGive(xTempSemaphore);
    }
	
    LCD.setCursor(0, 1);
    LCD.print(temp);
    LCD.print(" C");

    /**
      FLAG QUE INDICA O ESTADO DO AR CONDICIONADO
    */
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
