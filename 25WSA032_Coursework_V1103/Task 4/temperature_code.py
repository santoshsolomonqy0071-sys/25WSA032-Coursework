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




plt.figure(figsize=(12, 6))
plt.plot(Time, Temperature, color='orange', marker='o')
plt.plot(Moving_Avg_Time, Moving_Avg, color='green', label='Moving Average')
plt.title('Plot 3 - Smoothed Temperature vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Temperature (deg C)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('smoothed_temperature_vs_time.png')
plt.close()
print("Saved Plot 3")


plt.figure(figsize=(10, 6))
plt.hist(Temperature, bins=20, color='purple', alpha=0.7)
plt.title('Plot 4 - Temperature Histogram')
plt.xlabel('Temperature (deg C)')
plt.ylabel('Frequency')
plt.grid(True)
plt.tight_layout()
plt.savefig('temperature_histogram.png')
plt.close()
print("Saved Plot 4")



plt.figure(figsize=(12, 6))
plt.plot(Change_Rate_Time, Change_Rate, color='brown', marker='o')
plt.axhline(0, color='gray')
plt.title('Plot 5 - Rate of Change of Temperature vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Rate of Change of Temperature (deg C/s)')
plt.grid(True)
plt.tight_layout()  
plt.savefig('rate_of_change_temperature_vs_time.png')
plt.close()
print("Saved Plot 5")

