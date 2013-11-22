#include <LiquidCrystal.h>
#include <Stepper.h>


/********************************************************/
/*      Buttons definitions                             */
/********************************************************/
#define NO_BUTTON	0x00
#define LEFT_BUTTON	0x01
#define MID_BUTTON	0x02
#define RIGHT_BUTTON	0x04

volatile int g_button;
int g_button_selected;


/********************************************************/
/*      LCD definitions                                 */
/********************************************************/
LiquidCrystal g_lcd(13, 12, 7, 6, 5, 3);
int g_lcd_col;
int g_lcd_line;

/********************************************************/
/*      Motors definitions                              */
/********************************************************/
Stepper g_motor_left(400, 8, 9, 10, 11);
#define MOTOR_LEFT_SPEED	48
int g_motor_left_speed;
int g_motor_left_count;

/* Stepper g_motor_right(400, 8, 9, 10, 11); */
/* #define MOTOR_RIGHT_SPEED	50 */

/********************************************************/
/*      Menus definitions                               */
/********************************************************/
#define MENU_MAIN		0
#define MENU_MOTOR		1
#define MENU_PROG		2
#define MENU_MOVE		3
#define MENU_DIR		4
int g_menu_selected;

#define MENU_MAIN_DETECT	0
#define MENU_MAIN_MOTOR		1
#define MENU_MAIN_TEMP		2
#define MENU_MAIN_PROG		3
#define MENU_MAIN_END		4
const char *g_menu_main[] = { "Detection       ", "Moteur          ", "Temperature     ", "Program         " };
int g_menu_main_index;

#define MENU_MOTOR_MOVE		0
#define MENU_MOTOR_DIR		1
#define MENU_MOTOR_OBSTACLE	2
#define MENU_MOTOR_MAIN		3
#define MENU_MOTOR_END		4
const char *g_menu_motor[] = { "Move            ", "Dir             ", "Obstacle        ", "Main Menu       " };
int g_menu_motor_index;

#define MENU_SPEED_PLUS		0
#define MENU_SPEED_STOP		1
#define MENU_SPEED_MINUS	2
const char *g_menu_speed[] = { "  +    STOP   - " };
int g_cmd_speed_index;

#define MENU_DIR_REVERSE	0
#define MENU_DIR_EXIT		1
#define MENU_DIR_FORWARD	2
const char *g_menu_dir[] = { "Rever Exit Forwd" };
int g_cmd_dir_index;

#define MENU_PROG_1		0
#define MENU_PROG_2		1
#define MENU_PROG_3		2
#define MENU_PROG_MAIN		3
#define MENU_PROG_END		4
const char *g_menu_prog[] = { "Program 1       ", "Program 2       ", "Program 3       ", "Main menu       " };
int g_menu_prog_index;

/********************************************************/
/*      Actions definitions                             */
/********************************************************/
#define ACTION_NONE		0
#define ACTION_MOVE_REVERSE	1
#define ACTION_MOVE_STOP	2
#define ACTION_MOVE_FORWARD	3
#define ACTION_MOTOR_OBSTACLE	4
#define ACTION_MISC_TEMPERATURE	5
#define ACTION_MISC_DETECTION	6
#define ACTION_PROG_1		7
#define ACTION_PROG_2		8
#define ACTION_PROG_3		9

int g_action;
int g_action_detection;
int g_action_temperature;


void setup()
{
    /* set up the LCD's number of columns and rows: */
    g_lcd.begin(16, 2);

    /* Set the Stepper Speed */
    g_motor_left_speed = MOTOR_LEFT_SPEED;
    g_motor_left.setSpeed(g_motor_left_speed);
    g_motor_left_count = 0;

    /* initialize the serial communications: */
    Serial.begin(115200);

    /* Initialize the buttons input pin */
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);

    /* Initialize the sensor input pin */
    pinMode(A5, INPUT);

    /* Init interrupt for the 3 buttons */
    attachInterrupt(0, interrupt_call, FALLING);

    /* Init global variables */
    g_menu_main_index = 0;
    g_menu_motor_index = 0;
    g_cmd_speed_index = MENU_SPEED_STOP;
    g_cmd_dir_index = MENU_DIR_EXIT;
    g_menu_prog_index = 0;
    g_button = NO_BUTTON;
    g_lcd_col	= 0;
    g_lcd_line	= 0;

    g_action  = ACTION_NONE;
    g_action_detection	 = 2;
    g_action_temperature = 0;

    g_lcd.print("----> Ready !!!!");
    delay(2000);
    g_lcd.clear();
    g_lcd.print(g_menu_main[MENU_MAIN_DETECT]);
    g_menu_selected = MENU_MAIN;
}

