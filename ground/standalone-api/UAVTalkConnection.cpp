/*
 * UAVTalkConnection.cpp
 *
 *  Created on: Sep 19, 2013
 *      Author: gustavo
 */

#include "UAVTalkConnection.h"

UAVTalkConnection::UAVTalkConnection(std::istream* inStream, std::ostream* outStream, int telemetryLink, UAVObjManager* creator) {

	if(DEBUG)std::cout << "Initializing UAVTalkConnection" << std::endl;

	this->inputStream = inStream;
	this->outputStream = outStream;
	this->fatherManager = creator;
	this->stats = UAVTalkStats();
	this->checkIncome = std::thread(&UAVTalkConnection::UAVTalkCheckNewMessages, this);
	this->waitingForAckObj = NULL;
	this->waitingForAckStatus = ACK_STATUS_NONE;
	this->telemetryLink = telemetryLink;
}

bool UAVTalkConnection::UAVTalkSetOutputStream(std::ostream* outStream) {
	this->outputStream = outStream;
	return true;
}

std::ostream* UAVTalkConnection::UAVTalkGetOutputStream() {
	return this->outputStream;
}

bool UAVTalkConnection::UAVTalkSetInputStream(std::istream* inStream) {
	this->inputStream = inStream;
	return true;
}

std::istream* UAVTalkConnection::UAVTalkGetInputStream() {
	return this->inputStream;
}

bool UAVTalkConnection::UAVTalkSetObjectManager(UAVObjManager* objManager) {
	this->fatherManager = objManager;
	return true;
}

UAVObjManager* UAVTalkConnection::UAVTalkGetObjectManager() {
	return this->fatherManager;
}

bool UAVTalkConnection::UAVTalkSetStats(UAVTalkStats* stats) {
	this->stats = *stats;
	return true;
}

UAVTalkStats* UAVTalkConnection::UAVTalkGetStats() {
	return &(this->stats);
}

void UAVTalkConnection::UAVTalkResetStats() {
	this->stats.UAVTalkStatsReset();
}

int UAVTalkConnection::UAVTalkSendObject(UAVObject* obj, bool acked, int timeoutMs) {
	if(DEBUG) std::cout << "UAVTalkConnection sending object" << std::endl;

	UAVTalkPacket packet;
	packet.sync_val = UAVTALK_SYNC_VAL;
	if (acked)
		packet.message_type = UAVTALK_TYPE_OBJ_ACK;
	else
		packet.message_type = UAVTALK_TYPE_OBJ;
	int dataLength = obj->UAVObjGetNumBytes();
	packet.length = dataLength + 8;
	packet.object_id = obj->UAVObjGetID();
	if (!obj->UAVObjIsSingleInstance()) {
		packet.length += 2;
		packet.instance_id = obj->UAVObjGetInstance();
	}
	unsigned char* dataArray = (unsigned char*) malloc(sizeof(char) * dataLength);
	obj->UAVObjPack(dataArray);
	packet.data = dataArray;
	packet.checksum = this->UAVTalkComputeCRC(0, (unsigned char*)&packet.sync_val, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.message_type, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.length, 2);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.object_id, 4);
	if(!obj->UAVObjIsSingleInstance()) {
		packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.instance_id, 2);
	}
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, packet.data, dataLength);

	int displace = 0;
	unsigned char* outgoingData;
	if (!obj->UAVObjIsSingleInstance()) {
		outgoingData = (unsigned char*)malloc(11 + dataLength);
	} else {
		outgoingData = (unsigned char*)malloc(9 + dataLength);
	}

	memcpy(outgoingData + displace, &packet.sync_val, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.message_type, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.length, 2);
	displace += 2;
	memcpy(outgoingData + displace, &packet.object_id, 4);
	displace += 4;
	if (!obj->UAVObjIsSingleInstance()) {
		memcpy(outgoingData + displace, &packet.instance_id, 2);
		displace += 2;
	}
	memcpy(outgoingData + displace, packet.data, dataLength);
	displace += dataLength;
	memcpy(outgoingData + displace, &packet.checksum, 1);
	displace += 1;

	this->writeToTelemetry(outgoingData, displace);

	if(acked) {
		time_t currentTime = time(NULL);
		time_t timeoutTime = currentTime + timeoutMs;
		this->waitingForAckObj = obj;
		this->waitingForAckStatus = ACK_STATUS_NONE;
		while (time(NULL) < timeoutTime || waitingForAckStatus == ACK_STATUS_NONE) {
			continue;
		}
	}

	return this->waitingForAckStatus;
}

