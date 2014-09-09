#include <WaveHC.h>
#include <WaveUtil.h>


/********************************************************/
/*      Pin  definitions                                */
/********************************************************/



/********************************************************/
/*      Serial Motor definitions                        */
/********************************************************/
#define COMMAND_TEST			0x80 /* [0x80 Test] [Nb of bytes] [byte 1] [byte 2] ... [byte n] */
#define COMMAND_TEST2			0x81 /* [0x81 Test2] [data1] [data2] */
#define COMMAND_READY			0x82 /* [0x82 Ready] */
#define COMMAND_INIT_ERROR		0x83 /* [0x84 End of file] */
#define COMMAND_INIT_FAT_ERROR		0x84 /* [0x84 End of file] */
#define COMMAND_INIT_ROOT_ERROR		0x85 /* [0x85 End of file] */

#define COMMAND_PLAYING_FILE		0x86 /* [0x86 File is playing] */
#define COMMAND_PLAY_END		0x87 /* [0x87 End of file] */
#define COMMAND_FILE_NUMBER		0x88 /* [0x88 Number of file] */


#define COMMAND_FILE_NAME		0xA0 /* [0xA0 File name] [ n Data of filenanme ] */

#define COMMAND_START			0xFE /* [0xFE Start transmission] */

#define CMD_SEND_DATA_MAX		16
uint8_t g_send_mother[CMD_SEND_DATA_MAX];

#define CMD_DATA_MAX			6
uint8_t g_recv_mother[CMD_DATA_MAX];

/********************************************************/
/*      Process definitions                             */
/********************************************************/

#define PROCESS_RECEIVE_DO_NOTHING		0
#define PROCESS_RECEIVE_WAIT_COMMAND		1
uint8_t g_process_receive;

#define PROCESS_ACTION_INIT			0x01
#define PROCESS_ACTION_INIT_ERROR		0x02
#define PROCESS_ACTION_INIT_FAT_ERROR		0x04
#define PROCESS_ACTION_INIT_ROOT_ERROR		0x08
#define PROCESS_ACTION_PLAYING			0x10
uint16_t g_process_action;

#define PROCESS_COMMAND_LIST			0xD1 /* [0xD1 List] */
#define PROCESS_COMMAND_FILENAME		0xD2 /* [0xD2 Number of the file to get name ] */
#define PROCESS_COMMAND_PLAYFILE		0xD3 /* [0xD3 Play this file number */
#define PROCESS_COMMAND_STOP_PLAYING		0xD4 /* [0xD4 Stop playing] */
#define PROCESS_COMMAND_BEEP_KEY		0xD5 /* [0xD5 Playing Beep] */
#define PROCESS_COMMAND_NOTE			0xD6 /* [0xD6 Playing Note] */
#define PROCESS_COMMAND_MOTOR			0xD7 /* [0xD7 Playing motor sound] */
#define PROCESS_COMMAND_HELLO			0xD8 /* [0xD8 Playing hello sound] */
#define PROCESS_COMMAND_START			0xFE /* [0xFE Start transmission */
uint8_t g_process_command;

/********************************************************/
/*      Global definitions                              */
/********************************************************/
uint8_t g_time_count;
uint8_t g_recv_mother_nb;


SdReader g_card;    /* This object holds the information for the card */
FatVolume g_vol;    /* This holds the information for the partition on the card */

FatReader g_root;   /* This holds the information for the filesystem on the card */
FatReader g_file;   /* This is for the file we are playing */

/* buffer for directory reads */
#define MAX_FILES				10
dir_t	g_dirBuf[MAX_FILES];

 /* This is the only wave (audio) object, since we will only play one at a time */
WaveHC g_wave;

char g_file_beep[]     = "_beep.wav";
char g_file_motor[]    = "_motor.wav";
char g_file_start[]    = "_start.wav";
char *g_file_note[]   = {"_do.wav", "_re.wav", "_mi.wav", "_fa.wav", "_sol.wav", "_la.wav", "_si.wav"};

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