void interrupt_call(void)
{
    /* Do not set button if last button state is not threated */
    if (g_button == NO_BUTTON)
    {
	if (digitalRead(A0) == 1 )
	    g_button = LEFT_BUTTON;
	else if (digitalRead(A1) == 1 )
	    g_button = MID_BUTTON;
	else if (digitalRead(A2) == 1 )
	    g_button = RIGHT_BUTTON;
    }
}

void gestion_menu(int *menu_index, int menu_end)
{
    int idx;

    idx = *menu_index;
    g_button_selected = NO_BUTTON;

    /* Check which Button is hold */
    switch (g_button)
    {
	case LEFT_BUTTON:
	{
	    g_button_selected = g_button;
	    if (idx == 0)
		idx = menu_end - 1;
	    else
		idx--;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case RIGHT_BUTTON:
	{
	    g_button_selected = g_button;

	    if (idx == (menu_end - 1))
		idx = 0;
	    else
		idx++;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	case MID_BUTTON:
	{
	    g_button_selected = g_button;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	break;
	default :
	break;
    }
    /* save selection */
    *menu_index = idx;
}

void process_menu(void)
{
    if (g_menu_selected == MENU_MAIN)
    {
	gestion_menu(&g_menu_main_index, MENU_MAIN_END);
	switch (g_button_selected)
	{
	    case MID_BUTTON:
	    {
		switch (g_menu_main_index)
		{
		    case MENU_MAIN_DETECT:
		    {
			g_action = ACTION_MISC_DETECTION;
			g_action_detection   = 2;
		    }
		    break;
		    case MENU_MAIN_MOTOR:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_MOTOR;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_motor[g_menu_motor_index]);
		    }
		    break;
		    case MENU_MAIN_TEMP:
		    {
			g_action = ACTION_MISC_TEMPERATURE;
			g_action_temperature = 0;
		    }
		    break;
		    case MENU_MAIN_PROG:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_PROG;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_prog[g_menu_prog_index]);
		    }
		    break;
		}
	    }
	    break;
	    case LEFT_BUTTON:
	    case RIGHT_BUTTON:
	    {
		g_action = ACTION_NONE;
		g_lcd.clear();
		g_lcd.print(g_menu_main[g_menu_main_index]);
	    }
	    break;
	    default :
	    break;
	}
    }

    else if (g_menu_selected == MENU_MOTOR)
    {
	gestion_menu(&g_menu_motor_index, MENU_MOTOR_END);
	switch (g_button_selected)
	{
	    case MID_BUTTON:
	    {
		switch (g_menu_motor_index)
		{
		    case MENU_MOTOR_MOVE:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_MOVE;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_speed[0]);
			g_lcd.setCursor(0, 1);
			g_lcd.print("Speed L = ");g_lcd.print(g_motor_left_speed);
			/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */
		    }
		    break;
		    case MENU_MOTOR_DIR:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_DIR;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_dir[0]);
			g_lcd.setCursor(0, 1);
			g_lcd.print("Speed L = ");g_lcd.print(g_motor_left_speed);
			/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */
		    }
		    break;
		    case MENU_MOTOR_OBSTACLE:
		    {
			g_action = ACTION_MOTOR_OBSTACLE;
			g_action_detection    = 2;
		    }
		    break;
		    case MENU_MOTOR_MAIN:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_MAIN;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_main[g_menu_main_index]);
		    }
		    break;
		}
	    }
	    break;
	    case LEFT_BUTTON:
	    case RIGHT_BUTTON:
	    {
		g_action = ACTION_NONE;
		g_lcd.clear();
		g_lcd.print(g_menu_motor[g_menu_motor_index]);
	    }
	    break;
	    default :
	    break;
	}
    }
    else if (g_menu_selected == MENU_MOVE)
    {
	/* Check which Button is hold */
	switch (g_button)
	{
	    case LEFT_BUTTON:
	    {
		g_motor_left_speed += 2;
		g_motor_left.setSpeed(g_motor_left_speed);
		g_lcd.setCursor(0, 1);
		g_lcd.print("Speed L = ");g_lcd.print(g_motor_left_speed);
		/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    break;
	    case RIGHT_BUTTON:
	    {
		g_motor_left_speed -= 2;
		g_motor_left.setSpeed(g_motor_left_speed);
		g_lcd.setCursor(0, 1);
		g_lcd.print("Speed L = ");g_lcd.print(g_motor_left_speed);
		/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    break;
	    case MID_BUTTON:
	    {
		if (g_action != ACTION_MOVE_STOP)
		{
		    g_action = ACTION_MOVE_STOP;

		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Stop");
		}
		else
		{
		    g_action = ACTION_NONE;

		    /* in this case, there's an other menu */
		    g_menu_selected = MENU_MOTOR;

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.print(g_menu_motor[g_menu_motor_index]);
		}

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    break;
	}
    }
    else if (g_menu_selected == MENU_DIR)
    {
	/* Check which Button is hold */
	switch (g_button)
	{
	    case LEFT_BUTTON:
	    {
		g_action = ACTION_MOVE_REVERSE;

		g_motor_left.setSpeed(g_motor_left_speed);
		g_lcd.setCursor(0, 1);
		g_lcd.print("Reverse");
		/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;

	    }
	    break;
	    case RIGHT_BUTTON:
	    {
		g_action = ACTION_MOVE_FORWARD;

		g_motor_left.setSpeed(g_motor_left_speed);
		g_lcd.setCursor(0, 1);
		g_lcd.print("Forward");
		/* g_lcd.print(" Speed R = ");g_lcd.print(g_motor_right_speed); */

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    break;
	    case MID_BUTTON:
	    {
		/* in this case, there's an other menu */
		g_menu_selected = MENU_MOTOR;

		/* clear LCD before displaying new menu or action */
		g_lcd.clear();
		g_lcd.print(g_menu_motor[g_menu_motor_index]);

		/* end of threatment, re-enable the button for interrupt */
		g_button = NO_BUTTON;
	    }
	    break;
	}
    }
    else if (g_menu_selected == MENU_PROG)
    {
	gestion_menu(&g_menu_prog_index, MENU_MAIN_END);
	switch (g_button_selected)
	{
	    case MID_BUTTON:
	    {
		switch (g_menu_prog_index)
		{
		    case MENU_PROG_1:
		    {
			g_action = ACTION_PROG_1;
			g_action_detection = 2;
			g_motor_left_count = 0;
		    }
		    break;
		    case MENU_PROG_2:
		    {
			g_action = ACTION_PROG_2;
			g_action_detection = 2;
			g_motor_left_count = 0;
		    }
		    break;
		    case MENU_PROG_3:
		    {
			g_action = ACTION_PROG_3;
		    }
		    break;
		    case MENU_PROG_MAIN:
		    {
			/* in this case, there's an other menu */
			g_menu_selected = MENU_MAIN;

			/* clear LCD before displaying new menu or action */
			g_lcd.clear();
			g_lcd.print(g_menu_main[g_menu_main_index]);
		    }
		    break;
		}
	    }
	    break;
	    case LEFT_BUTTON:
	    case RIGHT_BUTTON:
	    {
 		g_action = ACTION_NONE;
		g_lcd.clear();
		g_lcd.print(g_menu_prog[g_menu_prog_index]);
	    }
	    break;
	    default :
	    break;
	}
    }
}

void process_action(void)
{
    int value;

    switch (g_action)
    {
	case ACTION_NONE:
	{
	    return;
	}
	break;
	case ACTION_MISC_DETECTION:
	{
	    value = digitalRead(A5);
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Obstacle  !!!   ");
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
	break;
	case ACTION_MISC_TEMPERATURE:
	{
	    value = (5.0 * analogRead(A4) * 100.0) / 1024;

	    if (g_action_temperature != value)
	    {
		g_lcd.setCursor(0, 1);
		g_lcd.print("Temp =  ");
		g_lcd.print(value);
		g_lcd.print("C     ");
	    }

	    g_action_temperature = value;
	}
	break;
	case ACTION_MOTOR_OBSTACLE:
	{
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Stop  !!!       ");
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
	break;
	case ACTION_MOVE_STOP:
	{

	}
	break;
	case ACTION_MOVE_REVERSE:
	{
	    g_motor_left.step(-1);
	}
	break;
	case ACTION_MOVE_FORWARD:
	{
	    g_motor_left.step(1);
	}
	break;
	case ACTION_PROG_1:
	{
	    value = digitalRead(A5);
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Obstacle! ");g_lcd.print(g_motor_left_count);
		}
	    }
	    else
	    {
		g_motor_left.step(10);
		g_motor_left_count+=10;

		g_lcd.setCursor(0, 1);
		g_lcd.print("Running.. ");g_lcd.print(g_motor_left_count);
	    }
	    g_action_detection = value;
	}
	break;
	case ACTION_PROG_2:
	{
	    value = digitalRead(A5);
	    if (value == 0)
	    {
		if (g_action_detection != value)
		{
		    g_lcd.setCursor(0, 1);
		    g_lcd.print("Obstacle ! ");g_lcd.print(g_motor_left_count);
		}
	    }
	    else
	    {
		g_lcd.setCursor(0, 1);
		g_lcd.print("Starting.. ");g_lcd.print(g_motor_left_count);
		g_motor_left.step(400);
		g_motor_left_count++;

		g_lcd.setCursor(0, 1);
		g_lcd.print("Running... ");g_lcd.print(g_motor_left_count);
	    }
	    g_action_detection = value;
	}
	break;

	default:
	break;
    }
}

void loop(void)
{
    process_menu();

    process_action();
}

#if 0

void looper()
{
    int value;
    int positionCounter;


    if ((g_select == LEFT_BUTTON) || (g_select == RIGHT_BUTTON))
    {
	lcd.clear();

	if (g_text_menu == 0)
	else if (g_text_menu == 1)
	    g_lcd.print("2) Moteur avance");
	else if (g_text_menu == 2)
	    g_lcd.print("3) Moteur recule");
	else if (g_text_menu == 3)
	    g_lcd.print("4) Moteur step");
	else if (g_text_menu == 4)
	    g_lcd.print("5) Menu vide");
	else if (g_text_menu == LAST_TEXT_MENU)
	    g_lcd.print("6) Menu vide");

	g_select = NO_BUTTON;
    }

    if (g_select == MID_BUTTON)
    {
	/* Manage obstacle detection */
	if (g_selected == 0)
	{
	    value = digitalRead(A5);
	    if (value == 0)
	    {
		if (g_last_value != value)
		{
		    g_lcd.clear();
		    g_lcd.print("Obstacle  !!!");
		}
	    }
	    else
	    {
		if (g_last_value != value)
		{
		    lcd.clear();
		}
	    }
	    g_last_value = value;
	    delay(50);
	}
	else if (g_selected == 1)
	{
	    myStepper.step(100);
	    g_selected = -1;
	}
	else if (g_selected == 2)
	{
	    myStepper.step(-100);
	    g_selected = -1;
	}
	else if (g_selected == 3)
	{
	    myStepper.step(1);
	    g_selected = -1;
	}
    }
}

void toto ()
{
    int value;
    if (Serial.available() > 0)
    {
        g_car = Serial.read();
        if (g_car =='*')
        {
	    lcd.clear();
	    g_col = 0;
	    g_line = 0;
        }
        else if ((g_car == 8) && (g_col > 0))
        {
	    g_col--;
	    lcd.setCursor(g_col, g_line);
	    lcd.print(" ");
	    g_col++;
        }
        else if (g_car == '!')
        {
	    lcd.clear();
	    g_col = 0;
	    g_line = 0;
	    lcd.setCursor(g_col, g_line);
	    value = analogRead(A5);
	    /*
		value = (5.0 * analogRead(A5) * 100.0) / 1024;
		g_analog_value[M5] = value;

		Serial.print("--> Temperature value = ");
		Serial.print(g_analog_value[M5]);
		Serial.print(" C\r\n");
	    */
	    lcd.print(value);
	    lcd.print(" C");
        }
	else
        {
	    lcd.setCursor(g_col, g_line);
	    lcd.print(g_car);
	    g_col++;
        }
        if (g_col == 16 )
        {
	    g_col = 0;
	    if (g_line == 1)
	    {
		g_line = 0;
	    }
	    else
	    {
		g_line = 1;
	    }
        }
    }
	    lcd.clear();
	    value = analogRead(A5) / 10;
	    lcd.print(value);
	    delay(200);
}
#endif
