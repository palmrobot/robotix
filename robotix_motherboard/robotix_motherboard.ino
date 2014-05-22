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
#define PIN_TEMPERATURE		A0
#define PIN_ANALOG1		A1

/* LCD */
#define PIN_LCD_RS		38
#define PIN_LCD_E		36
#define PIN_LCD_D4 		34
#define PIN_LCD_D5 		32
#define PIN_LCD_D6 		30
#define PIN_LCD_D7 		28


/* Notes */
#define PIN_NOTE1		A1
#define PIN_NOTE2		A2
#define PIN_NOTE3		A3
#define PIN_NOTE4		A4
#define PIN_NOTE5		A5
#define PIN_NOTE6		A6
#define PIN_NOTE7		A7

/********************************************************/
/*      Buttons definitions                             */
/********************************************************/
#define NO_BUTTON	0x00
#define LEFT_BUTTON	0x01
#define MIDDLE_BUTTON	0x02
#define RIGHT_BUTTON	0x04
#define UP_BUTTON	0x08
#define DOWN_BUTTON	0x10

volatile uint8_t g_button;
uint8_t g_button_selected;


/********************************************************/
/*      LCD definitions                                 */
/********************************************************/
LiquidCrystal g_lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
uint8_t g_lcd_col;
uint8_t g_lcd_line;

/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/

#define MOTOR_SEND_COMMAND_STOP			0x01 /* [0x01 Stop] */
#define MOTOR_SEND_COMMAND_FORWARD		0x02 /* [0x02 Forward] [Speed] [distance in cm] */
#define MOTOR_SEND_COMMAND_BACKWARD		0x03 /* [0x03 Backward] [Speed] [distance in cm] */
#define MOTOR_SEND_COMMAND_ROTATE_LEFT		0x04 /* [0x04 Rotate Left] [Speed] [degrees]   */
#define MOTOR_SEND_COMMAND_ROTATE_RIGHT		0x05 /* [0x05 Rotate Right] [Speed] [degrees]  */
#define MOTOR_SEND_COMMAND_TEST			0x06 /* [0x06 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define MOTOR_SEND_COMMAND_COUNTERS_CM		0x07 /* [0x07 Get counters in centimeters] */
#define MOTOR_SEND_COMMAND_COUNTERS		0x08 /* [0x08 Get counters] */
#define MOTOR_SEND_COMMAND_COUNTERS_DEG		0x09 /* [0x09 Get counters in degrees] */
#define MOTOR_SEND_COMMAND_DETECTION		0x0A /* [0x0A Get Detection */

#define MOTOR_SEND_COMMAND_START		0xFE /* [0xFE Start transmission] */

#define CMD_DATA_MAX				6
uint8_t g_send_motor[CMD_DATA_MAX];
uint8_t g_recv_motor[CMD_DATA_MAX];

#define SOUND_SEND_COMMAND_LIST			0xD1 /* [0xD1 List ] */
#define SOUND_SEND_COMMAND_FILENAME		0xD2 /* [0xD2 Number of the file to get name ] */
#define SOUND_SEND_COMMAND_PLAYFILE		0xD3 /* [0xD3 Play this file number */
#define SOUND_SEND_COMMAND_STOP_PLAYING		0xD4 /* [0xD4 Stop playing] */
#define SOUND_SEND_COMMAND_BEEP_KEY		0xD5 /* [0xD5 Playing Beep] */
#define SOUND_SEND_COMMAND_NOTE			0xD6 /* [0xD6 Playing noteDO Note] */
#define SOUND_SEND_COMMAND_MOTOR		0xD7 /* [0xD7 Playing motor sound] */
#define SOUND_SEND_COMMAND_HELLO		0xD8 /* [0xD8 Playing hello sound] */
#define SOUND_SEND_COMMAND_START		0xFE /* [0xFE Start transmission] */

uint8_t g_send_sound[CMD_DATA_MAX];

#define CMD_RECV_DATA_MAX				16
uint8_t g_recv_sound[CMD_RECV_DATA_MAX];

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
#define    MENU_ACTION_ROT		20
#define    MENU_ACTION_UNIT		21
#define    MENU_ACTION_ANALOG		22
#define    MENU_ACTION_PLAY_NOTE	23

typedef struct menu_t
{
    char name[16 + 1];
    uint8_t level;
    uint8_t idx;
    uint8_t action;
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
			{"<Analog        >", 1, MENU_PROG_2, MENU_ACTION_ANALOG},
			{"<Program 3      ", 1, MENU_PROG_3, MENU_ACTION_PROG3},
			{"", 1, MENU_PROG_END, MENU_ACTION_NONE}};


enum menu_motor_e
{
    MENU_MOTOR_SPEED = 0,
    MENU_MOTOR_UNIT,
    MENU_MOTOR_DIR,
    MENU_MOTOR_ROT,
    MENU_MOTOR_OBSTACLE,
    MENU_MOTOR_END,
};

menu_t g_menu_motor[] = { {"Speed          >", 1, MENU_MOTOR_SPEED, MENU_ACTION_SPEED},
			  {"<Unit          >", 1, MENU_MOTOR_UNIT, MENU_ACTION_UNIT},
			  {"<Dir           >", 1, MENU_MOTOR_DIR, MENU_ACTION_DIR},
			  {"<Rotation      >", 1, MENU_MOTOR_ROT, MENU_ACTION_ROT},
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
    MENU_SOUND_PLAY_NOTE,
    MENU_SOUND_PLAY_TEMP,
    MENU_SOUND_PLAY_DETECT,
    MENU_SOUND_END,
};

