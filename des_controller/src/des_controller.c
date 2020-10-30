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

typedef void*(*StateFunc)();

void *startStateHandler(Person person, Display *display)
{
	if(person.eventInput == LEFT_SCAN)
	{
		display->outputMessage = SCAN_ID;
		return lsStateHandler;
	}
	else if(person.eventInput == RIGHT_SCAN)
	{
		display->outputMessage = SCAN_ID;
		return rsStateHandler;
	}
	else
	{
		exit(EXIT_FAILURE);
	}
}

void *lsStateHandler(Person person, Display *display)
{
	printf(" left state \n");
	// go to leftUnblock
}


void *rsStateHandler(Person person, Display *display)
{
	printf(" right state \n");
}

int main(int argc, char* argv[]) {

	pid_t displayPID;
	Display display;
	Person person;
	int response;	// response from display.c
	int coid;		// connection id
	int rcvid;		// receive id
	int chid;		// channel id

	// PHASE I: create channel
	/* Get pid from command-line arguments */
	if(argc != 2) {
		printf("Controller: argument missing \n");
		exit(EXIT_FAILURE);
	}

	displayPID = atoi(argv[1]);

	/* Call ChannelCreate() to create a channel for the inputs process to attach */
	chid = ChannelCreate(0);
	if (chid == -1){
		perror("Controller: Failed to create the channel.\n");
		exit(EXIT_FAILURE);
	}

	/* Call ConnectAttach() to attach to display's channel */
	coid = ConnectAttach (ND_LOCAL_NODE, displayPID, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror ("Controller: Couldn't Connect Attach \n");
		exit (EXIT_FAILURE);
	}

	/* PHASE II: processing the message */
	/* Print controller's PID; inputs needs to know this PID */
	printf("The controller is running as PID: %d \n",getpid());

	StateFunc currentStateHandler = startStateHandler;

	while(1)
	{
		/* PHASE II - PART I: Call to receive Display object from controller */
		rcvid = MsgReceive(chid, &person, sizeof(person),NULL);
		if(rcvid == -1) /* error occur*/
		{
			perror("Message cannot be received \n");
			exit(EXIT_FAILURE);
		}

		// check if the current state is the starting state
//		if(currentStateHandler == startStateHandler)
//		{
//			printf("Waiting for Person...\n");
//		}

		printf("debug: person id is : %d\n", person.id);

		// get input event from Person object and advance state machine to next accepting state (or error state)
		// complete rest of Phase II for controller
		currentStateHandler = (StateFunc)(*currentStateHandler)(person, &display);

		/* PHASE II - sending the message to the display file*/
		if (MsgSend(coid, &display, sizeof(display), &response, sizeof(response)) == -1L) {
			printf("Controller: MsgSend had an error.\n");
			exit(EXIT_FAILURE);
		}

		/* PHASE II - PART II: Call for sending EOK back to the input */
		MsgReply(rcvid, EOK, &person, sizeof(Person));

	}

	/* PHASE III: destroy and detach the message when done */
	ChannelDestroy(chid);
	ConnectDetach(coid);

	return EXIT_SUCCESS;

}
