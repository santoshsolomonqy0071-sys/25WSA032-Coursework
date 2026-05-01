#include <math.h>
const int B = 4275000;         // B value of the thermistor
const int R0 = 100000;         // R0 = 100k
const int PinTempSensor = A0;  // Grove - Temperature Sensor connect to A0

void setup() {
  Serial.begin(9600);
}

const int Active_Rate = 1000;
const int Idle_Rate = 5000;
const int Power_Down_Rate = 30000;
const int Samples = 60;

const int Active = 1;
const int Idle = 2;
const int Power_Down = 3;

int Current_Mode = Active;


float Temperature_Data[Samples];
int Sample_Count = 0;
float Real_Part[Samples];
float I_Part[Samples];
float Magnitude[Samples];
float frequencies[Samples];

float Read_Temperature() {
  int a = analogRead(PinTempSensor);
  float R = 1023.0/a -1.0;
  R = R0 * R;
  float temperature = 1/(log(R/R0)/B + 1/298.15) - 273.15;
  return temperature;
}


void Collect_Temperature_data(){
  Sample_Count = 0;

  int Sample_Delay = Active_Rate;
  if(Current_Mode == Idle) Sample_Delay = Idle_Rate;
  if(Current_Mode == Power_Down) Sample_Delay = Power_Down_Rate;

  for (int i = 0; i < Samples; i++){
    Temperature_Data[i] = Read_Temperature();
    delay(Sample_Delay);
      
    }

  
}