bool UAVTalkConnection::UAVTalkSendObjectRequest(UAVObject* obj, int timeoutMs) {
	if(DEBUG) std::cout << "UAVTalkConnection sending object request" << std::endl;
	UAVTalkPacket packet;
	packet.sync_val = UAVTALK_SYNC_VAL;
	packet.message_type = UAVTALK_TYPE_OBJ_REQ;
	packet.length = 8;
	packet.object_id = obj->UAVObjGetID();
	if (!obj->UAVObjIsSingleInstance()) {
		packet.length += 2;
		packet.instance_id = obj->UAVObjGetInstance();
	}

	packet.checksum = this->UAVTalkComputeCRC(0, (unsigned char*)&packet.sync_val, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.message_type, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.length, 2);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.object_id, 4);
	if(!obj->UAVObjIsSingleInstance()) {
		packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.instance_id, 2);
	}

	int displace = 0;
	unsigned char* outgoingData;
	if (!obj->UAVObjIsSingleInstance()) {
		outgoingData = (unsigned char*)malloc(11);
	} else {
		outgoingData = (unsigned char*)malloc(9);
	}
	
	memcpy(outgoingData + displace, &packet.sync_val, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.message_type, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.length, 2);
	displace += 2;
	memcpy(outgoingData + displace, &packet.object_id, 4);
	displace += 4;
	if (!obj->UAVObjIsSingleInstance()) {
		memcpy(outgoingData + displace, &packet.instance_id, 2);
		displace += 2;
	}
	memcpy(outgoingData + displace, &packet.checksum, 1);
	displace += 1;

	this->writeToTelemetry(outgoingData, displace);

	return true;
}

//FIXME:Max size for uavobject 64 bytes
void UAVTalkConnection::UAVTalkCheckNewMessages() {
	if(DEBUG) std::cout << "UAVTalkCheckNewMessages thread is now running" << std::endl;
	unsigned char* receivedData = (unsigned char*)malloc(512);
	int readLength = 0;
	while (1) {
		readLength = this->readFromTelemetry(receivedData);
		unsigned char* data = (unsigned char*)malloc(readLength);
		if(DEBUG) {
			for(int i = 0; i < readLength; i++) {
				std::cout << std::hex << (int)*(data + i) << " ";
			}	
			std::cout << std::endl;
		}
		this->processNewMessage(receivedData, readLength);
		free(data);
	}
}

//TODO: verify the checksum
void UAVTalkConnection::processNewMessage(unsigned char* data, int remaining) {
	unsigned char sync_val;
	unsigned char type;
	unsigned short length;
	unsigned int objectID;
	unsigned char* objData;

	memcpy(&sync_val, data, 1);
	data++;
	remaining--;
	memcpy(&type, data, 1);
	data++;
	remaining--;
	memcpy(&length, data, 2);
	data++;
	remaining--;
	data++;
	remaining--;
	memcpy(&objectID, data, 4);
	data++;
	remaining--;
	data++;
	remaining--;
	data++;
	remaining--;
	data++;
	remaining--;

	if(sync_val != 0x3c) {
		if(DEBUG) std::cout << std::hex << "Not expected char found: " << sync_val << std::dec << "packet is going to be dropped." << std::endl;
		return;
	}

	if(DEBUG) std::cout << std::hex << "Processing message of type: " <<  (int)type << " with length: " << length << " for object: " << objectID << std::dec << std::endl;

	UAVObject* obj;
	UAVObjEvent event;
	switch (type) {
	case UAVTALK_TYPE_OBJ:
		if (fatherManager->UAVObjMngrIsSingleInstanceType(objectID)) {
			objData = (unsigned char*) malloc(length - 4);
			memcpy(objData, data, length - 4);
			obj = fatherManager->UAVObjGetByID(objectID);
		} else {
			unsigned short instance;
			memcpy(&instance, data, 2);
			objData = (unsigned char*) malloc(length - 6);
			memcpy(objData, data, length - 6);
			obj = fatherManager->UAVObjGetByID(objectID, instance);
		}
		if (obj == NULL) {
			//TODO: Raise error, shouldn't happen
			if(DEBUG) std::cout << "Object not found" << std::endl;
			break;
		}
		event = UAVObjEvent(obj, EV_UPDATED, objData);
		fatherManager->UAVObjMngrPushEvent(event);
		break;
	case UAVTALK_TYPE_OBJ_REQ:
		if (fatherManager->UAVObjMngrIsSingleInstanceType(objectID)) {
			objData = (unsigned char*) malloc(length - 4);
			memcpy(objData, data, length - 4);
			obj = fatherManager->UAVObjGetByID(objectID);
		} else {
			unsigned short instance;
			memcpy(&instance, data, 2);
			objData = (unsigned char*) malloc(length - 6);
			memcpy(objData, data, length - 6);
			obj = fatherManager->UAVObjGetByID(objectID, instance);
		}
		if (obj == NULL) {
			//TODO: Raise error, shouldn't happen
			if(DEBUG) std::cout << "Object not found" << std::endl;
			this->UAVTalkSendNack(objectID);
			break;
		}
		event = UAVObjEvent(obj, EV_UPDATE_REQ, objData);
		fatherManager->UAVObjMngrPushEvent(event);
		break;
	case UAVTALK_TYPE_OBJ_ACK:
		if (fatherManager->UAVObjMngrIsSingleInstanceType(objectID)) {
			objData = (unsigned char*) malloc(length - 4);
			memcpy(objData, data, length - 4);
			obj = fatherManager->UAVObjGetByID(objectID);
		} else {
			unsigned short instance;
			memcpy(&instance, data, 2);
			objData = (unsigned char*) malloc(length - 6);
			memcpy(objData, data, length - 6);
			obj = fatherManager->UAVObjGetByID(objectID, instance);
		}
		if (obj == NULL) {
			if(DEBUG) std::cout << "Object not found" << std::endl;
			this->UAVTalkSendNack(objectID);
			break;
		} else {
			this->UAVTalkSendAck(obj);
		}
		event = UAVObjEvent(obj, EV_UPDATED, objData);
		fatherManager->UAVObjMngrPushEvent(event);
		break;
	case UAVTALK_TYPE_ACK:
		obj = fatherManager->UAVObjGetByID(objectID);
		if(obj == this->waitingForAckObj) {
			this->waitingForAckStatus = ACK_STATUS_OK;
		}
		break;
	case UAVTALK_TYPE_NACK:
		obj = fatherManager->UAVObjGetByID(objectID);
		if (obj == this->waitingForAckObj) {
			this->waitingForAckStatus = ACK_STATUS_NOT;
		}
		break;
	default:
		//TODO: raise error and put it into the stats
		if(DEBUG) std::cout << "Wrong message type" << std::endl;
		break;
	}
}

