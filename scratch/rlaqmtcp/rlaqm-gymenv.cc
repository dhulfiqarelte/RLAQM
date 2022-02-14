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

#include "rlaqm-gymenv.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RLAqmGymEnv");

NS_OBJECT_ENSURE_REGISTERED (RLAqmGymEnv);

RLAqmGymEnv::RLAqmGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

RLAqmGymEnv::~RLAqmGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
RLAqmGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("RLAqmGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<RLAqmGymEnv> ()
  ;
  return tid;
}

void
RLAqmGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<OpenGymSpace>
RLAqmGymEnv::GetActionSpace()
{
  NS_LOG_FUNCTION (this);

  // 0- nop, 1- add incr., 2- add decr., 3-mul. incr., 4- mul. decr.
  uint32_t parameterNum = 1; /* A single action parameter - action id */
  int low = 0;
  int high = 4;
  std::vector<uint32_t> shape = {parameterNum,};
  std::string dtype = TypeNameGet<int> ();

  Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_INFO ("MyGetActionSpace: " << box);
  return box;

}

Ptr<OpenGymSpace>
RLAqmGymEnv::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);
  float low = 0.0;
  float high = 1000.0;
  std::vector<uint32_t> shape = {3,}; /* A obs - double qdelay in ms + utilization + drop_prob. */
  std::string dtype = TypeNameGet<float> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetObservationSpace: " << space);
  return space;
}

bool
RLAqmGymEnv::GetGameOver()
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
RLAqmGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> shape = {1,};
  Ptr<OpenGymBoxContainer<float> > box = CreateObject<OpenGymBoxContainer<float> >(shape);

  float val = 0.0;
  float val2 = 1.0;
  float val3 = 0.0;
  if (m_qd)
  {
    val = (float) m_qd->GetQDelayMs();
    val2 = (float) m_qd->GetUtilization();
    val3 = (float) m_qd->GetDropProb();
  }
  box->AddValue( val );
  box->AddValue( val2 );
  box->AddValue( val3 );

  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

float
RLAqmGymEnv::GetReward()
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
RLAqmGymEnv::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  std::stringstream ss;
  std::string myInfo = "info ";
  ss << "T: " << Simulator::Now().GetSeconds()<< " QDealy = " << m_qd->GetQDelayMs() << " r_in= " << m_qd->inrate() << " r_out= " << m_qd->outrate() << " DoEnqueue " << m_qd->M_rinEnQ()<< " DropProb =" << m_qd->GetDropProb() << " DropofProb = " << m_qd->Drop_Count() ;
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo + ss.str();
}

bool
RLAqmGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymBoxContainer<int> > box = DynamicCast<OpenGymBoxContainer<int> >(action);
  int action_id = box->GetValue(0);
  double p = m_qd->GetDropProb();
  switch (action_id)
  {
	 case 0:  //nop
		  break;
	 case 1:
		  p = p+0.01;
		  break;
	 case 2: 
	  	  p = p-0.01;
		  break;
	 case 3:
		  if (p>0)   p += 0.1; //p = p + 0.1 ;
		  else p = 0.01;
		  break;
	 case 4:  
		  p -=0.1; // - 0.1; 
		  break;
  }

  m_qd->SetDropProb( p );

  NS_LOG_UNCOND ("Drop prob.: " << p);
  return true;
}

void 
RLAqmGymEnv::NotifyAgent()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_timeStep, &RLAqmGymEnv::NotifyAgent, this);
   //m_qd->Reset();
  if (m_qd->GetUtilization()!=-1.0)
  {
  	Notify();
  }
}

void 
RLAqmGymEnv::SetQueueDisc( Ptr<QueueDisc> qd )
{
  NS_LOG_FUNCTION (this);
  m_qd = DynamicCast<RLAQMQueueDisc, QueueDisc>(qd);
  Simulator::Schedule (m_timeStep, &RLAqmGymEnv::NotifyAgent, this);
}



} // ns3 namespace
