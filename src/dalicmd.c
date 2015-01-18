#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <libusb-1.0/libusb.h>

static const int _VID = 0x04CC;
static const int _PID = 0x0802;
static uint8_t command[2] = {0, 0};
static uint8_t result[2] = { 0, 0 };

static int timeout = 0;

static struct option longopts[] = {
	{"group", required_argument, 0, 'g'},
	{"address", required_argument, 0, 'a'},
	{"set", required_argument, 0, 's'},
	{"cmd", required_argument, 0, 'c'},
	{"timeout", required_argument, 0, 't'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

static void printhelp() {
	printf("--group    -g xx\n"
		   "--address  -a xx\n"
		   "--set      -s xx\n"
		   "--cmd      -c xx\n"
		   "--timeout  -t xx\n"
		   "--help     -h   \n");
	exit(1);
	return;
}

static void parse_args(int argc, char** argv) {
	while(1) {
		int optindex;
		int c = getopt_long(argc, argv, "g:a:c:s:t:h", longopts, &optindex);
		long tmp = 0;
		if(c == -1)
			break;
		switch(c) {
			case 'g':
				tmp = strtol(optarg, NULL, 0) & 0x3F;
				command[0] |= 0x80;
				command[0] |= (tmp << 1);
				break;
			case 'a':
				tmp = strtol(optarg, NULL, 0) & 0x3F;
				command[0] &= 0x7f;
				command[0] |= (tmp << 1);
				break;
			case 'c':
				tmp = strtol(optarg, NULL, 0);
				command[0] |= 0x01;
				command[1] = (uint8_t)(tmp & 0xFF);
				break;
			case 's':
				tmp = strtol(optarg, NULL, 0);
				command[0] &= 0xfe;
				command[1] = (uint8_t)(tmp & 0xFF);
				break;
			case 'h':
				printhelp();
				break;
			case 't':
				timeout = strtol(optarg, NULL, 0);
			case '?':
			default:
				printhelp();
		}
	}
}

int main(int argc, char** argv) {
	int err;
	if(argc < 2)
		printhelp();
	parse_args(argc, argv);
	
	libusb_context* ctx = NULL;
	libusb_init(&ctx);
//	libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);
	libusb_device_handle* dev = NULL;
	dev = libusb_open_device_with_vid_pid(ctx, _VID, _PID);
	if(dev == NULL) {
		printf("opening device... %s\n", strerror(errno));
		exit(1);
	}
	err = libusb_set_auto_detach_kernel_driver(dev, 1);
	if (err < 0) {
		printf("failed to set auto detach kernel driver... %s\n", strerror(errno));
		exit(1);
	}
	err = libusb_claim_interface(dev, 0);
	if (err < 0) {
		printf("claiming interface... %s\n", strerror(errno));
		exit(1);
	}
	printf("COMMAND: %.2X %.2X\n", command[0], command[1]);
	int nw = 0;
	err = libusb_interrupt_transfer(dev, 0x01, command, 2, &nw, 0);
	if (err < 0) {
		printf("interrupt_transfer... %s\n", strerror(errno));
		exit(1);
	}

	err = libusb_interrupt_transfer(dev, 0x81, result, 2, &nw, timeout);
	printf("RESULT: %.2X %.2X\n", result[0], result[1]);

	libusb_release_interface(dev, 0);
	libusb_close(dev);
	libusb_exit(ctx);
}


