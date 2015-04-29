/*
 * UAVObjStats.h
 *
 *  Created on: Sep 13, 2013
 *      Author: gustavo
 */

#ifndef UAVOBJMNGRSTATS_H_
#define UAVOBJMNGRSTATS_H_

/**
 * Event manager statistics
 */
class UAVObjMngrStats {
public:
	unsigned int eventQueueErrors;
	unsigned int lastQueueErrorID;
	unsigned int numInsertedObjects;

	UAVObjMngrStats(){eventQueueErrors = 0; numInsertedObjects = 0; lastQueueErrorID = 0;};
	void reset(){eventQueueErrors = 0; lastQueueErrorID = 0;};
	void incrementInsertedObjects(){numInsertedObjects++;};

};


#endif /* UAVOBJMNGRSTATS_H_ */
