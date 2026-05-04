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
float Frequencies[Samples];

void loop() {
  Collect_Temperature_Data();

  float Dominant_Freq = Apply_DFT(); 

  Send_Data_to_PC();
}

float Read_Temperature() {
  int a = analogRead(PinTempSensor);
  float R = 1023.0/a -1.0;
  R = R0 * R;
  float temperature = 1/(log(R/R0)/B + 1/298.15) - 273.15;
  return temperature;
}


void Collect_Temperature_Data(){
  Serial.println(F("Temperature Samples: "));
  Sample_Count = 0;

  int Sample_Delay = Active_Rate;
  if(Current_Mode == Idle) Sample_Delay = Idle_Rate;
  if(Current_Mode == Power_Down) Sample_Delay = Power_Down_Rate;

  for (int i = 0; i < Samples; i++){
    Temperature_Data[i] = Read_Temperature();
    delay(Sample_Delay);
      
    }
  

  
}

float Apply_DFT() {
  float Sampling_Freq = 1.0;
  float Dominant_Freq = 0.0;
  float Max_Magnitude = 0.0;

  for (int k = 1; k < Samples; k++) {
    Real_Part[k] = 0.0;
    I_Part[k] = 0.0;
    

    for (int n = 0; n < Samples; n++) {
      float angle = 2.0 * PI * k * n / Samples;
      Real_Part[k] += Temperature_Data[n] * cos(angle);
      I_Part[k] -= Temperature_Data[n] * sin(angle);
      
    }

    Magnitude[k] = sqrt((Real_Part[k] * Real_Part[k]) + (I_Part[k] * I_Part[k]));

    Frequencies[k] = (k*Sampling_Freq) / Samples ;

    if (Magnitude[k] > Max_Magnitude) {
      Max_Magnitude = Magnitude[k];
      Dominant_Freq = Frequencies[k];
    }



  }
  Serial.print(F("Dominant Frequency = "));
  Serial.print(Dominant_Freq);
  Serial.println(F(" Hz"));


  return Dominant_Freq; 
}

void Send_Data_to_PC() {
  Serial.println("Time, Temperature, Frequency, Magnitude");

  for (int i = 1; i < Samples; i++) {
    Serial.print(i);
    Serial.print(", ");
    Serial.print(Temperature_Data[i]);
    Serial.print(", ");
    Serial.print(Frequencies[i]);
    Serial.print(", ");
    Serial.println(Magnitude[i]);
  }
}

