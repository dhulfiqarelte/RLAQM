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

#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/core-module.h"
#include "rlv1-queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RLv1QueueDisc");

NS_OBJECT_ENSURE_REGISTERED (RLv1QueueDisc);

TypeId RLv1QueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RLv1QueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<RLv1QueueDisc> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
  ;
  return tid;
}

RLv1QueueDisc::RLv1QueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

RLv1QueueDisc::~RLv1QueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

bool
RLv1QueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  if (GetCurrentSize () + item > GetMaxSize ())
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      DropBeforeEnqueue (item, FORCED_DROP);
	DropCounter ++ ; 
      return false;
    }
  NS_LOG_LOGIC(" QDELAY " << GetQDelayMs() );
  double u =  m_uv->GetValue ();
  if (u < m_drop_prob_cl)
  {
      NS_LOG_LOGIC ("AQM Drop -- dropping pkt");
      DropBeforeEnqueue (item, AQM_DROP);
	DropCounter ++ ; 
      return false;
  }
  

  bool retval = GetInternalQueue (0)->Enqueue (item);
  if (retval ) EnqCounter ++;
  //float ENC() { return DropCounter; }

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());
 

  return retval;
}

Ptr<QueueDiscItem>
RLv1QueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

Ptr<const QueueDiscItem>
RLv1QueueDisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

bool
RLv1QueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("RLv1QueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("RLv1QueueDisc needs no packet filter");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("RLv1QueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

void
RLv1QueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}



double
RLv1QueueDisc::GetQDelayMs (void)
{
  Ptr<const QueueDiscItem> item = DoPeek ();
  if (!item) return 0.0;
  Time delta = Simulator::Now () - item->GetTimeStamp ();
  return delta.ToDouble (Time::MS);
}

float
RLv1QueueDisc::GetReward (void)
{
  float Sigma = 0.5;  
  float DelayRef = 5.0 ; //ms as in the thesis 
  float DelayReward =  Sigma * (DelayRef - GetQDelayMs())/DelayRef; 
  //float MinDelay = ((GetInternalQueue (0)->GetNBytes () * 8.0 ) / (100 * 1000.0)); 
  float EnqRate=  1.0* EnqCounter / (1.0 + EnqCounter + DropCounter);
  float  EnqReward = (1.0-Sigma) *  ( EnqRate * 2 -1.0); // (1-)//(1.0-Sigma) *( MinDelay - DelayRef ) * EnqRate;
   float reward = DelayReward +   EnqReward; 

  if (reward < -1.0) reward = -1.0;
  if (reward > 1.0) reward =  1.0;
   EnqRate = 0;
   //EnqCounter = 0; 
   //DropCounter = 0; 
 
 //float reward = 1.0 - GetQDelayMs()/50.0; // Hardcoded target 50 ms
  return reward;
}

void 
RLv1QueueDisc::SetDropProb(float dropprob)
{
  m_drop_prob_cl = dropprob;
}

} // namespace ns3
