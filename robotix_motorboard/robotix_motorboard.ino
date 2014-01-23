/********************************************************/
/*      Pin  definitions                                */
/********************************************************/

#define PIN_DETECTION_LEFT		A0
#define PIN_DETECTION_RIGHT		A1

#define PIN_DETECTION_LEFT_ENABLE	6
#define PIN_DETECTION_RIGHT_ENABLE	7

#define PIN_MOT_L_SPEED			5
#define PIN_MOT_L_DIR			12
#define PIN_MOT_L_BRAKE			9

#define PIN_MOT_R_SPEED			11
#define PIN_MOT_R_DIR			13
#define PIN_MOT_R_BRAKE			8

/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_TEST			0x40 /* [0x40 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define COMMAND_COUNTERS		0x41 /* [0x41 Counters] [Left [right] */
#define COMMAND_READY			0x42 /* [0x42 Ready] */
#define COMMAND_STOP			0x43
#define COMMAND_DETECTED		0x44
#define COMMAND_RUNNING_DETECTION	0x45
#define COMMAND_START			0xFE /* [0xFE Start transmission] */

#define CMD_DATA_MAX			6
uint8_t g_send_mother[CMD_DATA_MAX];
uint8_t g_recv_mother[CMD_DATA_MAX];

/********************************************************/
/*      Process definitions                             */
/********************************************************/
#define PROCESS_RECEIVE_DO_NOTHING		0
#define PROCESS_RECEIVE_WAIT_COMMAND		1
unsigned char g_process_receive;

#define PROCESS_ACTION_INIT			0x01
#define PROCESS_ACTION_DISTANCE			0x02
#define PROCESS_ACTION_DETECTION		0x04
unsigned char g_process_action;

#define PROCESS_COMMAND_STOP			0x01 /* [0x01 Stop] */
#define PROCESS_COMMAND_FORWARD			0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_BACKWARD		0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_COMMAND_ROTATE_LEFT		0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_COMMAND_ROTATE_RIGHT		0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_COMMAND_TEST			0x06 /* [0x06 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_COMMAND_GET_COUNTERS_CM		0x07 /* [0x07 Get counters in centimeters] */
#define PROCESS_COMMAND_GET_COUNTERS		0x08 /* [0x08 Get counters] */
#define PROCESS_COMMAND_GET_COUNTERS_DEG	0x09 /* [0x09 Get counters in degrees] */
#define PROCESS_COMMAND_DETECTION		0x0A /* [0x0A Get Detection */
#define PROCESS_COMMAND_START			0xFE /* [0xFE Start transmission */

unsigned char g_process_command;

/********************************************************/
/*      Global definitions                              */
/********************************************************/

#define CONVERT_CENTIMETERS_TO_TICS(a)	(a * 31)
#define CONVERT_DEGREES_TO_TICS(a)	(a / 10)

#define CONVERT_TICS_TO_CENTIMETERS(a)	(a / 31)
#define CONVERT_TICS_TO_DEGREES(a)	(a * 10)


uint8_t g_time_count;
unsigned int g_motor_left_count;
unsigned int g_motor_right_count;

uint8_t g_recv_mother_nb;

uint8_t g_distance_left;
uint8_t g_distance_right;
uint8_t g_distance;
uint8_t g_distance_remaining;
uint8_t g_detection_left;
uint8_t g_detection_right;
uint8_t g_detection_left_old;
uint8_t g_detection_right_old;

#define DIRECTION_STOP			0
#define DIRECTION_FORWARD		1
#define DIRECTION_BACKWARD		2
#define DIRECTION_ROTATE_LEFT		3
#define DIRECTION_ROTATE_RIGHT		4
uint8_t g_direction;

unsigned int g_mode;


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
    /* Initialize the sensors input pin */
    pinMode(PIN_DETECTION_LEFT, INPUT);
    pinMode(PIN_DETECTION_RIGHT, INPUT);
    pinMode(PIN_DETECTION_LEFT_ENABLE, OUTPUT);
    digitalWrite(PIN_DETECTION_LEFT_ENABLE, HIGH);
    pinMode(PIN_DETECTION_RIGHT_ENABLE, OUTPUT);
    digitalWrite(PIN_DETECTION_RIGHT_ENABLE, HIGH);

    /* Init input/output left motor */
    pinMode(PIN_MOT_L_SPEED, OUTPUT);
    pinMode(PIN_MOT_L_DIR, OUTPUT);
    pinMode(PIN_MOT_L_BRAKE, OUTPUT);

    analogWrite(PIN_MOT_L_SPEED, 0);
    digitalWrite(PIN_MOT_L_DIR, HIGH);
    digitalWrite(PIN_MOT_L_BRAKE, LOW);

    /* Init input/output right motor */
    pinMode(PIN_MOT_R_SPEED, OUTPUT);
    pinMode(PIN_MOT_R_DIR, OUTPUT);
    pinMode(PIN_MOT_R_BRAKE, OUTPUT);

    analogWrite(PIN_MOT_R_SPEED, 0);
    digitalWrite(PIN_MOT_R_DIR, HIGH);
    digitalWrite(PIN_MOT_R_BRAKE, LOW);

    /* init process states */
    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;
    g_process_command   = 0;
    g_process_action    = PROCESS_ACTION_INIT;

    /* Init global variables */
    g_time_count	= 0;
    g_motor_left_count  = 0;
    g_motor_right_count = 0;
    g_detection_left	= 0;
    g_detection_right	= 0;
    g_detection_left_old	= 2;
    g_detection_right_old	= 2;
    g_direction		= 0;


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

