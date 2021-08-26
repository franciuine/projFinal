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

//********************************************//
//              DECLARAÇÕES                //

//Definindo pino 9 para o botão principal de liga e desliga
int botao = 9; //BOTAO PRA LIGAR!!
//Defindo o estado inicial do botão em Zero
int button_state = 0;
//Definindo o estado do botão após iniciar
int state = 0;
//Definindo LED VERMELHO no pino 13
int LED_VM = 13;
//Definindo LED VERDE no pino 8
int LED_VD = 8;
//Definindo pino do sensor de temperatura no pino A0
int pino_lm = A0;
//Definindo o motor "ARCONDICIONADO" no pino 10
int ar_cond = 10;
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
//funcoes do ar condic
void temperatura_baixa();
void temperatura_ambiente();
void temperatura_baixa();
void ar_condicionado();

//liga desliga
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Conectando");
  pinMode(pinLed, OUTPUT);
  //checa criação semaforo
  if (xSerialSemaphore == NULL){
    //criando mutex
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



/*
CONTROLE DE TEMPERATURA DE AR CONDICIONADO
*/

//if (Temperatura < 24.0)
void temperatura_baixa(){
	//LIGAR LED VERDE
}

//if (Temperatura > 28.0)
void temperatura_alta(){
	//LIGAR LED VERMELHO
}

//if (Temperatura > 24.0 && Temperatura <= 28)
void temperatura_ambiente(){
	//LIGAR LED AMARELO
}


//TAREFA: LIGAR AR CONDICIONADO
void ar_condicionado(){
	
}


























//DEFININDO INPUTS
void setup(){
  pinMode(botao, INPUT); 
  pinMode(pino_lm, INPUT);
  pinMode(LED_VM, OUTPUT);
  //pinMode(LED_AM, OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(ar_cond, OUTPUT);
  //Define a quantidade de colunas e linhas do LCD
  LCD.begin(16,2);
  //Imprime a mensagem no LCD
  //LCD.print("Temperatura:");
  // Muda o cursor para a primeira coluna e segunda linha do LCD
  //LCD.setCursor(0,1); 
  //LCD.print("            C");
  
 // Serial.begin(9600);
}

void loop(){
  //leitura do botao
  button_state = digitalRead(botao);
  // Muda o cursor para a primeira coluna e segunda linha do LCD
//  LCD.setCursor(0,1);
 // LCD.print(button_state);
  //testando o estado do sistema
  if(button_state == HIGH)
  {
    //ATUALIZA O ESTADO BOTAO APÓS SER PRESSIONADO
    state = !state;
  }
  
 // Serial.print("O state atual eh: ");
 // Serial.println(state);
  
//  delay(1300);  
  
  //SE state É 1 SIGNIFICA QUE COMEÇOU!
  //LIGAR REFRIGERAÇÃO SE TEMPERATURA ESTIVER MAIOR QUE 32
  if(state == 1){	
//    LCD.print("SISTEMA FUNCIONANDO!");
  
	// Faz a leitura da tensao no Sensor de Temperatura
	int SensorTempTensao = analogRead(pino_lm);  
  	// Converte a tensao lida
	float Tensao = SensorTempTensao*5;
	Tensao/=1023;
  	// Converte a tensao lida em Graus Celsius
	float Temperatura = (Tensao-0.5)*100;
    //printando a temperatura no LCD
  	LCD.setCursor(0,1);
    LCD.print(Temperatura);
    LCD.print(" C");
    //condicao pra ligar ou desliga refrigeração!
    if (Temperatura >= 32.0){
      digitalWrite(ar_cond, HIGH);
      LCD.setCursor(0,0);
      LCD.print("AC Ligado");   
    }
    //DESLIGA ARCONDICIONADO
    if (Temperatura < 20.0){
      digitalWrite(ar_cond, LOW);
      LCD.setCursor(0,0);     
      LCD.print("AC Desligado");
    }
    
    //Sinal de alerta! Ô SOR TÁ	BEM QUENTII..
    if (Temperatura > 28.0){
		digitalWrite(LED_VM, HIGH);
		digitalWrite(LED_VD, LOW); 
    }
    
    //Sinal de ok! Ô SOR, DÁ PÁ DESLIGA O AR?
    if (Temperatura <= 24.0){
		digitalWrite(LED_VM, LOW);
		digitalWrite(LED_VD, HIGH);
    }
    
    //Temperatura ideal! Vamos economizar recursos meus queridos.
    if (Temperatura > 24.0 && Temperatura <= 28){
	//digitalWrite(LED_VM, LOW);
	//digitalWrite(LED_VD, LOW);
	//digitalWrite(LED_AM, HIGH);
    }
	delay(1300);
  }
  //se o state estiver em Zero
  else{
	//Serial.println("SISTEMA DESLIGADO");
	//  digitalWrite(LED_VM, HIGH);
   // digitalWrite(LED_VD, HIGH);
   // digitalWrite(LED_AM, HIGH);
    digitalWrite(ar_cond, LOW);
  	delay(1300);
  }
}