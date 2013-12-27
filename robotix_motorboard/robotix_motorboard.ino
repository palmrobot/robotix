/********************************************************/
/*      Pin  definitions                                */
/********************************************************/

#define MOT_L_SPEED	5
#define MOT_L_DIR	12
#define MOT_L_BRAKE	9

#define MOT_R_SPEED	11
#define MOT_R_DIR	13
#define MOT_R_BRAKE	8


/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_TEST			0x40 /* [0x40 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define COMMAND_COUNTERS		0x41 /* [0x41 Counters] [Left in cm] [right in cm] */
#define COMMAND_READY			0x42 /* [0x42 Ready] */
#define COMMAND_STOP			0x43
#define COMMAND_START			0xFD /* [0xFD Start transmission] */
#define COMMAND_END			0xFE /* [0xFE End of transmission] */
#define COMMAND_ACK			0xFF /* [0xFF Ack of transmission] */

#define CMD_DATA_MAX			6
uint8_t g_send_mother[CMD_DATA_MAX];
uint8_t g_recv_mother[CMD_DATA_MAX];

/********************************************************/
/*      Process definitions                             */
/********************************************************/
#define PROCESS_RECEIVE_DO_NOTHING	 0
#define PROCESS_RECEIVE_WAIT_COMMAND	 1
unsigned char g_process_receive;

#define PROCESS_ACTION_INIT		 0x1
#define PROCESS_ACTION_DISTANCE		 0x2
#define PROCESS_ACTION_TEST		 0x3
unsigned char g_process_action;

#define PROCESS_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define PROCESS_COMMAND_FORWARD		0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_COMMAND_GET_COUNTERS	0x07 /* [0x07 Get counters] */
#define PROCESS_COMMAND_START		0xFD /* Start transmission */
#define PROCESS_COMMAND_END		0xFE /* End of transmission */
#define PROCESS_COMMAND_ACK		0xFF /* Ack of transmission */



unsigned char g_process_command;

/********************************************************/
/*      Global definitions                              */
/********************************************************/

#define CONVERT_CENTIMETERS_TO_TICS	40
#define CONVERT_DEGREES_TO_TICS		5
#define CONVERT_DEGREES_COEF		2

uint8_t g_time_count	  = 0;
unsigned int g_motor_left_count   = 0;
unsigned int g_motor_right_count  = 0;

uint8_t g_recv_mother_nb	= 0;

unsigned int g_distance_left		= 0;
unsigned int g_distance_right		= 0;
unsigned int g_distance			= 0;
unsigned int g_distance_remaining	= 0;

unsigned int g_mode		= 0;


void interrupt_left_call(void)
{
    g_motor_left_count++;
    g_distance = g_motor_right_count;
}

void interrupt_right_call(void)
{
    g_motor_right_count++;
}

void setup()
{
    /* Init input/output left motor */
    pinMode(MOT_L_SPEED, OUTPUT);
    pinMode(MOT_L_DIR, OUTPUT);
    pinMode(MOT_L_BRAKE, OUTPUT);

    analogWrite(MOT_L_SPEED, 0);
    digitalWrite(MOT_L_DIR, HIGH);
    digitalWrite(MOT_L_BRAKE, LOW);

    /* Init input/output right motor */
    pinMode(MOT_R_SPEED, OUTPUT);
    pinMode(MOT_R_DIR, OUTPUT);
    pinMode(MOT_R_BRAKE, OUTPUT);

    analogWrite(MOT_R_SPEED, 0);
    digitalWrite(MOT_R_DIR, HIGH);
    digitalWrite(MOT_R_BRAKE, LOW);

    /* init process states */
    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;
    g_process_command   = 0;
    g_process_action    = PROCESS_ACTION_INIT;

    /* Init global variables */
    g_time_count	= 0;
    g_motor_left_count  = 0;
    g_motor_right_count = 0;

    /* init pipes */
    g_recv_mother[0]	= 0;
    g_recv_mother_nb	= 0;

    g_send_mother[0]	= 0;

    /* Init interrupt for the left Motor */
    attachInterrupt(0, interrupt_left_call, FALLING);

    /* Init interrupt for the right Motor */
    attachInterrupt(1, interrupt_right_call, FALLING);

    delay(1000);

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);

}