#define PLAY_ONE_TIME		0
#define PLAY_REPEAT		1
uint8_t g_play_type;

void setup()
{
    uint8_t partition;
    /* Initialize the output pins for the DAC control. */
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(10, OUTPUT);

    /* init process states */
    g_process_receive   = PROCESS_RECEIVE_WAIT_COMMAND;
    g_process_command   = 0;
    g_process_action    = PROCESS_ACTION_INIT;

    if (!g_card.init(10))
    {
	g_process_action    = PROCESS_ACTION_INIT_ERROR;
    }
    else
    {
	/* enable optimize read - some cards may timeout. Disable if you're having problems */
	g_card.partialBlockRead(true);


	/* Now we will look for a FAT partition!
	 * we have up to 5 slots to look in
	 */
	for (partition = 0; partition < 5; partition++)
	{
	    if (g_vol.init(g_card, partition))
		break;
	}

	if (partition == 5)
	{
	    g_process_action    = PROCESS_ACTION_INIT_FAT_ERROR;
	}
	else
	{

	    /* Try to open the root directory */
	    if (!g_root.openRoot(g_vol))
	    {
		g_process_action    = PROCESS_ACTION_INIT_ROOT_ERROR;
	    }
	}
    }

    /* Init global variables */
    g_play_type = PLAY_ONE_TIME;

    /* init pipes */
    g_recv_mother[0]	= 0;
    g_recv_mother_nb	= 0;

    g_send_mother[0]	= 0;

    /* initialize serial communications at 115200 bps: */
    Serial.begin(115200);

    delay(100);
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
    uint8_t padding[CMD_SEND_DATA_MAX] = {0};

    if (len > CMD_SEND_DATA_MAX)
	len = CMD_SEND_DATA_MAX;

    /* Send Start of transmission */
    Serial.write(COMMAND_START);

    /* Write Command + Data */
    Serial.write(buffer, len);

    /* Write padding Data */
    Serial.write(padding, CMD_SEND_DATA_MAX - len);
}

uint8_t play_file(char *filename)
{
    if (g_wave.isplaying)
    {
	g_wave.stop();
	g_file.close();
    }

    if (g_file.open(g_root, filename))
    {
	if (g_wave.create(g_file))
	{
	    g_wave.play();

	    g_send_mother[0] = COMMAND_PLAYING_FILE;
	    send_mother(g_send_mother, 1);

	    return 1;
	}
    }
    return 0;
}

uint8_t play_file_dir(dir_t *filename)
{
    if (g_wave.isplaying)
    {
	g_wave.stop();
	g_file.close();
    }

    if (g_file.open(g_vol, *filename))
    {
	if (g_wave.create(g_file))
	{
	    g_wave.play();

	    g_send_mother[0] = COMMAND_PLAYING_FILE;
	    send_mother(g_send_mother, 1);

	    return 1;
	}
    }
    return 0;
}



uint8_t Read_card(FatReader dir)
{
    FatReader file;
    uint8_t nb_files = 0;

    while (dir.readDir(g_dirBuf[nb_files]) > 0 && nb_files < MAX_FILES )
    {
	/* Read every file in the directory one at a time
	 * Skip it if not a subdirectory and not a .WAV file and not a system file starting with '_'
	 */
	if ((!DIR_IS_SUBDIR(g_dirBuf[nb_files]) &&
		(strncmp_P((char *)&g_dirBuf[nb_files].name[8], PSTR("WAV"), 3)) != 0) ||
	    (g_dirBuf[nb_files].name[0] == '_'))
	{
	    continue;
	}

	if (!file.open(g_vol, g_dirBuf[nb_files]))
	{
	    continue;
	}

	/* check if we opened a new directory */
	if (!file.isDir())
	{
	    nb_files++;
	}
	file.close();
    }

    return(nb_files - 1);
}

