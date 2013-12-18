#include <LiquidCrystal.h>

/********************************************************/
/*      Pin  definitions                                */
/********************************************************/

/* Buttons */
#define PIN_INT_BUTTON		2
#define PIN_LEFT_BUTTON		6
#define PIN_RIGHT_BUTTON	4
#define PIN_MIDDLE_BUTTON	7
#define PIN_UP_BUTTON		8
#define PIN_DOWN_BUTTON		5

/* sensors */
#define PIN_DETECTION		A1
#define PIN_TEMPERATURE		A0

/* LCD */
#define PIN_LCD_RS		38
#define PIN_LCD_E		36
#define PIN_LCD_D4 		34
#define PIN_LCD_D5 		32
#define PIN_LCD_D6 		30
#define PIN_LCD_D7 		28

/********************************************************/
/*      Buttons definitions                             */
/********************************************************/
#define NO_BUTTON	0x00
#define LEFT_BUTTON	0x01
#define MIDDLE_BUTTON	0x02
#define RIGHT_BUTTON	0x04
#define UP_BUTTON	0x08
#define DOWN_BUTTON	0x10

volatile unsigned char g_button;
unsigned char g_button_selected;


/********************************************************/
/*      LCD definitions                                 */
/********************************************************/
LiquidCrystal g_lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
unsigned char g_lcd_col;
unsigned char g_lcd_line;

/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/

#define COMMAND_MOTOR_RECV_TEST			0x80 /* [0x80 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_MOTOR_RECV_COUNTERS		0x81 /* [0x81 Counters] [Left in cm] [right in cm] */
#define COMMAND_MOTOR_RECV_READY		0x82 /* [0x82 Ready] */

#define COMMAND_SOUND_RECV_TEST			0xD0 /* [0xD0 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_SOUND_RECV_TEST2		0xD1 /* [0xD1 Test2] [data1] [data2] */
#define COMMAND_SOUND_RECV_READY		0xD2 /* [0xD2 Ready] */


#define CMD_DATA_MAX			6
char g_send_motor[CMD_DATA_MAX];
char g_recv_motor[CMD_DATA_MAX];

char g_send_sound[CMD_DATA_MAX];
char g_recv_sound[CMD_DATA_MAX];

/********************************************************/
/*      Menus definitions                               */
/********************************************************/

#define MENU_ACTION_NONE		0
#define MENU_ACTION_TEMPERATURE		1
#define MENU_ACTION_DETECTION		2
#define MENU_ACTION_PROG		3
#define MENU_ACTION_MOTOR		4
#define MENU_ACTION_PROG1		5
#define MENU_ACTION_PROG2		6
#define MENU_ACTION_PROG3		7
#define MENU_ACTION_SPEED		8
#define MENU_ACTION_DIR			9
#define MENU_ACTION_OBSTACLE		10
#define MENU_ACTION_RUN			11
#define MENU_ACTION_ROTATE		12
#define MENU_ACTION_SQUARE		13
#define MENU_ACTION_DETECT_BACK		14
#define MENU_ACTION_MOTOR_MOVE		15

#define ACTION_MOVE_REVERSE	1
#define ACTION_MOVE_STOP	2
#define ACTION_MOVE_FORWARD	3
#define ACTION_MOTOR_OBSTACLE	4
#define ACTION_PROG_1		7
#define ACTION_PROG_2		8
#define ACTION_PROG_3		9

struct menu_t
{
    char name[16 + 1];
    unsigned char level;
    unsigned char idx;
    unsigned char action;
};


enum menu_main_e
{
    MENU_MAIN_DETECT = 0,
    MENU_MAIN_MOTOR,
    MENU_MAIN_MOTOR_MOVE,
    MENU_MAIN_TEMP,
    MENU_MAIN_PROG,
    MENU_MAIN_END
};


