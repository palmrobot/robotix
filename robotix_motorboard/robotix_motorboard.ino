/********************************************************/
/*      Pin  definitions                                */
/********************************************************/

#define MOT_L_SPEED	PIN3
#define MOT_L_DIR	PIN12
#define MOT_L_BRAKE	PIN9

#define MOT_R_SPEED	PIN11
#define MOT_R_DIR	PIN13
#define MOT_R_BRAKE	PIN8


/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_SEND_COUNTERS	0x88 /* [0x88 Send counters] [Left in cm] [right in cm] */

#define CMD_DATA_MAX			6
unsigned char g_send_mother[CMD_DATA_MAX];
unsigned char g_recv_mother[CMD_DATA_MAX];

/********************************************************/
/*      Process definitions                             */
/********************************************************/
unsigned char g_process_receive	        = 0;
#define PROCESS_RECEIVE_DO_NOTHING	 0
#define PROCESS_RECEIVE_WAIT_COMMAND	 1
#define PROCESS_RECEIVE_WAIT_DATA_1	 2
#define PROCESS_RECEIVE_WAIT_DATA_N	 3
#define PROCESS_RECEIVE_WAIT_DATA_VALUE  4


unsigned char g_process_heart;
#define PROCESS_HEART_DISTANCE		 0x1


#define PROCESS_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define PROCESS_COMMAND_FORWARD		0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define PROCESS_COMMAND_GET_COUNTERS	0x07 /* [0x07 Get counters] */
unsigned char g_process_command;


unsigned char g_process_send;


/********************************************************/
/*      Global definitions                              */
/********************************************************/

#define CONVERT_CENTIMETERS_TO_TICS	78
#define CONVERT_DEGREES_TO_TICS		5

unsigned int g_time_count	  = 0;
unsigned int g_motor_left_count   = 0;
unsigned int g_motor_right_count  = 0;

unsigned char g_receive_nb       = 0;
unsigned char g_receive_max      = 0;
unsigned char g_receive		 = 0;


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

    analogWrite(MOT_L_SPEED,0);
    digitalWrite(MOT_L_DIR, LOW);
    digitalWrite(MOT_L_BRAKE, LOW);

    /* Init input/output right motor */
    pinMode(MOT_R_SPEED, OUTPUT);
    pinMode(MOT_R_DIR, OUTPUT);
    pinMode(MOT_R_BRAKE, OUTPUT);

    analogWrite(MOT_R_SPEED,0);
    digitalWrite(MOT_R_DIR, LOW);
    digitalWrite(MOT_R_BRAKE, LOW);

    /* init process states */
    g_process_receive = 1;
    g_process_command = 0;
    g_process_heart   = 0;
    g_process_send	= 0;

    /* Init global variables */
    g_time_count	= 0;
    g_motor_left_count  = 0;
    g_motor_right_count = 0;

    g_receive		= PROCESS_COMMAND_NOTHING;

    /* init pipes */
    g_recv_mother[0]	= 0;
    g_send_mother[0]	= 0;

    /* Init interrupt for the left Motor */
    attachInterrupt(0, interrupt_left_call, FALLING);

    /* Init interrupt for the right Motor */
    attachInterrupt(1, interrupt_right_call, FALLING);

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);
}

void process_receive(void)
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
		g_receive = value;
		g_recv_mother[0] = 0;
		g_receive_nb  = 0;
		g_receive_max = 0;

		if ((g_receive == PROCESS_COMMAND_FORWARD) ||
		    (g_receive == PROCESS_COMMAND_BACKWARD) ||
		    (g_receive == PROCESS_COMMAND_ROTATE_LEFT) ||
		    (g_receive == PROCESS_COMMAND_ROTATE_RIGHT))
		{
		    g_receive_nb  = 0;
		    g_receive_max = 2;

		    /* reschedule read UART to get DATA */
		    g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
		}
		else if (g_receive == PROCESS_COMMAND_TEST)
		{
		    /* reschedule read UART to get the number of bytes to read */
		    g_process_receive = PROCESS_RECEIVE_WAIT_DATA_VALUE;
		}
		else
		{
		    g_receive_max = 0;

		    /* Do not reschedule read UART */
		    g_process_receive   = 0;
		    g_process_command	= g_recv_mother[0];
		}
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_VALUE)
	    {
		g_receive_nb  = 0;
		g_receive_max = value;
		if (g_receive_max > CMD_DATA_MAX)
		    g_receive_max = CMD_DATA_MAX;

		/* Do not reschedule read UART */
		g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_N)
	    {
		g_recv_mother[g_receive_nb] = value;
		g_receive_nb ++;

		if (g_receive_nb == g_receive_max)
		{
		    /* Do not reschedule read UART */
		    g_process_receive = 0;
		    g_process_command	= g_recv_mother[0];
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
	    g_process_heart   = 0;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_FORWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[0];
	    g_distance_remaining = g_recv_mother[1] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_HEART_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_BACKWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[0];
	    g_distance_remaining = g_recv_mother[1] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_HEART_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_LEFT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[0];
	    g_distance_remaining = g_recv_mother[1] * CONVERT_DEGREES_TO_TICS;

	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_HEART_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_RIGHT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[0];
	    g_distance_remaining = g_recv_mother[1] * CONVERT_DEGREES_TO_TICS;

	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_HEART_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS)
	{
	    g_send_mother[0] = PROCESS_COMMAND_GET_COUNTERS;
	    g_send_mother[1] = (g_motor_left_count / CONVERT_CENTIMETERS_TO_TICS);
	    g_send_mother[2] = (g_motor_right_count / CONVERT_CENTIMETERS_TO_TICS);
	    Serial.write(g_send_mother);

	    /* last reset current state */
	    g_process_command = 0;
	}
    }
}

void process_heart(void)
{
    unsigned char speed;

    if (g_process_heart)
    {
	if ((g_process_heart && PROCESS_HEART_DISTANCE) == PROCESS_HEART_DISTANCE)
	{
	    /* check if distance has been reached */
	    if (g_distance >= g_distance_remaining)
	    {
		g_process_command = PROCESS_COMMAND_STOP;

		/* last reset current state */
		g_process_heart   = 0;
	    }
	}
    }
}

void process_send(void)
{
    unsigned char speed;

    if (g_process_send)
    {
	if (g_process_send == CMD_SEND_COUNTERS)
	{
	    /* last reset current state */
	    g_process_send   = 0;
	}
    }
}


void loop()
{
    process_receive();
    process_command();
    process_heart();
    process_send();

    g_time_count++;
}


