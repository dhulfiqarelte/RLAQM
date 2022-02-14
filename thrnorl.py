import pandas as pd
from numpy import random
import matplotlib.pyplot as plt
import sys 
import matplotlib 

df=pd.read_csv(r'thlog.csv', header=None, names=['#','Time', 'FlowID' , 'Src' , 'DST' , 'Throughput'], sep=';')
df2=pd.read_csv(sys.argv[1], sep=';')
df['Throughput']=df['Throughput']*8
df = df[df['Src']=='10.0.1.1']
# Group by time  
name = df.groupby('Time').sum().reset_index()
x = name['Time'].values
y = name['Throughput'].values
y2 = [(y[j]-y[j-1])/1e6 for j in range(1,len(y))]
plt.figure(figsize=(8,2))
plt.plot(x[1:],y2)
plt.grid()
plt.xlabel('Time (sec)')
plt.ylabel('Mbps')
plt.savefig("thlog.png")
#plt.show()

plt.figure(figsize=(18,12))
for i in range(1,201):
    x = df[df['FlowID']==i]['Time'].values
    y = df[df['FlowID']==i]['Throughput'].values
    y2 = [(y[j]-y[j-1])/1e6 for j in range(1,len(y))]
    plt.plot(x[1:],y2)




plt.grid()
plt.xlabel('Time (sec)')
plt.ylabel('Mbps')
plt.savefig('flowthlog.png')
#plt.show()

plt.figure(figsize=(8,2))
plt.plot(df2.time.values/50.0, df2.delayMs.values*1000.0,color='red')
#plt.yscale('log')
plt.grid()
plt.xlabel('Time (sec)')
plt.ylabel('Queuing Delay (ms)')
plt.savefig('qdelay.png')
#plt.show()

