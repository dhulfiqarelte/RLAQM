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

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace ns3;

static std::ofstream THFILE;

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
	std::cout << "Starting ns-3\n";
	THFILE.open("thlog.csv");

	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1400));
	Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (13100000));
	Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (13100000));
	
	//
  // LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  // LogComponentEnable ("RLAQMQueueDisc", LOG_LEVEL_ALL);
  //LogComponentEnable ("RLAqmGymEnv", LOG_LEVEL_ALL);
	
  // Parameters of the environment
  double simulationTime = 3; //seconds
  // bool gymSim = true;

 
  

  
  //NodeContainer nodes;
  //nodes.Create (3);
  
  NodeContainer n;
  n.Create (3);

  // Install IP stack
  InternetStackHelper stack;
  stack.InstallAll ();


  PointToPointHelper pointToPoint1Gbps;
  // CsmaHelper pointToPoint1Gbps;
  pointToPoint1Gbps.SetDeviceAttribute  ("DataRate", StringValue ("1Gbps"));
  pointToPoint1Gbps.SetChannelAttribute ("Delay", StringValue ("1ms"));
//  pointToPoint1Gbps.SetChannelAttribute  ("DataRate",  DataRateValue (DataRate (5000000)));
//  pointToPoint1Gbps.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  pointToPoint1Gbps.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
  PointToPointHelper pointToPoint100Mbps;
  //CsmaHelper pointToPoint100Mbps;
  pointToPoint100Mbps.SetDeviceAttribute  ("DataRate", StringValue ("100Mbps"));
  pointToPoint100Mbps.SetChannelAttribute ("Delay", StringValue ("10ms"));
//  pointToPoint100Mbps.SetChannelAttribute  ("DataRate",  DataRateValue (DataRate (5000000)));
//  pointToPoint100Mbps.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  pointToPoint100Mbps.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
  
  /*NodeContainer n0n1;
  n0n1.Add(nodes.Get(0));
  n0n1.Add(nodes.Get(1));
  NodeContainer n1n2;
  n1n2.Add(nodes.Get(1));
  n1n2.Add(nodes.Get(2));
  */
  
   NodeContainer n0 = NodeContainer (n.Get (0), n.Get (1));
   NodeContainer n1 = NodeContainer (n.Get (1), n.Get (2));
  
  //NetDeviceContainer d0d1 = pointToPoint1Gbps.Install(n0n1);
  //NetDeviceContainer d1d2 = pointToPoint100Mbps.Install(n1n2);
  
     NetDeviceContainer c0 = pointToPoint1Gbps.Install (n0);
     NetDeviceContainer c1 = pointToPoint100Mbps.Install (n1);
  
  //QueueDiscContainer qdiscs = tchFifo.Install (d1d2.Get(0));
  // QueueDiscContainer qdiscs = tchFifo.Install (c1.Get(1));
  // Ptr<QueueDisc> q = qdiscs.Get (0);
 
  // myGymEnv->SetQueueDisc( q ); // TODO: Qdisc connected to gymenv

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  ipv4.Assign (c0);
 // Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);
 // Ipv4InterfaceContainer i0i1 = ipv4.Assign (c0);

  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  ipv4.Assign (c1);
  //Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
 //Ipv4InterfaceContainer i1i2 = ipv4.Assign (c1);

  //Ptr<Node> client = n0n1.Get(0);
  //Ptr<Node> server = n1n2.Get(1);
  
   Ptr<Node> client = n0.Get(0);
  Ptr<Node> server = n1.Get(1);

    //Address clientAddress = Address(i0i1.GetAddress(0));
    //Address serverAddress = Address(i1i2.GetAddress(1));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
// UDP Application 
 /* 
   NS_LOG_INFO ("Create UDP Applications.");
  uint16_t port = 4000;
  UdpServerHelper serverApp (port);
  ApplicationContainer apps = serverApp.Install (server);
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (1000.0));

  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 1000000000;
  Time interPacketInterval = Seconds (0.00005);		//168 Mbps
  UdpClientHelper clientApp (serverAddress, port);
  clientApp.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  clientApp.SetAttribute ("Interval", TimeValue (interPacketInterval));
  clientApp.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = clientApp.Install (client);
  apps.Start (Seconds (0.1));
  apps.Stop (Seconds (1000.0));
*/
  // std::ostringstream oss;
  // oss << "/NodeList/2/ApplicationList/0/$ns3::UdpServer/Rx";
  // Config::ConnectWithoutContext (oss.str (),MakeBoundCallback (&MyGym2Env::ServerSend, myGymEnv));
 
  // TCP Applications
  
  //NS_LOG_INFO ("Create TCP Applications.");
  //NS_LOG_INFO  ("TCP Server");
  // we use the TCP version other than the default ns3 tcp 
  	//Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpTahoe"));
 
  int nTCP = 1;
  uint16_t port = 51000;
  int sections = 5;
  int nFlows[sections] = {1,1,3,5,90};   //{1,2,5,10,100,10,5,2,1}
					// 1 - 0 - simTime
					// 1 - sectionTime - simTime - sectionTime
					// 3 - 2*sectionTime - simTime - 2*sectionTime
					// 5 - 3*sectionTime - simTime - 3*sectionTime
					// 90 - 4*sectionTime - simTime - 4*sectionTime
					
  int sectionTime = simulationTime / (2*sections-1);
  
  float startTime = 0.1;
  float stopTime = simulationTime;
  for (int i=0; i<sections; i++) {
		startTime = i * sectionTime;
		stopTime = simulationTime - i*sectionTime;

		PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
		ApplicationContainer apps = sink.Install (n.Get (2));
		apps.Start (Seconds (startTime));
		apps.Stop (Seconds (stopTime)); 
	 
		for (int j=0; j< nFlows[i]*nTCP; j++) {
			OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
			clientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
			clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("168Mbps")));
			ApplicationContainer clientApps1;
			AddressValue remoteAddress1 (InetSocketAddress (Ipv4Address ("10.0.2.2") , port));
			clientHelper.SetAttribute ("Remote", remoteAddress1);
			clientApps1.Add (clientHelper.Install (n.Get (0)));
			clientApps1.Start (Seconds (startTime+0.01));
			clientApps1.Stop (Seconds (stopTime));
		}
		
		port++;
  }

   
   pointToPoint1Gbps.EnablePcap("Router.pcap", c1.Get (0), true, true);
   pointToPoint1Gbps.EnablePcap("Sender.pcap", c0.Get (1), true, true);
   
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  ThroughputMonitor(&fmHelper ,allMon);

  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");
  THFILE.close();
  
  Simulator::Destroy ();
  NS_LOG_UNCOND ("Simulation exit");
}
