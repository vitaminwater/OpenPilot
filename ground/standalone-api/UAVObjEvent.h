/*
 * UAVObjEvent.h
 *
 *  Created on: Sep 13, 2013
 *      Author: gustavo
 */

#ifndef UAVOBJEVENT_H_
#define UAVOBJEVENT_H_

#include <stdlib.h>

/**
 * Event types generated by the objects.
 */
typedef enum {
    EV_NONE             = 0x00, /** No event */
    EV_UNPACKED         = 0x01, /** Object data updated by unpacking */
    EV_UPDATED          = 0x02, /** Object data updated by changing the data structure */
    EV_UPDATED_MANUAL   = 0x04, /** Object update event manually generated */
    EV_UPDATED_PERIODIC = 0x08, /** Object update from periodic event */
    EV_UPDATE_REQ       = 0x10 /** Request to update object data */
} UAVObjEventType;

class UAVObject;

/**
 * Event message, an instance of this class is sent in the event queue each time an event is generated
 */
class UAVObjEvent {
public:
	UAVObject* obj; /** A pointer to the object refered in this event. */
	UAVObjEventType eventType;
	unsigned char* data; /** Some events hold data relevant to it. */

	UAVObjEvent();
	UAVObjEvent(UAVObject* obj, UAVObjEventType event, unsigned char* data);
};


#endif /* UAVOBJEVENT_H_ */
