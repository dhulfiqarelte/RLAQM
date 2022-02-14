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
 
 /*
	n0 -------- n1 ---------- n2
	    1Gbps       90Mbps
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include "mygym2.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Interference-Pattern");


int main (int argc, char *argv[])
{
	//
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  LogComponentEnable ("MyGym2Env", LOG_LEVEL_ALL);
	
  // Parameters of the environment
  double simulationTime = 10; //seconds
  // bool gymSim = true;

//  if (gymSim) {
	  uint32_t simSeed = 1;  
	  bool enableFading = false;

	  double envStepTime = 0.1; //seconds, ns3gym env step time interval
	  uint32_t openGymPort = 5555;
      uint32_t testArg = 0;

	  std::cout<<"------start sim"<<std::endl;

	  CommandLine cmd;
	  // required parameters for OpenGym interface
	  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
	  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
	  // optional parameters
	  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
	  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
	  cmd.AddValue ("enableFading", "If fading should be enabled. Default: false", enableFading);
	  cmd.Parse (argc, argv);

		NS_LOG_UNCOND("Ns3Env parameters:");
	  NS_LOG_UNCOND("--simulationTime: " << simulationTime);
	  NS_LOG_UNCOND("--openGymPort: " << openGymPort);
	  NS_LOG_UNCOND("--envStepTime: " << envStepTime);
	  NS_LOG_UNCOND("--seed: " << simSeed);
	  NS_LOG_UNCOND("--testArg: " << testArg);
	  
	  RngSeedManager::SetSeed (1);
	  RngSeedManager::SetRun (simSeed);

	  // OpenGym Env
	  Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
	  //mygym parameterezes
	  Ptr<MyGym2Env> myGymEnv = CreateObject<MyGym2Env> (1);
	  myGymEnv->SetOpenGymInterface(openGymInterface);
	  // myGymEnv->SetSize("20M")
	  
	  // std::ostringstream oss;
	  // oss << "/NodeList/0/DeviceList/0/$ns3::NonCommunicatingNetDevice/Phy/AveragePowerSpectralDensityReport";
	  // Config::ConnectWithoutContext (oss.str (),MakeBoundCallback (&MyGym2Env::PerformCca, myGymEnv, 8));
  //}
  
  // Traffic Control
  TrafficControlHelper tchFifo;
  tchFifo.SetRootQueueDisc ("ns3::FifoQueueDisc");  //"MaxSize",StringValue("1K")

  
  NodeContainer nodes;
  nodes.Create (3);

  // Install IP stack
  InternetStackHelper stack;
  stack.InstallAll ();


  PointToPointHelper pointToPoint1Gbps;
  pointToPoint1Gbps.SetDeviceAttribute  ("DataRate", StringValue ("1Gbps"));
  pointToPoint1Gbps.SetChannelAttribute ("Delay", StringValue ("1ms"));
  pointToPoint1Gbps.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));
  PointToPointHelper pointToPoint100Mbps;
  pointToPoint100Mbps.SetDeviceAttribute  ("DataRate", StringValue ("90Mbps"));
  pointToPoint100Mbps.SetChannelAttribute ("Delay", StringValue ("1ms"));
  pointToPoint100Mbps.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));
  
  NodeContainer n0n1;
  n0n1.Add(nodes.Get(0));
  n0n1.Add(nodes.Get(1));
  NodeContainer n1n2;
  n1n2.Add(nodes.Get(1));
  n1n2.Add(nodes.Get(2));
  
  NetDeviceContainer d0d1 = pointToPoint100Mbps.Install(n0n1);
  NetDeviceContainer d1d2 = pointToPoint1Gbps.Install(n1n2);
  
  QueueDiscContainer qdiscs = tchFifo.Install (d1d2.Get(0));
  Ptr<QueueDisc> q = qdiscs.Get (0);
  
	// TraceSources
	// Enqueue: Enqueue a packet in the queue disc
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// Dequeue: Dequeue a packet from the queue disc
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// Requeue: Requeue a packet in the queue disc
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// Drop: Drop a packet stored in the queue disc
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// DropBeforeEnqueue: Drop a packet before enqueue
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// DropAfterDequeue: Drop a packet after dequeue
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// Mark: Mark a packet stored in the queue disc
	// Callback signature: ns3::QueueDiscItem::TracedCallback
	// PacketsInQueue: Number of packets currently stored in the queue disc
	// Callback signature: ns3::TracedValueCallback::Uint32
	// BytesInQueue: Number of bytes currently stored in the queue disc
	// Callback signature: ns3::TracedValueCallback::Uint32
	// SojournTime: Sojourn time of the last packet dequeued from the queue disc
	// Callback signature: ns3::Time::TracedCallback
	
  q->TraceConnectWithoutContext ("BytesInQueue", MakeBoundCallback (&MyGym2Env::StoredBytesChanged, myGymEnv));
  // Config::ConnectWithoutContext ("/NodeList/1/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime",
                                  // MakeCallback (&MyGym2Env::ServerSend, myGymEnv));
// Config::ConnectWithoutContext (oss.str (),MakeBoundCallback (&MyGym2Env::ServerSend, myGymEnv));

  
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);

  Ptr<Node> client = n0n1.Get(0);
  Ptr<Node> server = n1n2.Get(1);

//  Address clientAddress = Address(i0i1.GetAddress(0));
  Address serverAddress = Address(i1i2.GetAddress(1));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  uint16_t port = 4000;
  UdpServerHelper serverApp (port);
  ApplicationContainer apps = serverApp.Install (server);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 0;
  Time interPacketInterval = Seconds (0.01);		//100 Mbps
  UdpClientHelper clientApp (serverAddress, port);
  clientApp.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  clientApp.SetAttribute ("Interval", TimeValue (interPacketInterval));
  clientApp.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = clientApp.Install (client);
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

  // std::ostringstream oss;
  // oss << "/NodeList/2/ApplicationList/0/$ns3::UdpServer/Rx";
  // Config::ConnectWithoutContext (oss.str (),MakeBoundCallback (&MyGym2Env::ServerSend, myGymEnv));
  

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");
  myGymEnv->NotifySimulationEnd();
  
  Simulator::Destroy ();
  NS_LOG_UNCOND ("Simulation exit");
}
