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
SemaphoreHandle_t xTempSemaphore;
//struct ler dados do sensor
struct lerpino{
  int pino;
  float valor;
};



//********************************************//
//              DECLARAÇÕES                //
float Temperatura = 0;
//Definindo LED VERMELHO no pino 13
int LED_VM = 13;
//Definindo LED VERDE no pino 8
int LED_VD = 8;
//Definindo o motor "ARCONDICIONADO" no pino 10
int ar_cond = 10;
//Definindo pino do sensor de temperatura no pino A0
int pino_lm = A0;

//This lines defines the hanlder for structQueue to access it for tasks reference.
QueueHandle_t structQueue;

//funcoes
void task_leituraSensor (void *pvParameters);
void task_temperatura (void *pvParameters);
void task_mediaTemperatura (void *pvParameters);
void task_atuadores (void *pvParameters);
//liga desliga
void setup() {
	
  pinMode(pino_lm, INPUT);
  pinMode(LED_VM, OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(ar_cond, OUTPUT);
  
  //Define a quantidade de colunas e linhas do LCD
  LCD.begin(16,2);
  // put your setup code here, to run once:
  Serial.begin(9600);
  //checa criação semaforo
  if (xTempSemaphore == NULL){
    //criando mutex
    xTempSemaphore = xSemaphoreCreateMutex();
    if((xTempSemaphore) != NULL){
      //libera serial
      xSemaphoreGive((xTempSemaphore));
    }
  }

  //criar pilha das temperaturas do sensor LM35
  structQueue = xQueueCreate(10, sizeof(struct lerpino));

  //testa criação da pilha
  if(structQueue != NULL){
	//cria task com xTaskCreate
	//https://www.embarcados.com.br/rtos-para-iniciantes-com-arduino-e-freertos/
    //criar task produtora da pilha
    xTaskCreate(task_leituraSensor, "leituraSensor", 128, NULL, 2, NULL);
    //criar task produtora da pilha
    xTaskCreate(task_atuadores, "atuadores", 128, NULL, 2, NULL);
    //criar task consumidora da pilha
    xTaskCreate(task_display, "display", 128, NULL, 2, NULL);	
  }
}

void loop(){
//ja era!!!! FREERTOS N USA LOOP	
}

//funcao da task do input analogico, sensor LM35
void task_leituraSensor(void *pvParameters){
  while(1){
	int SensorTempTensao = analogRead(pino_lm);  
  	// Converte a tensao lida
	float Tensao = SensorTempTensao*5;
	Tensao/=1023;
  	// Converte a tensao lida em Graus Celsius
	float Temperatura_atual = (Tensao-0.5)*100;
	//mutex de controle da temperatura
	if(xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE){
		Temperatura = Temperatura_atual;
		xSemaphoreGive(xTempSemaphore);
	}
    //delay da leitura
    vTaskDelay(1);
  }
}
//funcao led
void task_atuadores(void *pvParameters){
	float temp = 0.0;
	while(1){
		//adquiri semaforo 
		if(xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE){
			temp = Temperatura; //pego a temperatura global
			//libera semaforo
			xSemaphoreGive(xTempSemaphore);
		}
		//condicao pra ligar ou desliga refrigeração!
		if (temp >= 32.0){
			digitalWrite(ar_cond, HIGH);
		}
		//DESLIGA ARCONDICIONADO
		if (temp < 20.0){
			digitalWrite(ar_cond, LOW);
		}
    
		//Sinal de alerta! Ô SOR TÁ	BEM QUENTII..
		if (temp > 28.0){
			digitalWrite(LED_VM, HIGH);
			digitalWrite(LED_VD, LOW); 
		}
    
		//Sinal de ok! Ô SOR, DÁ PÁ DESLIGA O AR?
		if (temp <= 24.0){
			digitalWrite(LED_VM, LOW);
			digitalWrite(LED_VD, HIGH);
		}
	}
}

void task_display(void *pvParameters){
	float temp = 0.0;
	while(1){
		//adquiri semaforo / se n tiver liberado, bloqueia a treahd por 5 tick 
		if(xSemaphoreTake(xTempSemaphore, (TickType_t)5) == pdTRUE){
			temp = Temperatura; //pego a temperatura global
			//libera semaforo
			xSemaphoreGive(xTempSemaphore);
		}
		//limpadinha na tela
		LCD.clear();
		LCD.setCursor(0,1);
		LCD.print(temp);
		LCD.print(" C");
		
		flag = digitalRead(ar_cond);
		
		if (flag == HIGH){
			LCD.setCursor(0,0);
			LCD.print("Ar Condicionado LIGADO!");	
		}
		if (flag == LOW){
			LCD.setCursor(0,0);
			LCD.print("Ar Condicionado DESLIGADO!");
		}	
	}
}