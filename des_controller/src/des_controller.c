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

#define ONLEFT   1
#define ONRIGHT  2
#define ERRORMSG -1
int doorSide;

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


	StateFunc CUR_STATE_HANDLER = &ID_SCAN_HANDLER;

	while(1)
	{
		// check if the current state is the starting state
		if(CUR_STATE_HANDLER == ID_SCAN_HANDLER)
		{
			printf("Waiting for Person...\n");
		}

		/* PHASE II - PART I: Call to receive Display object from controller */
		rcvid = MsgReceive(chid, &person, sizeof(person),NULL);
		if(rcvid == -1) /* error occur*/
		{
			perror("Message cannot be received \n");
			exit(EXIT_FAILURE);
		}

		display.person = person;

		/* PHASE II - PART II: Call for sending EOK back to the input */
		MsgReply(rcvid, EOK, &person, sizeof(Person));

		if(person.eventInput != EXIT)
		{
			// get input event from Person object and advance state machine to next accepting state (or error state)
			CUR_STATE_HANDLER = (StateFunc)(*CUR_STATE_HANDLER)(&display);
		}

//		/* PHASE II - sending the message to the display file*/
		if (MsgSend(coid, &display, sizeof(display), &response, sizeof(response)) == -1L) {
			printf("Controller: MsgSend had an error.\n");
			exit(EXIT_FAILURE);
		}
		display.errorMsg = 0;

		if(person.eventInput == EXIT)
		{
			printf("Exit Controller \n");
			break;
		}
	}

	/* PHASE III: destroy and detach the message when done */
	ChannelDestroy(chid);
	ConnectDetach(coid);

	return EXIT_SUCCESS;

}


void *ID_SCAN_HANDLER(Display *display)
{
	display->outputMessage = SCAN_ID;

	if(display->person.doorDirection == INBOUND)  //entering
	{
		doorSide = ONLEFT; // value 1
		if(display->person.eventInput == LEFT_SCAN){
			printf("doorSide == %d\n",doorSide);
			display->outputMessage = SCAN_ID;
			return DOOR_UNLOCK_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)   //leaving
	{
		doorSide = ONLEFT;
		if(display->person.eventInput == RIGHT_SCAN){
			display->outputMessage = SCAN_ID;
			return DOOR_UNLOCK_HANDLER;
		}
	}

	display->errorMsg = ERRORMSG;
	printf("stay the same function ID_SCAN_HANDLER\n");
	return ID_SCAN_HANDLER;
}

void *DOOR_UNLOCK_HANDLER(Display *display)
{
	if(display->person.doorDirection == INBOUND)  //entering
	{
		printf("GUARD_LEFT_UNLOCK doorSide %d\n",doorSide);
		if(display->person.eventInput == GUARD_LEFT_UNLOCK && doorSide == ONLEFT)
		{
			printf(" GUARD_LEFT_UNLOCK\n");
			display->outputMessage = LEFT_DOOR_UNLOCK;
			//return LEFT_OPEN_HANDLER;
			return DOOR_OPEN_HANDLER;
		}
		else if(display->person.eventInput == GUARD_RIGHT_UNLOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = RIGHT_DOOR_UNLOCK;
			//return RIGHT_OPEN_HANDLER;
			return DOOR_OPEN_HANDLER;
		}

	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == GUARD_RIGHT_UNLOCK && doorSide == ONLEFT)
		{
			display->outputMessage = RIGHT_DOOR_UNLOCK;
			//return RIGHT_OPEN_HANDLER;
			return DOOR_OPEN_HANDLER;
		}
		else if (display->person.eventInput == GUARD_LEFT_UNLOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = LEFT_DOOR_UNLOCK;
			//return LEFT_OPEN_HANDLER;
			return DOOR_OPEN_HANDLER;
		}
	}
	display->errorMsg = ERRORMSG;
	return DOOR_UNLOCK_HANDLER;
}

