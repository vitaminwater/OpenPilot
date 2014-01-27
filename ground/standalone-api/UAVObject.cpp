/*
 * UAVObject.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: gustavo
 */
#include "UAVObject.h"

UAVObject::UAVObject(){

}

UAVObject::UAVObject(UAVObjManager* manager, unsigned int id, std::string name, std::string description, std::string category, int isSingleInstance, int isSettings, int flightAccess, int gcsAccess, int isFlightAckNeeded, int isGcsAckNeeded, int flightTelemetryUpdateMode, int gcsTelemetryUpdateMode, int flightTelemetryUpdatePeriod, int gcsTelemetryUpdatePeriod, int loggingUpdatePeriod) {
	this->manager = manager;
	this->id = id;
	this->name = name;
	this->description = description;
	this->category = category;
	this->isSingleInstance = (isSingleInstance == 1)?true:false;
	this->isSettings = (isSettings == 1)?true:false;
	this->metadata = UAVObjMetadata(flightAccess, gcsAccess, isFlightAckNeeded, isGcsAckNeeded, flightTelemetryUpdateMode, gcsTelemetryUpdateMode, flightTelemetryUpdatePeriod, gcsTelemetryUpdatePeriod, loggingUpdatePeriod);
	this->nextUpdateTime = time(NULL) + this->metadata.gcsTelemetryUpdatePeriod;
}

unsigned int UAVObject::UAVObjGetID() const {

	return id;
}

bool UAVObject::UAVObjIsSingleInstance() const {
	return isSingleInstance;
}

time_t UAVObject::UAVObjGetNextUpdateTime() const {

	return nextUpdateTime;
}

void UAVObject::UAVObjSetNextUpdateTime(time_t lastUpdateTime) {
	this->nextUpdateTime = lastUpdateTime + this->metadata.gcsTelemetryUpdatePeriod;
}

int UAVObject::UAVObjSetMetadata(const UAVObjMetadata* dataIn) {
	return this->metadata.loadFromPointer(dataIn);
}

UAVObjMetadata UAVObject::UAVObjGetMetadata() {
	return this->metadata;
}

bool UAVObject::UAVObjIsReadOnly() const {
	return metadata.isReadOnly();
}

bool UAVObject::operator ==(const UAVObject &other) {

	if((this->UAVObjGetInstance() == other.UAVObjGetID()) && (this->UAVObjGetInstance() == other.UAVObjGetInstance()))
		return true;
	else
		return false;
}

int UAVObject::UAVObjGetUpdateMode() {
	return metadata.getGcsUpdateMode();
} 