menu_t g_menu_main[] = { {"Detection       ", 0, MENU_MAIN_DETECT, MENU_ACTION_DETECTION},
			 {"Motor Settings  ", 0, MENU_MAIN_MOTOR, MENU_ACTION_MOTOR},
			 {"Motor Move      ", 0, MENU_MAIN_MOTOR_MOVE, MENU_ACTION_MOTOR_MOVE},
			 {"Temperature     ", 0, MENU_MAIN_TEMP, MENU_ACTION_TEMPERATURE},
			 {"Program         ", 0, MENU_MAIN_PROG, MENU_ACTION_PROG},
			 {"", 0, MENU_MAIN_END, MENU_ACTION_NONE}};

enum menu_prog_e
{
    MENU_PROG_1 = 0,
    MENU_PROG_2,
    MENU_PROG_3,
    MENU_PROG_END
};

menu_t g_menu_prog[] = {{"Program 1       ", 1, MENU_PROG_1, MENU_ACTION_PROG1},
			{"Program 2       ", 1, MENU_PROG_2, MENU_ACTION_PROG2},
			{"Program 3       ", 1, MENU_PROG_3, MENU_ACTION_PROG3},
			{"", 1, MENU_PROG_END, MENU_ACTION_NONE}};


enum menu_motor_e
{
    MENU_MOTOR_SPEED = 0,
    MENU_MOTOR_DIR,
    MENU_MOTOR_OBSTACLE,
    MENU_MOTOR_END,
};

menu_t g_menu_motor[] = { {"Speed           ", 1, MENU_MOTOR_SPEED, MENU_ACTION_SPEED},
			  {"Dir             ", 1, MENU_MOTOR_DIR, MENU_ACTION_DIR},
			  {"Obstacle        ", 1, MENU_MOTOR_OBSTACLE, MENU_ACTION_OBSTACLE},
			  {"",1, MENU_MOTOR_END, MENU_ACTION_NONE}};

enum menu_motor_move_e
{
    MENU_MOTOR_MOVE_RUN = 0,
    MENU_MOTOR_MOVE_ROTATE,
    MENU_MOTOR_MOVE_SQUARE,
    MENU_MOTOR_MOVE_DETECT_BACK,
    MENU_MOTOR_MOVE_END,
};

menu_t g_menu_motor_move[] = { {"Move           ", 1, MENU_MOTOR_MOVE_RUN, MENU_ACTION_RUN},
			       {"Rotate         ", 1, MENU_MOTOR_MOVE_ROTATE, MENU_ACTION_ROTATE},
			       {"Square         ", 1, MENU_MOTOR_MOVE_SQUARE, MENU_ACTION_SQUARE},
			       {"Detect & back  ", 1, MENU_MOTOR_MOVE_DETECT_BACK, MENU_ACTION_DETECT_BACK},
			       {"",1, MENU_MOTOR_MOVE_END, MENU_ACTION_NONE}};



enum menu_speed_e
{
    MENU_SPEED_PLUS = 0,
    MENU_SPEED_STOP,
    MENU_SPEED_MINUS,
    MENU_SPEED_END = 1,
};

menu_t g_menu_speed[] = { {"  +    STOP   - ", 2, MENU_SPEED_PLUS, MENU_ACTION_NONE},
			  {"", 2, MENU_SPEED_END, MENU_ACTION_NONE}};

enum menu_dir_e
{
    MENU_DIR_REVERSE = 0,
    MENU_DIR_FORWARD,
    MENU_DIR_END = 1,
};

menu_t g_menu_dir[] = {{"Back Forward" , 2, MENU_DIR_REVERSE, MENU_ACTION_NONE},
		       {"", 2, MENU_DIR_END, MENU_ACTION_NONE}};


#define MENU_MAX_LEVEL 2

unsigned char g_menu_idx;
unsigned char g_menu_level;
unsigned char g_menu_action;
menu_t *g_menu;
menu_t *g_menu_prev;


/********************************************************/
/*      Process definitions                             */
/********************************************************/

/* process MENU */
unsigned char g_process_menu;

