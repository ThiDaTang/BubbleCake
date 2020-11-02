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
		else
		{
			person.eventInput = EXIT;
		}


//		/* PHASE II - sending the message to the display file*/
		if (MsgSend(coid, &display, sizeof(display), &response, sizeof(response)) == -1L) {
			printf("Controller: MsgSend had an error.\n");
			exit(EXIT_FAILURE);
		}

		if(person.eventInput == EXIT)
		{
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
	printf("ID_SCAN function test\n");
	display->outputMessage = SCAN_ID;

	if(display->person.doorDirection == INBOUND)  //entering
	{
		if(display->person.eventInput == LEFT_SCAN){
			display->person.curState = LEFT_STATE;
			printf("left_scan test");
			return DOOR_UNLOCK_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)   //leaving
	{
		if(display->person.eventInput == RIGHT_SCAN){
			display->person.curState = RIGHT_STATE;
			printf("right_scan test");
			return DOOR_UNLOCK_HANDLER;
		}
	}

	printf("stay the same function ID_SCAN_HANDLER\n");
	return ID_SCAN_HANDLER;
}

void *DOOR_UNLOCK_HANDLER(Display *display)
{
	if(display->person.doorDirection == INBOUND)  //entering
	{
		if(display->person.eventInput == GUARD_LEFT_UNLOCK)
		{
			display->outputMessage = LEFT_DOOR_UNLOCK;
			display->person.curState = LEFT_UNLOCK_STATE;
			printf("inbound left unlock test\n");
			return DOOR_OPEN_HANDLER;
		}
		else if(display->person.eventInput == GUARD_RIGHT_UNLOCK)
		{
			display->outputMessage = RIGHT_DOOR_UNLOCK;
			display->person.curState = RIGHT_UNLOCK_STATE;
			printf("inbound right unlock, after weight \n");
			return DOOR_OPEN_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == GUARD_RIGHT_UNLOCK)
		{
			display->outputMessage = RIGHT_DOOR_UNLOCK;
			display->person.curState = RIGHT_UNLOCK_STATE;
			printf("leave right unlock test\n");
			return DOOR_OPEN_HANDLER;
		}
		else if (display->person.eventInput == GUARD_LEFT_UNLOCK)
		{
			display->outputMessage = LEFT_DOOR_UNLOCK;
			display->person.curState = LEFT_UNLOCK_STATE;
			printf("outbound left unlock test\n");
			return DOOR_OPEN_HANDLER;
		}
	}
	printf("stay the same function DOOR_UNLOCK_HANDLER\n");
	return DOOR_UNLOCK_HANDLER;
}

void *DOOR_OPEN_HANDLER(Display *display)
{
	if(display->person.doorDirection == INBOUND)  //entering
	{
		if(display->person.eventInput == LEFT_OPEN)
		{
			display->person.curState = LEFT_OPEN_STATE;
			display->outputMessage = LEFT_DOOR_OPEN;
			printf("inbound left door open test \n");
			return WEIGHT_HANDLER;
		}
		else if(display->person.eventInput == RIGHT_OPEN)
		{
			display->person.curState = RIGHT_OPEN_STATE;
			display->outputMessage = RIGHT_DOOR_OPEN;
			printf("inbound right door open test \n");
			return DOOR_CLOSE_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == RIGHT_OPEN)
		{
			display->person.curState = RIGHT_OPEN_STATE;
			display->outputMessage = RIGHT_DOOR_OPEN;
			printf("outbound right door open test \n");
			return WEIGHT_HANDLER;
		}
		if(display->person.eventInput == LEFT_OPEN)
		{
			display->person.curState = LEFT_OPEN_STATE;
			display->outputMessage = LEFT_DOOR_OPEN;
			printf("outbound left door open test \n");
			return DOOR_CLOSE_HANDLER;
		}
	}

	printf("stay the same function DOOR_OPEN_HANDLER\n");
	return DOOR_OPEN_HANDLER;
}


void *WEIGHT_HANDLER(Display *display)
{
	printf("weight function \n");

	if(display->person.eventInput == WEIGHT_SCALE)
	{
		display->person.curState = WEIGHT_STATE;
		return DOOR_CLOSE_HANDLER;
	}

	printf("stay the same function WEIGHT_HANDLER\n");
	return WEIGHT_HANDLER;

}

void *DOOR_CLOSE_HANDLER(Display *display)
{

	if(display->person.doorDirection == INBOUND)  //entering
	{
		if(display->person.eventInput == LEFT_CLOSE)
		{
			display->outputMessage = LEFT_DOOR_CLOSED;
			display->person.curState = LEFT_CLOSE_STATE;
			printf("inbound left door close test \n");
			return DOOR_LOCK_HANDLER;
		}
		else if(display->person.eventInput == RIGHT_CLOSE)
		{
			printf("inbound right door close test\n");
			display->outputMessage = RIGHT_DOOR_CLOSED;
			display->person.curState = RIGHT_CLOSE_STATE;
			return DOOR_LOCK_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == RIGHT_CLOSE)
		{
			printf("right door close test\n");
			display->outputMessage = RIGHT_DOOR_CLOSED;
			display->person.curState = RIGHT_CLOSE_STATE;
			return DOOR_LOCK_HANDLER;
		}
		if(display->person.eventInput == LEFT_CLOSE)
		{
			display->outputMessage = LEFT_DOOR_CLOSED;
			display->person.curState = LEFT_CLOSE_STATE;
			printf("outbound left door close test \n");
			return DOOR_LOCK_HANDLER;
		}
	}

	printf("stay the same function DOOR_CLOSE_HANDLER\n");
	return DOOR_CLOSE_HANDLER;
}

void *DOOR_LOCK_HANDLER(Display *display)
{
	if(display->person.doorDirection == INBOUND)//entering
	{
		if(display->person.eventInput == GUARD_LEFT_LOCK)
		{
			printf("inbound gll \n");
			display->outputMessage = LEFT_DOOR_LOCKED;
			display->person.curState = LEFT_LOCK_STATE;
			return DOOR_UNLOCK_HANDLER;
		}
		else if(display->person.eventInput == GUARD_RIGHT_LOCK)
		{
			printf("inbound grl \n");
			display->outputMessage = RIGHT_DOOR_LOCKED;
			display->person.curState = RIGHT_LOCK_STATE;
			return EXIT_HANDLER;
		}
	}
	else if(display->person.doorDirection == OUTBOUND)  //leaving
	{
		if(display->person.eventInput == GUARD_RIGHT_LOCK)
		{
			printf("outbound gru \n");
			display->outputMessage = RIGHT_DOOR_LOCKED;
			display->person.curState = RIGHT_LOCK_STATE;
			return DOOR_UNLOCK_HANDLER;
		}
		else if(display->person.eventInput == GUARD_LEFT_LOCK)
		{
			printf("outbound gll \n");
			display->outputMessage = LEFT_DOOR_LOCKED;
			display->person.curState = LEFT_LOCK_STATE;
			return EXIT_HANDLER;
		}
	}


	return DOOR_LOCK_HANDLER;
}


void *EXIT_HANDLER(Display *display)
{
	display->person.curState = EXIT_STATE;
	return EXIT_HANDLER;
}
