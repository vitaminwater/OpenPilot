/*
 * UAVObjManager.h
 *
 *  Created on: Sep 13, 2013
 *      Author: gustavo
 */

#ifndef UAVOBJMANAGER_H_
#define UAVOBJMANAGER_H_

#include <mutex>
#include <list>
#include <queue>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "UAVObjEvent.h"
#include "UAVObjMngrStats.h"
#include "UAVTalkConnection.h"

#define QUEUE_WATCHER_REFRESH_TIME 1000
#define MINIMUM_OBJECT_UPDATE_TIME 5000

#define DEBUG 0

class UAVTalkConnection;

#define TELEMETRY_USBHID 1

class UAVObjManager {
private:
	std::recursive_mutex mutex; /** A mutex to guarantee thread safety.*/
	UAVObjMngrStats stats; /** Runnign stats.*/
	std::list<UAVObject*> listObjects; /** The list of all registered objects.*/
	std::queue<UAVObjEvent> eventQueue; /** A queue where events wait before being computed.*/
	std::thread periodicUpdateThread; /** The thread that checks for objects periodic updates and creates the corresponding event.*/
	std::thread queueWatcherThread; /** The thread that executes the generated events. */
	UAVTalkConnection* UAVTalk; /** A pointer to the UAVTalk link, used to propagate objects to the flight software.*/

	void checkPeriodicUpdates(); /** The function executed by periodicUpdateThread.*/
	void queueProcesser(); /** The function executed by queueWatcherThread.*/

public:
	/**
	 * Initialize the object manager, without a telemetry link.
	 * Not veru useful.
	 */
	UAVObjManager();

	/**
 	 * Initialize the object manager, with a given telemetry link.
	 * The streams may be the same as long as they implement the needed read/write functions.
	 * @param inStream An input stream for the UAVTalk protocol.
	 * @param outStream An output stream for the UAVTalk protocol.
	 * @param telemetryLink The type of telemetry to establish, only usb right now.
 	 */
	UAVObjManager(std::istream* inStream, std::ostream* outStream, int telemetryLink);

	UAVObjMngrStats UAVObjMngrGetStats();

	void UAVObjMngrClearStats();

	/**
	 * Registers an object in the manager.
	 * @param obj A pointer to the registered object.
	 * @return Returns true on success.
	 */
	bool UAVObjRegister(UAVObject* obj);

	/**
	 * Unregister an object based on its pointer.
	 * @param obj The pointer of the object to be unregistered.
	 */
	void UAVObjUnregister(UAVObject* obj);

	/**
	 * Unregisters a single instance of an object.
	 * @param id The id of the object to unregister.
	 * @param instance The instance of that object to unregister.
	 */
	void UAVObjUnregister(unsigned int id, unsigned int instance);

	/**
	 * Unregisters all the object with given id.
	 * @param id The id of the objects to unregister. 
	 */
	void UAVObjUnregisterAll(unsigned int id);

	/**
	 * Retrieves the pointer to a single instance object.
	 * @param id The id of the object, must be unique in the registred objects.
	 * @return The pointer to the object.
	 */
	UAVObject* UAVObjGetByID(unsigned int id);

	/**
	 * Retrieves the pointer to an object with multiple instances.
	 * @param id The id of the object to retrieve.
	 * @param instance The particular instance of the object.
	 * @return The pointer to the object.
	 */
	UAVObject* UAVObjGetByID(unsigned int id, unsigned int instance);

	/**
	 * Verifies if an object is single instance.
	 * @param id The id of the object.
	 * @return Return true if the object is single instance, false otherwise.
	 */
	bool UAVObjMngrIsSingleInstanceType(unsigned int id);

	/**
	 * Verifies if there are pending events.
	 * @return Returns true if there are.
	 */
	bool UAVObjMngrHasEvents();

	/**
	 * Places a new event on the queue.
	 * @param event The event to be placed.
  	 */
	void UAVObjMngrPushEvent(UAVObjEvent event);

	/**
	 * Removes the oldest event from the queue and processes it.
	 */
	void UAVObjMngrPopEvent();

	void UAVTalkSendObjectRequest(UAVObject* obj);

	void UAVTalkSendObject(UAVObject* obj, bool acked);

};


#endif /* UAVOBJMANAGER_H_ */
