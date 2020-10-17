#ifndef DES_H_
#define DES_H_
/****************************************************************************************
 * States of the finite state machine (FSM)
 * each enum value represent a state
 ****************************************************************************************/
#define NUM_STATES 8	// define the number of states in FSM
typedef enum {
	START_STATE,
	LEFT_SCAN_STATE,
	LEFT_UNLOCK_STATE,
	LEFT_OPEN_STATE,
	RIGHT_SCAN_STATE,
	RIGHT_UNLOCK_STATE,
	RIGHT_OPEN_STATE,
	EXIT_STATE
} State;

/****************************************************************************************
 * Input commands
 ****************************************************************************************/
#define NUM_INPUTS 12	// the number of input commands that drive the FSM.
typedef enum {
	LEFT_SCAN,
	RIGHT_SCAN,
	WEIGHT_SCALE,
	LEFT_OPEN,
	RIGHT_OPEN,
	LEFT_CLOSE,
	RIGHT_CLOSE,
	GUARD_RIGHT_UNLOCK,
	GUARD_RIGHT_LOCK,
	GUARD_LEFT_LOCK,
	GUARD_LEFT_UNLOCK,
	EXIT
} Input;

const char *inMessage[NUM_INPUTS] = { //TODO :: each input command. For example, "ls"
	"LS",	// left scan
	"RS",	// right scan
	"WS",	// weight scale
	"LO",	// left open
	"RO",	// right open
	"LC",	// left close
	"RC",	// right close
	"GRU",	// guard right unlock
	"GRL",	// guard right lock
	"GLL",	// guard left lock
	"GLU",	// guard right lock
	"EXIT"
};

/****************************************************************************************
 * Output messages from the FSM
 ****************************************************************************************/
#define NUM_OUTPUTS 12	// number of output messages from the FSM.
typedef enum {// assign an enum value, one for each output message from the FSM
	WAIT,
	ENTER_ID,
	LEFT_DOOR_UNLOCK,
	LEFT_DOOR_OPEN,
	ENTER_WEIGHT,
	LEFT_DOOR_CLOSED,
	LEFT_DOOR_LOCKED,
	RIGHT_DOOR_UNLOCK,
	RIGHT_DOOR_OPEN,
	RIGHT_DOOR_CLOSED,
	RIGHT_DOOR_LOCKED,
	START_MSG
} Output;

const char *outMessage[NUM_OUTPUTS] = {	// each output message. For example, "Person opened left door"
	"Waiting for Person...\n",
	"Enter the Person's ID:\n",
	"Left door unblocked by Guard\n",
	"Person opened left door\n",
	"Enter the Person's weight:\n",
	"Left door closed (automatically)\n",
	"Left door locked by Guard\n",
	"Right door unblocked by Guard\n",
	"Person opened right door\n",
	"Right door closed (automatically)\n",
	"Right door locked by Guard\n",
	"Enter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard right lock, gll=guard left lock, glu = guard left unlock) \n"
};

/****************************************************************************************
 * inputs client sends a Person struct to its server, the controller
 * fields for person's ID (int), weight (int), direction (inbound or outbound),
 * and some way to remember what state the Person is in. Suppose the Person in "Left Scan" state.
 * You need a way to represent that.
 ****************************************************************************************/
#define INBOUND 0
#define OUTBOUND 1

typedef struct {
	int id;
	int weight;
	int doorDirection;
	State personState;	// enum
} Person;

/****************************************************************************************
 * controller client sends a Display struct to its server, the display
 * fields for the output message and the Person. For the output message, I used an int, which
 * is the index into the outMessages array. That way I don't pass strings in my messages.
 * The Person field is needed, as some output message require information from the Person.
 * Specifically, those messages that display the Person's ID and weight.
 ****************************************************************************************/
typedef struct {
	Output outputMessage;	// enum
	Person person;			// struct
} Display;

#endif /* DES_H_ */
