/*
 * UAVObjMetadata.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: gustavo
 */

#include "UAVObjMetadata.h"

UAVObjMetadata::UAVObjMetadata() {

}

UAVObjMetadata::UAVObjMetadata(int flightAccess, int gcsAccess, int isFlightAckNeeded, int isGcsAckNeeded, int flightTelemetryUpdateMode, int gcsTelemetryUpdateMode, int flightTelemetryUpdatePeriod, int gcsTelemetryUpdatePeriod, int loggingUpdatePeriod) {

	this->flags = (flightAccess << UAVOBJ_ACCESS_SHIFT | gcsAccess << UAVOBJ_GCS_ACCESS_SHIFT | isFlightAckNeeded << UAVOBJ_TELEMETRY_ACKED_SHIFT | isGcsAckNeeded << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT | flightTelemetryUpdateMode << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT | gcsTelemetryUpdateMode << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT);
	this->loggingUpdatePeriod = loggingUpdatePeriod;
	this->flightTelemetryUpdatePeriod = flightTelemetryUpdatePeriod;
	this->gcsTelemetryUpdatePeriod = gcsTelemetryUpdatePeriod;
}

int UAVObjMetadata::loadFromPointer(const UAVObjMetadata* dataIn) {
	this->flags = dataIn->flags;
	this->flightTelemetryUpdatePeriod = dataIn->flightTelemetryUpdatePeriod;
	this->gcsTelemetryUpdatePeriod = dataIn->gcsTelemetryUpdatePeriod;
	this->loggingUpdatePeriod = dataIn->loggingUpdatePeriod;

	return 0;
}

bool UAVObjMetadata::isReadOnly() const {

	unsigned char filtered = this->flags;
	filtered = filtered & (0x00 | (0x01 << UAVOBJ_GCS_ACCESS_SHIFT));
	filtered = filtered >> UAVOBJ_GCS_ACCESS_SHIFT;
	return filtered;

}

int UAVObjMetadata::getGcsUpdateMode() {
	unsigned char filtered = this->flags;
	filtered = filtered & (0x00 | (0x01 << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT));
	filtered = filtered >> UAVOBJ_GCS_ACCESS_SHIFT;
	return filtered;
}


