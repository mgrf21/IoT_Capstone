
/*OneWire DS18S20, DS18B20, DS1822 Temperatur Ejemplo
//información en:
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// DallasTemperature library
// https://github.com/milesburton/Arduino-Temperature-Control-Libra
*********** 
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
  Based on the Dallas Temperature Library example
*********
*Details, in this link: <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
*
*Adaptado por Raúl Miranda-Tello, Miguel Ramírez Fonseca y Agustín Ruiz Amaya el 20220220
*
*SENSORES DE TEMPERATURA 
*DS_18B20         ESP32CAM
*   G              GND
*   R              5 V
*   Y              GPIO 2
*   
* PH-4502C        ESP32CAM       
*   VCC             5 V             
*   GND             GND             
*   P0              GPIO 15                       
*                   
*TSD Meter        ESP32CAM       
*    +              5 V             
*    -              GND            
*    A              GPIO 12         
*/                  


//Bibliotecas sensorDS18B20
#include <OneWire.h>  //  Comunicaciòn del sensor de temperatura
#include <DallasTemperature.h>

//deinicion de variables
#define ONE_WIRE_BUS 2  //El cable de datos está conectado al pin digital GPIO 12
#define sensortds_pin 12
#define sensorph_pin 15
#define muestra 20

//Objetos
OneWire oneWire(ONE_WIRE_BUS);  //Configura una instancia de oneWire para comunicarse con cualquier dispositivo OneWire ( 4.7K pull up)
DallasTemperature sensors(&oneWire); //Pasar nuestra referencia de oneWire al sensor de temperatura de Dallas

//variables del sensor temperatura DS18B20
float temperatura;

//Variables del sensor TDS Meter
int tdsVal,tds_PPM,n=3, fTP=17; //ejemplo de temperatura a 20 Grados
float tdsVoltaje;
int tdsValor, Vref = 5;

//Variables del sensor PH-4502C
float phVoltaje;
int phValor;
float cal = 3.5; //Valor de corrección por offset 


// Variables para promedio y odenar muestras
unsigned long int avgVal;
int buf[muestra];
int burb; 
float phSum, tdsSum;


void setup(void) {
  Serial.begin(115200);  //Inicia comunicación serial
  sensors.begin();        // Pone en marcha la biblioteca ONE-WIRE

//Configuración del pin analógico como entrada
  pinMode (sensorph_pin, INPUT);
  pinMode (sensortds_pin, INPUT);
  
//Espera (Opcional)
  delay (300);
}

void loop(void) {
//**********************************
// Llama a sensores.requestTemperatures() para emitir una temperatura global y solicitudes a todos los dispositivos en el bus
sensors.requestTemperatures(); 

// ¿Por qué "byIndex"?Puede tener más de un IC en el mismo bus. 0 se refiere al primer IC en el cable
temperatura = sensors.getTempCByIndex(0);  //TemC para celsius; TempF para farenheit
temperatura = temperatura-3;    // Ajuste de valor
Serial.print("Temperatura, ph y ppm: ");
 
//******************************************
// captura entrada análoga en el GPIO 15
//Almacena valores del sensor PH
for(int i=0;i<muestra;i++) 
  {
  buf[i]=analogRead(sensorph_pin); 
  delay(300); 
  } 
  for(int i=0;i<(muestra-1);i++)        //ordenar en forma ascendente los datos
  {
    for(int j=i+1;j<muestra;j++)
    {
      if(buf[i]>buf[j])
      {
        burb=buf[i];
        buf[i]=buf[j];
        buf[j]=burb;
      } 
    }
  } 
// Convierte la entrada análoga, la cual va de 0 - 4096 
avgVal = 0;                       // Inicializar buffer
for(int i=n;i<(muestra-n);i++)    //Suma de muestras centrales
avgVal = avgVal+buf[i]; 

// Convierte la entrada análoga, la cual va de 0 - 4096 para un voltaje (0 - 5V):
phVoltaje=(avgVal /(muestra-(2*n))) * (5.0/4096); //convierte a milivolt la CAD de 12 bits ESP32
phValor = phVoltaje+4.0;         //convierte los milivolt en un valor pH, ajuste de 3.5 a 4
 
//phValor = phSum + cal; //Opción de valor de calibración

//*******************************

// captura entrada análoga en el GPIO 12
//Almacena valores del sensor TDS

for(int i=0;i<muestra;i++)       // datos para un promedio
  { 
    buf[i]=analogRead(sensortds_pin);
    delay(250);
  }
  for(int i=0;i<(muestra-1);i++)        //ordenar en forma ascendente los datos
  {
    for(int j=i+1;j<muestra;j++)
    {
      if(buf[i]>buf[j])
      {
        burb=buf[i];
        buf[i]=buf[j];
        buf[j]=burb;
      }
    }
  }

avgVal = 0;                               // Inicializar buffer
for(int i=n;i<(muestra-n);i++)  //Suma de muestras centrales
avgVal = avgVal+buf[i]; 

// Convierte la entrada análoga, la cual va de 0 - 4096 para un voltaje (0 - 5V):
tdsVoltaje=(avgVal /(muestra-(2*n))) * (5.0/4096); //convierte a milivolt la CAD de 12 bits ESP32
tdsValor = tdsVoltaje*700;  // Puede considerarse 500, 650 o 700 en función del tipo de sensor

//Opción que considera la variación en la tempertura
/*float compensationCoefficient=1.0+0.02*(temperatura-25.0);    //formula compensación de temperature: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
float compensationVolatge=avgVal/compensationCoefficient;  //temperature compensation
tdsValor=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage a ppm 
*/

// Envìo por puerto serie el valor de los sensores:
Serial.print(temperatura); 
Serial.print(",");
Serial.print(phValor);
Serial.print(",");
Serial.println(tdsValor);
 
delay(1000);
  
}