/* process ACTION */
#define PROCESS_ACTION_NONE			0
#define PROCESS_ACTION_DETECTION		1
#define PROCESS_ACTION_TEMPERATURE		2
#define PROCESS_ACTION_PROG_1			3
#define PROCESS_ACTION_PROG_2			4
#define PROCESS_ACTION_PROG_3			5
#define PROCESS_ACTION_OBSTACLE			6
#define PROCESS_ACTION_SPEED			7
#define PROCESS_ACTION_DIR			8
#define PROCESS_ACTION_RUN			9
#define PROCESS_ACTION_ROTATE			10
#define PROCESS_ACTION_SQUARE			11
#define PROCESS_ACTION_DETECT_BACK		12
unsigned char g_process_action;


/* process SERIAL MOTOR */
#define PROCESS_RECV_DO_NOTHING	 0
#define PROCESS_RECV_WAIT_COMMAND	 1
#define PROCESS_RECV_WAIT_DATA_1	 2
#define PROCESS_RECV_WAIT_DATA_N	 3
#define PROCESS_RECV_WAIT_DATA_VALUE  4
unsigned char g_process_recv_motor;

/* process SERIAL SOUND */
unsigned char g_process_recv_sound;

/* process MOTOR */
#define PROCESS_MOTOR_SEND_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define PROCESS_MOTOR_SEND_COMMAND_FORWARD	0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define PROCESS_MOTOR_SEND_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define PROCESS_MOTOR_SEND_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define PROCESS_MOTOR_SEND_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define PROCESS_MOTOR_SEND_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define PROCESS_MOTOR_SEND_COMMAND_COUNTERS	0x07 /* [0x07 Get counters] */

unsigned char g_process_motor;

/* process SOUND */
unsigned char g_process_sound;

/********************************************************/
/*      Global definitions                              */
/********************************************************/
#define MOTOR_DIRECTION_FORWARD			0
#define MOTOR_DIRECTION_BACKWARD		1
unsigned char g_motor_left_direction;
unsigned char g_motor_right_direction;

#define MOTOR_STATE_END				1
#define MOTOR_STATE_DIST			2
unsigned char g_state_motor;


unsigned char g_motor_speed;
unsigned char g_motor_distance;
unsigned char g_motor_curr_dist;
unsigned char g_motor_degrees;

unsigned char g_action_detection;
unsigned char g_action_temperature;
unsigned char g_action_run;
unsigned char g_start;

unsigned char g_recv_motor_nb	= 0;
unsigned char g_recv_motor_max	= 0;

unsigned char g_recv_sound_nb	= 0;
unsigned char g_recv_sound_max	= 0;


void setup()
{
    /* Initialize the buttons input pin */
    pinMode(PIN_LEFT_BUTTON, INPUT);
    pinMode(PIN_RIGHT_BUTTON, INPUT);
    pinMode(PIN_MIDDLE_BUTTON, INPUT);
    pinMode(PIN_UP_BUTTON, INPUT);
    pinMode(PIN_DOWN_BUTTON, INPUT);

    /* Initialize the sensors input pin */
    g_action_detection	 = 2;
    pinMode(PIN_DETECTION, INPUT);

    g_action_temperature = 0;
    g_motor_speed = 0;

    /* Init interrupt 0 PIN2 for the 5 buttons */
    g_button_selected = NO_BUTTON;
    g_button = NO_BUTTON;
    pinMode(PIN_INT_BUTTON, INPUT);

    /* init global menu variables */
    g_menu_idx    = 0;
    g_menu_level  = 0;
    g_menu_action = MENU_ACTION_NONE;
    g_menu        = &g_menu_main[0];
    g_menu_prev   = g_menu;
    g_send_motor[0] = 0;
    g_recv_motor[0] = 0;
    g_send_sound[0] = 0;
    g_recv_sound[0] = 0;

    /* init process states */
    g_process_menu = 0;
    g_process_action = PROCESS_ACTION_NONE;
    g_process_recv_motor = PROCESS_RECV_WAIT_COMMAND;
    g_process_recv_sound = PROCESS_RECV_WAIT_COMMAND;
    g_process_motor = 0;
    g_process_sound = 0;

    /* Motor state */
    g_state_motor = 0;

    /* Sound state */


    /* Global stuff*/
    g_action_detection   = 0;
    g_action_temperature = 0;
    g_action_run	 = 0;
    g_motor_speed	 = 0;
    g_motor_left_direction = 0;
    g_motor_right_direction = 0;
    g_motor_distance	= 0;
    g_motor_degrees	= 0;
    g_motor_curr_dist	= 0;
    g_start		= 0;

    /* set up the LCD's number of columns and rows: */
    g_lcd_col	= 0;
    g_lcd_line	= 0;
    g_lcd.begin(16, 2);

    /* initialize the serial communications Motor board */
    Serial.begin(115200);

    /* initialize the serial communications Sound board */
    Serial1.begin(115200);

    attachInterrupt(0, interrupt_call, FALLING);

    /* start, and display first menu entry */
    g_lcd.clear();
}

