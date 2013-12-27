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

/* Motor -> Mother */
/*  0xFD  -> Start */
/*  Ack   <- 0xFF  */
/*  0xXX  -> DATA  */
/*  0xXX  -> DATA  */
/*  0xFE  -> End   */



#define MOTOR_SEND_COMMAND_STOP		0x01 /* [0x01 Stop] */
#define MOTOR_SEND_COMMAND_FORWARD	0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define MOTOR_SEND_COMMAND_BACKWARD	0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define MOTOR_SEND_COMMAND_ROTATE_LEFT	0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define MOTOR_SEND_COMMAND_ROTATE_RIGHT	0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define MOTOR_SEND_COMMAND_TEST		0x06 /* [0x06 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define MOTOR_SEND_COMMAND_COUNTERS	0x07 /* [0x07 Get counters] */
#define MOTOR_SEND_COMMAND_START	0xFD /* [0xFD Start transmission] */
#define MOTOR_SEND_COMMAND_END		0xFE /* [0xFE End of transmission] */
#define MOTOR_SEND_COMMAND_ACK		0xFF /* [0xFF Ack of transmission] */


#define CMD_DATA_MAX			6
uint8_t g_send_motor[CMD_DATA_MAX];
uint8_t g_recv_motor[CMD_DATA_MAX];

#define SOUND_SEND_COMMAND_LIST		0xD1 /* [0xD1 List ] */


uint8_t g_send_sound[CMD_DATA_MAX];
uint8_t g_recv_sound[CMD_DATA_MAX];

/********************************************************/
/*      Menus definitions                               */
/********************************************************/

#define    MENU_ACTION_NONE		0
#define    MENU_ACTION_TEMPERATURE	1
#define    MENU_ACTION_DETECTION	2
#define    MENU_ACTION_PROG		3
#define    MENU_ACTION_MOTOR		4
#define    MENU_ACTION_PROG1		5
#define    MENU_ACTION_PROG2		6
#define    MENU_ACTION_PROG3		7
#define    MENU_ACTION_SPEED		8
#define    MENU_ACTION_DIR		9
#define    MENU_ACTION_OBSTACLE		10
#define    MENU_ACTION_RUN		11
#define    MENU_ACTION_ROTATE		12
#define    MENU_ACTION_SQUARE		13
#define    MENU_ACTION_DETECT_BACK	14
#define    MENU_ACTION_MOTOR_MOVE	15
#define    MENU_ACTION_SOUND		16
#define    MENU_ACTION_LIST		17
#define    MENU_ACTION_PLAY_TEMP	18
#define    MENU_ACTION_PLAY_DETECT	19

typedef struct menu_t
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
    MENU_MAIN_SOUND,
    MENU_MAIN_PROG,
    MENU_MAIN_END
};


menu_t g_menu_main[] = { {"Detection      >", 0, MENU_MAIN_DETECT, MENU_ACTION_DETECTION},
			 {"<Motor Settings>", 0, MENU_MAIN_MOTOR, MENU_ACTION_MOTOR},
			 {"<Motor Move    >", 0, MENU_MAIN_MOTOR_MOVE, MENU_ACTION_MOTOR_MOVE},
			 {"<Temperature   >", 0, MENU_MAIN_TEMP, MENU_ACTION_TEMPERATURE},
			 {"<Sound         >", 0, MENU_MAIN_SOUND, MENU_ACTION_SOUND},
			 {"<Program        ", 0, MENU_MAIN_PROG, MENU_ACTION_PROG},
			 {"", 0, MENU_MAIN_END, MENU_ACTION_NONE}};

enum menu_prog_e
{
    MENU_PROG_1 = 0,
    MENU_PROG_2,
    MENU_PROG_3,
    MENU_PROG_END
};