menu_t g_menu_sound[] = { {"List           >", 1, MENU_SOUND_LIST, MENU_ACTION_LIST},
			  {"<Play Note     >", 1, MENU_SOUND_PLAY_NOTE, MENU_ACTION_PLAY_NOTE},
			  {"<Play Temp     >", 1, MENU_SOUND_PLAY_TEMP, MENU_ACTION_PLAY_TEMP},
			  {"<Play Detection ", 1, MENU_SOUND_PLAY_DETECT, MENU_ACTION_PLAY_DETECT},
			  {"", 1, MENU_SOUND_END, MENU_ACTION_NONE}};


enum menu_speed_e
{
    MENU_SPEED_PLUS = 0,
    MENU_SPEED_STOP,
    MENU_SPEED_MINUS,
    MENU_SPEED_END,
};

menu_t g_menu_speed[] = { {"  -    STOP   + ", 2, MENU_SPEED_PLUS, MENU_ACTION_NONE},
			  {"", 2, MENU_SPEED_END, MENU_ACTION_NONE}};



enum menu_unit_e
{
    MENU_UNIT_PLUS = 0,
    MENU_UNIT_STOP,
    MENU_UNIT_MINUS,
    MENU_UNIT_END,
};

menu_t g_menu_unit[] = { {"Deg  Count  Cm  ", 2, MENU_UNIT_PLUS, MENU_ACTION_NONE},
			  {"", 2, MENU_SPEED_END, MENU_ACTION_NONE}};

enum menu_dir_e
{
    MENU_DIR_REVERSE = 0,
    MENU_DIR_FORWARD,
    MENU_DIR_END,
};

menu_t g_menu_dir[] = {{"Back-----Forward" , 2, MENU_DIR_REVERSE, MENU_ACTION_NONE},
		       {"", 2, MENU_DIR_END, MENU_ACTION_NONE}};


enum menu_rot_e
{
    MENU_ROT_LEFT = 0,
    MENU_ROT_RIGHT,
    MENU_ROT_END,
};

menu_t g_menu_rot[] = {{"Left-------Right" , 2, MENU_ROT_LEFT, MENU_ACTION_NONE},
		       {"", 2, MENU_ROT_END, MENU_ACTION_NONE}};




#define MENU_MAX_LEVEL 2

uint8_t g_menu_idx;
uint8_t g_menu_level;
uint8_t g_menu_action;

menu_t *g_menu_tree[MENU_MAX_LEVEL+1];
uint8_t g_menu_tree_level;
uint8_t g_menu_tree_idx[MENU_MAX_LEVEL+1];
menu_t *g_menu;


/********************************************************/
/*      Process definitions                             */
/********************************************************/

/* process MENU */
uint8_t g_process_menu;


/* process MOTHER */
#define PROCESS_MOTHER_ACTION_TEMPERATURE	1
#define PROCESS_MOTHER_ACTION_ANALOG		2
uint8_t g_process_mother_action;

/* process ACTION */
#define PROCESS_ACTION_NONE			0
#define PROCESS_ACTION_DETECTION		1

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
#define PROCESS_ACTION_ROT			14
#define PROCESS_ACTION_UNIT			15
#define PROCESS_ACTION_PLAY_NOTE		16
uint8_t g_process_motor_action;


uint8_t g_process_sound_action;

/* process SERIAL MOTOR */
#define PROCESS_RECV_MOTOR_DO_NOTHING		0
#define PROCESS_RECV_MOTOR_WAIT_COMMAND		1
uint8_t g_process_recv_motor;

/* process SERIAL SOUND */
#define PROCESS_RECV_SOUND_DO_NOTHING		0
#define PROCESS_RECV_SOUND_WAIT_COMMAND		1
uint8_t g_process_recv_sound;

/* process MOTOR */
#define PROCESS_MOTOR_TEST			0x40 /* [0x40 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_MOTOR_COUNTERS			0x41 /* [0x41 Counters] [Left] [right] */
#define PROCESS_MOTOR_READY			0x42 /* [0x42 Ready] */
#define PROCESS_MOTOR_STOP			0x43
#define PROCESS_MOTOR_DETECT			0x44
#define PROCESS_MOTOR_RUNNING_DETECTION		0x45
#define PROCESS_MOTOR_START			0xFE /* Start transmission */
uint8_t g_process_motor;

/* process SOUND */
#define PROCESS_SOUND_TEST			0x80 /* [0x80 Test] [Nb of writes] [write 1] [write 2] ... [write n] */
#define PROCESS_SOUND_TEST2			0x81 /* [0x81 Test2] [data1] [data2] */
#define PROCESS_SOUND_READY			0x82 /* [0x82 Ready] */
#define PROCESS_SOUND_INIT_ERROR		0x83 /* [0x83 End of file] */
#define PROCESS_SOUND_INIT_FAT_ERROR		0x84 /* [0x84 End of file] */
#define PROCESS_SOUND_INIT_ROOT_ERROR		0x85 /* [0x85 End of file] */
#define PROCESS_SOUND_PLAYING_FILE		0x86 /* [0x86 File is playing] */
#define PROCESS_SOUND_PLAY_END			0x87 /* [0x87 End of file] */
#define PROCESS_SOUND_FILE_NUMBER		0x88 /* [0x88 File number] */
#define PROCESS_SOUND_FILE_NAME			0xA0 /* [0xA0 File name] [ n Data of filenanme ] */
#define PROCESS_SOUND_START			0xFE /* Start transmission */
uint8_t g_process_sound;

