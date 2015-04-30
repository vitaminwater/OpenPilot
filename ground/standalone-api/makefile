#General compiler flags
CC = g++
CFLAGS = -Wall -fpic -pthread -std=c++0x -c

#Flags to compile the HIDAPI - library for communication with HID devices.
CCC = gcc
CCFLAGS = -Wall -fpic -pthread -I/usr/local/include/libusb-1.0 -lrt -c

#Flags to create the UAVTalk library
LIBFLAGS = -Wall -shared -I/usr/local/include/libusb-1.0 `pkg-config libusb-1.0 --cflags --libs` -o
LIBUAVSO = libUAVTalk.so
LIBUAV = UAVTalk

#List of the objects that are part of the UAVObjectManager, UAVTalk protocol and USB link
OBJS = UAVObjManager.o UAVTalkConnection.o UAVObject.o UAVObjMetadata.o UAVObjEvent.o UAVTalkStats.o hid.o USBStream.o

### Full library, has all objects available. Use for deployment.
UAVOBJECTS = accels.o actuatorcommand.o actuatordesired.o actuatorsettings.o attitudeactual.o attitudesettings.o attitudesimulated.o ekfconfiguration.o ekfstatevariance.o faultsettings.o firmwareiapobj.o flightbatterysettings.o flightbatterystate.o flightplancontrol.o flightplansettings.o flightplanstatus.o flightstatus.o flighttelemetrystats.o gcsreceiver.o gcstelemetrystats.o groundtruth.o gyros.o gyrosbias.o hwsettings.o i2cstats.o manualcontrolcommand.o manualcontrolsettings.o mixersettings.o mixerstatus.o oplinkreceiver.o oplinksettings.o oplinkstatus.o ratedesired.o relaytuning.o stabilizationdesired.o stabilizationsettings.o systemalarms.o systemsettings.o systemstats.o velocityactual.o velocitydesired.o

### Selective library, has only some Objects. Use for DEBUG.
#UAVOBJECTS = flightstatus.o gcsreceiver.o manualcontrolcommand.o

#Flags to create a working program using the library
OUTPUTNAME = test
INPUTNAME = main.cpp
LFLAGS = -Wall -pthread -std=c++0x -L. -l$(LIBUAV)

all: $(OUTPUTNAME)

### Make the test program, the options can be set above. Should be changed to a dynamic script.
$(OUTPUTNAME): $(LIBUAVSO) $(INPUTNAME) $(UAVOBJECTS)
	$(CC) $(INPUTNAME) -o $(OUTPUTNAME) $(LFLAGS)

### Makes library Includes all the UAVObjects, usb connection, UAVObjectManager and UAVTalk protocol
$(LIBUAVSO): $(OBJS) $(UAVOBJECTS)
	$(CC) $(OBJS) $(UAVOBJECTS) $(LIBFLAGS) $(LIBUAVSO)

### Makes part of the library
UAVObjManager.o: UAVObjEvent.h UAVObjMngrStats.h UAVObject.h UAVTalkConnection.h UAVObjManager.h UAVObjManager.cpp
	$(CC) $(CFLAGS) UAVObjManager.cpp

UAVTalkConnection.o: UAVObject.h UAVObjEvent.h UAVTalkStats.h UAVObjManager.h UAVTalkConnection.h UAVTalkConnection.cpp
	$(CC) $(CFLAGS) UAVTalkConnection.cpp

UAVObject.o: UAVObjMetadata.h NotImplemented.h UAVObject.h UAVObject.cpp
	$(CC) $(CFLAGS) UAVObject.cpp

UAVObjMetadata.o: UAVObjMetadata.h UAVObjMetadata.cpp
	$(CC) $(CFLAGS) UAVObjMetadata.cpp

UAVObjEvent.o: UAVObjEvent.h UAVObjEvent.cpp
	$(CC) $(CFLAGS) UAVObjEvent.cpp

UAVTalkStats.o: UAVTalkStats.h UAVTalkStats.cpp
	$(CC) $(CFLAGS) UAVTalkStats.cpp

USBStream.o: USBStream.cpp USBStream.h
	$(CC) $(CFLAGS) USBStream.cpp

hid.o: hid.c hidapi.h
	$(CCC) $(CCFLAGS) hid.c
### 

######################
# Compile UAVOBJECTS #
######################

accels.o:accels.cpp accels.h
	$(CC) $(CFLAGS) accels.cpp

actuatorcommand.o:actuatorcommand.cpp actuatorcommand.h
	$(CC) $(CFLAGS) actuatorcommand.cpp

actuatordesired.o: actuatordesired.cpp actuatordesired.h
	$(CC) $(CFLAGS) actuatordesired.cpp

actuatorsettings.o: actuatorsettings.cpp actuatorsettings.h
	$(CC) $(CFLAGS) actuatorsettings.cpp
 
attitudeactual.o:attitudeactual.cpp attitudeactual.h
	$(CC) $(CFLAGS) attitudeactual.cpp

attitudesettings.o:attitudesettings.cpp attitudesettings.h
	$(CC) $(CFLAGS) attitudesettings.cpp

