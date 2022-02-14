#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import scipy.io as io
import gym
import tensorflow as tf
import tensorflow.contrib.slim as slim
import numpy as np
import matplotlib.pyplot as plt
from tensorflow import keras
from ns3gym import ns3env
import os
import sys
import random
from tensorflow.keras.models import load_model
import plotting2

os.system("export PYTHON=/usr/bin/python3.6")
os.system("(cd ../.. && ./waf build)")
cwSize = 5
env = gym.make('ns3-v0')
max_env_steps = 100
env._max_episode_steps = max_env_steps
env.reset()
ob_space = env.observation_space
ac_space =  env.action_space
print("Observation space: ", ob_space,  ob_space.shape)
print("Action space: ", ac_space, ac_space.dtype)
s_size = ob_space.shape[0]

class DqnAgent(object):
    def __init__(self, fname):
        super(DqnAgent, self).__init__()
        self.model = load_model(fname)

    def get_action(self, state):
        action = np.argmax(self.model.predict(state)[0]);
        return action

    def predict(self, next_state):
        return self.model.predict(next_state)[0]


env.reset()
agent0 = DqnAgent('best_model.h5')
#agent0 = DqnAgent('last_model.h5')
state = env.reset()
state = np.reshape(state, [1, s_size])
rewardsum = 0
stepIdx = 0

with open("log/rewards-Best.csv","w") as rewardFile:
    rewardFile.write("time;delayMs;Util;dropProb;reward\n")


    try:
        if True:
            for time in range(6000):
                stepIdx += 1
                action0 = agent0.get_action(state)
        
                actionVec= [action0]
                next_state, reward, done, info = env.step(actionVec)
                print("Step: ", stepIdx)
                print("action0: ",action0)
                print("---obs[MqDealy, Utili, DropP], reward, done, info: ", next_state, reward, done, info)
                
                rewardFile.write(str(time*12/600)+";")
                rewardFile.write(str(next_state[0])+";")
                rewardFile.write(str(next_state[1])+";")
                rewardFile.write(str(next_state[2])+";")
                #rewardFile.write(str(Dq)+";")
                rewardFile.write(str(reward)+"\n")
                print("\n --- Press Ctrl+C to close the agent. ---");
        
                state = np.reshape(next_state, [1, s_size])
        
                rewardsum += reward
    
    except:
        print("Closing...")
        print(sys.exc_info())

print("Reward sum: %f" % rewardsum)
plotting2.plot("Best")
print ("Drawing the throughput of the Bottleneck")
os.system('python3 ../../throughput.py')