/* Motor -> Mother */
/*  Start ->       */
/*  Cmd   ->       */
/*  Data1 ->       */
/*  Data2 ->       */
/*  Data3 ->       */
/*  Data4 ->       */
void send_mother(uint8_t *buffer, int len)
{
    uint8_t padding[CMD_DATA_MAX] = {0,0,0,0,0,0};

    if (len > CMD_DATA_MAX)
	len = CMD_DATA_MAX;

    /* Send Start of transmission */
    Serial.write(COMMAND_START);

    /* Write Command + Data */
    Serial.write(buffer, len);

    /* Write padding Data */
    Serial.write(padding, CMD_DATA_MAX - len);
}

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
		while (Serial.available() < CMD_DATA_MAX);

		for(g_recv_mother_nb = 0; g_recv_mother_nb < CMD_DATA_MAX; g_recv_mother_nb++)
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

void move(uint8_t speed, uint8_t dir)
{
    digitalWrite(PIN_MOT_L_BRAKE, LOW);
    digitalWrite(PIN_MOT_R_BRAKE, LOW);

    if (dir == DIRECTION_FORWARD)
    {
	digitalWrite(PIN_MOT_R_DIR, LOW);
	digitalWrite(PIN_MOT_L_DIR, LOW);
    }
    else if (dir == DIRECTION_BACKWARD)
    {
	digitalWrite(PIN_MOT_L_DIR, HIGH);
	digitalWrite(PIN_MOT_R_DIR, HIGH);
    }
    else if (dir == DIRECTION_ROTATE_LEFT)
    {
	digitalWrite(PIN_MOT_L_DIR, LOW);
	digitalWrite(PIN_MOT_R_DIR, HIGH);
    }
    else
    {
	digitalWrite(PIN_MOT_L_DIR, HIGH);
	digitalWrite(PIN_MOT_R_DIR, LOW);
    }
    analogWrite(PIN_MOT_L_SPEED, speed);
    analogWrite(PIN_MOT_R_SPEED, speed);

    g_direction	= dir;
}


void stop(void)
{
    if (g_direction == DIRECTION_STOP)
	return;

    if (g_direction == DIRECTION_FORWARD)
    {
	move(100, DIRECTION_BACKWARD);
    }
    else if (g_direction == DIRECTION_BACKWARD)
    {
	move(100, DIRECTION_FORWARD);
    }
    else if (g_direction == DIRECTION_ROTATE_LEFT)
    {
	move(100, DIRECTION_ROTATE_RIGHT);
    }
    else
    {
	move(100, DIRECTION_ROTATE_LEFT);
    }
    delay(100);

    digitalWrite(PIN_MOT_L_BRAKE, HIGH);
    digitalWrite(PIN_MOT_R_BRAKE, HIGH);
    analogWrite(PIN_MOT_L_SPEED, 0);
    analogWrite(PIN_MOT_R_SPEED, 0);

    g_direction = DIRECTION_STOP;
}

