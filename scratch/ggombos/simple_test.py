#!/usr/bin/python3.6
# -*- coding: utf-8 -*-

import gym
import argparse
from ns3gym import ns3env
import os

__author__ = "Piotr Gawlowicz"
__copyright__ = "Copyright (c) 2018, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "gawlowicz@tkn.tu-berlin.de"

# os.system("export PYTHON=/usr/bin/python3.6")		#<--waf ezzel fog forditani
# os.system("$PYTHON -m pip install --ignore-installed src/opengym/model/ns3gym")		#<--ez akkor kell ha valtoztattam a src/gym-ben
# os.system("(cd ../.. && ./waf configure && ./waf build)")	
# os.system("(cd ../.. && ./waf build)")				#<--ez erdemes, mert a gym lenyeli a waf build kimenetelet
# ggombos@mono:~/ns3Gym/ns3-gym/scratch/ggombos$ ./simple_test.py

print("Before make")
env = gym.make('ns3-v0')
print("After make")
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0

try:
	obs = env.reset()
	print("Step: ", stepIdx)
	print("---obs: ", obs)

	while True:
		stepIdx += 1

		action = env.action_space.sample()
		print("---action: ", action)
		obs, reward, done, info = env.step(action)

		print("Step: ", stepIdx)
		print("---obs, reward, done, info: ", obs, reward, done, info)

		if done:
			break

except KeyboardInterrupt:
	print("Ctrl-C -> Exit")
finally:
	env.close()
	print("Done")