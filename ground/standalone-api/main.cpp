/*
############################################################################
################ Don't run without reading all the comments ################
########################## REMOVE YOUR PROPELLERS ##########################
############################################################################
*/

#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <sys/time.h>
#include <ctime>
#include <unistd.h>

#include "USBStream.h"
#include "UAVObjManager.h"
#include "flightstatus.h"
#include "gcsreceiver.h"

int main() {

	//Creates a USBStream for the usb connected CC3D - this must be changed for revo and maybe different version of fw/bu.
	USBStream us(0x20a0, 0x415b);

	//Initializes the UAVObjectManager and add UAVObjects.
	UAVObjManager* manager = new UAVObjManager(&us, &us, TELEMETRY_USBHID); //important to tell which type of telemetry is being used.
	UAVObject* obj = new FlightStatus(manager);
	UAVObject* obj2 = new GCSReceiver(manager);
	UAVObject* obj3 = new ManualControlCommand(manager);

	//Adds objects to the manager
	manager->UAVObjRegister(obj);
	manager->UAVObjRegister(obj2);
	manager->UAVObjRegister(obj3);

	//Update some fields on the UAVObject
	//This specific object is used to control the CC3D with a computer instead of a radio controller
	//In my configuration arming the quadcopter is achieved by setting the throttle to 0 and the yaw to maximum right
	//Keep in mind that these values have to be according to the gcs configuration
	((GCSReceiver*)obj2)->dataFields.Channel[0] = 1000; //throttle
	((GCSReceiver*)obj2)->dataFields.Channel[1] = 1500; //roll
	((GCSReceiver*)obj2)->dataFields.Channel[2] = 1500; //pitch
	((GCSReceiver*)obj2)->dataFields.Channel[3] = 2000; //yaw
	((GCSReceiver*)obj2)->dataFields.Channel[4] = 0; //FlightMode - I left it not set but the firmware seems to change this by itself.
	((GCSReceiver*)obj2)->dataFields.Channel[5] = 0; //Not set
	((GCSReceiver*)obj2)->dataFields.Channel[6] = 0; //Not set
	((GCSReceiver*)obj2)->dataFields.Channel[7] = 0; //Not set

	//I was using these as debug to check if the library was working, this object is always updated by the CC3D.
	//Any changes to it are rewritten
	//((FlightStatus*)obj)->dataFields.Armed = 2;
	//((FlightStatus*)obj)->dataFields.FlightMode = 1;
	//((FlightStatus*)obj)->dataFields.ControlSource = 2;

	//Sends an object to the quad
	//false means that I'm not expecting the reception to be confimed, more information on the OpenPilot wiki
	manager->UAVTalkSendObject(obj2, false);

	timeval timeOld, timeNew, beginTime;
	gettimeofday(&beginTime, 0);
	while(1) {
		gettimeofday(&timeNew, 0);
		//So that the CC3D doesn't go into failsafe mode the value printed by this command must always be below 100ms.
		std::cout << "Last cycle was: " << (timeNew.tv_usec - timeOld.tv_usec)/1000 << " miliseconds ago " << (timeNew.tv_sec - beginTime.tv_sec) << " seconds after start." << std::endl;
		gettimeofday(&timeOld, 0);

		//Sending the object again, read comment above
		manager->UAVTalkSendObject(obj2, false);

		//Request a object example.
		manager->UAVTalkSendObjectRequest(obj);

		//Print objects to the screen
		std::cout << ((FlightStatus*)obj)->prettyPrint();
		std::cout << ((GCSReceiver*)obj2)->prettyPrint();

		//The object requested above was important because it contains the information about the fligh state of the CC3D
		//If Armed equals 2 it means that the quad is armed, 0 is unarmed(?) and 1 is failsafe
		//The quad goes into a low spin of the rotors then.
		//Don't run this if you have the propellors in place, the behavior of 1300 throttle is relative to the configuration on the GCS so it might be dangerous in your configuration.
		if(((FlightStatus*)obj)->dataFields.Armed == 2) {
			((GCSReceiver*)obj2)->dataFields.Channel[0] = 1300; //throttle
			((GCSReceiver*)obj2)->dataFields.Channel[1] = 1500; //roll
			((GCSReceiver*)obj2)->dataFields.Channel[2] = 1500; //pitch
			((GCSReceiver*)obj2)->dataFields.Channel[3] = 1500; //yaw
		}

		//Lets the program sleep for a while, we are doing almost nothing in each cycle there no need to run at full speed.
		usleep(50000);
		
	}

	//Closes the iostream. (I think it is not implemented :) Hehe )
	us.close();

	return 0;

}
