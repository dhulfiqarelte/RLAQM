/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 Eotvos Lorand University, Budapest, Hungary
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
 * Authors: Sandor Laki <lakis@inf.elte.hu>
 */

#include "aqm-rlv1-env.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RLAqm1GymEnv");

NS_OBJECT_ENSURE_REGISTERED (RLAqm1GymEnv);

RLAqm1GymEnv::RLAqm1GymEnv ()
{
  NS_LOG_FUNCTION (this);
}

RLAqm1GymEnv::~RLAqm1GymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
RLAqm1GymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("RLAqm1GymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<RLAqm1GymEnv> ()
  ;
  return tid;
}

void
RLAqm1GymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<OpenGymSpace>
RLAqm1GymEnv::GetActionSpace()
{
  NS_LOG_FUNCTION (this);

  // drop_prob
  uint32_t parameterNum = 1; /* A single action parameter - float drop probability */
  float low = 0.0;
  float high = 1.0;
  std::vector<uint32_t> shape = {parameterNum,};
  std::string dtype = TypeNameGet<float> ();

  Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_INFO ("MyGetActionSpace: " << box);
  return box;

}

Ptr<OpenGymSpace>
RLAqm1GymEnv::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);
  float low = 0.0;
  float high = 1000.0;
  std::vector<uint32_t> shape = {1,}; /* A single observation - double qdelay in ms */
  std::string dtype = TypeNameGet<float> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetObservationSpace: " << space);
  return space;
}

bool
RLAqm1GymEnv::GetGameOver()
{
  NS_LOG_FUNCTION (this);
  static int stepCounter = 0;
  bool isGameOver = false;

  if (++stepCounter == 50000) /* For testing... */
  {
	  isGameOver = true;
  }

  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

Ptr<OpenGymDataContainer>
RLAqm1GymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> shape = {1,};
  Ptr<OpenGymBoxContainer<float> > box = CreateObject<OpenGymBoxContainer<float> >(shape);

  float val = 0.0;
  if (m_qd)
  {
    val = (float) m_qd->GetQDelayMs();
  }
  box->AddValue( val );

  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

float
RLAqm1GymEnv::GetReward()
{
  NS_LOG_FUNCTION (this);
  float reward = 1.0;
  if (m_qd)
  {
    reward = m_qd->GetReward(); //QDelayMs();
  }

  NS_LOG_UNCOND ("MyGetReward: " << reward);
  return reward;
}

std::string
RLAqm1GymEnv::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  std::stringstream ss;
  std::string myInfo = "info ";
  ss << "T: " << Simulator::Now();
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo + ss.str();
}

bool
RLAqm1GymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymBoxContainer<float> > box = DynamicCast<OpenGymBoxContainer<float> >(action);
  float m_drop_prob_cl = box->GetValue(0);
  m_qd->SetDropProb( m_drop_prob_cl );

  NS_LOG_UNCOND ("Drop prob.: " << m_drop_prob_cl);
  return true;
}

void 
RLAqm1GymEnv::NotifyAgent()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_timeStep, &RLAqm1GymEnv::NotifyAgent, this);
  Notify();
}

void 
RLAqm1GymEnv::SetQueueDisc( Ptr<QueueDisc> qd )
{
  NS_LOG_FUNCTION (this);
  m_qd = DynamicCast<RLv1QueueDisc, QueueDisc>(qd);
  Simulator::Schedule (m_timeStep, &RLAqm1GymEnv::NotifyAgent, this);
}



} // ns3 namespace
