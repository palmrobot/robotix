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
#define COMMAND_TEST			0x40 /* [0x40 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_COUNTERS		0x41 /* [0x41 Counters] [Left in cm] [right in cm] */
#define COMMAND_READY			0x42 /* [0x42 Ready] */


#define CMD_DATA_MAX			6
uint8_t g_send_mother[CMD_DATA_MAX];
uint8_t g_recv_mother[CMD_DATA_MAX];

/********************************************************/
/*      Process definitions                             */
/********************************************************/
#define PROCESS_RECEIVE_DO_NOTHING	 0
#define PROCESS_RECEIVE_WAIT_COMMAND	 1
#define PROCESS_RECEIVE_WAIT_DATA_1	 2
#define PROCESS_RECEIVE_WAIT_DATA_N	 3
#define PROCESS_RECEIVE_WAIT_DATA_VALUE  4
unsigned char g_process_receive;

#define PROCESS_ACTION_INIT		 0x1
#define PROCESS_ACTION_DISTANCE		 0x2
unsigned char g_process_action;

#define PROCESS_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define PROCESS_COMMAND_FORWARD		0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define PROCESS_COMMAND_GET_COUNTERS	0x07 /* [0x07 Get counters] */
unsigned char g_process_command;

/********************************************************/
/*      Global definitions                              */
/********************************************************/

#define CONVERT_CENTIMETERS_TO_TICS	78
#define CONVERT_DEGREES_TO_TICS		5

unsigned int g_time_count	  = 0;
unsigned int g_motor_left_count   = 12;
unsigned int g_motor_right_count  = 13;

unsigned char g_recv_mother_nb	= 0;
unsigned char g_recv_mother_max	= 0;

int g_distance_left		= 0;
int g_distance_right		= 0;
int g_distance			= 0;
int g_distance_remaining	= 0;

unsigned int g_mode		= 0;


void interrupt_left_call(void)
{
    g_motor_left_count++;
}

void interrupt_right_call(void)
{
    g_motor_right_count++;
    g_distance = g_motor_right_count;
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
    g_recv_mother_max	= 0;

    g_send_mother[0]	= 0;

    /* Init interrupt for the left Motor */
    attachInterrupt(0, interrupt_left_call, FALLING);

    /* Init interrupt for the right Motor */
    attachInterrupt(1, interrupt_right_call, FALLING);

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);

    delay(500);

}

void process_receive(void)
{
    char value;

    if (g_process_receive)
    {
	g_recv_mother_nb = 0;

	/* if we get a valid char, read char */
	while (g_recv_mother_nb < 3)
	{
	    if (Serial.available() > 0)
	    {
		/* get incoming byte: */
		g_recv_mother[g_recv_mother_nb] = Serial.read();
		g_recv_mother_nb++;
	    }
	}

	if (g_recv_mother_nb > 0)
	    g_process_receive = 0;
    }
}

void process_receive2(void)
{
    char value;

    if (g_process_receive)
    {
	/* if we get a valid char, read char */
	if (Serial.available() > 0)
	{
	    /* get incoming byte: */
	    value = Serial.read();

	    if (g_process_receive == PROCESS_RECEIVE_WAIT_COMMAND)
	    {
		g_recv_mother[0] = value;
		g_recv_mother_nb++;

		if ((g_recv_mother[0] == PROCESS_COMMAND_FORWARD) ||
		    (g_recv_mother[0] == PROCESS_COMMAND_BACKWARD) ||
		    (g_recv_mother[0] == PROCESS_COMMAND_ROTATE_LEFT) ||
		    (g_recv_mother[0] == PROCESS_COMMAND_ROTATE_RIGHT))
		{
		    g_recv_mother_max = 3;

		    /* reschedule read UART to get DATA */
		    g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
		}
		else if (g_recv_mother[0] == PROCESS_COMMAND_TEST)
		{
		    /* reschedule read UART to get the number of bytes to read */
		    g_process_receive = PROCESS_RECEIVE_WAIT_DATA_VALUE;
		}
		else
		{
		    g_recv_mother_max = 1;

		    /* Do not reschedule read UART */
		    g_process_receive   = 0;
		    g_process_command = g_recv_mother[0];
		}
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_VALUE)
	    {
		g_recv_mother_max = value;
		g_recv_mother_nb++;

		if (g_recv_mother_max > CMD_DATA_MAX)
		    g_recv_mother_max = CMD_DATA_MAX;

		/* Do not reschedule read UART */
		g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_N)
	    {
		g_recv_mother[g_recv_mother_nb] = value;
		g_recv_mother_nb ++;

		if (g_recv_mother_nb == g_recv_mother_max)
		{
		    /* Do not reschedule read UART */
		    g_process_receive = 0;
		    g_process_command = g_recv_mother[0];
		}
		else
		{
		    /* reschedule read UART to get next DATA */
		    g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
		}
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
	    /* g_motor_left_count  = 0; */

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = g_recv_mother[2] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_BRAKE, LOW);
	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_BRAKE, LOW);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = 0; /* PROCESS_ACTION_DISTANCE; */

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_BACKWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = g_recv_mother[2] * CONVERT_CENTIMETERS_TO_TICS;

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
	    g_distance_remaining = g_recv_mother[2] * CONVERT_DEGREES_TO_TICS;

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
	    g_distance_remaining = g_recv_mother[2] * CONVERT_DEGREES_TO_TICS;

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
	    g_send_mother[1] = g_time_count; /* (g_motor_left_count / CONVERT_CENTIMETERS_TO_TICS); */
	    g_send_mother[2] = g_time_count; /* (g_motor_right_count / CONVERT_CENTIMETERS_TO_TICS); */
	    Serial.write(g_send_mother, 3);

	    g_process_receive  = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_action   = 0;

	    /* last reset current state */
	    g_process_command = 0;
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

	    Serial.write(g_send_mother, 1);

	    /* check if distance has been reached */
	    g_process_action = 0;
	}
	else if (g_process_action == PROCESS_ACTION_DISTANCE)
	{
	    /* check if distance has been reached */
	    if (g_distance >= g_distance_remaining)
	    {
		g_process_command = PROCESS_COMMAND_STOP;

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


