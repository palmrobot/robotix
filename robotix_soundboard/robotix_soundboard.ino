
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
#define PROCESS_SERIAL_DO_NOTHING	0
#define PROCESS_SERIAL_WAIT_COMMAND	1
#define PROCESS_SERIAL_WAIT_DATA_1	2
#define PROCESS_SERIAL_WAIT_DATA_2	3

#define COMMAND_NOTHING			0
#define COMMAND_SPEED			1
#define COMMAND_ROTATE			2

#define MODE_STOP			0
#define MODE_FORWARD			1
#define MODE_BACKWARD			2
#define MODE_ROTATE_LEFT		3
#define MODE_ROTATE_RIGHT		4

unsigned int g_time_count	 = 0;
unsigned int g_motor_left_count  = 0;
unsigned int g_motor_right_count = 0;

char g_process_serial		= 0;
char g_process_command		= 0;
char g_command			= 0;
char g_data1			= 0;
char g_data2			= 0;
int g_distance_left		= 0;
int g_distance_right		= 0;
unsigned int g_mode		= 0;

void interrupt_left_call(void)
{
    g_motor_left_count++;
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
    g_process_serial	= 0;
    g_process_command	= 0;

    g_command		= COMMAND_NOTHING;
    g_data		= 0;

    /* Init interrupt for the left Motor */
    attachInterrupt(0, interrupt_left_call, FALLING);

    /* Init interrupt for the right Motor */
    attachInterrupt(1, interrupt_right_call, FALLING);

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);
}

void process_serial(void)
{
    char value;

    if (g_process_serial)
    {
	/* if we get a valid char, read char */
	if (Serial.available() > 0)
	{
	    /* get incoming byte: */
	    value = (char)Serial.read();

	    if (g_process_serial == PROCESS_SERIAL_WAIT_COMMAND)
	    {
		g_command = value;
		g_data1 = 0;
		g_data2 = 0;

		if ((g_command == COMMAND_SPEED) ||
		    (g_command == COMMAND_ROTATE))
		{
		    /* reschedule read UART to get DATA */
		    g_process_serial = PROCESS_SERIAL_WAIT_DATA_1;
		}
		else
		{
		    /* Do not reschedule read UART */
		    g_process_serial = 0;
		}
	    }
	    else if (g_process_serial == PROCESS_SERIAL_WAIT_DATA_1)
	    {
		g_data1 = value;

		/* reschedule read UART to get DATA 2 */
		g_process_serial = PROCESS_SERIAL_WAIT_DATA_2;
	    }
	    else if (g_process_serial == PROCESS_SERIAL_WAIT_DATA_2)
	    {
		g_data2 = value;

		/* Do not reschedule read UART */
		g_process_serial = 0;
	    }
	}
    }
}

void process_command(void)
{
    if (g_process_command)
    {

    }
}

void loop()
{
    process_serial();
    process_command();
    g_time_count++;
}






    // if we get a valid byte, read analog ins:
    if (Serial.available() > 0)
    {
	if (mode == MODE_COMMAND)
	{
	    Serial.print("Wait Command: ");
	    // get incoming byte:
	    inCommand = Serial.read();

	    /* Stop */
	    if (inCommand == 'S')
	    {
		digitalWrite(BRAKE, HIGH); // controle brake
		Serial.println(" Brake 1" );
	    } /* start */
	    else  if (inCommand == 'T')
	    {
		digitalWrite(BRAKE, LOW); // controle brake
		Serial.println(" Brake 0" );
	    }
	    else  if (inCommand == 'A')
	    {
		digitalWrite(SENS_AV, HIGH);// controle sens
		Serial.println(" Avance" );
	    }
	    else  if (inCommand == 'B')
	    {
		digitalWrite(SENS_AV, LOW);// controle sens
		Serial.println(" Recule" );
	    }
	    else  if (inCommand == 'V')
	    {
		mode = MODE_DATA;
		Serial.println(" Vitesse" );
	    }
	    else
	    {
		Serial.println("" );
	    }
	}
	else /* MODE_DATA */
	{
	    /* get incoming byte: */
	    inData = Serial.read();
	    if (inCommand == 'V')
	    {
		if (isDigit(inData))
		{
		    inString += (char)inData;
		    Serial.write(inData);
		}
		else
		{
		    analogWrite(VITESSE,inString.toInt());
		    Serial.println("" );
		    Serial.print("Vitesse = " );
		    Serial.println(inString.toInt());
		    inString = "";
		    mode = MODE_COMMAND;
		}
	    }
	    else
	    {
		Serial.print("Data read = " );
		Serial.print(inData);
		mode = MODE_COMMAND;
	    }
	}
    }
}



