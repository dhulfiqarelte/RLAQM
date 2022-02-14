from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import numpy as np
import sys


def getReward(QDelayMs,EnqCounter,DropCounter):
	Sigma = 0.5  
	DelayRef = 5.0 #ms as in the thesis 
	DelayReward = Sigma * (DelayRef - QDelayMs)/DelayRef
	EnqRate=  1.0* EnqCounter / (1.0 + EnqCounter + DropCounter)
	EnqReward = (1.0-Sigma) *  ( EnqRate * 2 -1.0)
	reward = DelayReward +   EnqReward

	if (reward < -1.0): reward = -1.0
	if (reward > 1.0): reward =  1.0
	return reward

def plot(delayRange,RateRange,DropRange,xname,yname):
	fig = plt.figure()
	ax = fig.add_subplot(111, projection='3d')	
	for delay in delayRange:
		for rate in RateRange:
			for drop in DropRange:
				reward = getReward(delay,rate,drop)
				if len(delayRange) >1 and len(RateRange) >1:
					ax.scatter(delay, rate, reward)#, c=c, marker=m)
				elif len(delayRange) >1 and len(DropRange) >1:
					ax.scatter(delay, drop, reward)#, c=c, marker=m)
				elif len(DropRange) >1 and len(RateRange) >1:
					ax.scatter(drop, rate, reward)#, c=c, marker=m)
			
	ax.set_xlabel(xname)
	ax.set_ylabel(yname)
	ax.set_zlabel('Reward')
		
	plt.savefig("reward"+xname+yname+".jpg",format='jpg', dpi=500,bbox_inches='tight')
	plt.close(fig)

print("Delay-EnqCount")
plot(range(1,20),range(0,100),[0],'Delay', 'EnqCount')
print("Delay-Drop")
plot(range(1,20),[100],range(0,100),'Delay', 'Drop')
print("Drop-EnqCount")
plot([0],range(0,100),range(0,100),'Drop', 'EnqCount')
print("Finish")
