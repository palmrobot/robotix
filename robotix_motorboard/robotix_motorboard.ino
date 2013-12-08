
/********************************************************/
/*      Motors definitions                              */
/********************************************************/

#define MOT_L_SPEED	PIN3
#define MOT_L_DIR	PIN12
#define MOT_L_BRAKE	PIN9

#define MOT_R_SPEED	PIN11
#define MOT_R_DIR	PIN13
#define MOT_R_BRAKE	PIN8

/********************************************************/
/*      Process definitions                             */
/********************************************************/
#define PROCESS_RECEIVE_DO_NOTHING	 0
#define PROCESS_RECEIVE_WAIT_COMMAND	 1
#define PROCESS_RECEIVE_WAIT_DATA_1	 2
#define PROCESS_RECEIVE_WAIT_DATA_N	 3
#define PROCESS_RECEIVE_WAIT_DATA_VALUE  4

#define PROCESS_HEART_DISTANCE		 0x1

/* 0x01 : Stop */
/* 0x02 0xC8 0x0A : Move Forward speed 200 10cm */
/* 0x03 0xC8 0x00 : Move Forward speed 200 until obstacle */
/* 0x03 0xC8 0x10 : Move Backward speed 200 16cm */
/* 0x04 0xC8 0xB4 : Rotate left speed 200 180 degrees */
/* 0x05 0xC8 0x5A : Rotate right speed 200 90 degrees */
/* 0x06 0x03 0x01 0x02 0x03 : Test with 3 bytes */

#define PROCESS_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define PROCESS_COMMAND_FORWARD		0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define PROCESS_COMMAND_GET_COUNTERS	0x07 /* [0x07 Get counters] */

#define PROCESS_COMMAND_SEND_COUNTERS	0x88 /* [0x88 Send counters] [Left in cm] [right in cm] */


#define MAX_READ_DATA			6


#define CONVERT_CENTIMETERS_TO_TICS	78
#define CONVERT_DEGREES_TO_TICS		5

unsigned int g_time_count	  = 0;
unsigned int g_motor_left_count   = 0;
unsigned int g_motor_right_count  = 0;


unsigned char g_process_receive	        = 0;
unsigned char g_process_receive_nb      = 0;
unsigned char g_process_receive_max     = 0;
unsigned char g_receive			= 0;
unsigned char g_data[MAX_READ_DATA];

unsigned char g_process_command		= 0;
unsigned char g_process_heart		= 0;
unsigned char g_process_send		= 0;

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

    /* Init global variables */
    g_time_count	= 0;
    g_motor_left_count  = 0;
    g_motor_right_count = 0;

    g_receive		= PROCESS_COMMAND_NOTHING;
    g_data		= 0;

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
		g_data[0] = 0;
		g_process_receive_nb  = 0;
		g_process_receive_max = 0;

		if ((g_receive == PROCESS_COMMAND_FORWARD) ||
		    (g_receive == PROCESS_COMMAND_BACKWARD) ||
		    (g_receive == PROCESS_COMMAND_ROTATE_LEFT) ||
		    (g_receive == PROCESS_COMMAND_ROTATE_RIGHT))
		{
		    g_process_receive_nb  = 0;
		    g_process_receive_max = 2;

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
		    g_process_receive_max = 0;

		    /* Do not reschedule read UART */
		    g_process_receive   = 0;
		    g_process_command	= g_data[0];
		}
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_VALUE)
	    {
		g_process_receive_nb  = 0;
		g_process_receive_max = value;
		if (g_process_receive_max > MAX_READ_DATA)
		    g_process_receive_max = MAX_READ_DATA;

		/* Do not reschedule read UART */
		g_process_receive = PROCESS_RECEIVE_WAIT_DATA_N;
	    }
	    else if (g_process_receive == PROCESS_RECEIVE_WAIT_DATA_N)
	    {
		g_data[g_process_receive_nb] = value;
		g_process_receive_nb ++;

		if (g_process_receive_nb == g_process_receive_max)
		{
		    /* Do not reschedule read UART */
		    g_process_receive = 0;
		    g_process_command	= g_data[0];
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

	    speed		 = g_data[0];
	    g_distance_remaining = g_data[1] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_BACKWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_data[0];
	    g_distance_remaining = g_data[1] * CONVERT_CENTIMETERS_TO_TICS;

	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_LEFT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_data[0];
	    g_distance_remaining = g_data[1] * CONVERT_DEGREES_TO_TICS;

	    digitalWrite(MOT_L_DIR, LOW);
	    digitalWrite(MOT_R_DIR, HIGH);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_RIGHT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_data[0];
	    g_distance_remaining = g_data[1] * CONVERT_DEGREES_TO_TICS;

	    digitalWrite(MOT_L_DIR, HIGH);
	    digitalWrite(MOT_R_DIR, LOW);

	    analogWrite(MOT_L_SPEED, speed);
	    analogWrite(MOT_R_SPEED, speed);

	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	    g_process_heart   = PROCESS_DISTANCE;

	    /* last reset current state */
	    g_process_command = 0;
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS)
	{
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

