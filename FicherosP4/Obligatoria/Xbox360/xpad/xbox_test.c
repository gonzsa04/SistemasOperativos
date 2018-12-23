/*
 * Extracted from jstest.c developed by Vojtech Pavlik
 */

/*
 * This program can be used to test all the features of the Linux
 * joystick API. It is also intended to serve as an example 
 * implementation for those who wish to learn
 * how to write their own joystick using applications.
 */


#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <linux/joystick.h>

#define NAME_LENGTH 128

int main (int argc, char **argv)
{
	int fd;
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";

	struct ff_effect effect, effect_weak, effect_strong;
	struct input_event gain, play, stop;
	const char * device_file_name = "/dev/input/event0";

	int i;

	for (i=1; i<argc; ++i) {
		if (strncmp(argv[i], "--help", 64) == 0) {
			printf("Usage: %s /dev/input/eventXX\n", argv[0]);
			printf("Tests the force feedback driver\n");
			exit(1);
		}
		else {
			device_file_name = argv[i];
		}
	}

	/* Open device */
	fd = open(device_file_name, O_RDWR);
	if (fd == -1) {
		perror("Open device file");
		exit(1);
	}
	printf("Device %s opened\n", device_file_name);

	memset(&gain, 0, sizeof(gain));
	gain.type = EV_FF;
	gain.code = FF_GAIN;
	gain.value = 0x9FFF; /* [0, 0xFFFF]) */

	printf("Setting master gain to 75%% ... ");
	fflush(stdout);
	if (write(fd, &gain, sizeof(gain)) != sizeof(gain)) {
	  perror("Error:");
	} else {
	  printf("OK\n");
	}

	/****************************/
	/*  strong rumbling effect  */
	/****************************/
	memset(&effect_strong,0,sizeof(effect));
	effect_strong.type = FF_RUMBLE;
	effect_strong.id = -1;
	effect_strong.u.rumble.strong_magnitude = 0xffff;
	effect_strong.u.rumble.weak_magnitude = 0;
	effect_strong.replay.length = 900; // 900 ms
	effect_strong.replay.delay = 0;

	printf("Uploading effect # (Strong rumble, with heavy motor) ... ");
	fflush(stdout);
	if (ioctl(fd, EVIOCSFF, &effect_strong) == -1) {
		perror("Error");
	} else {
		printf("OK (id %d)\n", effect_strong.id);
	}

	if (argc < 2 || argc > 3 || !strcmp("--help", argv[1])) {
		puts("");
		puts("Usage: jstest [<mode>] <device>");
		puts("");
		puts("");
		exit(1);
	}
	if ((fd = open(argv[argc - 1], O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	/**************************/
	/*  weak rumbling effect  */
	/**************************/
	memset(&effect_weak,0,sizeof(effect));
	effect_weak.type = FF_RUMBLE;
	effect_weak.id = -1;
	effect_weak.u.rumble.strong_magnitude = 0;
	effect_weak.u.rumble.weak_magnitude = 0xffff;
	effect_weak.replay.length =  900; // 900 ms
	effect_weak.replay.delay = 0;

	printf("Uploading effect # (Weak rumble) ... ");
	fflush(stdout);
	if (ioctl(fd, EVIOCSFF, &effect_weak) == -1) {
		perror("Error:");
	} else {
		printf("OK (id %d)\n", effect_weak.id);
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

	printf("Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d.\n",
		name, axes, buttons, version >> 16, (version >> 8) & 0xff, version & 0xff);
	printf("Testing ... (interrupt to exit)\n");

/*
 * Event interface, single line readout.
 */

	if (argc == 2 ) {

		int *axis;
		int *button;
		int i;
		struct js_event js;

		axis = calloc(axes, sizeof(int));
		button = calloc(buttons, sizeof(char));

		while (1) {
			if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
				perror("\njstest: error reading");
				exit (1);
			}

			switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
			}

			printf("\r");


			if (buttons) {
				printf("Buttons: ");
				for (i = 0; i < buttons; i++)
					if (i==0)effect_weak.u.rumble.strong_magnitude = 0xffff;
			}

			fflush(stdout);
		}
	}

	/*************/
	/* Main loop */
	/*************/
	printf("\n\n");
	for (i=1; i<16; i++)
	{
		/* Set master gain to x% if supported */
		memset(&gain, 0, sizeof(gain));
		gain.type = EV_FF;
		gain.code = FF_GAIN;
		gain.value = i<<12;

		printf("Setting master gain to %i%% ... ", i*100/16);
		fflush(stdout);
		if (write(fd, &gain, sizeof(gain)) != sizeof(gain)) {
		  perror("Error:");
		} else {
		  printf("OK\n");
		}

		/* Start the effect */
		memset(&play,0,sizeof(play));
		play.type = EV_FF;
		play.code = effect_strong.id;
		play.value = 1;

		if (write(fd, (const void*) &play, sizeof(play)) == -1) {
			perror("Play effect");
			exit(1);
		}
		printf("Strong\n");
		sleep(1);
	}

	for (i=1; i<16; i++)
	{
		/* Set master gain to x% if supported */
		memset(&gain, 0, sizeof(gain));
		gain.type = EV_FF;
		gain.code = FF_GAIN;
		gain.value = i<<12;

		printf("Setting master gain to %i%% ... ", i*100/16);
		fflush(stdout);
		if (write(fd, &gain, sizeof(gain)) != sizeof(gain)) {
		  perror("Error:");
		} else {
		  printf("OK\n");
		}
		/* Start the effect */
		memset(&play,0,sizeof(play));
		play.type = EV_FF;
		play.code = effect_weak.id;
		play.value = 1;

		if (write(fd, (const void*) &play, sizeof(play)) == -1) {
			perror("Play effect");
			exit(1);
		}
		printf("Weak\n");
		sleep(1);
	}
	/* Stop the effect */
	printf("Stopping effects\n");
	memset(&stop,0,sizeof(stop));
	stop.type = EV_FF;
	stop.code =  effect.id;
	stop.value = 0;

	if (write(fd, (const void*) &stop, sizeof(stop)) == -1) {
		perror("");
		exit(1);
	}


	return -1;
}
