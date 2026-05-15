#include <math.h>
const int B = 4275000;         // B value of the thermistor
const int R0 = 100000;         // R0 = 100k
const int PinTempSensor = A0;  // Grove - Temperature Sensor connect to A0

void setup() {
  Serial.begin(9600);
}



// Sampling Delays for each power mode in milliseconds 
const int Active_Rate = 1000; // 1 second between samples
const int Idle_Rate = 5000; // 5 seconds between samples 
const int Power_Down_Rate = 30000; //30 seconds between samples 
const int Samples = 60; // number of samples to collect per cycle 



// Power mode constants used for better readbility
const int Active = 1;
const int Idle = 2;
const int Power_Down = 3;


// Starts in active mode to collect as much data as possible at the start
int Current_Mode = Active;



// Arrays to store temperature readings and DFT results
float Temperature_Data[Samples]; //Raw temperature readings in degrees C
int Sample_Count = 0; // Tracks how many samples collected in a cycle 
float Real_Part[Samples]; //Real Component of DFT output 
float I_Part[Samples]; // Imaginary component of DFT output
float Magnitude[Samples]; //Magnitude of each frequency component 
float Frequencies[Samples]; //Frequecny value for each DFT bin

//Variables for adaptive power mode switching 
int Idle_Count = 0; //counts consecutive cycles in idle/power down mode 
int Cycle_Count = 0; //number of cycles completed 


//Moving Average uses last 10 cycles to smooth out variation spikes
const int Cycle_Limit = 10; 
//Stores recent variation values for moving average
float Variation_History[Cycle_Limit]; 




//Reads current temperature from the sensor using themistor resistance formula 
float Read_Temperature() { 
  int a = analogRead(PinTempSensor);  //read raw analog value
  float R = 1023.0/a -1.0; //covert to resistance ration
  R = R0 * R; //calculate actual ration in ohm
  float temperature = 1.0/(log(R/R0)/B + 1.0/298.15) - 273.15; //We were given the Steinhart-Hart equations
  return temperature;
}

//Collects 60 samples at the current mode's sampling rate
void Collect_Temperature_Data(){
  Serial.println(F("Temperature Samples: "));
  Sample_Count = 0;



// Choose delay based on current power mode
  int Sample_Delay = Active_Rate;
  if(Current_Mode == Idle) Sample_Delay = Idle_Rate;
  if(Current_Mode == Power_Down) Sample_Delay = Power_Down_Rate;

  for (int i = 0; i < Samples; i++){
    Temperature_Data[i] = Read_Temperature();   // Store each reading in array
    Serial.println(Temperature_Data[i]);  // Send reading to PC via serial
    delay(Sample_Delay);   // Wait before next sample

      
    }
  

  
}



// Applies Discrete Fourier Transform to find dominant frequency in temperature signal
float Apply_DFT() {
  float Sampling_Freq = 1.0;    // 1 Hz sampling frequency in Active mode
  float Dominant_Freq = 0.0;  // Will store the frequency with highest magnitude
  float Max_Magnitude = 0.0;  // Tracks the highest magnitude found so far

  // Loop through each frequency bin 
  for (int k = 1; k < Samples; k++) {
    Real_Part[k] = 0.0;
    I_Part[k] = 0.0;
    
    // Inner loop sums contribution of each sample to this frequency bin
    for (int n = 0; n < Samples; n++) {
      float angle = 2.0 * PI * k * n / Samples;
      Real_Part[k] += Temperature_Data[n] * cos(angle);  // Real part of DFT
      I_Part[k] -= Temperature_Data[n] * sin(angle); // Imaginary part of DFT
      
    }


    // Calculate magnitude from real and imaginary parts using Pythagoras

    Magnitude[k] = sqrt((Real_Part[k] * Real_Part[k]) + (I_Part[k] * I_Part[k]));


    // Calculate the actual frequency this bin represents
    Frequencies[k] = (k*Sampling_Freq) / Samples ;

    // Track which frequency has the highest magnitude
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
// Sends collected data to PC in CSV format for analysis in Python
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


// Calculates total variation by summing absolute differences between consecutive samples
// High variation means temperature is changing a lot, low means it is stable
float Calcuate_Variation() {
  float Total_Variation = 0.0;

  for (int i = 1; i < Samples; i++) {
    Total_Variation += abs(Temperature_Data[i] - Temperature_Data[i-1]);
  }

  Serial.print(F("Total Variation = "));
  Serial.println(Total_Variation);

  return Total_Variation;
}

// Calculates moving average of variation across recent cycles
// Smooths out spikes so power mode decisions are not made on a single unusual cycle

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


// Adjusts sampling rate based on Nyquist theorem - must sample at least twice
// the dominant frequency to avoid missing signal changes
int Adjust_Sampling_Rate(float Dominant_Freq) {
  float Nyquist_Rate = 2.0 * Dominant_Freq; // Minimum rate needed to capture signal

  // Clamp rate between 0.5 Hz and 4 Hz to avoid extremes
  if(Nyquist_Rate < 0.5) Nyquist_Rate = 0.5;
  if(Nyquist_Rate > 4.0) Nyquist_Rate = 4.0;

  // Convert frequency (Hz) to delay in milliseconds
  int New_Delay = (int)(1000.0/Nyquist_Rate);

  
  Serial.print(F("Adjusted Sampling Rate = "));
  Serial.print(Nyquist_Rate);
  Serial.println(F(" Hz"));

  return New_Delay;
}

// Decides which power mode to use based on moving average variation
// High variation = Active (sample fast), low variation = Idle or Power Down (save energy)
void Decide_Power_Mode(float Moving_Avg) {

  if (Moving_Avg > 1.0) {   // Temperature changing a lot then stay in Active mode
    Current_Mode = Active;
    Idle_Count = 0; // Reset idle counter since we are active again
    Serial.println(F("Active Mode"));


  } else if (Moving_Avg > 0.5) { // Temperature changing a little and switch to Idle mode
    Current_Mode = Idle;
    Idle_Count ++;
    Serial.println(F("Idle Mode"));

  } else {  // Temperature is stable then count idle cycles
    Idle_Count ++;
    if (Idle_Count >= 5) {  // Been idle for 5+ cycles, switch to Power Down to save energy
      Current_Mode = Power_Down;
      Serial.println(F("Power Down Mode"));

    }else {
      Current_Mode = Idle;
      Serial.println("Idle Mode");
    }
  
  }

}
void loop() {
  Collect_Temperature_Data(); //Step1 collect samples

  float Dominant_Freq = Apply_DFT();  //Step2 find dominant frequency using DFT

  Send_Data_to_PC(); //Step3 

  float Variation = Calcuate_Variation(); //Step4 Calculate how much temperature varied


  //Step5 store varitation in history buffer 
  Variation_History[Cycle_Count % Cycle_Limit] = Variation; 
  Cycle_Count++;

  float Moving_Avg = Calculate_Moving_Average(); //Step6 smooth variation using moving average 

  int Current_Delay = Adjust_Sampling_Rate(Dominant_Freq); //Step7 adapt sampling rate 

  Decide_Power_Mode(Moving_Avg); //Step8 decide between the power modes. 

}

