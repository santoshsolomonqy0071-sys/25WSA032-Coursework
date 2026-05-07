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
int Idle_Count = 0;
int Cycle_Count = 0;
const int Cycle_Limit = 10;
float Variation_History[Cycle_Limit]; 





float Read_Temperature() {
  int a = analogRead(PinTempSensor);
  float R = 1023.0/a -1.0;
  R = R0 * R;
  float temperature = 1.0/(log(R/R0)/B + 1.0/298.15) - 273.15;
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
    Serial.println(Temperature_Data[i]);
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




float Calcuate_Variation() {
  float Total_Variation = 0.0;

  for (int i = 1; i < Samples; i++) {
    Total_Variation += abs(Temperature_Data[i] - Temperature_Data[i-1]);
  }

  Serial.print(F("Total Variation = "));
  Serial.println(Total_Variation);

  return Total_Variation;
}

float Calculate_Moving_Average() {
  float Sum = 0.0;
  int Count = min(Cycle_Count, Cycle_Limit);

  for (int i = 0; i < Count; i++) {
    Sum += Variation_History[i];

  }

  float Moving_Avg = Sum/Count;

  Serial.print(F("Moving Average = "));
  Serial.println(Moving_Avg);

  return Moving_Avg;




}

int Adjust_Sampling_Rate(float Dominant_Freq) {
  float Nyquist_Rate = 2.0 * Dominant_Freq;

  if(Nyquist_Rate < 0.5) Nyquist_Rate = 0.5;
  if(Nyquist_Rate > 4.0) Nyquist_Rate = 4.0;

  int New_Delay = (int)(1000.0/Nyquist_Rate);

  
  Serial.print(F("Adjusted Sampling Rate = "));
  Serial.print(Nyquist_Rate);
  Serial.println(F(" Hz"));

  return New_Delay;
}

void Decide_Power_Mode(float Moving_Avg) {

  if (Moving_Avg > 1.0) {
    Current_Mode = Active;
    Idle_Count = 0;
    Serial.println(F("Active Mode"));


  } else if (Moving_Avg > 0.5) {
    Current_Mode = Idle;
    Idle_Count ++;
    Serial.println(F("Idle Mode"));

  } else {
    Idle_Count ++;
    if (Idle_Count >= 5) {
      Current_Mode = Power_Down;
      Serial.println(F("Power Down Mode"));

    }else {
      Current_Mode = Idle;
      Serial.println("Idle Mode");
    }
  
  }

}
void loop() {
  Collect_Temperature_Data();

  float Dominant_Freq = Apply_DFT(); 

  Send_Data_to_PC();

  float Variation = Calcuate_Variation();

  Variation_History[Cycle_Count % Cycle_Limit] = Variation;
  Cycle_Count++;

  float Moving_Avg = Calculate_Moving_Average();

  Adjust_Sampling_Rate(Dominant_Freq);

  Decide_Power_Mode(Moving_Avg);

}

