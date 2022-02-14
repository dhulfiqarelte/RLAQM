from xml.etree import ElementTree as ET
import sys
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as pylab
et=ET.parse(sys.argv[1])
bitrates=[]
losses=[]
delays=[]
for flow in et.findall("FlowStats/Flow"):
	for tpl in et.findall("Ipv4FlowClassifier/Flow"):
		if tpl.get('flowId')==flow.get('flowId'):
			break
	if tpl.get('destinationPort')=='654':
		continue
	losses.append(int(flow.get('lostPackets')))
	
	rxPackets=int(flow.get('rxPackets'))
	if rxPackets==0:
		bitrates.append(0)
	else:
		t0=float(flow.get('timeFirstRxPacket')[:-2])
		t1=float(flow.get("timeLastRxPacket")[:-2])
		duration=(t1-t0)*1e-9
		bitrates.append(8*int(flow.get("rxBytes"))/duration*1e-3)
		delays.append(float(flow.get('delaySum')[:-2])*1e-9/rxPackets)

pylab.subplot(311)
pylab.hist(bitrates,bins=40)
pylab.xlabel("Flow Bit Rates (b/s)")
pylab.ylabel("Number of Flows")


pylab.subplot(312)
pylab.hist(bitrates,bins=40)
pylab.xlabel("No of Lost Packets")
pylab.ylabel("Number of Flows")

pylab.subplot(313)
pylab.hist(bitrates,bins=10)
pylab.xlabel("Delay in Seconds")
pylab.ylabel("Number of Flows")

pylab.subplots_adjust(hspace=0.4)
pylab.savefig("results.pdf")
