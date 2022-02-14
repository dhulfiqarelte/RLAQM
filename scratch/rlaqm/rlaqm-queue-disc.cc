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
                   QueueSizeValue (QueueSize ("5000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("BNCapacity", "Capacity of outgoing link (bottleneck) in Bytes/sec.",
                   DoubleValue (100000000.0/8),
                   MakeDoubleAccessor (&RLAQMQueueDisc::m_capacity),
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
  double u =  m_uv->GetValue ();
  //if (GetQDelayMs()<1.0) m_drop_prob_cl = 0.0;
  if (u < m_drop_prob_cl)
  {
      NS_LOG_LOGIC ("AQM Drop -- dropping pkt");
      DropBeforeEnqueue (item, AQM_DROP);
      return false;
  }
  //m_rout += item->GetSize();
  bool retval = GetInternalQueue (0)->Enqueue (item);
  if (retval)
	  s_in += item->GetSize();

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
  s_out += item->GetSize();
 
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

  return (1000.0*GetInternalQueue (0)->GetNBytes ()) / m_capacity;
/*  Ptr<const QueueDiscItem> item = DoPeek ();
  if (!item) return 0.0;
  Time delta = Simulator::Now () - item->GetTimeStamp ();
  return delta.ToDouble (Time::MS);*/
}

double 
RLAQMQueueDisc::GetUtilization (void)
{
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
//  if ((GetQDelayMs()<1.0) && (m_drop_prob_cl>0.0)) return -1.0*m_drop_prob_cl;
//  if (m_util == -1) return -1.0;
  float reward = (GetUtilization()-0.5) + (2.0/(1.0+GetQDelayMs()/5.0)-1.5);
  return reward;
}

void 
RLAQMQueueDisc::SetDropProb(float dropprob)
{
  if (dropprob>0.5) m_drop_prob_cl = 0.5;
  else if (dropprob<0.0) m_drop_prob_cl = 0.0;
  else m_drop_prob_cl = dropprob;
}

float
RLAQMQueueDisc::GetDropProb()
{
  return m_drop_prob_cl;
}

void
RLAQMQueueDisc::Reset()
{
//  if (m_rin==0.0) { // || GetQDelayMs()<1.0) {
//	  m_util = -1.0;
//  }
//  else {
	double delta = (Simulator::Now()-m_last_time).GetSeconds();
  	double res = 0.0;
//	if ((m_rin>0) && (delta > 0.0))
		res = double(m_rout)/(m_capacity*delta);
  	m_util = res>1.0?1.0:res;
//  }
  m_rin = 0;
  m_rout = 0;
  m_last_time = Simulator::Now();
}

} // namespace ns3