/********************************************************/
/*      Global definitions                              */
/********************************************************/

#define MOTOR_DIRECTION_FORWARD			0
#define MOTOR_DIRECTION_BACKWARD		1
uint8_t g_motor_left_direction;
uint8_t g_motor_right_direction;

#define MOTOR_LEFT_ROTATION			0
#define MOTOR_RIGHT_ROTATION			1
uint8_t g_motor_rotation;

#define ACTION_RUN_INIT				1
#define ACTION_ROT_INIT				2
#define ACTION_BACK_INIT			3
#define ACTION_RUN				4
#define ACTION_STOP				5
#define ACTION_DETECTED				6
#define ACTION_END				7
#define ACTION_PLAY_NOTE			8
uint8_t g_action;

#define MOTOR_UNIT_CM				0
#define MOTOR_UNIT_COUNT			1
#define MOTOR_UNIT_DEGREES			2
uint8_t g_motor_unit;

uint8_t g_motor_speed;
uint8_t g_motor_distance;
uint16_t g_motor_curr_dist_right;
uint16_t g_motor_curr_dist_right_old;
uint16_t g_motor_curr_dist_left;
uint16_t g_motor_curr_dist_left_old;
uint8_t g_motor_degrees;

uint8_t g_action_detection_left;
uint8_t g_action_detection_right;
uint8_t g_action_detection_left_old;
uint8_t g_action_detection_right_old;
uint8_t g_action_temperature;
uint8_t g_action_note;
uint8_t g_action_analog1;
uint8_t g_start;
uint8_t g_file_number_max;
uint8_t g_file_number;
uint8_t g_file_name[CMD_RECV_DATA_MAX];

uint8_t g_recv_motor_nb;
uint8_t g_recv_sound_nb;

#define MOTOR_ROTATE_SPEED			100

#define PLAY_ONE_TIME				0
#define PLAY_REPEAT				1

enum note_e
{
    NOTE_DO = 0,
    NOTE_RE,
    NOTE_MI,
    NOTE_FA,
    NOTE_SO,
    NOTE_LA,
    NOTE_SI,
    NOTE_END
};


