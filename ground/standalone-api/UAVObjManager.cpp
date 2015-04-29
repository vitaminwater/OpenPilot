/*
 * UAVObjManager.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: gustavo
 */

#include "UAVObjManager.h"

UAVObjManager::UAVObjManager() {

	//Start thread that generates the periodic updates
	periodicUpdateThread = std::thread(&UAVObjManager::checkPeriodicUpdates, this);

	//Start thread that answers to event on the queue
	queueWatcherThread = std::thread(&UAVObjManager::queueProcesser, this);

}

UAVObjManager::UAVObjManager(std::istream* inStream, std::ostream* outStream, int telemetryLink) {

	if(DEBUG)std::cout << "Initializing UAVObjManager" << std::endl;

	//Start thread that generates the periodic updates
	periodicUpdateThread = std::thread(&UAVObjManager::checkPeriodicUpdates, this);

	//Start thread that answers to event on the queue
	queueWatcherThread = std::thread(&UAVObjManager::queueProcesser, this);

	/*TODO: Inicializar um output stream para o tipo de link desejado, neste caso usb */
	UAVTalk = new UAVTalkConnection(inStream, outStream, telemetryLink, this);

}

UAVObjMngrStats UAVObjManager::UAVObjMngrGetStats() {
	return stats;
}

void UAVObjManager::UAVObjMngrClearStats() {
	stats.reset();
}

bool UAVObjManager::UAVObjRegister(UAVObject* obj) {

	if(DEBUG) std::cout << "Registring object with id: " << std::hex << obj->UAVObjGetID() << std::dec << std::endl;

	listObjects.push_back(obj);

	stats.incrementInsertedObjects();

	return true;

}

void UAVObjManager::UAVObjUnregister(UAVObject* obj) {
	listObjects.remove(obj);
}

void UAVObjManager::UAVObjUnregister(unsigned int id, unsigned int instance) {

	for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
		if(((**it).UAVObjGetID() == id) && ((**it).UAVObjGetInstance() == instance))
			listObjects.erase(it);
	}
}

void UAVObjManager::UAVObjUnregisterAll(unsigned int id) {

	for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
		if(((**it).UAVObjGetID() == id))
			listObjects.erase(it);
	}
}

UAVObject* UAVObjManager::UAVObjGetByID(unsigned int id) {

	for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
		if((**it).UAVObjGetID() == id)
			return (*it);
	}

	return NULL;
}

UAVObject* UAVObjManager::UAVObjGetByID(unsigned int id, unsigned int instance) {

	for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
		if(((**it).UAVObjGetID() == id) && ((**it).UAVObjGetInstance() == instance))
			return (*it);
	}

	return NULL;
}

void UAVObjManager::checkPeriodicUpdates(){

	if(DEBUG) std::cout << "checkPeriodicUpdates thread is now running" << std::endl;
	while(1) {
		time_t currentTime = time(NULL);
		int nextUpdateCountdown = MINIMUM_OBJECT_UPDATE_TIME;

		for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
			if((**it).UAVObjGetUpdateMode() != UPDATEMODE_MANUAL){
				if((**it).UAVObjGetNextUpdateTime() <= currentTime) {
					this->UAVTalkSendObjectRequest((*it));
					(**it).UAVObjSetNextUpdateTime(currentTime);
				} else if((**it).UAVObjGetNextUpdateTime() - currentTime < nextUpdateCountdown){
					nextUpdateCountdown = (**it).UAVObjGetNextUpdateTime() - currentTime;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(nextUpdateCountdown));
	}
	if(DEBUG) std::cout << "checkPeriodicUpdates thread is now stopping" << std::endl;
}

void UAVObjManager::queueProcesser() {

	if(DEBUG) std::cout << "queueProcesser thread is now running" << std::endl;
	while(1) {
		int nextWatchCountdown = QUEUE_WATCHER_REFRESH_TIME;

		if(this->UAVObjMngrHasEvents())
			this->UAVObjMngrPopEvent();
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(nextWatchCountdown));
		}
	}
	if(DEBUG) std::cout << "queueProcesser thread is now stoping" << std::endl;
}

bool UAVObjManager::UAVObjMngrIsSingleInstanceType(unsigned int id) {
	for(std::list<UAVObject*>::iterator it = listObjects.begin(); it != listObjects.end(); ++it) {
			if(((**it).UAVObjGetID() == id))
				return (**it).UAVObjIsSingleInstance();
		}

	return false;
}

bool UAVObjManager::UAVObjMngrHasEvents() {
	return (eventQueue.size() > 0);
}

void UAVObjManager::UAVObjMngrPushEvent(UAVObjEvent event) {
	if(DEBUG) std::cout << "Pushing new event into queue" << std::endl;

	eventQueue.push(event);
}

void UAVObjManager::UAVObjMngrPopEvent() {

	if(DEBUG) std::cout << "UAVObjManager processing new event" << std::endl;

	UAVObjEvent event = eventQueue.front();

	switch(event.eventType) {
	case EV_UPDATED:
		event.obj->UAVObjUnpack(event.data);
		break;
	case EV_UPDATE_REQ:
		this->UAVTalk->UAVTalkSendObject(event.obj, false, 1000);
		break;
	default:
		//TODO raise error
		break;
	}

	free(event.data);
	eventQueue.pop();
}

void UAVObjManager::UAVTalkSendObjectRequest(UAVObject* obj){
	if(DEBUG)std::cout << "Manager was asked to send a Object Request" << std::endl;
	UAVTalk->UAVTalkSendObjectRequest(obj, 0);
}

void UAVObjManager::UAVTalkSendObject(UAVObject* obj, bool acked){
	if(DEBUG)std::cout << "Manager was asked to send a Object" << std::endl;
	UAVTalk->UAVTalkSendObject(obj, acked, 0);
}



