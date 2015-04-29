/*
 * UAVObjMetadata.h
 *
 *  Created on: Sep 13, 2013
 *      Author: gustavo
 */

#ifndef UAVOBJMETADATA_H_
#define UAVOBJMETADATA_H_


/**
 * Object update mode, used by multiple modules (e.g. telemetry and logger)
 */
typedef enum {
    UPDATEMODE_MANUAL    = 0, /** Manually update object, by calling the updated() function */
    UPDATEMODE_PERIODIC  = 1, /** Automatically update object at periodic intervals */
    UPDATEMODE_ONCHANGE  = 2, /** Only update object when its data changes */
    UPDATEMODE_THROTTLED = 3 /** Object is updated on change, but not more often than the interval time */
} UAVObjUpdateMode;

/**
 * Object access types
 */
typedef enum {
    ACCESS_READWRITE = 0,
    ACCESS_READONLY  = 1
} UAVObjAccessType;

/*
 * Shifts and masks used to read/write metadata flags.
 */
#define UAVOBJ_ACCESS_SHIFT                    0
#define UAVOBJ_GCS_ACCESS_SHIFT                1
#define UAVOBJ_TELEMETRY_ACKED_SHIFT           2
#define UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT       3
#define UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT     4
#define UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT 6
#define UAVOBJ_UPDATE_MODE_MASK                0x3

/**
 * Object metadata, each object has a meta object that holds its metadata. The metadata define
 * properties for each object and can be used by multiple modules (e.g. telemetry and logger)
 *
 * The bits in the flag field are defined as:
 *
 *   Bit(s)  Name                     Meaning
 *   ------  ----                     -------
 *      0    access                   Defines the access level for the local transactions (readonly=1 and readwrite=0)
 *      1    gcsAccess                Defines the access level for the local GCS transactions (readonly=1 and readwrite=0), not used in the flight s/w
 *      2    telemetryAcked           Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
 *      3    gcsTelemetryAcked        Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
 *    4-5    telemetryUpdateMode      Update mode used by the telemetry module (UAVObjUpdateMode)
 *    6-7    gcsTelemetryUpdateMode   Update mode used by the GCS (UAVObjUpdateMode)
 */
class UAVObjMetadata {
public:
	unsigned char flags; /** Defines flags for update and logging modes and whether an update should be ACK'd (bits defined above) */
	unsigned short flightTelemetryUpdatePeriod; /** Update period used by the telemetry module (only if telemetry mode is PERIODIC) */
	unsigned short gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
	unsigned short loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */

	UAVObjMetadata();
	UAVObjMetadata(int flightAccess, int gcsAccess, int isFlightAckNeeded, int isGcsAckNeeded, int flightTelemetryUpdateMode, int gcsTelemetryUpdateMode, int flightTelemetryUpdatePeriod, int gcsTelemetryUpdatePeriod, int loggingUpdatePeriod);
	int loadFromPointer(const UAVObjMetadata* dataIn);
	bool isReadOnly() const;
	int getGcsUpdateMode();
};

#endif /* UAVOBJMETADATA_H_ */
