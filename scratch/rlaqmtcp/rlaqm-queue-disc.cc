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
#include "rlaqm-queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RLAQMQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (RLAQMQueueDisc);

TypeId RLAQMQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RLAQMQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<RLAQMQueueDisc> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("BNCapacity", "Capacity of outgoing link (bottleneck) in Bytes/sec.",
                   DoubleValue (100000000.0/8),
                   MakeDoubleAccessor (&RLAQMQueueDisc::m_capacity),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh",
                      "Maximum average length threshold in packets/bytes",
                      DoubleValue (500),
                      MakeDoubleAccessor (&RLAQMQueueDisc::m_maxTh),
                      MakeDoubleChecker<double> ())
  ;
  return tid;
}

RLAQMQueueDisc::RLAQMQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

RLAQMQueueDisc::~RLAQMQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

bool
RLAQMQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  m_rin += item->GetSize();

  if (GetCurrentSize () + item > GetMaxSize ())
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      DropBeforeEnqueue (item, FORCED_DROP);
      return false;
    }
  NS_LOG_LOGIC(" QDELAY " << GetQDelayMs() );
  //double u =  m_uv->GetValue ();
  //float u = 1.0;
  //if (u < m_drop_prob_cl)
  //if (m_rinEnQ > m_maxTh)  // m_maxTh = 15 (RED)
  if (0.8 < m_drop_prob_cl)
  {
      NS_LOG_LOGIC ("AQM Drop -- dropping pkt");
      DropBeforeEnqueue (item, AQM_DROP);
      DropCount++;
      //float Drop = DropCount ;
      //zacum = zacum - Drop;
      //Drop = 0 ; 
      m_drop_prob_cl = 0; 
      
      if (GetQDelayMs()  > 50 ) m_drop_prob_cl =1 ;
//      if (zacum < 800) 
//      {
//      m_drop_prob_cl = 0; 
//       
//      }

      return false;
  }
  
  if (m_drop_prob_cl == 0)
  {
  bool retval = GetInternalQueue (0)->Enqueue (item);
  return retval;
  
  }
  
//  if (zacum > 1000)
//      {
//      DropBeforeEnqueue (item, FORCED_DROP);
//      m_drop_prob_cl = 1;
//      DropCount = 0;
//      return false;
//      }
  m_rinEnQ += item->GetSize();
  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return retval;
}

Ptr<QueueDiscItem>
RLAQMQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

   m_rout += item->GetSize();
  return item;
}

Ptr<const QueueDiscItem>
RLAQMQueueDisc::DoPeek (void)
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
RLAQMQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("RLAQMQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("RLAQMQueueDisc needs no packet filter");
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
      NS_LOG_ERROR ("RLAQMQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

void
RLAQMQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}



double
RLAQMQueueDisc::GetQDelayMs (void)
{
  Ptr<const QueueDiscItem> item = DoPeek ();
  if (!item) { m_drop_prob_cl = 0; return 0.0;}
  Time delta = Simulator::Now () - item->GetTimeStamp ();
  return delta.ToDouble (Time::MS);
}

double
RLAQMQueueDisc::inrate()
{
  Ptr<const QueueDiscItem> item = DoPeek ();
  if (!item) return 0.0;
  return double(m_rin);
}

double
RLAQMQueueDisc::outrate()
{
 
  return double(m_rout);
}

double
RLAQMQueueDisc::M_rinEnQ()
{
 
  return double(m_rinEnQ);
}

int
RLAQMQueueDisc::Drop_Count()
{
 
  return DropCount;
}
double 
RLAQMQueueDisc::GetUtilization (void)
{
   	double delta = (Simulator::Now()-m_last_time).GetSeconds();
  	//double res = double(m_rout)/(1.0+MIN(m_rin,m_capacity*delta));
     double res = double(m_rout)/(m_capacity * delta);
  	m_util = res>1.0?1.0:res;
  return m_util;
}
/*  if (m_rin==0.0) return 1.0;
  double delta = (Simulator::now()-m_last_time).GetSeconds();
  double res = m_rout/MIN(m_rin,m_capacity*delta);
  return res>1.0?1.0:res;
}*/


float
RLAQMQueueDisc::GetReward (void)
{
 float reward; 
//float reward = 0.5*GetUtilization() + 0.3/(1.0+GetQDelayMs()/5.0) + 0.2*(1.0-m_drop_prob_cl);
//      if (GetUtilization() > 0.85) reward = 1;
//       if (GetUtilization() < 0.85 && GetUtilization()> 0.70 ) reward = 0.75;
//       if (GetUtilization() < 0.70 && GetUtilization()> 0.5 ) reward = 0.5;
//       if (GetUtilization() < 0.50) reward = 0.25;

float delay_ref = 10; //ms
float sigma = 0.5;
float delay_reward = sigma * (delay_ref - GetQDelayMs());
float MinDelay = (m_rin * 8) /  (m_capacity) ;
float Enq_Rate = m_rin / (m_rin + DropCount);
float enq_reward= (1-sigma) * (MinDelay - delay_ref)* Enq_Rate;
reward = delay_reward - enq_reward;
//if (reward > 1 ) reward =1;
//if (reward < -1 ) reward = -1;

  return reward;
}

void 
RLAQMQueueDisc::SetDropProb(float dropprob)
{
  if (dropprob>1.0) m_drop_prob_cl = 1.0;
  else if (dropprob<0.0) m_drop_prob_cl = 0.0;
  else m_drop_prob_cl = dropprob;
}

float
RLAQMQueueDisc::GetDropProb()
{
  if (GetQDelayMs() == 0) m_drop_prob_cl = 0; 
  //if (GetQDelayMs() > 10) m_drop_prob_cl = 1;
  //float z = m_rin - m_rout;
  //zacum = zacum + z ; 
  //if (zacum > 1000.00) m_drop_prob_cl = 1;
  return m_drop_prob_cl;
}

void
RLAQMQueueDisc::Reset()
{
  if (m_rin==0.0) {
	  m_util = 0.0;
  }
  else {
	//double delta = (Simulator::Now()-m_last_time).GetSeconds();
  	//double res = double(m_rout)/(1.0+MIN(m_rin,m_capacity*delta));
   //double res = double(m_rout)/(m_capacity);
  	//m_util = res>1.0?1.0:res;
  }
  //m_rin = 0;
  //m_rout = 0;
  m_drop_prob_cl = 0;
  m_last_time = Simulator::Now();
}

} // namespace ns3
