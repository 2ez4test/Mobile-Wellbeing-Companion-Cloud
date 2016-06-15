import serial
from time import gmtime, strftime, sleep

DEVICE = '/dev/ttyAMA0'
BAUD = 9600

ser = serial.Serial(DEVICE, BAUD)
while True:
	print "Waiting incoming signal..."
	data = ser.inWaiting() # waiting for incoming signal
	if data != 0: # the moment when sensor wake up and send signal
	   message = ser.read(data) # read data in format
	   print ("%s: %s" % (strftime("%a, %d %b %Y %H:%M:%S", gmtime()), message))
           filename = "/usr/local/pnp4nagios/var/spool/pi_temp_" + str(int(time.time()))
	   f=open(filename,"w")
	   print >> f, "DATATYPE::SERVICEPERFDATA\tTIMET::%d\tHOSTNAME::localhost\tSERVICEDESC::Temperature\tSERVICEPERFDATA::reading=%s\tSERVICECHECKCOMMAND::check_temp\tHOSTSTATE::UP\tHOSTSTATETYPE::HARD\tSERVICESTATE::0\tSERVICESTATETYPE::1" % (int(time.time()),message[7:11])
	sleep(5) # sleep 5 seconds, then wake up to wait incomming signals
		 # usualy sleep half of interval time
		 # to save memory (in this case: sensor sleep interval = 10s)