/* Mother <- Motor */
/*  Start <- 0xFD  */
/*  0xFF  -> Ack   */
/*  DATA  <- 0xZZ  */
/*  DATA  <- 0xZZ  */
/*  End   <- 0xFE  */
/*  0xFF  -> Ack   */
void send_mother(uint8_t *buffer, int len)
{
    uint8_t padding[4] = {0,0,0,0};

    if (len > 4)
	len = 4;

    /* Send Start of transmission */
    Serial.write(COMMAND_START);

    /* Write Command + Data */
    Serial.write(buffer, len);

    /* Write padding Data */
    Serial.write(padding, 4 - len);
}


/* Mother -> Motor */
/*  0xFD  -> Start */
/*  Ack   <- 0xFF  */
/*  0xXX  -> DATA  */
/*  0xXX  -> DATA  */
/*  0xFE  -> End   */
/*  Ack   <- 0xFF  */
void process_receive(void)
{
    uint8_t value;

    if (g_process_receive)
    {
	/* if we get a valid char, read char */
	if (Serial.available() > 0)
	{
	    /* get Start Byte */
	    value = Serial.read();
	    if (value == COMMAND_START)
	    {
		/* Wait for serial */
		while (Serial.available() < 4);

		for(g_recv_mother_nb = 0; g_recv_mother_nb < 4; g_recv_mother_nb++)
		{
		    /* get incoming write: */
		    g_recv_mother[g_recv_mother_nb] = Serial.read();
		}
		/* Set action plan */
		g_process_command = g_recv_mother[0];

		/* Disable communication ,wait for message treatment */
		g_process_receive   = 0;
	    }
	}
    }
}

void process_command(void)
{
    unsigned char speed;
    unsigned char i;

    if (g_process_command)
    {
	if (g_process_command == PROCESS_COMMAND_STOP)
	{
	    /* Brake immediatly */
	    digitalWrite(MOT_L_BRAKE, HIGH);
	    digitalWrite(MOT_R_BRAKE, HIGH);

	    /* Set speed to 0 */
	    analogWrite(MOT_L_SPEED,0);
	    analogWrite(MOT_R_SPEED,0);

	    /* unset the brake */
	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = 0;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_FORWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = g_recv_mother[2] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = PROCESS_ACTION_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_BACKWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = (g_recv_mother[2] * CONVERT_CENTIMETERS_TO_TICS);

	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = PROCESS_ACTION_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_LEFT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = (g_recv_mother[2] * CONVERT_DEGREES_COEF ) / CONVERT_DEGREES_TO_TICS;

	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = PROCESS_ACTION_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_RIGHT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = (g_recv_mother[2] * CONVERT_DEGREES_COEF ) / CONVERT_DEGREES_TO_TICS;
	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action  = PROCESS_ACTION_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS)
	{
	    g_send_mother[0] = COMMAND_COUNTERS;
	    g_send_mother[1] = (uint8_t)(g_motor_left_count / CONVERT_CENTIMETERS_TO_TICS);
	    g_send_mother[2] = (uint8_t)(g_motor_right_count / CONVERT_CENTIMETERS_TO_TICS);
	    send_mother(g_send_mother, 3);

	    g_process_receive  = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_command  = 0;
	}
    }
}

void process_action(void)
{
    if (g_process_action)
    {
	if (g_process_action == PROCESS_ACTION_INIT)
	{
	    g_send_mother[0] = COMMAND_READY;
	    send_mother(g_send_mother, 1);

	    g_process_action = 0;
	}
	else if (g_process_action == PROCESS_ACTION_DISTANCE)
	{
	    /* check if distance has been reached */
	    if (g_distance >= g_distance_remaining)
	    {
		/* Brake immediatly */
		digitalWrite(MOT_L_BRAKE, HIGH);
		digitalWrite(MOT_R_BRAKE, HIGH);

		/* Set speed to 0 */
		analogWrite(MOT_L_SPEED,0);
		analogWrite(MOT_R_SPEED,0);

		g_send_mother[0] = COMMAND_STOP;
		send_mother(g_send_mother, 1);

		/* last reset current state */
		g_process_action = 0;
	    }
	}
    }
}

void loop()
{
    process_receive();
    process_command();
    process_action();

    g_time_count++;
}