attitudesimulated.o:attitudesimulated.cpp attitudesimulated.h
	$(CC) $(CFLAGS) attitudesimulated.cpp

ekfconfiguration.o:ekfconfiguration.cpp ekfconfiguration.h
	$(CC) $(CFLAGS) ekfconfiguration.cpp

ekfstatevariance.o:ekfstatevariance.cpp ekfstatevariance.h
	$(CC) $(CFLAGS) ekfstatevariance.cpp

faultsettings.o:faultsettings.cpp faultsettings.h
	$(CC) $(CFLAGS) faultsettings.cpp

firmwareiapobj.o:firmwareiapobj.cpp firmwareiapobj.h
	$(CC) $(CFLAGS) firmwareiapobj.cpp

flightbatterysettings.o:flightbatterysettings.cpp flightbatterysettings.h
	$(CC) $(CFLAGS) flightbatterysettings.cpp

flightbatterystate.o:flightbatterystate.cpp flightbatterystate.h
	$(CC) $(CFLAGS) flightbatterystate.cpp

flightplancontrol.o:flightplancontrol.cpp flightplancontrol.h
	$(CC) $(CFLAGS) flightplancontrol.cpp

flightplansettings.o:flightplansettings.cpp flightplansettings.h
	$(CC) $(CFLAGS) flightplansettings.cpp

flightplanstatus.o:flightplanstatus.cpp flightplanstatus.h
	$(CC) $(CFLAGS) flightplanstatus.cpp

flightstatus.o:flightstatus.cpp flightstatus.h
	$(CC) $(CFLAGS) flightstatus.cpp

flighttelemetrystats.o:flighttelemetrystats.cpp flighttelemetrystats.h
	$(CC) $(CFLAGS) flighttelemetrystats.cpp

gcsreceiver.o: gcsreceiver.cpp gcsreceiver.h
	$(CC) $(CFLAGS) gcsreceiver.cpp

gcstelemetrystats.o:gcstelemetrystats.cpp gcstelemetrystats.h
	$(CC) $(CFLAGS) gcstelemetrystats.cpp

groundtruth.o:groundtruth.cpp groundtruth.h
	$(CC) $(CFLAGS) groundtruth.cpp

gyros.o:gyros.cpp gyros.h
	$(CC) $(CFLAGS) gyros.cpp

gyrosbias.o:gyrosbias.cpp gyrosbias.h
	$(CC) $(CFLAGS) gyrosbias.cpp

hwsettings.o:hwsettings.cpp hwsettings.h
	$(CC) $(CFLAGS) hwsettings.cpp

i2cstats.o:i2cstats.cpp i2cstats.h
	$(CC) $(CFLAGS) i2cstats.cpp

manualcontrolcommand.o:manualcontrolcommand.cpp manualcontrolcommand.h
	$(CC) $(CFLAGS) manualcontrolcommand.cpp

manualcontrolsettings.o:manualcontrolsettings.cpp manualcontrolsettings.h
	$(CC) $(CFLAGS) manualcontrolsettings.cpp

mixersettings.o:mixersettings.cpp mixersettings.h
	$(CC) $(CFLAGS) mixersettings.cpp

mixerstatus.o:mixerstatus.cpp mixerstatus.h
	$(CC) $(CFLAGS) mixerstatus.cpp

oplinkreceiver.o:oplinkreceiver.cpp oplinkreceiver.h
	$(CC) $(CFLAGS) oplinkreceiver.cpp

oplinksettings.o:oplinksettings.cpp oplinksettings.h
	$(CC) $(CFLAGS) oplinksettings.cpp

oplinkstatus.o:oplinkstatus.cpp oplinkstatus.h
	$(CC) $(CFLAGS) oplinkstatus.cpp

ratedesired.o:ratedesired.cpp ratedesired.h
	$(CC) $(CFLAGS) ratedesired.cpp

relaytuning.o:relaytuning.cpp relaytuning.h
	$(CC) $(CFLAGS) relaytuning.cpp

stabilizationdesired.o:stabilizationdesired.cpp stabilizationdesired.h
	$(CC) $(CFLAGS) stabilizationdesired.cpp

stabilizationsettings.o:stabilizationsettings.cpp stabilizationsettings.h
	$(CC) $(CFLAGS) stabilizationsettings.cpp

systemalarms.o:systemalarms.cpp systemalarms.h
	$(CC) $(CFLAGS) systemalarms.cpp

systemsettings.o:systemsettings.cpp systemsettings.h
	$(CC) $(CFLAGS) systemsettings.cpp

systemstats.o:systemstats.cpp systemstats.h
	$(CC) $(CFLAGS) systemstats.cpp

velocityactual.o:velocityactual.cpp velocityactual.h
	$(CC) $(CFLAGS) velocityactual.cpp

velocitydesired.o:velocitydesired.cpp velocitydesired.h
	$(CC) $(CFLAGS) velocitydesired.cpp
######################

### Install rule for embedded linux systems. It was easier in my system, change to fit.
#install:$(LIBUAVSO)
#	cp *.h /opt/UAVTalkLib
#	cp $(LIBUAVSO) /opt/UAVTalkLib

clean:
	rm -rf *~ *.o *.so *.txt $(OUTPUTNAME)