void interrupt_call(void)
{
    /* Do not set button if last button state is not threated */
    if (g_button == NO_BUTTON)
    {
	if (digitalRead(PIN_LEFT_BUTTON))
	    g_button = LEFT_BUTTON;
	else if (digitalRead(PIN_RIGHT_BUTTON))
	    g_button = RIGHT_BUTTON;
	else if (digitalRead(PIN_MIDDLE_BUTTON))
	    g_button = MIDDLE_BUTTON;
	else if (digitalRead(PIN_UP_BUTTON))
	    g_button = UP_BUTTON;
	else if (digitalRead(PIN_DOWN_BUTTON))
	    g_button = DOWN_BUTTON;
    }
}


void motor_forward(unsigned char distance, unsigned char speed)
{
    g_motor_left_direction	= MOTOR_DIRECTION_FORWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_FORWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    g_send_motor[3] = '\0';

    Serial.write(g_send_motor);
}


void motor_backward(unsigned char distance, unsigned char speed)
{
    g_motor_left_direction	= MOTOR_DIRECTION_BACKWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_BACKWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    g_send_motor[3] = '\0';

    Serial.write(g_send_motor);
}

void gestion_menu(void)
{
    g_button_selected = NO_BUTTON;

    /* Check which Button is hold */
    switch (g_button)
    {
	case LEFT_BUTTON:
	{
	    g_button_selected = g_button;
	    if (g_menu_idx > 0)
	    {
		g_menu_idx--;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
	    }

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case RIGHT_BUTTON:
	{
	    g_button_selected = g_button;

	    if (g_menu[0].name[0] != '\0')
	    {
		g_menu_idx++;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
	    }

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case MIDDLE_BUTTON:
	{
	    g_button_selected = g_button;

	    if (g_menu_level < MENU_MAX_LEVEL)
	    {
		g_menu_level++;
		g_menu_idx = 0;

		g_menu_prev = g_menu;
	    }

	    g_menu_action = g_menu[g_menu_idx].action;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case UP_BUTTON:
	{
	    g_button_selected = g_button;

	    if (g_menu_level > 0)
	    {
		g_menu_level--;
		g_menu_idx = 0;
		g_menu = g_menu_prev;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
	    }

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case DOWN_BUTTON:
	{
	    g_button_selected = g_button;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	default :
	{
	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
    }
}

void process_menu(void)
{
    if (g_process_menu)
    {
	gestion_menu();

	if (g_button_selected == MIDDLE_BUTTON)
	{
	    switch (g_menu_action)
	    {
		case MENU_ACTION_PROG:
		{
		    g_menu = &g_menu_prog[0];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}break;
		case MENU_ACTION_MOTOR:
		{
		    g_menu = &g_menu_motor[0];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}break;
		case MENU_ACTION_DETECTION:
		{
		    g_process_action = PROCESS_ACTION_DETECTION;
		    g_action_detection   = 2;

		}break;
		case MENU_ACTION_TEMPERATURE:
		{
		    g_process_action = PROCESS_ACTION_TEMPERATURE;
		    g_action_temperature = 0;
		}break;
		case MENU_ACTION_PROG1:
		{
		    g_process_action = PROCESS_ACTION_PROG_1;
		}break;
		case MENU_ACTION_PROG2:
		{
		    g_process_action = PROCESS_ACTION_PROG_2;
		}break;
		case MENU_ACTION_PROG3:
		{
		    g_process_action = PROCESS_ACTION_PROG_3;
		}break;
		case MENU_ACTION_SPEED:
		{
		    g_menu = &g_menu_speed[0];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		    g_process_action = PROCESS_ACTION_SPEED;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_DIR:
		{
		    g_menu = &g_menu_dir[0];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		    g_process_action = PROCESS_ACTION_DIR;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_OBSTACLE:
		{
		    g_process_action = PROCESS_ACTION_OBSTACLE;
		}break;
		case MENU_ACTION_RUN:
		{
		    if (g_motor_left_direction == MOTOR_DIRECTION_FORWARD)
		    {
			g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_FORWARD;
		    }
		    else
		    {
			g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_BACKWARD;
		    }
		    g_send_motor[1] = g_motor_speed;
		    g_motor_distance = 20;
		    g_send_motor[2] = g_motor_distance;
		    g_send_motor[3] = 0;
		    /* data = (char*)g_send_motor; */
		    Serial.write(g_send_motor);

		    g_process_action = PROCESS_ACTION_RUN;
		    g_process_menu = 0;
		    g_action_run = 0;
		}break;
		case MENU_ACTION_ROTATE:
		{
		    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_ROTATE_RIGHT;
		    g_send_motor[1] = g_motor_speed;
		    g_motor_degrees = 90;
		    g_send_motor[2] = g_motor_degrees;
		    g_send_motor[3] = 0;
		    Serial.write(g_send_motor);

		    g_process_action = PROCESS_ACTION_ROTATE;
		    g_process_menu = 0;
		    g_action_run = 0;
		}break;
		case MENU_ACTION_SQUARE:
		{
		    g_process_action = PROCESS_ACTION_SQUARE;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_DETECT_BACK:
		{
		    motor_forward(0,0);
		    g_process_action = PROCESS_ACTION_DETECT_BACK;
		    g_process_menu = 0;
		}break;
	    }
	}
    }
}

void process_action(void)
{
    int value;

    if (g_process_action)
    {
	if (g_process_action == PROCESS_ACTION_DETECTION)
	{
	    value = digitalRead(PIN_DETECTION);
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Obstacle  ! ");
		}
	    }
	    else
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Detecting.....  ");
		}
	    }
	    g_action_detection = value;
	}
	else if (g_process_action == PROCESS_ACTION_TEMPERATURE)
	{
	    value = (5.0 * analogRead(PIN_TEMPERATURE) * 100.0) / 1024;

	    if (g_action_temperature != value)
	    {
		g_lcd.setCursor(0, 1);
		g_lcd.print("Temp =  ");
		g_lcd.print(value);
		g_lcd.print("C     ");
	    }

	    g_action_temperature = value;
	}
	else if (g_process_action == PROCESS_ACTION_OBSTACLE)
	{
	    value = digitalRead(PIN_DETECTION);
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Stop  !       ");
		}
	    }
	    else
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Running.....    ");
		}
	    }
	    g_action_detection = value;
	}
	else if (g_process_action == PROCESS_ACTION_SPEED)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case LEFT_BUTTON:
		{
		    if (g_motor_speed < 255 )
			g_motor_speed++;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("                ");
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Speed = ");
		    g_lcd.print(g_motor_speed);

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case RIGHT_BUTTON:
		{
		    if (g_motor_speed > 0)
			g_motor_speed--;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("                ");
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Speed = ");
		    g_lcd.print(g_motor_speed);

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case MIDDLE_BUTTON:
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("                ");
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Stop !! ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    if (g_menu_level > 0)
		    {
			g_menu_level--;
		    }
		    g_menu_idx = 0;
		    g_menu = g_menu_prev;

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		    g_process_menu = 1;
		    g_process_action = PROCESS_ACTION_NONE;

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_action == PROCESS_ACTION_DIR)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case LEFT_BUTTON:
		{
		    g_motor_left_direction = MOTOR_DIRECTION_BACKWARD;
		    g_motor_right_direction = MOTOR_DIRECTION_BACKWARD;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("New dir Backward");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case RIGHT_BUTTON:
		{
		    g_motor_left_direction = MOTOR_DIRECTION_FORWARD;
		    g_motor_right_direction = MOTOR_DIRECTION_FORWARD;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("New dir Forward ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    if (g_menu_level > 0)
		    {
			g_menu_level--;
		    }
		    g_menu_idx = 0;
		    g_menu = g_menu_prev;

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		    g_process_menu = 1;
		    g_process_action = PROCESS_ACTION_NONE;

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_action == PROCESS_ACTION_RUN)
	{
	    if (g_state_motor == MOTOR_STATE_END)
	    {
		if (g_menu_level > 0)
		{
		    g_menu_level--;
		}
		g_menu_idx = 0;
		g_menu = g_menu_prev;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
		g_process_menu = 1;

		g_process_action = PROCESS_ACTION_NONE;

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    else if(g_state_motor == MOTOR_STATE_DIST)
	    {
		if (g_action_run == 0)
		{
		    g_lcd.setCursor(0, 0);
		    g_lcd.print("Speed = ");  g_lcd.print(g_motor_speed);
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Dist = "); g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		    g_action_run = 1;
		    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_COUNTERS;
		    Serial.write(g_send_motor);
		}
		else
		{
		    g_lcd.setCursor(7, 1);
		    g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_COUNTERS;
		    Serial.write(g_send_motor);
		}
	    }
	}
	else if (g_process_action == PROCESS_ACTION_ROTATE)
	{
	    if (g_state_motor == MOTOR_STATE_END)
	    {
		if (g_menu_level > 0)
		{
		    g_menu_level--;
		}
		g_menu_idx = 0;
		g_menu = g_menu_prev;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
		g_process_menu = 1;
		g_process_action = PROCESS_ACTION_NONE;

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    else if(g_state_motor == MOTOR_STATE_DIST)
	    {
		if (g_action_run == 0)
		{
		    g_lcd.setCursor(0, 0);
		    g_lcd.print("Speed = ");  g_lcd.print(g_motor_speed);
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Dist = "); g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		    g_action_run = 1;
		    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_COUNTERS;
		    Serial.write(g_send_motor);
		}
		else
		{
		    g_lcd.setCursor(7, 1);
		    g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		    g_send_motor[0] = PROCESS_MOTOR_SEND_COMMAND_COUNTERS;
		    Serial.write(g_send_motor);
		}
	    }
	}
    }
}

void process_recv_motor(void)
{
    char value;

    if (g_process_recv_motor)
    {
	/* if we get a valid char, read char */
	if (Serial.available() > 0)
	{
	    /* get incoming byte: */
	    value = Serial.read();

	    if (g_process_recv_motor == PROCESS_RECV_WAIT_COMMAND)
	    {
		g_recv_motor[0] = value;
		g_recv_sound_nb++;

		if ((g_recv_motor[0] == COMMAND_MOTOR_RECV_COUNTERS))
		{
		    g_recv_motor_max = 3;

		    /* reschedule read UART to get DATA */
		    g_process_recv_motor = PROCESS_RECV_WAIT_DATA_N;
		}
		else if (g_recv_motor[0] == COMMAND_MOTOR_RECV_TEST)
		{
		    /* reschedule read UART to get the number of bytes to read */
		    g_process_recv_motor = PROCESS_RECV_WAIT_DATA_VALUE;
		}
		else
		{
		    g_recv_motor_max = 1;

		    /* Do not reschedule read UART */
		    g_process_recv_motor   = 0;
		    g_process_motor = g_recv_motor[0];
		}
	    }
	    else if (g_process_recv_motor == PROCESS_RECV_WAIT_DATA_VALUE)
	    {
		g_recv_motor_max = value;
		g_recv_sound_nb++;

		if (g_recv_motor_max > CMD_DATA_MAX)
		    g_recv_motor_max = CMD_DATA_MAX;

		/* Do not reschedule read UART */
		g_process_recv_motor = PROCESS_RECV_WAIT_DATA_N;
	    }
	    else if (g_process_recv_motor == PROCESS_RECV_WAIT_DATA_N)
	    {
		g_recv_motor[g_recv_motor_nb] = value;
		g_recv_motor_nb ++;

		if (g_recv_motor_nb == g_recv_motor_max)
		{
		    /* Do not reschedule read UART */
		    g_process_recv_motor = 0;
		    g_process_motor = g_recv_motor[0];
		}
		else
		{
		    /* reschedule read UART to get next DATA */
		    g_process_recv_motor = PROCESS_RECV_WAIT_DATA_N;
		}
	    }
	}
    }
}

void process_recv_sound(void)
{
    char value;

    if (g_process_recv_sound)
    {
	/* if we get a valid char, read char */
	if (Serial1.available() > 0)
	{
	    /* get incoming byte: */
	    value = Serial1.read();

	    if (g_process_recv_sound == PROCESS_RECV_WAIT_COMMAND)
	    {
		g_recv_sound[0] = value;
		g_recv_sound_nb++;

		if ((g_recv_sound[0] == COMMAND_SOUND_RECV_TEST2))
		{
		    g_recv_sound_max = 3;

		    /* reschedule read UART to get DATA */
		    g_process_recv_sound  = PROCESS_RECV_WAIT_DATA_N;
		}
		else if (g_recv_sound[0] == COMMAND_SOUND_RECV_TEST)
		{
		    /* reschedule read UART to get the number of bytes to read */
		    g_process_recv_sound = PROCESS_RECV_WAIT_DATA_VALUE;
		}
		else
		{
		    g_recv_sound_max = 1;

		    /* Do not reschedule read UART */
		    g_process_recv_sound   = 0;
		    g_process_sound = g_recv_sound[0];
		}
	    }
	    else if (g_process_recv_sound == PROCESS_RECV_WAIT_DATA_VALUE)
	    {
		g_recv_sound_max = value;
		g_recv_sound_nb++;

		if (g_recv_sound_max > CMD_DATA_MAX)
		    g_recv_sound_max = CMD_DATA_MAX;

		/* Do not reschedule read UART */
		g_process_recv_sound = PROCESS_RECV_WAIT_DATA_N;
	    }
	    else if (g_process_recv_sound == PROCESS_RECV_WAIT_DATA_N)
	    {
		g_recv_sound[g_recv_sound_nb] = value;
		g_recv_sound_nb ++;

		if (g_recv_sound_nb == g_recv_sound_max)
		{
		    /* Do not reschedule read UART */
		    g_process_recv_sound = 0;
		    g_process_sound = g_recv_sound[0];
		}
		else
		{
		    /* reschedule read UART to get next DATA */
		    g_process_recv_sound = PROCESS_RECV_WAIT_DATA_N;
		}
	    }
	}
    }
}

void process_motor(void)
{
    if (g_process_motor)
    {
	if (g_process_motor == COMMAND_MOTOR_RECV_COUNTERS)
	{
	    g_motor_curr_dist = g_recv_motor[1];
	    g_process_recv_motor = PROCESS_RECV_WAIT_COMMAND;
	    g_process_motor = 0;
	}
	else if (g_process_motor == COMMAND_MOTOR_RECV_READY)
	{
	    g_lcd.setCursor(0, 0);
	    g_lcd.print("Motor Ready !!");
	    g_start++;
	    g_process_recv_motor = PROCESS_RECV_WAIT_COMMAND;
	    g_process_motor = 0;
	    if (g_start == 2)
	    {
		delay(2000);
		g_process_menu = 1;
		g_lcd.clear();
		g_lcd.print(g_menu[0].name);
	    }
	}
    }
}

void process_sound(void)
{
    if (g_process_sound)
    {
	if (g_process_sound == COMMAND_SOUND_RECV_READY)
	{
	    g_lcd.setCursor(0, 1);
	    g_lcd.print("Sound Ready !!");
	    g_start++;
	    g_process_recv_sound = PROCESS_RECV_WAIT_COMMAND;
	    g_process_sound = 0;
	    if (g_start == 2)
	    {
		delay(2000);
		g_process_menu = 1;
		g_lcd.clear();
		g_lcd.print(g_menu[0].name);
	    }
	}
    }
}


void loop(void)
{
    process_menu();
    process_action();
    process_recv_motor();
    process_recv_sound();
    process_motor();
    process_sound();
}