void process_command(void)
{
    unsigned char speed;
    unsigned char i;

    if (g_process_command)
    {
	if (g_process_command == PROCESS_COMMAND_STOP)
	{
	    stop();
	    g_process_action  &= ~PROCESS_ACTION_DISTANCE;
	}
	else if (g_process_command == PROCESS_COMMAND_FORWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = CONVERT_CENTIMETERS_TO_TICS(g_recv_mother[2]);

	    move(speed, DIRECTION_FORWARD);

	    g_process_action  |= PROCESS_ACTION_DISTANCE;
	}
	else if (g_process_command == PROCESS_COMMAND_BACKWARD)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = CONVERT_CENTIMETERS_TO_TICS(g_recv_mother[2]);

	    move(speed, DIRECTION_BACKWARD);

	    g_process_action  |= PROCESS_ACTION_DISTANCE;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_LEFT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = CONVERT_DEGREES_TO_TICS(g_recv_mother[2]);
	    move(speed, DIRECTION_ROTATE_LEFT);

	    g_process_action  |= PROCESS_ACTION_DISTANCE;
	}
	else if (g_process_command == PROCESS_COMMAND_ROTATE_RIGHT)
	{
	    g_distance		= 0;
	    g_motor_right_count = 0;
	    g_motor_left_count  = 0;

	    speed		 = g_recv_mother[1];
	    g_distance_remaining = CONVERT_DEGREES_TO_TICS(g_recv_mother[2]);
	    move(speed, DIRECTION_ROTATE_RIGHT);

	    g_process_action  |= PROCESS_ACTION_DISTANCE;
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS_CM)
	{
	    g_send_mother[0] = COMMAND_COUNTERS;
	    g_send_mother[1] = (uint8_t)(CONVERT_TICS_TO_CENTIMETERS(g_motor_left_count) >> 8 );
	    g_send_mother[2] = (uint8_t)(CONVERT_TICS_TO_CENTIMETERS(g_motor_left_count) & 0xFF);
	    g_send_mother[3] = (uint8_t)(CONVERT_TICS_TO_CENTIMETERS(g_motor_right_count) >> 8);
	    g_send_mother[4] = (uint8_t)(CONVERT_TICS_TO_CENTIMETERS(g_motor_right_count) & 0xFF);
	    send_mother(g_send_mother, 5);
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS_DEG)
	{
	    g_send_mother[0] = COMMAND_COUNTERS;
	    g_send_mother[1] = (uint8_t)(CONVERT_TICS_TO_DEGREES(g_motor_left_count) >> 8 );
	    g_send_mother[2] = (uint8_t)(CONVERT_TICS_TO_DEGREES(g_motor_left_count) & 0xFF);
	    g_send_mother[3] = (uint8_t)(CONVERT_TICS_TO_DEGREES(g_motor_right_count) >> 8);
	    g_send_mother[4] = (uint8_t)(CONVERT_TICS_TO_DEGREES(g_motor_right_count) & 0xFF);
	    send_mother(g_send_mother, 5);
	}
	else if (g_process_command == PROCESS_COMMAND_GET_COUNTERS)
	{
	    g_send_mother[0] = COMMAND_COUNTERS;
	    g_send_mother[1] = (uint8_t)(g_motor_left_count >> 8);
	    g_send_mother[2] = (uint8_t)(g_motor_left_count & 0xFF);
	    g_send_mother[3] = (uint8_t)(g_motor_right_count >> 8);
	    g_send_mother[4] = (uint8_t)(g_motor_right_count & 0xFF);
	    send_mother(g_send_mother, 5);
	}
	else if (g_process_command == PROCESS_COMMAND_DETECTION)
	{
	    if ((g_process_action & PROCESS_ACTION_DETECTION) == PROCESS_ACTION_DETECTION)
	    {
		g_process_action &= ~PROCESS_ACTION_DETECTION;
	    }
	    else
	    {
		g_send_mother[0] = COMMAND_DETECTED;
		g_send_mother[1] = analogRead(PIN_DETECTION_LEFT);
		g_send_mother[2] = analogRead(PIN_DETECTION_RIGHT);
		send_mother(g_send_mother, 3);

		g_process_action  |= PROCESS_ACTION_DETECTION;
	    }
	}

	g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;
	g_process_command = 0;
    }
}

void process_action(void)
{
    if (g_process_action)
    {
	if ((g_process_action & PROCESS_ACTION_INIT) == PROCESS_ACTION_INIT)
	{
	    g_send_mother[0] = COMMAND_READY;
	    send_mother(g_send_mother, 1);

	    g_process_action &= ~PROCESS_ACTION_INIT;
	}
	if ((g_process_action & PROCESS_ACTION_DISTANCE) == PROCESS_ACTION_DISTANCE)
	{
	    g_detection_left  = analogRead(PIN_DETECTION_LEFT);
	    g_detection_right = analogRead(PIN_DETECTION_RIGHT);

	    /* check if distance has been reached */
	    if (((g_detection_left > 200) || (g_detection_right > 200)) ||
		(g_distance >= g_distance_remaining) && (g_distance_remaining > 0))
	    {
		stop();

		if ((g_detection_left == 1) || (g_detection_right == 1))
		{
		    g_send_mother[0] = COMMAND_RUNNING_DETECTION;
		    g_send_mother[1] = g_detection_left;
		    g_send_mother[2] = g_detection_right;
		    g_send_mother[3] = g_distance;
		    send_mother(g_send_mother, 4);
		}
		else
		{
		    g_send_mother[0] = COMMAND_STOP;
		    g_send_mother[1] = g_distance;
		    send_mother(g_send_mother, 2);
		}

		/* last reset current state */
		g_process_action &= ~PROCESS_ACTION_DISTANCE;
	    }
	}
	if ((g_process_action & PROCESS_ACTION_DETECTION) == PROCESS_ACTION_DETECTION)
	{
	    g_detection_left  = analogRead(PIN_DETECTION_LEFT);
	    g_detection_right = analogRead(PIN_DETECTION_RIGHT);

	    if ((g_detection_left_old != g_detection_left) ||
		(g_detection_right_old != g_detection_right))
	    {
		g_send_mother[0] = COMMAND_DETECTED;
		g_send_mother[1] = g_detection_left;
		g_send_mother[2] = g_detection_right;
		send_mother(g_send_mother, 3);

		g_detection_left_old = g_detection_left;
		g_detection_right_old = g_detection_right;
		delay(250);
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


