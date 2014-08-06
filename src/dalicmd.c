#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <libusb-1.0/libusb.h>

static const int _VID = 0x04CC;
static const int _PID = 0x0802;

int main(int argc, char** argv) {
	int err;
	if ( argc < 3 || strlen(argv[1]) < 2) {
		printf("usage: dalicmd <g|s><c|d> <address> <command or direct control>\n");
		exit(0);
	}
	libusb_context* ctx = NULL;
	libusb_init(&ctx);
//	libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);
	libusb_device_handle* dev = NULL;
	dev = libusb_open_device_with_vid_pid(ctx, _VID, _PID);
	err = libusb_set_auto_detach_kernel_driver(dev, 1);
	if (err < 0) {
		printf("interrupt_transfer... %s\n", strerror(errno));
		exit(1);
	}
	err = libusb_claim_interface(dev, 0);
	if (err < 0) {
		printf("claiming interface... %s\n", strerror(errno));
		exit(1);
	}
	uint8_t command[2] = {0, 0};
	
	switch(argv[1][0]) {
		case 'g':
			command[0] |= 0x80;
		break;
		case 's':
			command[0] &= 0x7f;
		break;
	}

	switch(argv[1][1]) {
		case 'c':
			command[0] |= 0x01;
		break;
		case 'd':
			command[0] &= 0xfe;
		break;
	}

	uint8_t address = atoi(argv[2]) & 0x3f;
	command[0] |= (address << 1);
	command[1] = atoi(argv[3]);

	printf("COMMAND: %.2X %.2X\n", command[0], command[1]);

	int nw = 0;
	err = libusb_interrupt_transfer(dev, 0x01, command, 2, &nw, 0);
	if (err < 0) {
		printf("interrupt_transfer... %s\n", strerror(errno));
		exit(1);
	}
	libusb_exit(ctx);
}