menu_t g_menu_prog[] = {{"Ping Pong      >", 1, MENU_PROG_1, MENU_ACTION_DETECT_BACK},
			{"<Program 2     >", 1, MENU_PROG_2, MENU_ACTION_PROG2},
			{"<Program 3      ", 1, MENU_PROG_3, MENU_ACTION_PROG3},
			{"", 1, MENU_PROG_END, MENU_ACTION_NONE}};


enum menu_motor_e
{
    MENU_MOTOR_SPEED = 0,
    MENU_MOTOR_DIR,
    MENU_MOTOR_OBSTACLE,
    MENU_MOTOR_END,
};

menu_t g_menu_motor[] = { {"Speed          >", 1, MENU_MOTOR_SPEED, MENU_ACTION_SPEED},
			  {"<Dir           >", 1, MENU_MOTOR_DIR, MENU_ACTION_DIR},
			  {"<Obstacle       ", 1, MENU_MOTOR_OBSTACLE, MENU_ACTION_OBSTACLE},
			  {"",1, MENU_MOTOR_END, MENU_ACTION_NONE}};

enum menu_motor_move_e
{
    MENU_MOTOR_MOVE_RUN = 0,
    MENU_MOTOR_MOVE_ROTATE,
    MENU_MOTOR_MOVE_SQUARE,
    MENU_MOTOR_MOVE_DETECT_BACK,
    MENU_MOTOR_MOVE_END,
};

menu_t g_menu_motor_move[] = { {"Move          >", 1, MENU_MOTOR_MOVE_RUN, MENU_ACTION_RUN},
			       {"<Rotate       >", 1, MENU_MOTOR_MOVE_ROTATE, MENU_ACTION_ROTATE},
			       {"<Square       >", 1, MENU_MOTOR_MOVE_SQUARE, MENU_ACTION_SQUARE},
			       {"<Ping Pong     ", 1, MENU_MOTOR_MOVE_DETECT_BACK, MENU_ACTION_DETECT_BACK},
			       {"",1, MENU_MOTOR_MOVE_END, MENU_ACTION_NONE}};


enum menu_sound_e
{
    MENU_SOUND_LIST = 0,
    MENU_SOUND_PLAY_TEMP,
    MENU_SOUND_PLAY_DETECT,
    MENU_SOUND_END,
};

menu_t g_menu_sound[] = {{"List           >", 1, MENU_SOUND_LIST, MENU_ACTION_LIST},
			{"<Play Temp     >", 1, MENU_SOUND_PLAY_TEMP, MENU_ACTION_PLAY_TEMP},
			{"<Play Detection ", 1, MENU_SOUND_PLAY_DETECT, MENU_ACTION_PLAY_DETECT},
			{"", 1, MENU_SOUND_END, MENU_ACTION_NONE}};


enum menu_speed_e
{
    MENU_SPEED_PLUS = 0,
    MENU_SPEED_STOP,
    MENU_SPEED_MINUS,
    MENU_SPEED_END = 1,
};

menu_t g_menu_speed[] = { {"  -    STOP   + ", 2, MENU_SPEED_PLUS, MENU_ACTION_NONE},
			  {"", 2, MENU_SPEED_END, MENU_ACTION_NONE}};

enum menu_dir_e
{
    MENU_DIR_REVERSE = 0,
    MENU_DIR_FORWARD,
    MENU_DIR_END = 1,
};

menu_t g_menu_dir[] = {{"Back-----Forward" , 2, MENU_DIR_REVERSE, MENU_ACTION_NONE},
		       {"", 2, MENU_DIR_END, MENU_ACTION_NONE}};




#define MENU_MAX_LEVEL 2

unsigned char g_menu_idx;
unsigned char g_menu_level;
unsigned char g_menu_action;

menu_t *g_menu_tree[MENU_MAX_LEVEL+1];
unsigned char g_menu_tree_level;
menu_t *g_menu;


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
#define PROCESS_ACTION_LIST			13
unsigned char g_process_action;


/* process SERIAL MOTOR */
#define PROCESS_RECV_MOTOR_DO_NOTHING		0
#define PROCESS_RECV_MOTOR_WAIT_COMMAND		1
unsigned char g_process_recv_motor;

