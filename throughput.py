import pandas as pd
from numpy import random
import matplotlib.pyplot as plt
import sys 
import matplotlib 

df=pd.read_csv(r'thlog.csv', header=None, names=['#','Time', 'FLowID' , 'Src' , 'DST' , 'Throughput'], sep=';')
df['Throughput']=df['Throughput']/df['Time']*8
# Group by time  
name = df.groupby('Time')
df = name.sum()
df['Throughput'].plot()
plt.legend()
plt.xlabel('Time (sec)')
plt.ylabel('Mbps')
plt.savefig("thlog.pdf")
#plt.show()
