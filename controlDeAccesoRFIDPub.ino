#include <DS3231.h>
#include <SimpleDHT.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#include <MFRC522.h>
#define sclk 13  
#define mosi 11  
#define cs   6
#define dc   5
#define rst  7  

#define RST_PIN  9    
#define SS_PIN  10   
MFRC522 mfrc522(SS_PIN, RST_PIN); 

Adafruit_ST7735 tft = Adafruit_ST7735(cs,  dc, rst);
DS3231 reloj;
RTCDateTime dt;

int pinDHT11 = 2;
SimpleDHT11 dht11;

const int pinZumbador = 8;
const int pinRele = 3;

byte ActualUID[4]; 
byte Usuario1[4]= {0x70, 0x07, 0x51, 0x80} ; //código del usuario 1
byte Usuario2[4]= {0xC1, 0x2F, 0xD6, 0x0E} ; //código del usuario 2

void setup()
   {
        Serial.begin(9600);

        SPI.begin();        //Iniciamos el Bus SPI
        mfrc522.PCD_Init(); // Iniciamos el MFRC522
        
        // Initialize DS3231
        reloj.begin();
        reloj.setDateTime(__DATE__, __TIME__);
    
        tft.initR();             // Inicializa la pantalla
        tft.setRotation(1);     
        pintaInterfaz();

        pinMode(pinZumbador, OUTPUT);
        pinMode(pinRele, OUTPUT);

        digitalWrite(pinRele,LOW);
}

void loop() {
  dt = reloj.getDateTime();
  tft.setTextSize(2);
  tft.setCursor(30, 30);
  tft.setTextColor(ST7735_BLACK,ST7735_WHITE);
  if(dt.hour < 10) tft.print("0");
  tft.print(dt.hour);
  tft.print(":");
  if(dt.minute < 10) tft.print("0");
  tft.print(dt.minute);
  tft.print(":");
  if(dt.second < 10) tft.print("0");
  tft.print(dt.second);
  tft.setCursor(20, 50);
  if(dt.day < 10) tft.print("0");
  tft.print(dt.day);
  tft.print("/");
  if(dt.month < 10) tft.print("0");
  tft.print(dt.month);
  tft.print("/");
  tft.print(dt.year);

  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.print("Read DHT11 failed");
    return;
  }

  tft.setCursor(20, 70);
  tft.print("TEMP. ");
  tft.print((int)temperature);
  tft.print("*C");
  tft.setCursor(20, 90);
  tft.print("HUMED. ");
  tft.print((int)humidity);
  tft.print("%");

    // Revisamos si hay nuevas tarjetas  presentes
  if ( mfrc522.PICC_IsNewCardPresent()) 
        {  
      //Seleccionamos una tarjeta
            if ( mfrc522.PICC_ReadCardSerial()) 
            {
                  // Enviamos serialemente su UID
                  Serial.print(F("Card UID:"));
                  for (byte i = 0; i < mfrc522.uid.size; i++) {
                          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                          Serial.print(mfrc522.uid.uidByte[i], HEX);   
                          ActualUID[i]=mfrc522.uid.uidByte[i];          
                  } 
                  Serial.print("     ");                 
                  //comparamos los UID para determinar si es uno de nuestros usuarios  
                  if(compareArray(ActualUID,Usuario1)){
                    Serial.println("Acceso concedido...");
                    accesoConcedido();
                  }else if(compareArray(ActualUID,Usuario2)){
                    Serial.println("Acceso concedido...");
                    accesoConcedido();
                  }else{
                    Serial.println("Acceso denegado...");
                    accesoDenegado();
                  }
                  // Terminamos la lectura de la tarjeta tarjeta  actual
                  mfrc522.PICC_HaltA();
          
            }
      
  }

  delay(1000);
}

// Pinta interfaz
void pintaInterfaz(){
  tft.fillScreen(ST7735_WHITE);

  tft.fillRect (0, 0, 160, 20 ,ST7735_CYAN);
  tft.setCursor(20, 2);
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(2);
  tft.print("MARCA ACME");

  tft.setCursor(85, 120);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_RED);
  tft.print("(C) DFM 2017");
}

// Acceso concedido
void accesoConcedido(){
  tft.fillScreen(ST7735_GREEN);
  tft.setTextColor(ST7735_WHITE,ST7735_GREEN);
  tft.setCursor(40, 40);
  tft.print("ACCESO");
  tft.setCursor(25, 60);
  tft.print("CONCEDIDO");
  tono(1);
  digitalWrite(pinRele,HIGH);
  delay(3000);
  digitalWrite(pinRele,LOW);
  pintaInterfaz();
}

// Acceso denegado
void accesoDenegado(){
  tft.fillScreen(ST7735_BLUE);
  tft.setTextColor(ST7735_WHITE,ST7735_BLUE);
  tft.setCursor(40, 40);
  tft.print("ACCESO");
  tft.setCursor(30, 60);
  tft.print("DENEGADO");
  tono(2);
  delay(3000);
  pintaInterfaz();
}

void tono(int retardo){
    unsigned char i;
       for(i=0;i<100;i++)
      {
        digitalWrite(pinZumbador,HIGH);
        delay(retardo);//wait for 2ms
        digitalWrite(pinZumbador,LOW);
        delay(retardo);//wait for 2ms
      }
}

//Función para comparar dos vectores
 boolean compareArray(byte array1[],byte array2[])
{
  if(array1[0] != array2[0])return(false);
  if(array1[1] != array2[1])return(false);
  if(array1[2] != array2[2])return(false);
  if(array1[3] != array2[3])return(false);
  return(true);
}