/* process SERIAL SOUND */
#define PROCESS_RECV_SOUND_DO_NOTHING		0
#define PROCESS_RECV_SOUND_WAIT_COMMAND		1
unsigned char g_process_recv_sound;

/* process MOTOR */
#define PROCESS_MOTOR_TEST			0x40 /* [0x40 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_MOTOR_COUNTERS			0x41 /* [0x41 Counters] [Left in cm] [right in cm] */
#define PROCESS_MOTOR_READY			0x42 /* [0x42 Ready] */
#define PROCESS_MOTOR_STOP			0x43
#define PROCESS_MOTOR_START			0xFD /* Start transmission */
#define PROCESS_MOTOR_END			0xFE /* End of transmission */
#define PROCESS_MOTOR_ACK			0xFF /* Ack of transmission */
unsigned char g_process_motor;

/* process SOUND */
#define PROCESS_SOUND_TEST			0x80 /* [0x80 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_SOUND_TEST2			0x81 /* [0x81 Test2] [data1] [data2] */
#define PROCESS_SOUND_READY			0x82 /* [0x82 Ready] */
#define PROCESS_SOUND_START			0xFD /* Start transmission */
#define PROCESS_SOUND_END			0xFE /* End of transmission */
#define PROCESS_SOUND_ACK			0xFF /* Ack of transmission */

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


uint8_t g_motor_speed;
uint8_t g_motor_distance;
uint8_t g_motor_curr_dist;
uint8_t g_motor_degrees;

unsigned char g_action_detection;
unsigned char g_action_temperature;
unsigned char g_action_run;
unsigned char g_start;

unsigned char g_recv_motor_nb	= 0;
unsigned char g_recv_sound_nb	= 0;


