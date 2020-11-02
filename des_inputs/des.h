#ifndef DES_H_
#define DES_H_

/****************************************************************************************
 * States of the finite state machine (FSM)
 * each enum value represent a state
 ****************************************************************************************/
#define NUM_STATES 12	// define the number of states in FSM
typedef enum {
	START_STATE,
	LEFT_STATE,
	LEFT_UNLOCK_STATE,
	LEFT_OPEN_STATE,
	LEFT_CLOSE_STATE,
	LEFT_LOCK_STATE,
	WEIGHT_STATE,
	RIGHT_STATE,
	RIGHT_UNLOCK_STATE,
	RIGHT_OPEN_STATE,
	RIGHT_CLOSE_STATE,
	RIGHT_LOCK_STATE,
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



const char *inMessage[NUM_INPUTS] = {
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
#define NUM_OUTPUTS 10	// number of output messages from the FSM.
typedef enum {// assign an enum value, one for each output message from the FSM
	SCAN_ID,
	LEFT_DOOR_UNLOCK,
	LEFT_DOOR_OPEN,
	LEFT_DOOR_CLOSED,
	LEFT_DOOR_LOCKED,
	RIGHT_DOOR_UNLOCK,
	RIGHT_DOOR_OPEN,
	RIGHT_DOOR_CLOSED,
	RIGHT_DOOR_LOCKED,
} Output;

const char *outMessage[NUM_OUTPUTS] = {	// each output message. For example, "Person opened left door"
	"Person scanned ID. ID = ",
	"Left door unblocked by Guard\n",
	"Person opened left door\n",
	"Left door closed (automatically)\n",
	"Left door locked by Guard\n",
	"Right door unblocked by Guard\n",
	"Person opened right door\n",
	"Right door closed (automatically)\n",
	"Right door locked by Guard\n"
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
	Input eventInput;	// enum
	int curState;
} Person;

/****************************************************************************************
 * controller client sends a Display struct to its server, the display
 * fields for the output message and the Person. For the output message, I used an int, which
 * is the index into the outMessages array. That way I don't pass strings in my messages.
 * The Person field is needed, as some output message require information from the Person.
 * Specifically, those messages that display the Person's ID and weight.
 ****************************************************************************************/
typedef struct {
	Output outputMessage;	// output message enum
	Person person;			// struct
} Display;


/****************************************************************************************
 * functions pointer
 ****************************************************************************************/
//States
typedef void*(*StateFunc)();
void *START_HANDLER(Display *display);
void *ID_SCAN_HANDLER(Display *display);
void *DOOR_UNLOCK_HANDLER(Display *display);
void *DOOR_OPEN_HANDLER(Display *display);
void *WEIGHT_HANDLER(Display *display);
void *DOOR_CLOSE_HANDLER(Display *display);
void *DOOR_LOCK_HANDLER(Display *display);
void *EXIT_HANDLER(Display *display);


/****************************************************************************************
 * boolean to evaluate if the user command to the input process is valid or invalid.
 * Loop back for another input if the user enters an invalid command
 ****************************************************************************************/
#define VALID_INPUT 0
#define INVALID_INPUT 1

#endif
