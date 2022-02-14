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

#ifndef RLAQM_GYMENV_H
#define RLAQM_GYMENV_H

#include "ns3/stats-module.h"
#include "ns3/opengym-module.h"

#include "rlaqm-queue-disc.h"

namespace ns3 {

class RLAqmGymEnv : public OpenGymEnv
{
public:
  RLAqmGymEnv ();
  virtual ~RLAqmGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);

  void NotifyAgent();
  void SetQueueDisc( Ptr<QueueDisc> qd );

private:
  Ptr<RLAQMQueueDisc> m_qd;
  bool m_started {false};
  Time m_timeStep {Seconds(0.02)}; // 20 ms
};

}


#endif // RLAQM_GYMENV_H
