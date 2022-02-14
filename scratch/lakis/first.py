# /*
#  * This program is free software; you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License version 2 as
#  * published by the Free Software Foundation;
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program; if not, write to the Free Software
#  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#  */


'''
TODOs
1 - kellenek node-ok, amin a cliensek es a serverek lesznek
2 - kellenek a queue discek
	- minden belsore mindrop es ctv log
	- marker a kulso nodeokra
3 - statok (flow szamitas, ctv-k)
'''

'''
n1 ----- n2 ----- n3
   1gbps   100mbps

   
./waf --pyrun scratch/ggombos/first.py

'''
import ns.applications
import ns.core
import ns.internet
import ns.network
import ns.point_to_point
import ns.traffic_control

import sys

ns.core.LogComponentEnable("UdpEchoClientApplication", ns.core.LOG_LEVEL_INFO)
ns.core.LogComponentEnable("UdpEchoServerApplication", ns.core.LOG_LEVEL_INFO)

nodes = ns.network.NodeContainer()
nodes.Create(3)

stack = ns.internet.InternetStackHelper()
stack.Install(nodes)

#ppvMarkerQueueDisc = ns.traffic_control.TrafficControlHelper()
#ppvMarkerQueueDisc.SetRootQueueDisc("ns3::PpvMarkerQueueDisc", "MaxSize", ns.core.StringValue ("1000p"),"PpovPointsFile",ns.core.StringValue ("scratch/vG.pmarker.ppov.txt"))

#ppvSchedQueueDisc = ns.traffic_control.TrafficControlHelper()
#ppvSchedQueueDisc.SetRootQueueDisc("ns3::PpvSchedulerQueueDisc", "MaxSize", ns.core.StringValue ("1000p"))

pointToPoint1Gbps = ns.point_to_point.PointToPointHelper()
pointToPoint1Gbps.SetDeviceAttribute("DataRate", ns.core.StringValue("1Gbps"))
pointToPoint1Gbps.SetChannelAttribute("Delay", ns.core.StringValue("1ms"))
pointToPoint100Mbps = ns.point_to_point.PointToPointHelper()
pointToPoint100Mbps.SetDeviceAttribute("DataRate", ns.core.StringValue("100Mbps"))
pointToPoint100Mbps.SetChannelAttribute("Delay", ns.core.StringValue("1ms"))

n0n1 = ns.network.NodeContainer()
n0n1.Add(nodes.Get(0))
n0n1.Add(nodes.Get(1))

n1n2 = ns.network.NodeContainer()
n1n2.Add(nodes.Get(1))
n1n2.Add(nodes.Get(2))

d0d1 = pointToPoint1Gbps.Install(n0n1)
d1d2 = pointToPoint100Mbps.Install(n1n2)

#ppvSchedQueueDisc.Install(dev)
address = ns.internet.Ipv4AddressHelper()
address.SetBase(ns.network.Ipv4Address("10.0.1.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i0i1 = address.Assign(d0d1)

address.SetBase(ns.network.Ipv4Address("10.1.2.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i1i2 = address.Assign(d1d2)


client = n0n1.Get(0)
server = n1n2.Get(1)

client_intf = i0i1.GetAddress(0)
server_intf = i1i2.GetAddress(1)

print(client_intf,"-->",server_intf)

echoServer = ns.applications.UdpEchoServerHelper(9)

# serverApps = echoServer.Install(nodes.Get(1))
serverApps = echoServer.Install(server)
serverApps.Start(ns.core.Seconds(1.0))
serverApps.Stop(ns.core.Seconds(11.0))

# echoClient = ns.applications.UdpEchoClientHelper(interfaces.GetAddress(1), 9)
echoClient = ns.applications.UdpEchoClientHelper(server_intf, 9)
echoClient.SetAttribute("MaxPackets", ns.core.UintegerValue(3))
echoClient.SetAttribute("Interval", ns.core.TimeValue(ns.core.Seconds(1.0)))
echoClient.SetAttribute("PacketSize", ns.core.UintegerValue(1024))

# clientApps = echoClient.Install(nodes.Get(0))
clientApps = echoClient.Install(client)
clientApps.Start(ns.core.Seconds(2.0))
clientApps.Stop(ns.core.Seconds(10.0))

ns.internet.Ipv4GlobalRoutingHelper.PopulateRoutingTables()
#routingStream = ns.network.OutputStreamWrapper("out.txt",4)	#4-out
#ns.internet.Ipv4GlobalRoutingHelper.PrintRoutingTableAllAt (ns.core.Seconds (2.), routingStream);

ns.core.Simulator.Run()
#print(qdiscContainer.Get(0).GetStats())

ns.core.Simulator.Destroy()
