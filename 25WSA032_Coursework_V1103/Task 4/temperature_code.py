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
