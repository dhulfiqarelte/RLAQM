#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import scipy.io as io
import gym
import tensorflow as tf
#import tensorflow.contrib.slim as slim
import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
from tensorflow import keras
from ns3gym import ns3env
import os
import random
from tensorflow.keras.models import load_model
import plotting
import matplotlib
from matplotlib import pyplot as plt
import pandas as pd

def plot_flowth(idx):
    df=pd.read_csv(r'../../thlog.csv', header=None, names=['#','Time', 'FlowID' , 'Src' , 'DST' , 'Throughput'], sep=';')
    df['Throughput']=df['Throughput']*8
    #df = df[df['Src']=='10.0.1.1']
    df = df[df['Src'].str.contains('10.0.')]
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
    plt.savefig("log/thlog_{}.png".format(idx))
    plt.close()

    plt.figure(figsize=(8,2))
    for i in range(1,201):
        x = df[df['FlowID']==i]['Time'].values
        y = df[df['FlowID']==i]['Throughput'].values
        y2 = [(y[j]-y[j-1])/1e6 for j in range(1,len(y))]
        plt.plot(x[1:],y2)
    plt.grid()
    plt.xlabel('Time (sec)')
    plt.ylabel('Mbps')
    plt.savefig('log/flowthlog_{}.png'.format(idx))
    plt.close()


os.system("export PYTHON=/usr/bin/python3.6")
os.system("(cd ../.. && ./waf build)")
cwSize = 5
env = gym.make('ns3-v0')
total_episodes = 50
#max_env_steps = 100
#env._max_episode_steps = max_env_steps
env.reset()
epsilon = 0.9				# exploration rate
epsilon_min = 0.01
epsilon_decay = 0.9
ob_space = env.observation_space
ac_space =	env.action_space
print("Observation space: ", ob_space,	ob_space.shape)
print("Action space: ", ac_space, ac_space.dtype)
s_size = ob_space.shape[0]
time_history = []
rew_history = []
maxrew = 0

class DqnAgent(object):
	def __init__(self, inNum, outNum):
		super(DqnAgent, self).__init__()
		self.model = keras.Sequential()
		self.model.add(keras.layers.Dense(inNum, input_shape=(inNum,), activation='relu'))
		self.model.add(keras.layers.Dense(32, activation='relu'))
		self.model.add(keras.layers.Dense(32, activation='relu'))
		self.model.add(keras.layers.Dense(outNum, activation='softmax'))

		self.model.compile(optimizer=tf.compat.v1.train.AdamOptimizer(0.001), loss='mse', metrics=['accuracy'])

	def get_action(self, state):
		#print(state)
		if state[0][0]<1.0 and state[0][2]==0.0:
		   action = 0 #env.action_space.sample()
		elif state[0][0]<1.0 and state[0][2]>0.0:
		   action = 4
		elif np.random.rand(1) < epsilon:
		   action = env.action_space.sample()
		else:
		   action = np.argmax(self.model.predict(state)[0]);
		return action

	def predict(self, next_state):
		return self.model.predict(next_state)[0]

	def fit(self, state, target, action):
		target_f = self.model.predict(state)
		#print(target_f)
		target_f[0][action] = target
		self.model.fit(state, target_f, epochs=1, verbose=0)

	def save_model(self, fname):
		self.model.save(fname)

	def load_model(self, fname):
		del self.model
		self.model = load_model(fname)


env.reset()
agent0 = DqnAgent(3, cwSize)

for e in range(total_episodes):
	state = env.reset()
	state = np.reshape(state, [1, s_size])
	rewardsum = 0
	stepIdx = 0;#print("episode: %d/%d" % (e+1, total_episodes))
	print("episode: %d/%d" % (e+1, total_episodes))
	with open("log/rewards-"+str(e)+".csv","w") as rewardFile:
		rewardFile.write("time;delayMs;Util;dropProb;reward\n")
		for time in range(6000):
			stepIdx += 1
			action0 = agent0.get_action(state)

			actionVec= [action0]
			next_state, reward, done, info = env.step(actionVec)
			print("Step: ", stepIdx)
			print("action0: ",action0)
			print("---obs, reward, done, info: ", next_state, reward, done, info)
			rewardFile.write(str(time)+";")
			rewardFile.write(str(0.001*next_state[0])+";")
			rewardFile.write(str(next_state[1])+";")
			rewardFile.write(str(next_state[2])+";")
			rewardFile.write(str(reward)+"\n")

			if done:
				print("episode: {}/{}, time: {}, rew: {}, eps: {:.2}"
					  .format(e, total_episodes, time, rewardsum, epsilon))
				break
	#		 if next_state[0]==0.0 and next_state[1]=1.0 and next_state[2]=0.0:
	#			 continue
#			if reward == -1:
#				continue



			next_state = np.reshape(next_state, [1, s_size])


			# Train
			target0 = reward
			if not done:
				target0 = reward + 0.95 * np.amax(agent0.predict(next_state)[0])
			#	print("TARGET0 = %f" % target0)

			agent0.fit(state, target0, action0)
			state = next_state
			rewardsum += reward
		if epsilon > epsilon_min: epsilon *= epsilon_decay
	if (rewardsum > maxrew):
		maxrew = rewardsum
		agent0.save_model('best_model.h5')
	time_history.append(time)
	rew_history.append(rewardsum)
	agent0.save_model('model-%d.h5' % e)
	plotting.plot(e)  # ez többször szamolja ugyanazt
	plot_flowth(e)

agent0.save_model('last_model.h5')

plt.figure()
plt.plot(range(len(time_history)), time_history, label='TimeH')
plt.plot(range(len(rew_history)), rew_history, label='rewSum')
plt.legend()
plt.xlabel('Episode')
plt.ylabel('Time')
plt.savefig("out2.pdf")


#curve0 = np.zeros(shape=(101,101))
#
#for i in range(101):
#	 for j in range(101):
#		 state = np.array([i,j])
#		 state = np.reshape(state, [1, 2])
#
#		 curve0[i,j] = agent0.get_action(state)
		
#print("Save curves to MATLAB file")
#io.savemat("curves_2d.mat", {
#				 '0':curve0,
#				}
#		   )
