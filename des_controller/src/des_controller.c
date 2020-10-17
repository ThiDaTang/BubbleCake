#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <unistd.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include "../../des_inputs/des.h"

int main(int argc, char* argv[]) {

	pid_t displayPID;
	Person person;
	int coid;		// connection id
	int rcvid;		// receive id
	int chid;		// channel id

	// PHASE I: create channel
	/* Get pid from command-line arguments */
	if(argc != 2) {
		printf("Usage: message of controller <display-PID>\n");
		exit(EXIT_FAILURE);
	}

	displayPID = atoi(argv[1]);

	/* Call ChannelCreate() to create a channel for the inputs process to attach */
	chid = ChannelCreate(0);
	if (chid == -1){
		perror("Failed to create the channel.\n");
		exit(EXIT_FAILURE);
	}

	/* Call ConnectAttach() to attach to display's channel */
	coid = ConnectAttach (ND_LOCAL_NODE, displayPID, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror ("Couldn't Connect Attach \n");
		exit (EXIT_FAILURE);
	}

	/* Print controller's PID; inputs needs to know this PID */
	printf("Controller's PID: %d \n",getpid());

	/* PHASE II: processing the message */
	while(1)
	{
		/* PHASE II - PART I: Call to receive Display object from controller */
		rcvid = MsgReceive(chid, &person, sizeof(person),NULL);
		if(rcvid == -1) /* error occur*/
		{
			perror("Message cannot be received \n");
			exit(EXIT_FAILURE);
		}

		/* PHASE II - PART II:Call for sending EOK back to the controller */
		MsgReply(rcvid, EOK, &person, sizeof(person));

		// get input event from Person object and advance state machine to next accepting state (or error state)
		// complete rest of Phase II for controller
		printf("Printout in the Controller\n");
	}

	/* PHASE III: destroy and detach the message when done */
	ChannelDestroy(chid);
	ConnectDetach(coid);

	return EXIT_SUCCESS;

}
