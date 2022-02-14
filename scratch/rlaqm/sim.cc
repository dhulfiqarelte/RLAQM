/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ELTE Eötvös Loránd University, Budapest, Hungary
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
 * Author: Sandor Laki <lakis@inf.elte.hu>
 */
 
 /*
	n0 -------- n1 ---------- n2
	    1Gbps       100Mbps
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <string>
#include <cassert>
#include <map>
#include <vector>

#include "rlaqm-queue-disc.h"
#include "rlaqm-gymenv.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace ns3;

static std::ofstream THFILE;

void
InstQueueSizeTrace (Ptr<OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newValue << std::endl;
}

void
CheckQueueSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();


  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue);

  std::ofstream fPlotQueueAvg ("pie_delay.csv", std::ios::out|std::ios::app);
  fPlotQueueAvg << Simulator::Now ().GetSeconds () << " " << (double)(qSize)*8.0 / 1e8 << std::endl;
  fPlotQueueAvg.close ();
}



void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon)
{	
	flowMon->CheckForLostPackets(); 
	std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
	Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
	{	
		THFILE<<"FI;"<< Simulator::Now().GetSeconds()<<";";
		Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
		THFILE << stats->first <<";"<< fiveTuple.sourceAddress <<";"<<fiveTuple.destinationAddress<<";";
		//std::cout<<"Duration		: "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
		//std::cout<<"Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
		THFILE<< stats->second.rxBytes << std::endl;
	}	
	Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon);		
}



int main (int argc, char *argv[])
{
	ns3::LogComponentDisable ("ns3::RLAqmGymEnv",LOG_LEVEL_ALL);
	
	std::cout << "Starting ns-3\n";
	THFILE.open("thlog.csv");

	//Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1400));
	Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (131072*3)); //4
	Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*3));
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
	  // 42 = headers size
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1400 - 42));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));
	Config::SetDefault ("ns3::RLAQMQueueDisc::MaxSize", StringValue ("5000p"));
  // Parameters of the environment
  double simulationTime = 120; //seconds

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
	  Ptr<RLAqmGymEnv> myGymEnv = CreateObject<RLAqmGymEnv> ();
	  myGymEnv->SetOpenGymInterface(openGymInterface);
	  
  
  // Traffic Control
  TrafficControlHelper tchFifo;
  tchFifo.SetRootQueueDisc ("ns3::RLAQMQueueDisc", "BNCapacity", DoubleValue(1e7/8), "MaxSize",StringValue("5000p"));

  
  NodeContainer n;
  n.Create (2);
  NodeContainer srcs;
  NodeContainer sinks;
  srcs.Create(50);
  sinks.Create(50);
  // Install IP stack
  InternetStackHelper stack;
  stack.InstallAll ();


  PointToPointHelper pointToPoint1Gbps;
  // CsmaHelper pointToPoint1Gbps;
  pointToPoint1Gbps.SetDeviceAttribute  ("DataRate", StringValue ("1Gbps"));
  pointToPoint1Gbps.SetChannelAttribute ("Delay", StringValue ("1ms"));
//  pointToPoint1Gbps.SetChannelAttribute  ("DataRate",  DataRateValue (DataRate (5000000)));
//  pointToPoint1Gbps.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  pointToPoint1Gbps.SetQueue ("ns3::DropTailQueue");
  PointToPointHelper pointToPoint100Mbps;
  //CsmaHelper pointToPoint100Mbps;
  pointToPoint100Mbps.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  pointToPoint100Mbps.SetChannelAttribute ("Delay", StringValue ("50ms"));
//  pointToPoint100Mbps.SetChannelAttribute  ("DataRate",  DataRateValue (DataRate (5000000)));
//  pointToPoint100Mbps.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  pointToPoint100Mbps.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("5000p"));


   TrafficControlHelper tchPfifo;
   uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
   tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
  
   NodeContainer n0[50]; 
   NetDeviceContainer c0[50];
   NodeContainer n01[50]; 
   NetDeviceContainer c01[50];
   Ipv4InterfaceContainer sinkips[50];
   for (int i=0;i<50;++i) {
	   n0[i] = NodeContainer (srcs.Get (i), n.Get (0));
           c0[i] = pointToPoint1Gbps.Install (n0[i]);
	   tchPfifo.Install(c0[i]);
	   n01[i] = NodeContainer (n.Get (1), sinks.Get (i));
           c01[i] = pointToPoint1Gbps.Install (n01[i]);
	   tchPfifo.Install(c01[i]);
   }

   NodeContainer n1 = NodeContainer (n.Get (0), n.Get (1));
   NetDeviceContainer c1 = pointToPoint100Mbps.Install (n1);
 

  //QueueDiscContainer qdiscs = tchFifo.Install (d1d2.Get(0));
  QueueDiscContainer qdiscs = tchFifo.Install (c1.Get(0));
  Ptr<QueueDisc> q = qdiscs.Get (0);

  tchPfifo.Install (c1.Get(1));
 
  myGymEnv->SetQueueDisc( q ); // TODO: Qdisc connected to gymenv

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> qsizeStream = asciiTraceHelper.CreateFileStream ("inst-qsize.plotme");
  q->TraceConnectWithoutContext ("BytesInQueue", MakeBoundCallback (&InstQueueSizeTrace, qsizeStream));

  Ipv4AddressHelper ipv4;

  for (int i=0;i<50;++i) {
	  std::stringstream ss;
	  ss << "10.0." << i+1 << ".0";
        ipv4.SetBase (ss.str().c_str(), "255.255.255.0");
  	ipv4.Assign (c0[i]);
  }

  for (int i=0;i<50;++i) {
	  std::stringstream ss;
	  ss << "10.10." << i+1 << ".0";
        ipv4.SetBase (ss.str().c_str(), "255.255.255.0");
  	sinkips[i] = ipv4.Assign (c01[i]);
  }



  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (c1);
  
//  Ptr<Node> client = n.Get(0);
//  Ptr<Node> server = n.Get(1);

  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // TCP Applications
  
  //NS_LOG_INFO ("Create TCP Applications.");
  //NS_LOG_INFO  ("TCP Server");
  // we use the TCP version other than the default ns3 tcp 
  //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpTahoe"));
 
  int nTCP = 1;
  int srcIdx = 0;
  uint16_t port = 51000;
  int sections = 5;
  int nFlows[sections] = {1,1,3,5,20};   //{1,2,5,10,100,10,5,2,1}
					// 1 - 0 - simTime
					// 1 - sectionTime - simTime - sectionTime
					// 3 - 2*sectionTime - simTime - 2*sectionTime
					// 5 - 3*sectionTime - simTime - 3*sectionTime
					// 90 - 4*sectionTime - simTime - 4*sectionTime
					
  int sectionTime = round(simulationTime / ((2*sections)-1));
  
  float startTime = 0.1;
  float stopTime = simulationTime;
  std::vector<ApplicationContainer> appvector;
  int vn=0;

  for (int i=0; i<sections; i++) {
		startTime = i * sectionTime;
		stopTime = simulationTime - i*sectionTime;

		for (int j=0; j< nFlows[i]; j++) {
			PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port+i));
			appvector.push_back(sink.Install (sinks.Get ((int)(srcIdx/nTCP))));
			appvector[vn].Start (Seconds (startTime));
			appvector[vn].Stop (Seconds (stopTime));
	       		++vn;

			OnOffHelper tcp_sender("ns3::TcpSocketFactory", Address(InetSocketAddress ( sinkips[(int)(srcIdx/nTCP)].GetAddress(1), port+i)));
			tcp_sender.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
			tcp_sender.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
			//tcp_sender.SetAttribute ("MaxBytes", UintegerValue (0));
			tcp_sender.SetAttribute 
				        ("DataRate", DataRateValue (DataRate ("1Gb/s")));
			tcp_sender.SetAttribute 
				              ("PacketSize", UintegerValue (1400));
			//tcp_sender.SetConstantRate (DataRate ("168Mbps"), 1400);
			appvector.push_back(tcp_sender.Install (srcs.Get ((int)(srcIdx/nTCP))));
			appvector[vn].Start (Seconds (startTime+0.01));
			appvector[vn].Stop (Seconds (stopTime-0.01));
			++vn;
			++srcIdx;
		}
		
		port++;
  }
 
  /*
 
  // Create a packet sink to receive packets on n2
   PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
   //ApplicationContainer apps = sink.Install (n1n2.Get (1));
    ApplicationContainer apps = sink.Install (n.Get (2));
   apps.Start (Seconds (0.0));
   apps.Stop (Seconds (1000.0)); 
   // Create the OnOff applications to send TCP to the server
   OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
      

      //clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      //clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
      clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("168Mbps")));
     // First Flow in the node client 
   ApplicationContainer clientApps1;
   NS_LOG_INFO  ("TCP Flow1");
      AddressValue remoteAddress1 (InetSocketAddress (Ipv4Address ("10.0.2.2") , port));
	clientHelper.SetAttribute ("Remote", remoteAddress1);
	clientApps1.Add (clientHelper.Install (n.Get (0)));
	clientApps1.Start (Seconds (1.0));
	clientApps1.Stop (Seconds (800.0));
*/
	/*
   // Second  Flow in the node client 
   ApplicationContainer clientApps2;
   NS_LOG_INFO  ("TCP Flow2");
      AddressValue remoteAddress2 (InetSocketAddress (i1i2.GetAddress(1) , 5200));
	clientHelper.SetAttribute ("Remote", remoteAddress2);
	clientApps2.Add (clientHelper.Install (client));
	clientApps2.Start (Seconds (1.0));
	clientApps2.Stop (Seconds (950.0));
 */
 // Generate PCAP files at the sender and the sender 
   //pointToPoint1Gbps.EnablePcap("Router.pcap", d1d2.Get (0), true, true);
   //pointToPoint1Gbps.EnablePcap("Sender.pcap", d0d1.Get (0), true, true);
   
//   pointToPoint1Gbps.EnablePcap("Router.pcap", c1.Get (0), true, true);
//   pointToPoint1Gbps.EnablePcap("Sender.pcap", c0.Get (1), true, true);
   
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  ThroughputMonitor(&fmHelper ,allMon);

  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");
  THFILE.close();
  openGymInterface->NotifySimulationEnd();
  
  Simulator::Destroy ();
  NS_LOG_UNCOND ("Simulation exit");
}
