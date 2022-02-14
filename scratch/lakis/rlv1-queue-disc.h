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

#ifndef RLV1_QUEUE_DISC_H
#define RLV1_QUEUE_DISC_H

#include "ns3/queue-disc.h"
#include "ns3/random-variable-stream.h"


namespace ns3 {

class UniformRandomVariable;

/**
 * \ingroup traffic-control
 *
 * Queue disc for OpenGym implementing RL-AQM policy
 *
 */
class RLv1QueueDisc : public QueueDisc {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief RLv1QueueDisc constructor
   *
   * Creates a queue with a depth of 1000 packets by default
   */
  RLv1QueueDisc ();

  virtual ~RLv1QueueDisc();

  // Reasons for dropping packets
  static constexpr const char* AQM_DROP = "AQM drop";  //!< AQM drop: proactive
  static constexpr const char* FORCED_DROP = "Forced drop";      //!< Drops due to queue limit: reactive

  double GetQDelayMs (void);
  float GetReward (void);
  void SetDropProb(float dropprob);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);

  float m_drop_prob_cl {0.0}; // drop prob.
  Ptr<UniformRandomVariable> m_uv;
};

} // namespace ns3

#endif /* RLV1_QUEUE_DISC_H */
