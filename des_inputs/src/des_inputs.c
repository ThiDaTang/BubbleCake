#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <limits.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "../des.h"

int main(int argc, char* argv[])
{
	int coid;
	char userInput[5];
	pid_t controllerID;
	int response;		// response from controller.c
	Person person;
	int inputStatus = VALID_INPUT;

	/* Validate the command-line argument */
	if(argc != 2)
	{
		printf("Input: Argument missing \n ");
		exit(EXIT_FAILURE);
	}


	/* PHASE I: Setting up connection*/
	// Get the controller's PID from the command-line
	controllerID = atoi(argv[1]);

	/* establish a connection */
	coid = ConnectAttach (ND_LOCAL_NODE, controllerID, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror ("Input: Couldn't Connect Attach \n");
		exit (EXIT_FAILURE);
	}

	while(1)
	{
		printf("Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, \n"
				"ro = right open, lc = left closed, rc = right closed , gru = guard right unlock, \n"
				"grl = guard right lock, gll = guard left lock, glu = guard left unlock) \n");
		scanf("%s", userInput);

		if (strcasecmp(userInput, inMessage[LEFT_SCAN]) == 0)	// user input: ls
		{
			printf("Enter the person_id: \n");
			scanf("%d", &(person.id));
			person.doorDirection = INBOUND;
			person.eventInput = LEFT_SCAN;
		}
		else if (strcasecmp(userInput, inMessage[RIGHT_SCAN]) == 0)	// user input: rs
		{
			printf("Enter the person_id: \n");
			scanf("%d", &(person.id));
			person.doorDirection = OUTBOUND;
			person.eventInput = RIGHT_SCAN;
		}
		else if (strcasecmp(userInput, inMessage[WEIGHT_SCALE]) == 0) // user input: ws
		{
			printf("Enter the weight:\n");
			scanf("%d", &(person.weight));
			person.eventInput = WEIGHT_SCALE;
		}
		else if (strcasecmp(userInput, inMessage[LEFT_OPEN]) == 0) // user input: lo
		{
			person.eventInput = LEFT_OPEN;
		}
		else if (strcasecmp(userInput, inMessage[RIGHT_OPEN]) == 0) //user input: ro
		{
			person.eventInput = RIGHT_OPEN;
		}
		else if (strcasecmp(userInput, inMessage[LEFT_CLOSE]) == 0) // //user input: lc
		{
			person.eventInput = LEFT_CLOSE;
		}
		else if (strcasecmp(userInput, inMessage[RIGHT_CLOSE]) == 0) // user input: rc
		{
			person.eventInput = RIGHT_CLOSE;
		}
		else if (strcasecmp(userInput, inMessage[GUARD_RIGHT_UNLOCK]) == 0) // user input: gru
		{
			person.eventInput = GUARD_RIGHT_UNLOCK;
		}
		else if (strcasecmp(userInput, inMessage[GUARD_RIGHT_LOCK]) == 0) // user input: grl
		{
			person.eventInput = GUARD_RIGHT_LOCK;
		}
		else if (strcasecmp(userInput, inMessage[GUARD_LEFT_LOCK]) == 0) // user input: gll
		{
			person.eventInput = GUARD_LEFT_LOCK;
		}
		else if (strcasecmp(userInput, inMessage[GUARD_LEFT_UNLOCK]) == 0) // user input: glu
		{
			person.eventInput = GUARD_LEFT_UNLOCK;
		}
		else if (strcasecmp(userInput, inMessage[EXIT]) == 0) // user input: exit
		{
			person.eventInput = EXIT;
		}
		else
		{
			printf("Invalid input\n");
			inputStatus = INVALID_INPUT;
		}

		/* PHASE II: Message passing */
		if (inputStatus == VALID_INPUT)
		{
			if (MsgSend(coid, &person, sizeof(person), &response, sizeof(response)) == -1L) {
				printf("Input: MsgSend had an error.\n");
				exit(EXIT_FAILURE);
			}
		}

		if (strcasecmp(userInput, inMessage[EXIT]) == 0) // user input: exit
		{
			break;
		}
	}

	/* PHASE III: Disconnect */
	ConnectDetach(coid);

	return EXIT_SUCCESS;
}
