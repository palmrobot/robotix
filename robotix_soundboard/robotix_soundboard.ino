/********************************************************/
/*      Pin  definitions                                */
/********************************************************/



/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_TEST			0x80 /* [0x80 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_TEST2			0x81 /* [0x81 Test2] [data1] [data2] */
#define COMMAND_READY			0x82 /* [0x82 Ready] */
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
unsigned char g_process_action;

#define PROCESS_COMMAND_LIST			0xD1 /* [0xD1 List] */
#define PROCESS_COMMAND_START			0xFE /* [0xFE Start transmission */
unsigned char g_process_command;

/********************************************************/
/*      Global definitions                              */
/********************************************************/
uint8_t g_time_count;
uint8_t g_recv_mother_nb;


void setup()
{
    /* Initialize the sensors input pin */


    /* init process states */
    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;
    g_process_command   = 0;
    g_process_action    = PROCESS_ACTION_INIT;

    /* Init global variables */


    /* init pipes */
    g_recv_mother[0]	= 0;
    g_recv_mother_nb	= 0;

    g_send_mother[0]	= 0;

    delay(1000);

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);

}

/* Sound -> Mother */
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


void process_command(void)
{
    unsigned char speed;
    unsigned char i;

    if (g_process_command)
    {
	if (g_process_command == PROCESS_COMMAND_LIST)
	{
	    g_process_receive = PROCESS_RECEIVE_WAIT_COMMAND;

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
	    send_mother(g_send_mother, 1);

	    g_process_action &= ~PROCESS_ACTION_INIT;
	}
    }
}

void loop()
{
    process_receive();
    process_command();
    process_action();
}


