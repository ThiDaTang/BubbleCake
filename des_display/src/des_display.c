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

	Output outputMessage;
	Input inputCommand;
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
		rcvid = MsgReceive(chid, &inputCommand, sizeof(inputCommand), NULL);

		printf("Printout from Display\n");

		// PHASE II - PART II:Call for sending EOK back to the controller
		MsgReply(rcvid, EOK, &outputMessage, sizeof(Output));
	}

	/* PHASE III: destroy the message */
	ChannelDestroy(chid);

	return EXIT_SUCCESS;
}