void setup()
{
    /* Initialize the buttons input pin */
    pinMode(PIN_LEFT_BUTTON, INPUT);
    pinMode(PIN_RIGHT_BUTTON, INPUT);
    pinMode(PIN_MIDDLE_BUTTON, INPUT);
    pinMode(PIN_UP_BUTTON, INPUT);
    pinMode(PIN_DOWN_BUTTON, INPUT);

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
    g_menu_tree_idx[g_menu_tree_level] = g_menu_idx;

    g_send_motor[0] = 0;
    g_recv_motor[0] = 0;
    g_send_sound[0] = 0;
    g_recv_sound[0] = 0;

    /* init process states */
    g_process_menu = 0;
    g_process_mother_action = 0;
    g_process_motor_action = 0;
    g_process_sound_action = 0;
    g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;
    g_process_recv_sound = PROCESS_RECV_SOUND_WAIT_COMMAND;
    g_process_motor = 0;
    g_process_sound = 0;

    /* Detection */
    g_action_detection_left_old  = 2;
    g_action_detection_right_old = 2;
    g_action_detection_left   = 0;
    g_action_detection_right  = 0;

    /* Global stuff*/
    g_action_note = 0;
    g_action_temperature = 0;
    g_action_analog1	 = 0;
    g_motor_speed	 = 120;
    g_motor_left_direction = 0;
    g_motor_right_direction = 0;
    g_motor_rotation	= 0;
    g_motor_distance	= 0;
    g_motor_degrees	= 180;
    g_motor_unit	= 0;
    g_motor_curr_dist_right	= 0;
    g_motor_curr_dist_right_old	= 0;
    g_motor_curr_dist_left	= 0;
    g_motor_curr_dist_left_old	= 0;
    g_file_number_max		= 255;
    g_file_number		= 0;

    g_start		= 0;

    g_action = 0;


    /* set up the LCD's number of columns and rows: */
    g_lcd_col	= 0;
    g_lcd_line	= 0;
    g_lcd.begin(16, 2);

    attachInterrupt(0, interrupt_call, FALLING);

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
/*  Start ->       */
/*  Cmd   ->       */
/*  Data1 ->       */
/*  Data2 ->       */
/*  Data3 ->       */
/*  Data4 ->       */
void send_motor(uint8_t *buffer, int len)
{
    uint8_t padding[CMD_DATA_MAX] = {0,0,0,0,0,0};

    if (len > CMD_DATA_MAX)
	len = CMD_DATA_MAX;

    /* Send Start of transmission */
    Serial.write(MOTOR_SEND_COMMAND_START);

    /* Write Command + Data */
    Serial.write(buffer, len);

    /* Write padding Data */
    Serial.write(padding, CMD_DATA_MAX - len);
}

/* Mother -> Sound */
/*  Start ->       */
/*  Cmd   ->       */
/*  Data1 ->       */
/*  Data2 ->       */
/*  Data3 ->       */
/*  Data4 ->       */
void send_sound(uint8_t *buffer, int len)
{
    uint8_t padding[CMD_DATA_MAX] = {0,0,0,0,0,0};

    if (len > CMD_DATA_MAX)
	len = CMD_DATA_MAX;

    /* Send Start of transmission */
    Serial1.write(SOUND_SEND_COMMAND_START);

    /* Write Command + Data */
    Serial1.write(buffer, len);

    /* Write padding Data */
    Serial1.write(padding, CMD_DATA_MAX - len);
}


void motor_forward(uint8_t distance, uint8_t speed)
{
    play_motor(PLAY_REPEAT);

    g_motor_left_direction	= MOTOR_DIRECTION_FORWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = MOTOR_SEND_COMMAND_FORWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    send_motor(g_send_motor, 3);
}

void motor_rotate(uint8_t degrees)
{
    g_motor_rotation		= MOTOR_LEFT_ROTATION;

    g_send_motor[0] = MOTOR_SEND_COMMAND_ROTATE_LEFT;
    g_send_motor[1] = MOTOR_ROTATE_SPEED;
    g_send_motor[2] = degrees;
    send_motor(g_send_motor, 3);
}


void motor_backward(uint8_t distance, uint8_t speed)
{
    g_motor_left_direction	= MOTOR_DIRECTION_BACKWARD;
    g_motor_speed		= speed;
    g_motor_distance		= distance;

    g_send_motor[0] = MOTOR_SEND_COMMAND_BACKWARD;
    g_send_motor[1] = g_motor_speed;
    g_send_motor[2] = g_motor_distance;
    send_motor(g_send_motor, 3);
}

void go_up_menu(void)
{

    if (g_menu_tree_level > 0)
    {
	g_menu_tree_level--;

	/* get saved idx */
	g_menu_idx = g_menu_tree_idx[g_menu_tree_level];
	g_menu = g_menu_tree[g_menu_tree_level];

	/* Play beep */
	beep(PLAY_ONE_TIME);

	/* clear LCD before displaying new menu or action */
	g_lcd.clear();
	g_lcd.print(g_menu[g_menu_idx].name);
    }

    g_process_menu = 1;

    /* end of threatment, re-enable the button for interrupt */
    g_button = NO_BUTTON;

}

void beep(uint8_t type)
{
    g_send_sound[0] = SOUND_SEND_COMMAND_BEEP_KEY;
    g_send_sound[1] = type;
    send_sound(g_send_sound, 2);
}

void play_motor(uint8_t type)
{
    g_send_sound[0] = SOUND_SEND_COMMAND_MOTOR;
    g_send_sound[1] = type;
    send_sound(g_send_sound, 2);
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

		/* Play beep */
		beep(PLAY_ONE_TIME);

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

		/* Play beep */
		beep(PLAY_ONE_TIME);

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

	    /* save current idx */
	    g_menu_tree_idx[g_menu_tree_level] = g_menu_idx;

	    /* set lower menu to first item */
	    g_menu_idx = 0;

	    /* increase tree level */
	    g_menu_tree_level++;

	    /* Play beep */
	    beep(PLAY_ONE_TIME);

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case UP_BUTTON:
	{
	    g_button_selected = g_button;

	    go_up_menu();
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
		    g_action_detection_left_old  = 2;
		    g_action_detection_right_old = 2;
		    g_action_detection_left   = 0;
		    g_action_detection_right  = 0;

		    g_process_motor_action = PROCESS_ACTION_DETECTION;

		    g_send_motor[0]  = MOTOR_SEND_COMMAND_DETECTION;
		    send_motor(g_send_motor, 1);

		    g_process_menu = 0;
		}break;
		case MENU_ACTION_TEMPERATURE:
		{
		    g_process_mother_action = PROCESS_MOTHER_ACTION_TEMPERATURE;
		    g_action_temperature = 0;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_ANALOG:
		{
		    g_process_mother_action = PROCESS_MOTHER_ACTION_ANALOG;
		    g_action_analog1 = 0;
		    g_process_menu = 0;
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

		    g_process_motor_action = PROCESS_ACTION_SPEED;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_UNIT:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_unit[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    g_lcd.setCursor(0, 1);
		    if (g_motor_unit == MOTOR_UNIT_COUNT)
			g_lcd.print("Unit = Count  ");
		    else  if (g_motor_unit == MOTOR_UNIT_DEGREES)
			g_lcd.print("Unit = Degrees");
		    else
			g_lcd.print("Unit = Cm     ");

		    g_process_motor_action = PROCESS_ACTION_UNIT;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_DIR:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_dir[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    g_lcd.setCursor(0, 1);
		    if (g_motor_left_direction == MOTOR_DIRECTION_BACKWARD)
		    {
			g_lcd.print("Dir : Backward  ");
		    }
		    else
		    {
			g_lcd.print("Dir : Forward   ");
		    }

		    g_process_motor_action = PROCESS_ACTION_DIR;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_ROT:
		{
		    g_menu_tree[g_menu_tree_level] = &g_menu_rot[0];
		    g_menu = g_menu_tree[g_menu_tree_level];

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu[g_menu_idx].name);

		    g_lcd.setCursor(0, 1);
		    if (g_motor_rotation == MOTOR_LEFT_ROTATION)
		    {
			g_lcd.print("Rotation Left   ");
		    }
		    else
		    {
			g_lcd.print("Rotation Right  ");
		    }

		    g_process_motor_action = PROCESS_ACTION_ROT;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_OBSTACLE:
		{
		    g_process_motor_action = 0; /*PROCESS_ACTION_OBSTACLE;*/
		}break;
		case MENU_ACTION_RUN:
		{
		    delay(1000);
		    g_action = ACTION_RUN_INIT;

		    g_process_motor_action = PROCESS_ACTION_RUN;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_ROTATE:
		{
		    delay(1000);
		    g_action = ACTION_ROT_INIT;

		    g_process_motor_action = PROCESS_ACTION_RUN;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_SQUARE:
		{
		    delay(1000);
		    g_process_motor_action = PROCESS_ACTION_SQUARE;
		    g_process_menu = 0;
		}break;
		case MENU_ACTION_DETECT_BACK:
		{
		    delay(1000);
		    g_process_motor_action = PROCESS_ACTION_DETECT_BACK;
		    g_process_menu = 0;

		    g_action = ACTION_RUN;
		}break;
		case MENU_ACTION_LIST:
		{
		    g_send_sound[0] = SOUND_SEND_COMMAND_FILENAME;
		    g_send_sound[1] = g_file_number;
		    send_sound(g_send_sound, 2);

		    g_lcd.clear();

		    g_process_menu = 0;
		}break;
		case MENU_ACTION_PLAY_NOTE:
		{
		    g_process_sound_action |= PROCESS_ACTION_PLAY_NOTE;
		    g_process_menu = 0;

		    g_action = ACTION_PLAY_NOTE;
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

/********************************************************************************************************/
/*													*/
/*				Mother									*/
/*													*/
/********************************************************************************************************/
void process_mother_action(void)
{
    int value;
    int analog_value;

    if (g_process_mother_action)
    {
	if (g_process_mother_action == PROCESS_MOTHER_ACTION_TEMPERATURE)
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
		go_up_menu();

		g_process_mother_action = 0;
		g_action_temperature = 0;
	    }
	    else
	    {
		g_button = NO_BUTTON;
	    }
	}
	else if (g_process_mother_action == PROCESS_MOTHER_ACTION_ANALOG)
	{
	    analog_value = analogRead(PIN_ANALOG1);

	    if (analog_value != g_action_analog1 )
	    {
		g_lcd.setCursor(0, 1);
		g_lcd.print("Analog = ");
		g_lcd.print(analog_value);
		g_lcd.print("    ");
		g_action_analog1 = analog_value;
		delay(500);
	    }

	    if (g_button == UP_BUTTON)
	    {
		go_up_menu();

		g_process_mother_action = 0;
		g_action_analog1 = 0;
	    }
	    else
	    {
		g_button = NO_BUTTON;
	    }
	}
    }
}

/********************************************************************************************************/
/*													*/
/*				Motor									*/
/*													*/
/********************************************************************************************************/

/* Read Serial from motor only when last message has been treaten */
void process_recv_motor(void)
{
    uint8_t value;

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
		while (Serial.available() < CMD_DATA_MAX);

		for(g_recv_motor_nb = 0; g_recv_motor_nb < CMD_DATA_MAX; g_recv_motor_nb++)
		{
		    /* get incoming write: */
		    g_recv_motor[g_recv_motor_nb] = Serial.read();
		}
		/* Set action plan */
		g_process_motor = g_recv_motor[0];

		/* Disable communication ,wait for message treatment */
		g_process_recv_motor   = 0;
	    }
	}
    }
}

/* execute action of message received from motor board */
void process_motor(void)
{
    if (g_process_motor)
    {
	if (g_process_motor == PROCESS_MOTOR_COUNTERS)
	{
	    g_motor_curr_dist_left = (g_recv_motor[1] << 8) + (g_recv_motor[2]);
	    g_motor_curr_dist_right = (g_recv_motor[3] << 8) + (g_recv_motor[4]);
	}
	else if (g_process_motor == PROCESS_MOTOR_STOP)
	{
	    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
	    send_sound(g_send_sound, 1);

	    g_action = ACTION_STOP;
	}
	else if (g_process_motor == PROCESS_MOTOR_RUNNING_DETECTION)
	{
	    g_action_detection_left  = g_recv_motor[1];
	    g_action_detection_right = g_recv_motor[2];

	    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
	    send_sound(g_send_sound, 1);

	    g_action = ACTION_DETECTED;
	}
	else if (g_process_motor == PROCESS_MOTOR_DETECT)
	{
	    g_action_detection_left  = g_recv_motor[1];
	    g_action_detection_right = g_recv_motor[2];

	    if (g_action_detection_left < 30 )
	    {
		if (g_action_detection_left_old != g_action_detection_left)
		{
		    g_lcd.setCursor(0, 0);
		    g_lcd.print("L Obstacle !  ");
		    g_lcd.print(g_action_detection_left);g_lcd.print("  ");
		}
	    }
	    else
	    {
		if (g_action_detection_left_old != g_action_detection_left)
		{
		    g_lcd.setCursor(0, 0);
		    g_lcd.print("L Detecting.. ");
		    g_lcd.print(g_action_detection_left);g_lcd.print("  ");
		}
	    }
	    g_action_detection_left_old = g_action_detection_left;

	    if (g_action_detection_right < 30 )
	    {
		if (g_action_detection_right_old != g_action_detection_right)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("R Obstacle !  ");
		    g_lcd.print(g_action_detection_right);g_lcd.print("  ");
		}
	    }
	    else
	    {
		if (g_action_detection_right_old != g_action_detection_right)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("R Detecting.  ");
		    g_lcd.print(g_action_detection_right);g_lcd.print("  ");
		}
	    }
	    g_action_detection_right_old = g_action_detection_right;

	    g_process_motor_action = PROCESS_ACTION_DETECTION;
	}
	else if (g_process_motor == PROCESS_MOTOR_READY)
	{
	    g_lcd.setCursor(8, 0);
	    g_lcd.print("Ready !");
	    g_start++;

	    if (g_start == 2)
	    {
		delay(2000);
		g_process_menu = 1;
		g_lcd.clear();
		g_lcd.print(g_menu[0].name);

	    }
	}

	/* Message has been read, restart read serial */
	g_process_recv_motor = PROCESS_RECV_MOTOR_WAIT_COMMAND;

	/* Current process is treated */
	g_process_motor = 0;
    }
}

/* Running in continue to treat some action required by motor board */
void process_motor_action(void)
{
    if (g_process_motor_action)
    {
	if (g_process_motor_action == PROCESS_ACTION_DETECTION)
	{
	    /* If we want to quit this mode, then display menu */
	    if (g_button == UP_BUTTON)
	    {
		/* Disable detection */
		g_send_motor[0]  = MOTOR_SEND_COMMAND_DETECTION;
		send_motor(g_send_motor, 1);

		go_up_menu();
		g_process_motor_action = 0;
	    }
	    else
	    {
		g_button = NO_BUTTON;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_SPEED)
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
		    g_lcd.print("Stop !          ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    go_up_menu();

		    g_process_motor_action = 0;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_UNIT)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case RIGHT_BUTTON:
		{
		    g_motor_unit = MOTOR_UNIT_CM;
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Unit = Cm     ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case LEFT_BUTTON:
		{
		    g_motor_unit = MOTOR_UNIT_DEGREES;
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Unit = Degrees");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case MIDDLE_BUTTON:
		{
		    g_motor_unit = MOTOR_UNIT_COUNT;
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Unit = Count  ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    go_up_menu();

		    g_process_motor_action = 0;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_DIR)
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
		    go_up_menu();

		    g_process_motor_action = 0;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_ROT)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case LEFT_BUTTON:
		{
		    g_motor_rotation = MOTOR_LEFT_ROTATION;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Rotation Left  ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case RIGHT_BUTTON:
		{
		    g_motor_rotation = MOTOR_RIGHT_ROTATION;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Rotation Right  ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;

		case MIDDLE_BUTTON:
		{
		    if(g_motor_degrees <= 250)
			g_motor_degrees += 5;
		    g_lcd.setCursor(0, 0);
		    g_lcd.print(g_motor_degrees); g_lcd.print(" Dg    ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case DOWN_BUTTON:
		{
		    if(g_motor_degrees >= 5)
			g_motor_degrees -= 5;
		    g_lcd.setCursor(0, 0);
		    g_lcd.print(g_motor_degrees); g_lcd.print(" Dg    ");

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    go_up_menu();

		    g_process_motor_action = 0;
		}
		break;
		default:
		{
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}break;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_RUN)
	{
	    if (g_action == ACTION_RUN_INIT)
	    {
		if (g_motor_left_direction == MOTOR_DIRECTION_FORWARD)
		{
		    g_send_motor[0] = MOTOR_SEND_COMMAND_FORWARD;
		    play_motor(PLAY_REPEAT);
		}
		else
		{
		    g_send_motor[0] = MOTOR_SEND_COMMAND_BACKWARD;
		}

		g_lcd.clear();
		g_lcd.setCursor(0, 0);
		g_lcd.print("S "); g_lcd.print(g_motor_speed);
		g_lcd.print(" D "); g_lcd.print(g_motor_distance);
		if (g_motor_unit == MOTOR_UNIT_CM)
		    g_lcd.print("cm  ");

		g_send_motor[1] = g_motor_speed;
		g_motor_distance = 15;
		g_send_motor[2] = g_motor_distance;
		send_motor(g_send_motor, 3);

		g_action = ACTION_RUN;
	    }
	    else if (g_action == ACTION_ROT_INIT)
	    {
		g_motor_unit = MOTOR_UNIT_DEGREES;

		if (g_motor_rotation == MOTOR_LEFT_ROTATION)
		{
		    g_send_motor[0] = MOTOR_SEND_COMMAND_ROTATE_LEFT;
		}
		else
		{
		    g_send_motor[0] = MOTOR_SEND_COMMAND_ROTATE_RIGHT;
		}

		g_lcd.clear();
		g_lcd.setCursor(0, 0);
		g_lcd.print("Speed "); g_lcd.print(MOTOR_ROTATE_SPEED);
		g_lcd.print(" Deg "); g_lcd.print(g_motor_degrees);

		g_send_motor[1]  = MOTOR_ROTATE_SPEED;
		g_send_motor[2]  = g_motor_degrees;
		send_motor(g_send_motor, 3);

		g_action = ACTION_RUN;
	    }
	    else if (g_action == ACTION_RUN)
	    {
		if (g_button == UP_BUTTON)
		{
		    /* send stop to motor */
		    g_send_motor[0] = MOTOR_SEND_COMMAND_STOP;
		    send_motor(g_send_motor, 1);

		    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
		    send_sound(g_send_sound, 1);

		    go_up_menu();

		    g_motor_curr_dist_left = 0;
		    g_motor_curr_dist_right = 0;
		    g_process_motor_action = 0;
		    g_action = 0;
		}
		else
		{
		    g_button = NO_BUTTON;

		    if ((g_motor_curr_dist_left != g_motor_curr_dist_left_old) ||
			(g_motor_curr_dist_right != g_motor_curr_dist_right_old))
		    {
			g_lcd.setCursor(0, 1);
			g_lcd.print("D "); g_lcd.print(g_motor_curr_dist_left);
			g_lcd.print("<->"); g_lcd.print(g_motor_curr_dist_right);
			if (g_motor_unit == MOTOR_UNIT_CM)
			    g_lcd.print("cm  ");

			g_motor_curr_dist_left_old = g_motor_curr_dist_left;
			g_motor_curr_dist_right_old = g_motor_curr_dist_right;
		    }

		    if (g_motor_unit == MOTOR_UNIT_CM)
		    {
			g_send_motor[0] = MOTOR_SEND_COMMAND_COUNTERS_CM;
		    }
		    else if (g_motor_unit == MOTOR_UNIT_DEGREES)
		    {
			g_send_motor[0] = MOTOR_SEND_COMMAND_COUNTERS_DEG;
		    }
		    else
		    {
			g_send_motor[0] = MOTOR_SEND_COMMAND_COUNTERS;
		    }

		    delay(200);
		    send_motor(g_send_motor, 1);
		}
	    }
	    else if ((g_action == ACTION_STOP) || (g_action == ACTION_DETECTED))
	    {
		g_lcd.setCursor(0, 0);
		if (g_action == ACTION_STOP)
		    g_lcd.print("Stop !          ");
		else
		    g_lcd.print("Detection !     ");

		g_action = ACTION_END;
	    }
	    else if (g_action == ACTION_END)
	    {
		if (g_button == UP_BUTTON)
		{
		    go_up_menu();

		    g_motor_curr_dist_left = 0;
		    g_motor_curr_dist_right = 0;
		    g_process_motor_action = 0;
		    g_action = 0;
		}
		else
		    g_button = NO_BUTTON;
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_ROTATE)
	{
	    if (g_button == UP_BUTTON)
	    {
		go_up_menu();

		g_process_motor_action = 0;
		g_action = 0;
	    }
	    else
		g_button = NO_BUTTON;

	    if (g_action == ACTION_ROT_INIT)
	    {
		motor_rotate(180);
	    }
	}
	else if (g_process_motor_action == PROCESS_ACTION_DETECT_BACK)
	{
	    if (g_button == UP_BUTTON)
	    {
		go_up_menu();

		g_process_motor_action = 0;
		g_action = 0;
	    }
	    else
		g_button = NO_BUTTON;

	    if (g_action == ACTION_RUN)
	    {
		delay(500);
		/* move slowly until reach obstacle */
		motor_forward(0,g_motor_speed);

		/* no action, wait for detection */
		g_action = 0;
	    }
	    else if (g_action == ACTION_DETECTED)
	    {
		delay(500);
		motor_rotate(180);

		/* no action, wait for detection */
		g_action = 0;
	    }
	    else if (g_action == ACTION_STOP)
	    {
		g_action = ACTION_RUN;
	    }
	}
    }
}

/********************************************************************************************************/
/*													*/
/*				Sound									*/
/*													*/
/********************************************************************************************************/


void process_recv_sound(void)
{
    uint8_t value;

    if (g_process_recv_sound)
    {
	/* if we get a valid char, read char */
	if (Serial1.available() > 0)
	{
	    /* get Start Byte */
	    value = Serial1.read();
	    if (value == PROCESS_SOUND_START)
	    {
		/* Wait for serial */
		while (Serial1.available() < CMD_RECV_DATA_MAX);

		for(g_recv_sound_nb = 0; g_recv_sound_nb < CMD_RECV_DATA_MAX; g_recv_sound_nb++)
		{
		    /* get incoming write: */
		    g_recv_sound[g_recv_sound_nb] = Serial1.read();
		}
		/* Set action plan */
		g_process_sound = g_recv_sound[0];

		/* Disable communication ,wait for message treatment */
		g_process_recv_sound   = 0;
	    }
	}
    }
}

void process_sound(void)
{
    if (g_process_sound)
    {
	if (g_process_sound == PROCESS_SOUND_PLAYING_FILE)
	{

	}
	else if (g_process_sound == PROCESS_SOUND_PLAY_END)
	{

	}
	else if (g_process_sound == PROCESS_SOUND_FILE_NAME)
	{
	    strcpy((char*)g_file_name,(char*)&g_recv_sound[1]);

	    g_lcd.setCursor(0, 0);
	    if (g_file_number > 0)
		g_lcd.print("< File : ");
	    else
		g_lcd.print("  File : ");
	    g_lcd.print(g_file_number);
	    g_lcd.print("/");
	    g_lcd.print(g_file_number_max);
	    g_lcd.print("     ");
	    g_lcd.setCursor(15, 0);
	    if (g_file_number < g_file_number_max)
		g_lcd.print(">");

	    g_lcd.setCursor(0, 1);
	    if (g_file_name[0] != '\0')
	    {
		g_lcd.print((char*)g_file_name);
		g_lcd.print("         ");
	    }

	    g_process_sound_action |= PROCESS_ACTION_LIST;
	}
	else if (g_process_sound == PROCESS_SOUND_FILE_NUMBER)
	{
	    g_file_number_max = g_recv_sound[1];
	}
	else if ((g_process_sound == PROCESS_SOUND_READY) ||
	    (g_process_sound == PROCESS_SOUND_INIT_ERROR) ||
	    (g_process_sound == PROCESS_SOUND_INIT_FAT_ERROR) ||
	    (g_process_sound == PROCESS_SOUND_INIT_ROOT_ERROR))
	{
	    g_lcd.setCursor(8, 1);
	    if (g_process_sound == PROCESS_SOUND_READY)
	    {
		g_lcd.print("Ready !");

		/* Get max file number */
		g_send_sound[0] = SOUND_SEND_COMMAND_LIST;
		send_sound(g_send_sound, 1);
	    }
	    else if (g_process_sound == PROCESS_SOUND_INIT_ERROR)
		g_lcd.print("Error !");
	    else if (g_process_sound == PROCESS_SOUND_INIT_FAT_ERROR)
		g_lcd.print("FAT Err");
	    else
		g_lcd.print("Root Er");

	    g_start++;

	    if (g_start == 2)
	    {
		delay(2000);
		g_process_menu = 1;
		g_lcd.clear();
		g_lcd.print(g_menu[0].name);
	    }
	}

	/* Message has been read, restart read serial */
	g_process_recv_sound = PROCESS_RECV_SOUND_WAIT_COMMAND;

	/* Current process is over */
	g_process_sound = 0;
    }
}

void process_sound_action(void)
{
    int analog_value;

    if (g_process_sound_action)
    {
	if ((g_process_sound_action & PROCESS_ACTION_LIST) == PROCESS_ACTION_LIST)
	{
	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case LEFT_BUTTON:
		{
		    if (g_file_number > 0)
			g_file_number--;

		    g_send_sound[0] = SOUND_SEND_COMMAND_FILENAME;
		    g_send_sound[1] = g_file_number;
		    send_sound(g_send_sound, 2);

		    g_process_sound_action &= ~PROCESS_ACTION_LIST;

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case RIGHT_BUTTON:
		{
		    if (g_file_number < g_file_number_max)
			g_file_number++;

		    g_send_sound[0] = SOUND_SEND_COMMAND_FILENAME;
		    g_send_sound[1] = g_file_number;
		    send_sound(g_send_sound, 2);

		    g_process_sound_action &= ~PROCESS_ACTION_LIST;

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case MIDDLE_BUTTON:
		{
		    g_send_sound[0] = SOUND_SEND_COMMAND_PLAYFILE;
		    g_send_sound[1] = g_file_number;
		    send_sound(g_send_sound, 2);

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		case UP_BUTTON:
		{
		    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
		    send_sound(g_send_sound, 1);

		    go_up_menu();
		    g_process_sound_action &= ~PROCESS_ACTION_LIST;
		}
		break;
		case DOWN_BUTTON:
		{
		    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
		    send_sound(g_send_sound, 1);

		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		}
		break;
		default:
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		break;
	    }
	}
	else if ((g_process_sound_action & PROCESS_ACTION_PLAY_NOTE) == PROCESS_ACTION_PLAY_NOTE)
	{
	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE1);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_DO;
		send_sound(g_send_sound, 2);
	    }

	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE2);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_RE;
		send_sound(g_send_sound, 2);
	    }

	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE3);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_FA;
		send_sound(g_send_sound, 2);
	    }

	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE4);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_SO;
		send_sound(g_send_sound, 2);
	    }

	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE5);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_LA;
		send_sound(g_send_sound, 2);
	    }

	    /* Read Analog part */
	    analog_value = analogRead(PIN_NOTE6);

	    if (analog_value < 30)
	    {
		g_send_sound[0] = SOUND_SEND_COMMAND_NOTE;
		g_send_sound[1] = NOTE_SI;
		send_sound(g_send_sound, 2);
	    }

	    delay(100);

	    /* Check which Button is hold */
	    switch (g_button)
	    {
		case UP_BUTTON:
		{
		    g_send_sound[0] = SOUND_SEND_COMMAND_STOP_PLAYING;
		    send_sound(g_send_sound, 1);

		    go_up_menu();
		    g_process_sound_action &= ~PROCESS_ACTION_PLAY_NOTE;
		}
		break;
		default:
		    /* end of threatment, re-enable the button for interrupt */
		    g_button = NO_BUTTON;
		break;
	    }
	}
    }
}

void loop(void)
{
    process_menu();
    process_recv_motor();
    process_recv_sound();
    process_motor();
    process_motor_action();
    process_sound();
    process_sound_action();
    process_mother_action();
}
