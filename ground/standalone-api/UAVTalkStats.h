/*
 * UAVTalkStats.h
 *
 *  Created on: Sep 12, 2013
 *      Author: gustavo
 */

#ifndef UAVTALKSTATS_H_
#define UAVTALKSTATS_H_

//TODO: Isto não está bem de acordo com o produto actual

typedef enum { UAVTALK_STATE_ERROR = 0, UAVTALK_STATE_SYNC, UAVTALK_STATE_TYPE, UAVTALK_STATE_SIZE, UAVTALK_STATE_OBJID, UAVTALK_STATE_INSTID, UAVTALK_STATE_TIMESTAMP, UAVTALK_STATE_DATA, UAVTALK_STATE_CS, UAVTALK_STATE_COMPLETE } UAVTalkRxState;

class UAVTalkStats {
public:
	unsigned int  txBytes;
	unsigned int  rxBytes;
	unsigned int  txObjectBytes;
	unsigned int  rxObjectBytes;
	unsigned int  rxObjects;
	unsigned int  txObjects;
	unsigned int  txErrors;
	unsigned int  rxErrors;

	UAVTalkStats();
	UAVTalkStats(const UAVTalkStats &other);
	void UAVTalkStatsReset();
	UAVTalkStats& operator=(const UAVTalkStats &other);
};


#endif /* UAVTALKSTATS_H_ */