void *DOOR_OPEN_HANDLER(Display *display)
{

	if(display->person.doorDirection == INBOUND )  //entering
	{
		if(display->person.eventInput == LEFT_OPEN && doorSide == ONLEFT)
		{
			display->outputMessage = LEFT_DOOR_OPEN;
			printf("inbound left door open test \n");
			return WEIGHT_HANDLER;
		}
		else if(display->person.eventInput == RIGHT_OPEN && doorSide == ONRIGHT)
		{
			display->outputMessage = RIGHT_DOOR_OPEN;
			printf("inbound right door open test \n");
			return DOOR_CLOSE_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == RIGHT_OPEN && doorSide == ONLEFT)
		{
			display->outputMessage = RIGHT_DOOR_OPEN;
			printf("outbound right door open test \n");
			return WEIGHT_HANDLER;
		}
		if(display->person.eventInput == LEFT_OPEN && doorSide == ONRIGHT)
		{
			display->outputMessage = LEFT_DOOR_OPEN;
			printf("outbound left door open test \n");
			return DOOR_CLOSE_HANDLER;
		}
	}

	display->errorMsg = ERRORMSG;
	printf("stay the same function DOOR_OPEN_HANDLER\n");
	return DOOR_OPEN_HANDLER;
}


void *WEIGHT_HANDLER(Display *display)
{
	printf("weight function \n");

	if(display->person.eventInput == WEIGHT_SCALE)
	{
		display->outputMessage = WEIGHT_INFO;
		return DOOR_CLOSE_HANDLER;
	}

	printf("stay in same function WEIGHT_HANDLER\n");
	display->errorMsg = ERRORMSG;
	return WEIGHT_HANDLER;

}

void *DOOR_CLOSE_HANDLER(Display *display)
{

	if(display->person.doorDirection == INBOUND)  //entering
	{
		if(display->person.eventInput == LEFT_CLOSE && doorSide == ONLEFT)
		{
			display->outputMessage = LEFT_DOOR_CLOSED;
			doorSide = ONRIGHT;
			return DOOR_LOCK_HANDLER;
		}
		else if(display->person.eventInput == RIGHT_CLOSE && doorSide == ONRIGHT)
		{
			display->outputMessage = RIGHT_DOOR_CLOSED;
			return DOOR_LOCK_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND )  //leaving
	{
		if(display->person.eventInput == RIGHT_CLOSE && doorSide == ONLEFT)
		{
			display->outputMessage = RIGHT_DOOR_CLOSED;
			doorSide = ONRIGHT;
			return DOOR_LOCK_HANDLER;
		}
		if(display->person.eventInput == LEFT_CLOSE  && doorSide == ONRIGHT)
		{
			display->outputMessage = LEFT_DOOR_CLOSED;
			return DOOR_LOCK_HANDLER;
		}
	}
	display->errorMsg = ERRORMSG;
	return DOOR_CLOSE_HANDLER;
}

void *DOOR_LOCK_HANDLER(Display *display)
{
	if(display->person.doorDirection == INBOUND)//entering
	{
		if(display->person.eventInput == GUARD_LEFT_LOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = LEFT_DOOR_LOCKED;
			return DOOR_UNLOCK_HANDLER;
		}
		else if(display->person.eventInput == GUARD_RIGHT_LOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = RIGHT_DOOR_LOCKED;
			return ID_SCAN_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == GUARD_RIGHT_LOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = RIGHT_DOOR_LOCKED;
			return DOOR_UNLOCK_HANDLER;
		}
		else if(display->person.eventInput == GUARD_LEFT_LOCK && doorSide == ONRIGHT)
		{
			display->outputMessage = LEFT_DOOR_LOCKED;
			return ID_SCAN_HANDLER;
		}

		return DOOR_LOCK_HANDLER;
	}
	display->errorMsg = ERRORMSG;
	return DOOR_LOCK_HANDLER;
}

