/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */

#include "mygym2.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyGym2Env");

NS_OBJECT_ENSURE_REGISTERED (MyGym2Env);

MyGym2Env::MyGym2Env ()
{
  NS_LOG_FUNCTION (this);
  // m_currentNode = 0;
  // m_currentChannel = 0;
  // m_collisionTh = 3;
  // m_channelNum = 1;
  // m_channelOccupation.clear();
}

MyGym2Env::MyGym2Env (uint32_t channelNum)
{
  NS_LOG_FUNCTION (this);
  // m_currentNode = 0;
  // m_currentChannel = 0;
  // m_collisionTh = 3;
  // m_channelNum = channelNum;
  // m_channelOccupation.clear();
}

MyGym2Env::~MyGym2Env ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGym2Env::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGym2Env")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<MyGym2Env> ()
  ;
  return tid;
}

void
MyGym2Env::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<OpenGymSpace>
MyGym2Env::GetActionSpace()
{
  NS_LOG_FUNCTION (this);
  // Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (m_channelNum);
  // 1- up,
  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (1);
  NS_LOG_UNCOND ("GetActionSpace: " << space);
  return space;
}

Ptr<OpenGymSpace>
MyGym2Env::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);
  float low = 0.0;
  float high = 1.0;
//  std::vector<uint32_t> shape = {m_channelNum,};
  std::vector<uint32_t> shape = {1,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetObservationSpace: " << space);
  return space;
}

bool
MyGym2Env::GetGameOver()
{
  NS_LOG_FUNCTION (this);
  bool isGameOver = false;

/*  uint32_t collisionNum = 0;
  for (auto& v : m_collisions)
    collisionNum += v;

  if (collisionNum >= m_collisionTh){
    isGameOver = true;
  }*/
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

Ptr<OpenGymDataContainer>
MyGym2Env::GetObservation()
{
  NS_LOG_FUNCTION (this);
  // std::vector<uint32_t> shape = {m_channelNum,};
  std::vector<uint32_t> shape = {1,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);

  box->AddValue(m_PktBytes);
/*  for (uint32_t i = 0; i < m_channelOccupation.size(); ++i) {
    uint32_t value = m_channelOccupation.at(i);
    box->AddValue(value);
  }
*/
  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

float
MyGym2Env::GetReward()
{
  NS_LOG_FUNCTION (this);
  float reward = m_PktBytes / 20000000;
/*  if (m_channelOccupation.size() == 0){
    return 0.0;
  }
  uint32_t occupied = m_channelOccupation.at(m_currentChannel);
  if (occupied == 1) {
    reward = -1.0;
    m_collisions.erase(m_collisions.begin());
    m_collisions.push_back(1);
  } else {
    m_collisions.erase(m_collisions.begin());
    m_collisions.push_back(0);
  }*/
  NS_LOG_UNCOND ("MyGetReward: " << reward);
  return reward;
}

std::string
MyGym2Env::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  std::string myInfo = "info";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}

bool
MyGym2Env::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);
  // Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  // uint32_t nextChannel = discrete->GetValue();
  // m_currentChannel = nextChannel;

  // NS_LOG_UNCOND ("Current Channel: " << m_currentChannel);
  return true;
}


bool
MyGym2Env::CheckIfReady()
{
  NS_LOG_FUNCTION (this);
  return m_PktBytes == 0;
  // return m_channelOccupation.size() == m_channelNum;
}

void
MyGym2Env::ClearObs()
{
  NS_LOG_FUNCTION (this);
  // m_channelOccupation.clear();
}

void
MyGym2Env::StoredBytesChanged (Ptr<MyGym2Env> entity,uint32_t old_bytes,uint32_t new_bytes)
{
  std::cout<<"--------------serversend--- "<<old_bytes<<" -> "<<new_bytes<<std::endl;
  //m_PktBytes = new_bytes;
  
  // if (entity->CheckIfReady()){
	  entity->Notify();
	  entity->ClearObs();
  // }
}
/*
void
MyGym2Env::PerformCca (Ptr<MyGym2Env> entity, uint32_t channelId, Ptr<const SpectrumValue> avgPowerSpectralDensity)
{
  double power = Integral (*(avgPowerSpectralDensity));
  double powerDbW = 10 * std::log10(power);
  double threshold = -60;
  uint32_t busy = powerDbW > threshold;
  NS_LOG_UNCOND("Channel: " << channelId << " CCA: " << busy << " RxPower: " << powerDbW);

  entity->CollectChannelOccupation(channelId, busy);

  if (entity->CheckIfReady()){
    entity->Notify();
    entity->ClearObs();
  }
}*/

} // ns3 namespace