
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>

int main(int argc, char** argv)
{
	struct ff_effect effect, effect_weak, effect_strong;
	struct input_event gain, play, stop;
	int fd;
	const char * device_file_name = "/dev/input/event0";

	int i;

	printf("HOLD FIRMLY YOUR WHEEL OR JOYSTICK TO PREVENT DAMAGES\n\n");

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


	/***************************************/
	/* Set master gain to 75% if supported */
	/***************************************/
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
	

	exit(0);
}