bool UAVTalkConnection::UAVTalkSendAck(UAVObject* obj) {

	if(DEBUG) std::cout << "Sending ACK" << std::endl;

	UAVTalkPacket packet;
	packet.sync_val = UAVTALK_SYNC_VAL;
	packet.message_type = UAVTALK_TYPE_ACK;
	packet.length = 8;
	packet.object_id = obj->UAVObjGetID();
	if (!obj->UAVObjIsSingleInstance()) {
		packet.length += 2;
		packet.instance_id = obj->UAVObjGetInstance();
	}

	packet.checksum = this->UAVTalkComputeCRC(0, (unsigned char*)&packet.sync_val, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.message_type, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.length, 2);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.object_id, 4);
	if(!obj->UAVObjIsSingleInstance()) {
		packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.instance_id, 2);
	}

	int displace = 0;
	unsigned char* outgoingData;
	if (!obj->UAVObjIsSingleInstance()) {
		outgoingData = (unsigned char*)malloc(11);
	} else {
		outgoingData = (unsigned char*)malloc(9);
	}
	
	memcpy(outgoingData + displace, &packet.sync_val, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.message_type, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.length, 2);
	displace += 2;
	memcpy(outgoingData + displace, &packet.object_id, 4);
	displace += 4;
	if (!obj->UAVObjIsSingleInstance()) {
		memcpy(outgoingData + displace, &packet.instance_id, 2);
		displace += 2;
	}
	memcpy(outgoingData + displace, &packet.checksum, 1);
	displace += 1;

	this->writeToTelemetry(outgoingData, displace);

	return true;
}

bool UAVTalkConnection::UAVTalkSendNack(unsigned int objectID) {

	if(DEBUG) std::cout << "Sending NACK" << std::endl;

	UAVTalkPacket packet;
	packet.sync_val = UAVTALK_SYNC_VAL;
	packet.message_type = UAVTALK_TYPE_ACK;
	packet.length = 8;
	packet.object_id = objectID;

	packet.checksum = this->UAVTalkComputeCRC(0, (unsigned char*)&packet.sync_val, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.message_type, 1);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.length, 2);
	packet.checksum = this->UAVTalkComputeCRC(packet.checksum, (unsigned char*)&packet.object_id, 4);

	int displace = 0;
	unsigned char* outgoingData;
	outgoingData = (unsigned char*)malloc(9);
	
	memcpy(outgoingData + displace, &packet.sync_val, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.message_type, 1);
	displace += 1;
	memcpy(outgoingData + displace, &packet.length, 2);
	displace += 2;
	memcpy(outgoingData + displace, &packet.object_id, 4);
	displace += 4;
	memcpy(outgoingData + displace, &packet.checksum, 1);
	displace += 1;

	this->writeToTelemetry(outgoingData, displace);

	return true;
}

unsigned char UAVTalkConnection::UAVTalkComputeCRC(unsigned char initialValue, const unsigned char* data, int length) {

	int len = length;
	unsigned char crc8 = initialValue;

	while (len--) {
		crc8 = crc_table[crc8 ^ *data++];
	}

	return crc8;
}

int UAVTalkConnection::readFromTelemetry(unsigned char* outAddr) {
	switch(this->telemetryLink) {
	case TELEMETRY_USBHID:
		return ((USBStream*)(this->inputStream))->read(outAddr, 64);
	}

	return 0;
}

int UAVTalkConnection::writeToTelemetry(unsigned char* outAddr, int length) {
	switch(this->telemetryLink) {
	case TELEMETRY_USBHID:
		return ((USBStream*)(this->outputStream))->write(outAddr, length);
	}

	return 0;
}
