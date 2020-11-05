#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include "../../des_inputs/des.h"

int main (void) {

	Display display;
	int rcvid;		// receive id
	int chid;		// channel id

	/* PHASE I: create a channel */
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("Display: Failed to create the channel\n");
		exit(EXIT_FAILURE);
	}

	printf("The display is running as PID: %d\n", getpid());

	/* PHASE II: processing the message */
	while(1) {

		// PHASE II - PART I: Call to receive Display object from controller
		rcvid = MsgReceive(chid, &display, sizeof(display), NULL);
		if(rcvid == -1) /* error occur*/
		{
			perror("Message cannot be received \n");
			exit(EXIT_FAILURE);
		}

		printf("errorMsg %d\n",display.errorMsg);
		if(display.errorMsg != -1 && display.person.eventInput != EXIT)
		{
			printf("%s", outMessage[display.outputMessage]);

			if(display.person.eventInput == LEFT_SCAN || display.person.eventInput == RIGHT_SCAN)
			{
				printf("%d\n", display.person.id);
			}
			else if(display.person.eventInput == WEIGHT_SCALE)
			{
				printf("%d\n", display.person.weight);
			}
		}

		// PHASE II - PART II:Call for sending EOK back to the controller
		MsgReply(rcvid, EOK, &display, sizeof(Display));


		if(display.person.eventInput == EXIT)
		{
			printf("Exit Display \n");
			break;
		}
	}

	/* PHASE III: destroy the message */
	ChannelDestroy(chid);

	return EXIT_SUCCESS;
}
