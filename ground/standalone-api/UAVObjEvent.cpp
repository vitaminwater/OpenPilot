/*
 * UAVobjEvent.cpp
 *
 *  Created on: Sep 22, 2013
 *      Author: gustavo
 */

#include "UAVObjEvent.h"

UAVObjEvent::UAVObjEvent() {
	this->obj = NULL;
	this->eventType = EV_NONE;
	this->data = NULL;
}

UAVObjEvent::UAVObjEvent(UAVObject* obj, UAVObjEventType eventType, unsigned char* data) {
	this->obj = obj;
	this->eventType = eventType;
	this->data = data;
}

