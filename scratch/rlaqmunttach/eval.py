
from scapy.all import *
from plotting import *
import matplotlib
import os, sys
import numpy as np

pkts_in = ('/home/p4/ns3-gym-gitlab/ns3-gym/Sender.pcap')
pkts_out = ('/home/p4/ns3-gym-gitlab/ns3-gym/Router.pcap')

#x = len (pkts_in)
#print (x)



packets_in = rdpcap(pkts_in).filter(lambda p: "TCP" in p)
packets_out = rdpcap(pkts_out).filter(lambda p: "TCP" in p)
out_pointer = 0

print("number ingoing packets: "+str(len(packets_in)))
print("number outgoing packets: "+str(len(packets_out)))
resLst = []
length = len(packets_out)
dropLst = []
basePacket = packets_in[0]
counterDrops = 0
for packet in packets_in:
    if (length == out_pointer):
        break
    out_packet = packets_out[out_pointer]
    tcp_in = packet['TCP']
    tcp_out = out_packet['TCP']
    match = tcp_in.seq == tcp_out.seq
    if(match):
        out_pointer+=1
        resLst.append((packet, out_packet))
    else:
        counterDrops = counterDrops + 1
        #print("Packet dropped in time ->: " + str(packet.time - basePacket.time))
        dropLst.append(packet)
        print("number drops: " + str(counterDrops))
        print("number matched packets: "+str(len(resLst)))
trace = resLst
        #packets_in = pcap-in-trace
        #return packets_in, resLs
#packets_in [3].show()   
 # plot pcap trace (ResLst) -- Match 
plt.figure(1)
fig = plt.gcf()
fig.canvas.set_window_title('PCAP Trace')
plt.subplot(211)
x_values = [ ]
y_values = [ ] 
basetime = trace[0][0].time
for tuple in trace:
    a= tuple[0]
    b = tuple[1] 
    diff = float(b.time - a.time)*1000.0
    x_val = (a.time - basetime)
    y_val = diff
    x_values.append(x_val)
    y_values.append(y_val)
plt.plot(x_values, y_values)
plt.ylim(ymin=0, ymax=20)
plt.ylabel('delay [ms]')
ax = plt.gca()
ax.set_xticklabels([])
plt.subplot(212)
x_values = []
y_values = []
basetime = trace[0][0].time
p = []
n = 10
for i in range(0, n):
    p.append(trace[0][1])
i = 0.0
for tuple in trace:
    i += 1
    p[n - 1] = tuple[1]
    if (i < n):
        diff = float(p[n - 1].time - p[0].time) / i  # error correction for the n first entries
    else:
        diff = float(p[n - 1].time - p[0].time) / n  # microseconds per packet
    x_val = (p[n - 1].time - basetime)
    if diff == 0:
        y_val = 0
    else:
        y_val = 1 / diff
    x_values.append(x_val)
    y_values.append(y_val)
    for j in range(0, n - 1):
        p[j] = p[j + 1]
plt.plot(x_values, y_values)
plt.ylim([0, 1000])
plt.ylabel('Rate [pps]')
plt.xlabel('time [s]')
#if not noTitle:
 #   plt.suptitle('bmv2 pcap analysis')
fig = plt.gcf()
fig.set_size_inches(6, 3, forward=True)
plt.savefig('figures/pcap-Trace.pdf', bbox_inches='tight')

# plot pcap bandwidth 
plt.figure(2)
fig = plt.gcf()
fig.canvas.set_window_title('Pcap Out-interface Bandwidth')
x_values = []
y_values = []
basetime = trace[0][0].time
p = []
n = 50
for i in range(0, n):
    p.append(trace[0][1])
i = 0
for tuple in trace:
    i+=1
    p[n-1] = tuple[1]
    if(i < n):
        diff = (p[n - 1].time - p[0].time) / i #error correction for the n first entries
    else:
        diff = (p[n-1].time - p[0].time)/n # microseconds per packet
    x_val = (p[n-1].time - basetime)
    if diff == 0:
        y_val = 0
    else:
        y_val = 1/diff
    x_values.append(x_val)
    y_values.append(y_val)
    for j in range(0, n-1):
        p[j] = p[j+1]
plt.ylim(ymin = 0 , ymax = 500)
plt.plot(x_values, y_values)
plt.ylabel('rate [pps]')
plt.xlabel('time [s]')
fig = plt.gcf()
fig.set_size_inches(6, 3, forward=True)
plt.savefig('figures/pcapBandwidth.pdf', bbox_inches='tight')

# Bandwith at the income interface 
plt.figure(3)
fig = plt.gcf()
fig.canvas.set_window_title('Pcap In-interface  Bandwidth')
x_values = []
y_values = []
in_trace = packets_in
basetime = in_trace[0].time
p = []
n = 50
for i in range(0, n):
    p.append(in_trace[0])
i = 0
for p_in in in_trace:
    i+=1
    p[n-1] = p_in
    if(i < n):
        diff = (p[n - 1].time - p[0].time) / i #error correction for the n first entries
    else:
        diff = (p[n-1].time - p[0].time)/n # microseconds per packet
    x_val = (p[n-1].time - basetime)
    if diff == 0:
        y_val = 0
    else:
        y_val = 1/diff
    x_values.append(x_val)
    y_values.append(y_val)
    for j in range(0, n-1):
        p[j] = p[j+1]
plt.ylim (ymin = 0 , ymax = 500)
plt.plot(x_values, y_values)
plt.ylabel('rate [pps]')
plt.xlabel('time [s]')
fig = plt.gcf()
fig.set_size_inches(6, 3, forward=True)
plt.savefig('figures/pcapInBandwidth.pdf', bbox_inches='tight')

# Queue Delay 
plt.figure(4)
fig = plt.gcf()
fig.canvas.set_window_title('Pcap Queue Delay')
x_values = []
y_values = []
basetime = trace[0][0].time
for tuple in trace:
    packet = tuple[1] ##only egress is interesting for us
    x_val = float(packet.time-basetime)
    if packet['IP'].len < 500:
        continue
    tcp = packet['TCP']
    payload = tcp.payload
    data = raw(payload)
    a = orb(data[0])
    b = orb(data[1])
    c = orb(data[2])
    d = orb(data[3])
    delay = (a << 24) + (b << 16) + (c << 8) + d
    y_val = delay/1000.0
    x_values.append(x_val)
    y_values.append(y_val)
plt.ylim(ymin = 0 , ymax = 20)
plt.plot(x_values, y_values)
plt.plot([0, x_values[len(x_values) - 1] + 0.2], [5, 5], '--', color='C5')
plt.ylabel('queue delay [ms]')
plt.xlabel('time [s]')
fig = plt.gcf()
fig.set_size_inches(6, 3, forward=True)
plt.savefig('figures/pcapQueueDelay.pdf', bbox_inches='tight')
#show the figures
plt.show()