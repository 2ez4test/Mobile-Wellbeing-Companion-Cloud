# git clone https://github.com/baderj/python-ant.git
# git clone https://gist.github.com/c41d2bbe0aeded3506cf.git

import sys
import time
from ant.core import driver, node, event, message, log
from ant.core.constants import CHANNEL_TYPE_TWOWAY_RECEIVE, TIMEOUT_NEVER

class HRM(event.EventCallback):

    def __init__(self, serial, netkey):
        self.serial = serial
        self.netkey = netkey
        self.antnode = None
        self.channel = None

    def start(self):
        print("Loading...")
        self._start_antnode()
        self._setup_channel()
        self.channel.registerCallback(self)
        print("Waiting for incoming signal...")

    def stop(self):
        if self.channel:
            self.channel.close()
            self.channel.unassign()
        if self.antnode:
            self.antnode.stop()

    def __enter__(self):
        return self

    def __exit__(self, type_, value, traceback):
        self.stop()

    def _start_antnode(self):
        stick = driver.USB2Driver(self.serial)
        self.antnode = node.Node(stick)
        self.antnode.start()

    def _setup_channel(self):
        key = node.NetworkKey('N:ANT+', self.netkey)
        self.antnode.setNetworkKey(0, key)
        self.channel = self.antnode.getFreeChannel()
        self.channel.name = 'C:HRM'
        self.channel.assign('N:ANT+', CHANNEL_TYPE_TWOWAY_RECEIVE)
        self.channel.setID(120, 0, 0)
        self.channel.setSearchTimeout(TIMEOUT_NEVER)
        self.channel.setPeriod(8070)
        self.channel.setFrequency(57)
        self.channel.open()

    #f = open('/home/pi/heart_rate_monitoring/heartrate_log.txt', "a")

    def process(self, msg):
	timeInSeconds = int((time.time()))
	f = open('/home/pi/heart_rate_monitoring/heartrate_log.txt', "a")
        if isinstance(msg, message.ChannelBroadcastDataMessage):
           	 print >> f, str(timeInSeconds) + "," + format(ord(msg.payload[-1]))	  
			 f.close()
	    	 #print("Heart rate: {}".format(ord(msg.payload[-1])))
	else:
	    print("");

SERIAL = '/dev/ttyUSB0'
NETKEY = 'B9A521FBBD72C345'.decode('hex')

with HRM(serial=SERIAL, netkey=NETKEY) as hrm:
    hrm.start()
    while True:
        try:
            time.sleep(2)
        except KeyboardInterrupt:
	    f.close()
            sys.exit(0)
