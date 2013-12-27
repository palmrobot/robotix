/********************************************************/
/*      Pin  definitions                                */
/********************************************************/



/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_TEST			0x80 /* [0x80 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_TEST2			0x81 /* [0x81 Test2] [data1] [data2] */
#define COMMAND_READY			0x82 /* [0x82 Ready] */


#define CMD_DATA_MAX			6
char g_send_mother[CMD_DATA_MAX];
char g_recv_mother[CMD_DATA_MAX];

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
unsigned char g_process_action;

#define PROCESS_COMMAND_LIST		0xD1 /* [0xD1 List] */

#define PROCESS_COMMAND_STOP		0x01
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

unsigned int g_time_count	  = 0;


unsigned char g_receive_nb       = 0;
unsigned char g_receive_max      = 0;
unsigned char g_receive		 = 0;


void setup()
{
    /* Init input/output */

    /* init process states */
    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;
    g_process_command   = 0;
    g_process_action    = PROCESS_ACTION_INIT;

    /* Init global variables */
    g_time_count	= 0;
    g_receive		= 0;

    /* init pipes */
    g_recv_mother[0]	= 0;
    g_send_mother[0]	= 0;

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);

    delay(700);
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
	if (g_process_command == PROCESS_COMMAND_LIST)
	{
	    


	    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;

	    /* last reset current state */
	    g_process_command = 0;
	}
    }
}

void process_action(void)
{
    if (g_process_action)
    {
	if ((g_process_action & PROCESS_ACTION_INIT) == PROCESS_ACTION_INIT)
	{
	    g_send_mother[0] = COMMAND_READY;
	    g_send_mother[1] = 0;

	    Serial.write(g_send_mother);

	    /* check if distance has been reached */
	    g_process_action &= ~PROCESS_ACTION_INIT;
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


