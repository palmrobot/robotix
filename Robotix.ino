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
#define MOTOR_LEFT_SPEED	50

/* Stepper g_motor_right(400, 8, 9, 10, 11); */
/* #define MOTOR_RIGHT_SPEED	50 */

/********************************************************/
/*      Menus definitions                               */
/********************************************************/
#define MENU_MAIN		0
#define MENU_MOTOR		1
#define MENU_PROG		2
#define MENU_MOVE		3
int g_menu_selected;

#define MENU_MAIN_DETEC		0
#define MENU_MAIN_MOTOR		1
#define MENU_MAIN_TEMP		2
#define MENU_MAIN_PROG		3
#define MENU_MAIN_END		4
const char *g_menu_main[] = { "Detection", "Moteur", "Temperature", "Programme" };
int g_menu_main_index;

#define MISC_TEMPERATURE	1
#define MISC_DETECTION		2
int g_misc_action;

#define MENU_MOTOR_MOVE		0
#define MENU_MOTOR_OBSTACLE	1
#define MENU_MOTOR_MAIN		2
#define MENU_MOTOR_END		3
const char *g_menu_motor[] = { "Move", "Obstacle", "Main Menu" };
int g_menu_motor_index;

#define MOTOR_OBSTACLE		1
int g_motor_action;


#define MENU_MOVE_MOTOR		0
#define MENU_MOVE_DIRECTION	1

#define MENU_MOVE_PLUS		0
#define MENU_MOVE_MINUS		1
#define MENU_MOVE_STOP		2
#define MENU_MOVE_REVERSE	3
#define MENU_MOVE_FORWARD	4
#define MENU_MOVE_EXIT		5
#define MENU_MOTOR_END		6
const char *g_menu_move[] = { "  +    STOP   -", "Rever Exit Forwd" };
int g_menu_move_index;

#define MENU_PROG_1		0
#define MENU_PROG_2		1
#define MENU_PROG_3		2
#define MENU_PROG_MAIN		3
#define MENU_PROG_END		4
const char *g_menu_programme[] = { "Prog1", "Prog2", "Prog3", "Main menu" };
int g_menu_prog_index;

#define PROG_1			1
#define PROG_2			2
#define PROG_3			3
int g_prog_action;


/* char g_car; */
/* int g_text_menu; */
/* int g_select; */
/* int g_selected; */
/* int g_last_value; */

void setup()
{
    /* set up the LCD's number of columns and rows: */
    g_lcd.begin(16, 2);

    /* Set the Stepper Speed */
    g_motor_left.setSpeed(MOTOR_LEFT_SPEED);

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
    g_menu_move_index = MENU_MOVE_DIRECTION;
    g_menu_prog_index = 0;
    g_menu_seleted = 0;
    g_button = NO_BUTTON;
    g_lcd_col	= 0;
    g_lcd_line	= 0;

    g_misc_action  = 0;
    g_motor_action = 0;
    g_prog_action  = 0;

    /* g_text_menu = 0; */
    /* g_select	= 0; */
    /* g_selected	= 0; */
    /* g_last_value= 2; */

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
    g_button_seleted = NO_BUTTON;

    /* Check which Button is hold */
    if (g_button == LEFT_BUTTON)
    {
	g_button_seleted = g_button;
	if (idx == 0)
	    idx = menu_end;
	else
	   idx--;

	/* end of threatment, re-enable the button for interrupt */
	g_button = NO_BUTTON;
    }
    else if (g_button == RIGHT_BUTTON)
    {
	g_button_selected = g_button;

    	if (idx == menu_end)
	    idx = 0;
	else
	    idx++;

	/* end of threatment, re-enable the button for interrupt */
	g_button = NO_BUTTON;
    }
    else if (g_button == MID_BUTTON)
    {
	g_button_selected = g_button;

	/* end of threatment, re-enable the button for interrupt */
	g_button = NO_BUTTON;
    }

    /* save selection */
    *menu_index = idx;
}