void setup()
{
    /* Initialize the buttons input pin */
    pinMode(PIN_LEFT_BUTTON, INPUT);
    pinMode(PIN_RIGHT_BUTTON, INPUT);
    pinMode(PIN_MIDDLE_BUTTON, INPUT);
    pinMode(PIN_UP_BUTTON, INPUT);
    pinMode(PIN_DOWN_BUTTON, INPUT);

    /* Initialize the sensors input pin */
    pinMode(PIN_DETECTION, INPUT);

    /* Init interrupt 0 PIN2 for the 5 buttons */
    g_button_selected = NO_BUTTON;
    g_button = NO_BUTTON;
    pinMode(PIN_INT_BUTTON, INPUT);

    /* init global menu variables */
    g_menu_idx    = 0;
    g_menu_tree_level = 0;
    g_menu_action = MENU_ACTION_NONE;
    g_menu_tree[g_menu_tree_level]     = &g_menu_main[0];
    g_menu = g_menu_tree[g_menu_tree_level];

    g_send_motor[0] = 0;
    g_recv_motor[0] = 0;
    g_send_sound[0] = 0;
    g_recv_sound[0] = 0;

    /* init process states */
    g_process_menu = 0;
    g_process_action = 0;
    g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;
    g_process_recv_sound = PROCESS_RECV_SOUND_WAIT_COMMAND;
    g_process_motor = 0;
    g_process_sound = 0;

    /* Motor state */
    g_state_motor = 0;

    /* Sound state */


    /* Global stuff*/
    g_action_detection   = 2;
    g_action_temperature = 0;
    g_action_run	 = 0;
    g_motor_speed	 = 120;
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

    attachInterrupt(0, interrupt_call, FALLING);

    delay(800);

    /* initialize the serial communications Motor board */
    Serial.begin(115200);

    /* initialize the serial communications Sound board */
    Serial1.begin(115200);

    /* start, and display first menu entry */
    g_lcd.clear();

    g_lcd.setCursor(0, 0);
    g_lcd.print("Motor : ");
    g_lcd.setCursor(0, 1);
    g_lcd.print("Sound : ");
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


/* Mother -> Motor */
/*  0xFD  -> Start */
/*  Ack   <- 0xFF  */
/*  0xXX  -> DATA  */
/*  0xXX  -> DATA  */
/*  0xFE  -> End   */
/*  Ack   <- 0xFF  */
void send_motor(uint8_t *buffer, int len)
{
    uint8_t padding[4] = {0,0,0,0};

    if (len > 4)
	len = 4;

    /* Send Start of transmission */
    Serial.write(MOTOR_SEND_COMMAND_START);

    /* Write Command + Data */
    Serial.write(buffer, len);

    /* Write padding Data */
    Serial.write(padding, 4 - len);

}

void motor_forward(unsigned char distance, unsigned char speed)
{
    g_motor_left_direction	= MOTOR_DIRECTION_FORWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = MOTOR_SEND_COMMAND_FORWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    send_motor(g_send_motor, 3);
}


void motor_backward(unsigned char distance, unsigned char speed)
{
    g_motor_left_direction	= MOTOR_DIRECTION_BACKWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = MOTOR_SEND_COMMAND_BACKWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    send_motor(g_send_motor, 3);
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

	    if (g_menu[g_menu_idx+1].name[0] != '\0')
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

	    g_menu_action = g_menu[g_menu_idx].action;

	    /* set lower menu to first item */
	    g_menu_idx = 0;

	    /* increase tree level */
	    g_menu_tree_level++;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case UP_BUTTON:
	{
	    g_button_selected = g_button;

	    if (g_menu_tree_level > 0)
	    {
		g_menu_tree_level--;
		g_menu_idx = 0;
		g_menu = g_menu_tree[g_menu_tree_level];

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
		    g_menu_tree[g_menu_tree_level] = &g_menu_prog[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}break;
		case MENU_ACTION_MOTOR:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_motor[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}break;
		case MENU_ACTION_MOTOR_MOVE:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_motor_move[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    g_action_run = 0;
		}break;
		case MENU_ACTION_SOUND:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_sound[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}break;
		case MENU_ACTION_DETECTION:
		{
		    g_process_action = PROCESS_ACTION_DETECTION;
		    g_action_detection   = 2;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_TEMPERATURE:
		{
		    g_process_action = PROCESS_ACTION_TEMPERATURE;
		    g_action_temperature = 0;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_PROG1:
		{
		    g_process_action = 0;/*PROCESS_ACTION_PROG_1;*/
		}break;
		case MENU_ACTION_PROG2:
		{
		    g_process_action = 0; /*PROCESS_ACTION_PROG_2;*/
		}break;
		case MENU_ACTION_PROG3:
		{
		    g_process_action = 0; /* PROCESS_ACTION_PROG_3;*/
		}break;
		case MENU_ACTION_SPEED:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_speed[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Speed = ");
		    g_lcd.print(g_motor_speed);

		    g_process_action = PROCESS_ACTION_SPEED;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_DIR:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_dir[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    if (g_motor_left_direction == MOTOR_DIRECTION_BACKWARD)
		    {
			g_lcd.setCursor(0, 1);
			g_lcd.print("Dir : Backward  ");
		    }
		    else
		    {
			g_lcd.setCursor(0, 1);
			g_lcd.print("Dir : Forward   ");
		    }

		    g_process_action = PROCESS_ACTION_DIR;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_OBSTACLE:
		{
		    g_process_action = 0; /*PROCESS_ACTION_OBSTACLE;*/
		}break;
		case MENU_ACTION_RUN:
		{
		    if (g_motor_left_direction == MOTOR_DIRECTION_FORWARD)
		    {
			g_send_motor[0] = MOTOR_SEND_COMMAND_FORWARD;
		    }
		    else
		    {
			g_send_motor[0] = MOTOR_SEND_COMMAND_BACKWARD;
		    }

		    g_lcd.clear();
		    g_lcd.setCursor(0, 0);
		    g_lcd.print("Speed = "); g_lcd.print(g_motor_speed);
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Dist = "); g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");

		    g_send_motor[1] = g_motor_speed;
		    g_motor_distance = 10;
		    g_send_motor[2] = g_motor_distance;
		    send_motor(g_send_motor, 3);

		    g_state_motor = MOTOR_STATE_DIST;

		    g_process_action = PROCESS_ACTION_RUN;
		    g_process_menu = 0;
		    g_action_run = 0;

		}break;
		case MENU_ACTION_ROTATE:
		{
		    g_send_motor[0] = MOTOR_SEND_COMMAND_ROTATE_RIGHT;
		    g_send_motor[1] = g_motor_speed;
		    g_motor_degrees = 90;
		    g_send_motor[2] = g_motor_degrees;
		    send_motor(g_send_motor, 3);

		    g_state_motor = MOTOR_STATE_DIST;

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
		case MENU_ACTION_LIST:
		{
		    g_send_motor[0] = SOUND_SEND_COMMAND_LIST;
		    send_motor(g_send_motor, 1);

		    g_process_action = 0;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_PLAY_TEMP:
		{

		}break;
		case MENU_ACTION_PLAY_DETECT:
		{

		}break;
	    }
	}
    }
}

void process_action(void)
{
    int value;
    int analog_value;

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

	    if (g_button == UP_BUTTON)
	    {
		/* Return to last menu */
		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
		g_process_menu = 1;
		g_process_action = PROCESS_ACTION_NONE;
		g_action_temperature = 0;
	    }
	    else
	    {
		if (g_button != NO_BUTTON)
		    g_button = NO_BUTTON;
	    }
	}
	else if (g_process_action == PROCESS_ACTION_TEMPERATURE)
	{
	    analog_value = analogRead(PIN_TEMPERATURE);

	    if ((analog_value > (g_action_temperature + 1)) ||
		(analog_value < (g_action_temperature - 1)))
	    {
		value = (500.0 * analog_value) / 1024;

		g_lcd.setCursor(0, 1);
		g_lcd.print("Temp =  ");
		g_lcd.print(value);
		g_lcd.print("C     ");
		g_action_temperature = analog_value;
	    }

	    if (g_button == UP_BUTTON)
	    {
		/* Return to last menu */
		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu[g_menu_idx].name);
		g_process_menu = 1;
		g_process_action = PROCESS_ACTION_NONE;
		g_action_temperature = 0;
	    }
	    else
	    {
		if (g_button != NO_BUTTON)
		    g_button = NO_BUTTON;
	    }

	}
	else if (g_process_action == PROCESS_ACTION_SPEED)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case RIGHT_BUTTON:
		{
		    if (g_motor_speed < 254 )
			g_motor_speed+=2;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("                ");
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Speed = ");
		    g_lcd.print(g_motor_speed);

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case LEFT_BUTTON:
		{
		    if (g_motor_speed > 1)
			g_motor_speed-=2;

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
		    g_lcd.print("Stop ! ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    if (g_menu_tree_level > 0)
		    {
			g_menu_tree_level--;
			g_menu_idx = 0;
			g_menu = g_menu_tree[g_menu_tree_level];

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu[g_menu_idx].name);
		    }

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
		    g_lcd.print("Dir : Backward  ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case RIGHT_BUTTON:
		{
		    g_motor_left_direction = MOTOR_DIRECTION_FORWARD;
		    g_motor_right_direction = MOTOR_DIRECTION_FORWARD;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Dir : Forward   ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    if (g_menu_tree_level > 0)
		    {
			g_menu_tree_level--;
			g_menu_idx = 0;
			g_menu = g_menu_tree[g_menu_tree_level];

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu[g_menu_idx].name);
		    }

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
		g_motor_curr_dist = 0;

		if (g_menu_tree_level > 0)
		{
		    g_menu_tree_level--;
		    g_menu_idx = 0;
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}

		g_process_menu = 1;
		g_state_motor = 0;

		g_process_action = PROCESS_ACTION_NONE;

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    else if(g_state_motor == MOTOR_STATE_DIST)
	    {
		delay(500);
		g_lcd.setCursor(7, 1);
		g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		g_send_motor[0] = MOTOR_SEND_COMMAND_COUNTERS;
		send_motor(g_send_motor, 1);
		g_process_menu = 1;
		g_action_run = 0;
	    }
	}
	else if (g_process_action == PROCESS_ACTION_ROTATE)
	{
	    if (g_state_motor == MOTOR_STATE_END)
	    {
		g_motor_curr_dist = 0;

		if (g_menu_tree_level > 0)
		{
		    g_menu_tree_level--;
		    g_menu_idx = 0;
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);
		}

		g_process_menu = 1;
		g_state_motor = 0;

		g_process_action = PROCESS_ACTION_NONE;

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    else if(g_state_motor == MOTOR_STATE_DIST)
	    {
		delay(500);
		g_lcd.setCursor(7, 1);
		g_lcd.print(g_motor_curr_dist);g_lcd.print("cm  ");
		g_send_motor[0] = MOTOR_SEND_COMMAND_COUNTERS;
		send_motor(g_send_motor, 1);
		g_process_menu = 1;
		g_action_run = 0;
	    }
	}
	else if (g_process_action == PROCESS_ACTION_LIST)
	{
	    



	}
    }
}


/* Mother <- Motor */
/*  Start <- 0xFD  */
/*  0xFF  -> Ack   */
/*  DATA  <- 0xZZ  */
/*  DATA  <- 0xZZ  */
/*  End   <- 0xFE  */
/*  0xFF  -> Ack   */
void process_recv_motor(void)
{
    uint8_t value;
    uint8_t sz;

    if (g_process_recv_motor)
    {
	/* if we get a valid char, read char */
	if (Serial.available() > 0)
	{
	    /* get Start Byte */
	    value = Serial.read();
	    if (value == PROCESS_MOTOR_START)
	    {
		/* Wait for serial */
		while (Serial.available() < 4);

		for(g_recv_motor_nb = 0; g_recv_motor_nb < 4; g_recv_motor_nb++)
		{
		    /* get incoming write: */
		    g_recv_motor[g_recv_motor_nb] = Serial.read();
		    /* g_lcd.print("."); */
		    /* g_lcd.print(g_recv_motor[g_recv_motor_nb],DEC); */
		}
		/* Set action plan */
		g_process_motor = g_recv_motor[0];

		/* Disable communication ,wait for message treatment */
		g_process_recv_motor   = 0;
	    }
	}
    }
}

void process_motor(void)
{
    if (g_process_motor)
    {
	if (g_process_motor == PROCESS_MOTOR_COUNTERS)
	{
	    g_motor_curr_dist = g_recv_motor[1];
	    g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;
	    g_process_motor = 0;
	    g_process_action = PROCESS_ACTION_RUN;
	}
	else if (g_process_motor == PROCESS_MOTOR_STOP)
	{
	    g_state_motor = MOTOR_STATE_END;

	    g_lcd.setCursor(7, 1);
	    g_lcd.print("Stop !  ");
	    delay(500);
	    g_process_motor = 0;
	    g_process_action = PROCESS_ACTION_RUN;
	    g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;
	}
	else if (g_process_motor == PROCESS_MOTOR_READY)
	{
	    g_lcd.setCursor(8, 0);
	    g_lcd.print("Ready !");
	    g_start++;
	    g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;
	    g_process_motor = 0;
	    if (g_start == 1)
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
	if (g_process_sound == PROCESS_SOUND_READY)
	{
	    g_lcd.setCursor(8, 1);
	    g_lcd.print("Ready !");
	    g_start++;
	    g_process_recv_sound = PROCESS_RECV_SOUND_WAIT_COMMAND;
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
    /* process_recv_sound(); */
    process_motor();
    /* process_sound(); */
}
