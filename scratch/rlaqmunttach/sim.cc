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

#include "rlaqm-queue-disc.h"
#include "rlaqm-gymenv.h"
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
 // FI | Time | FlowId | Src | Dst | rxByte (Second)
		
		Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
   if (fiveTuple.sourceAddress != "10.0.1.1"){
   //THFILE << "ACK-FLOW" <<";"<< std::endl;
   continue;}
   THFILE<<"FI;"<< Simulator::Now().GetSeconds()<<";";
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

//	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1400));
//	Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (13100000));
//	Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (13100000));
	
	//
  //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  // LogComponentEnable ("RLAQMQueueDisc", LOG_LEVEL_ALL);
  //LogComponentEnable ("RLAqmGymEnv", LOG_LEVEL_ALL);
	
  // Parameters of the environment
  double simulationTime = 120; //seconds
  // bool gymSim = true;

//  if (gymSim) {
	  uint32_t simSeed = 1;  
	  bool enableFading = false;

	  double envStepTime = 0.5; //seconds, ns3gym env step time interval
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
//	  openGymInterface = OpenGymInterface::Get(openGymPort);
	  //mygym parameterezes
	  Ptr<RLAqmGymEnv> myGymEnv = CreateObject<RLAqmGymEnv> ();
	  myGymEnv->SetOpenGymInterface(openGymInterface);
	  // myGymEnv->SetSize("20M")
	  
	  // std::ostringstream oss;
	  // oss << "/NodeList/0/DeviceList/0/$ns3::NonCommunicatingNetDevice/Phy/AveragePowerSpectralDensityReport";
	  // Config::ConnectWithoutContext (oss.str (),MakeBoundCallback (&MyGym2Env::PerformCca, myGymEnv, 8));
  //}
  
  // Traffic Control
  TrafficControlHelper tchFifo;
  tchFifo.SetRootQueueDisc ("ns3::RLAQMQueueDisc", "MaxSize",StringValue ("10p"),"BNCapacity", DoubleValue(1000.0/8)); //"MaxSize",StringValue ("10p"));// //"MaxSize",StringValue("1K")

  
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
  QueueDiscContainer qdiscs = tchFifo.Install (c1.Get(0));
  Ptr<QueueDisc> q = qdiscs.Get (0);
 
  myGymEnv->SetQueueDisc( q ); // TODO: Qdisc connected to gymenv

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  ipv4.Assign (c0);
 // Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);
 // Ipv4InterfaceContainer i0i1 = ipv4.Assign (c0);

  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  ipv4.Assign (c1);
  //Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (c1);

  //Ptr<Node> client = n0n1.Get(0);
  //Ptr<Node> server = n1n2.Get(1);
  
   Ptr<Node> client = n0.Get(0);
  Ptr<Node> server = n1.Get(1);

    //Address clientAddress = Address(i0i1.GetAddress(0));
    Address serverAddress = Address(i1i2.GetAddress(1));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
// UDP Application 
  
   //NS_LOG_INFO ("Create UDP Applications.");
  uint16_t port2 = 4000;
  UdpServerHelper serverApp (port2);
  ApplicationContainer apps = serverApp.Install (server);
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (1000.0));

  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 1000000000;
  Time interPacketInterval = Seconds (0.00005);		//168 Mbps
  UdpClientHelper clientApp (serverAddress, port2);
  clientApp.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  clientApp.SetAttribute ("Interval", TimeValue (interPacketInterval));
  clientApp.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = clientApp.Install (client);
  apps.Start (Seconds (0.4));
  apps.Stop (Seconds (100.0));

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
   
   //pointToPoint1Gbps.EnablePcap("Router.pcap", c1.Get (0), true, true);
   //pointToPoint1Gbps.EnablePcap("Sender.pcap", c0.Get (1), true, true);
   
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