void send_file_name(uint8_t file_number)
{
    uint8_t j, i;

    g_send_mother[0] = COMMAND_FILE_NAME;
    j = 1;

    for (i = 0; i < 11; i++)
    {
	if (g_dirBuf[file_number].name[i] == ' ')
	    continue;

	if (i == 8)
	    g_send_mother[j++] = '.';

	g_send_mother[j++] = g_dirBuf[file_number].name[i];
    }

    send_mother(g_send_mother, j);
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
    uint8_t file_number;
    uint8_t note;

    if (g_process_command)
    {
	if (g_process_command == PROCESS_COMMAND_LIST)
	{
	    g_send_mother[0] = COMMAND_FILE_NUMBER;
	    g_send_mother[1] = Read_card(g_root);
	    send_mother(g_send_mother, 2);
	}
	else if (g_process_command == PROCESS_COMMAND_FILENAME)
	{
	    file_number =  g_recv_mother[1];
	    if ((file_number >= 0) && (file_number < MAX_FILES))
	    {
		send_file_name(file_number);
	    }
	    else
	    {
		send_file_name(0);
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_PLAYFILE)
	{
	    file_number =  g_recv_mother[1];
	    if (play_file_dir(&g_dirBuf[file_number]))
	    {
		g_process_action |= PROCESS_ACTION_PLAYING;
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_BEEP_KEY)
	{
	    g_play_type = g_recv_mother[1];
	    if (play_file(g_file_beep))
	    {
		g_process_action |= PROCESS_ACTION_PLAYING;
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_MOTOR)
	{
	    g_play_type = g_recv_mother[1];
	    if (play_file(g_file_motor))
	    {
		g_process_action |= PROCESS_ACTION_PLAYING;
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_HELLO)
	{
	    g_play_type = g_recv_mother[1];
	    if (play_file(g_file_start))
	    {
		g_process_action |= PROCESS_ACTION_PLAYING;
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_NOTE)
	{
	    note =  g_recv_mother[1];

	    if ((note >= NOTE_DO) && (note < NOTE_END))
	    {
		if (play_file(g_file_note[note]))
		{
		    g_process_action |= PROCESS_ACTION_PLAYING;
		}
	    }
	}
	else if (g_process_command == PROCESS_COMMAND_STOP_PLAYING)
	{
	    if (g_wave.isplaying)
	    {
		g_wave.stop();
		g_file.close();
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
	if ((g_process_action & PROCESS_ACTION_INIT_ERROR) == PROCESS_ACTION_INIT_ERROR)
	{
	    g_send_mother[0] = COMMAND_INIT_ERROR;
	    send_mother(g_send_mother, 1);

	    g_process_action &= ~PROCESS_ACTION_INIT_ERROR;
	}
	if ((g_process_action & PROCESS_ACTION_INIT_FAT_ERROR) == PROCESS_ACTION_INIT_FAT_ERROR)
	{
	    g_send_mother[0] = COMMAND_INIT_FAT_ERROR;
	    send_mother(g_send_mother, 1);

	    g_process_action &= ~PROCESS_ACTION_INIT_FAT_ERROR;
	}
	if ((g_process_action & PROCESS_ACTION_INIT_ROOT_ERROR) == PROCESS_ACTION_INIT_ROOT_ERROR)
	{
	    g_send_mother[0] = COMMAND_INIT_ROOT_ERROR;
	    send_mother(g_send_mother, 1);

	    g_process_action &= ~PROCESS_ACTION_INIT_ROOT_ERROR;
	}
	if ((g_process_action & PROCESS_ACTION_PLAYING) == PROCESS_ACTION_PLAYING)
	{
	    if (!g_wave.isplaying)
	    {
		if (g_play_type == PLAY_REPEAT)
		{
		    g_wave.play();
		}
		else
		{
		    g_send_mother[0] = COMMAND_PLAY_END;
		    send_mother(g_send_mother, 1);
		    g_file.close();

		    g_process_action &= ~PROCESS_ACTION_PLAYING;
		}
	    }
	}
    }
}

void loop()
{
    process_receive();
    process_command();
    process_action();
}