void process_menu(void)
{
    if (g_menu_selected == MENU_MAIN)
    {
	gestion_menu(&g_menu_main_index, MENU_MAIN_END);
	if (g_button_selected == MID_BUTTON)
	{
	    switch (g_menu_main_index)
	    {
		case MENU_MAIN_DETEC:
		{
		    g_misc_action = MISC_DETECTION;
		}
		break;
		case MENU_MAIN_MOTOR:
		{
		    /* in this case, there's an other menu */
		    g_menu_selected = MENU_MOTOR;

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.setCursor(0, 0);
		    g_lcd.print(g_menu_motor[g_menu_motor_index]);
		}
		break;
		case MENU_MAIN_TEMP:
		{
		    g_misc_action = MISC_TEMPERATURE;
		}
		break;
		case MENU_MAIN_PROG:
		{
		    /* in this case, there's an other menu */
		    g_menu_selected = MENU_PROG;

		    /* clear LCD before displaying new menu or action */
		    g_lcd.clear();
		    g_lcd.setCursor(0, 0);
		    g_lcd.print(g_menu_prog[g_menu_prog_index]);
		}
		break;
	    }
	}
	else
	{
	    g_lcd.setCursor(0, 0);
	    g_lcd.print(g_menu_main[g_menu_main_index]);
	}
    }

    else if (g_menu_selected == MENU_MOTOR)
    {
	gestion_menu(&g_menu_motor_index, MENU_MOTOR_END);
	if (g_button_selected == MID_BUTTON)
	{
	    switch (g_menu_motor_index)
	    {
		case MENU_MOTOR_AVANCE:
		{
		    g_motor_action = MOTOR_AVANCE;
		}
		break;
		case MENU_MOTOR_RECULE:
		{
		    g_motor_action = MOTOR_RECULE;
		}
		break;
		case MENU_MOTOR_STOP:
		{
		    g_motor_action = MOTOR_STOP;
		}
		break;
		case MENU_MOTOR_OBSTACLE:
		{
		    g_motor_action = MOTOR_OBSTACLE;
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
	else
	{
	    g_lcd.print(g_menu_motor[g_menu_motor_index]);
	}
    }
    else if (g_menu_selected == MENU_MOVE)
    {
	g_button_seleted = NO_BUTTON;

	/* Check which Button is hold */
	if (g_button == LEFT_BUTTON)
	{
	    g_button_seleted = g_button;
	    if (idx == 0)
		idx = menu_end;
	    else
		idx--;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	else if (g_button == RIGHT_BUTTON)
	{
	    g_button_selected = g_button;

	    if (idx == menu_end)
		idx = 0;
	    else
		idx++;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}
	else if (g_button == MID_BUTTON)
	{
	    g_button_selected = g_button;

	    /* end of threatment, re-enable the button for interrupt */
	    g_button = NO_BUTTON;
	}

    }
    else if (g_menu_selected == MENU_PROG)
    {
	gestion_menu(&g_menu_mprog_index, MENU_MAIN_END);
	if (g_button_selected == MID_BUTTON)
	{
	    switch (g_menu_prog_index)
	    {
		case MENU_PROG_1:
		{
		    g_prog_action = PROG_1;
		    lcd.setCursor(0, 1);

		}
		break;
		case MENU_PROG_2:
		{
		    g_prog_action = PROG_2;
		}
		break;
		case MENU_PROG_3:
		{
		    g_prog_action = PROG_3;
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
	else
	{
	    g_lcd.print(g_menu_prog[g_menu_prog_index]);
	}
    }
}

void process_action(void)
{

}

void loop(void)
{
    process_menu();

    process_action();
}


void looper()
{
    int value;
    int positionCounter;


    if ((g_select == LEFT_BUTTON) || (g_select == RIGHT_BUTTON))
    {
	lcd.clear();

	if (g_text_menu == 0)
	else if (g_text_menu == 1)
	    lcd.print("2) Moteur avance");
	else if (g_text_menu == 2)
	    lcd.print("3) Moteur recule");
	else if (g_text_menu == 3)
	    lcd.print("4) Moteur step");
	else if (g_text_menu == 4)
	    lcd.print("5) Menu vide");
	else if (g_text_menu == LAST_TEXT_MENU)
	    lcd.print("6) Menu vide");

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
		    lcd.clear();
		    lcd.print("Obstacle  !!!");
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
