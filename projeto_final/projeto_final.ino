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

// variaveis
int flag; //mudar pra bool
int k;
int i;

//vetor da media de temperatura
float temp_media[10];
//setar porta do led
const int pinLed = 11;
//This lines defines the hanlder for structQueue to access it for tasks reference.
QueueHandle_t structQueue;

//funcoes
void task_analogRead (void *pvParameters);
void task_temperatura (void *pvParameters);
void task_mediaTemperatura (void *pvParameters);
void task_led (void *pvParameters);

//liga desliga
void setup() {
  // put your setup code here, to run once:
  // baud rate do arduino
  Serial.begin(9600); //taxa
  
  Serial.println("Conectando");
  pinMode(pinLed, OUTPUT);
  //checa criação semaforo
  if (xSerialSemaphore == NULL){
    //criando mutex inicializa semaforo
    xSerialSemaphore = xSemaphoreCreateMutex();
    if((xSerialSemaphore) != NULL){
      //libera serial
      xSemaphoreGive((xSerialSemaphore));
    }
  }

  //criar pilha das temperaturas do sensor LM35
  structQueue = xQueueCreate(10, sizeof(struct lerpino));

  //testa criação da pilha
  if(structQueue != NULL){
  //cria task com xTaskCreate
  //https://www.embarcados.com.br/rtos-para-iniciantes-com-arduino-e-freertos/
    //criar task produtora da pilha
    xTaskCreate(task_analogRead, "analogRead", 128, NULL, 2, NULL);
    //criar task consumidora da pilha
    xTaskCreate(task_temperatura, "temperatura", 128, NULL, 2, NULL);
    //criar task pra calcula a media da temperatura
    xTaskCreate(task_mediaTemperatura, "mediaTemperatura", 128, NULL, 2, NULL);
    //criar task produtora da pilha
    xTaskCreate(task_led, "led", 128, NULL, 2, NULL);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

//funcao da task do input analogico, sensor LM35
void task_analogRead(void *pvParameters){
  while(1){
    Serial.print("TASK ANALOGREAD ");
    Serial.print(millis());
    Serial.println(" ms");
    //acessando struct
    struct lerpino pinoatual;
    pinoatual.pino = 0;
    pinoatual.valor = (float(analogRead(A0))*5 / (1023))/0.01;
    //inserir na pilha
    xQueueSend(structQueue, &pinoatual, portMAX_DELAY);
    //delay da leitura
    vTaskDelay(1);
  }
}

//funcao temperatura
void task_temperatura(void *pvParameters){
  while(1){
    Serial.print("TASK TEMPERATURA ");
    Serial.print(millis());
    Serial.println(" ms");
    struct lerpino pinoatual;
    if(xQueueReceive(structQueue, &pinoatual, portMAX_DELAY)==pdPASS){
      temp_media[k]=pinoatual.valor;
      //checa se encheou o vetor
      if(k<10){
        i = k;
        flag = 0;
        if(xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE){
          //prints da serial
          Serial.print("Temperatura lida atual: ");
          Serial.println(pinoatual.valor);
          //printa posicao do buffer
          Serial.println(k);
          xSemaphoreGive(xSerialSemaphore);
          k=k+1;
        }
      } else{
        i = 0;
        flag = 1;
      }
    }
  }
}

//funcao temperatura media
void task_mediaTemperatura(void *pvParameters){
  while(1){
    Serial.print("TASK MEDIA TEMPERATURA ");
    Serial.print(millis());
    Serial.println(" ms");
    //media da temperatura
    float media;
    //acumulador do buffer
    float acumulador;
    //se a flag é 1, calcula a media
    if(flag==1){
      for(int j=0; j<10; j++){
        acumulador = acumulador + temp_media[j];
    }
    //divide pelo tamanho do buffer pra calcular a media
    media = acumulador/10;
    //reseta a flag que indica se o buffer ta cheio ou n
    flag=0;
    //reseta a var de cont. do buffer
    k=0;

    if(xSemaphoreTake(xSerialSemaphore, (TickType_t)5)==pdTRUE){
      Serial.print("Media: ");
      Serial.println(media);
      media=0;
      acumulador=0;
      xSemaphoreGive(xSerialSemaphore);
    }
  }
    //caso contrario n faz leitura
    else{
      flag=0;
      i=k;
    }  
  }
}










//funcao led
void task_led(void *pvParameters){
  while(1){
    Serial.print("TASK LED ");
    Serial.print(millis());
    Serial.println(" ms");
    
    
    
    //intensidade de da luz
    int intensidade;
    //guarda oq foi consumido do buffer, a temperatura
    int aux;
    if(i>0){
      //consome o buffer
      aux = temp_media[i];
      if(aux>29){
        intensidade = map(aux, 30, 80, 0, 2500);
        tone(pinLed, intensidade);
      }
    }
    else{
      i=0;
      noTone(pinLed);
    }
  }
}
