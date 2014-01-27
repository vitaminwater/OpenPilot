/*
 * usbstream.cpp
 *
 *  Created on: Sep 26, 2013
 *      Author: gustavo
 */

#include "USBStream.h"

using namespace std;

USBStream::USBStream(int vid, int pid) {
	this->vid = vid;
	this->pid = pid;
	
	if (hid_init()) {
		handle =  NULL;
		if(DEBUG)std::cout << "HID not functional." << std::endl;
	} else
		handle = hid_open(this->vid, this->pid, NULL);

	if(!handle) {
		if(DEBUG)std::cout << "Could not open device." << std::endl;
	}
	
}

int USBStream::write(const unsigned char* s, streamsize n) {
	if(DEBUG)std::cout << "USB write called and written data was: ";

	if(DEBUG) {
		std::cout << std::hex;
		for(int i = 0; i < n; i++) {
			std::cout << (int)*(s+i) << " ";
		}
		std::cout << std::dec << std::endl;
	}
	
	int i = -1;
	char* data = (char*)malloc(64);
	memset(data , 0, 64);
	memcpy(data+2, s, n);
	*data = 2;
	*(data + 1) = n;
	if(this->handle) {
		i = hid_write(this->handle, (unsigned char*)data, 64);
	}

	free(data);
	return i;
}

int USBStream::read(unsigned char* s, streamsize n) {

	char* bytes = (char*)malloc(n*sizeof(char));

	int res = -2;
	if(this->handle)
		res = hid_read(this->handle, (unsigned char*)bytes, n);

	if(res < 0 || *bytes != 0x01) {
		if(DEBUG)std::cout << "Read error" << std::endl;
		return res;
	}

	res = 0;
	memcpy(&res, (bytes + 1), 1);
	memcpy(s, (bytes + 2), res);

	if(DEBUG)std::cout << "USB read called and " << std::dec << res << " bytes read." << std::endl;

	return res;

}
