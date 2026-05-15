import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


df = pd.read_csv('temperature_data.csv', skipinitialspace=True)
df.columns = df.columns.str.strip() 
Time = df['Time'].values
Temperature = df['Temperature'].values
Frequency = df['Frequency'].values
Magnitude = df['Magnitude'].values

window_size = 10
Moving_Avg = np.convolve(Temperature, np.ones(window_size)/window_size, mode='valid')
Moving_Avg_Time = Time[window_size - 1:]

Change_Rate = np.diff(Temperature) 
Change_Rate_Time = Time[1:]


plt.figure(figsize=(12, 6))
plt.plot(Time, Temperature, color='blue', marker='o' )
plt.title('Plot 1 - Temperature vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Temperature (deg C)')
plt.grid(True)
plt.tight_layout()
plt.savefig('temperature_vs_time.png')
plt.close()
print("Saved Plot 1")




plt.figure(figsize=(12, 6))
plt.plot(Frequency, Magnitude, color='red', marker='x')
plt.title('Plot 2 - Frequency vs Magnitude')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)
plt.tight_layout()
plt.savefig('frequency_vs_magnitude.png')
plt.close()
print("Saved Plot 2")
