/*
 * usbostream.h
 *
 *  Created on: Sep 26, 2013
 *      Author: gustavo
 */

#ifndef USBSTREAM_H_
#define USBSTREAM_H_

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "hidapi.h"

#define DEBUG 0

using namespace std;

class USBStream: public iostream {
private:
	hid_device *handle;
	int vid, pid;

public:
	USBStream(int vid, int pid);
	int write(const unsigned char* s, streamsize n);
	int read(unsigned char* s, streamsize n);
	void close(){};

};

#endif /* USBSTREAM_H_ */
