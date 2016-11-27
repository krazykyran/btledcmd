/*
 * main.c
 *
 *  Created on: Nov 21, 2016
 *      Author: kyran mcglasson
 *     Company: SAGE Automation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define RED_LED		"13"
#define RED_PATH	"/sys/class/gpio/gpio13/"
#define GREEN_LED	"26"
#define GREEN_PATH	"/sys/class/gpio/gpio26/"
#define BLUE_LED	"12"
#define BLUE_PATH	"/sys/class/gpio/gpio12/"

#define GPIO_SYSFS	"/sys/class/gpio/"

enum E_ACTION {
	E_ACTION_NULL,
	E_ACTION_OFF,
	E_ACTION_ON,
	E_ACTION_FLASH,
	E_ACTION_ERROR,
	E_ACTION_SETUP,
	E_ACTION_CLOSE
};

const char *program_name;

void print_usage(FILE *stream)
{
	fprintf(stream, "\nRaspberry Pi GPIO RGB LED control for Bluetooth detection applications.\n");
	fprintf(stream, "\n");
	fprintf(stream, "Usage: %s [vh] -a [off|on|flash|error] -n num \n", program_name);
	fprintf(stream, "  -a : action required, can be one of the following ...\n");
	fprintf(stream, "         setup - configure LED GPIO for output and open sysfs.\n");
	fprintf(stream, "         on - set only the green (power) LED on.\n");
	fprintf(stream, "         off - reset all LEDs off.\n");
	fprintf(stream, "         flash - flash the blue (detected) LED on.\n");
	fprintf(stream, "                 use the -n option to set the number of flashes.\n");
	fprintf(stream, "         error - set only the red (erroR) LED on.\n");
	fprintf(stream, "         close - reset LED GPIO configuration and close sysfs.\n");
	fprintf(stream, "  -n : number of flashes when using the flash option in -a.\n");
	fprintf(stream, "  -h : help, displays the usage options of this command.\n");
	fprintf(stream, "  -d : debug output.\n");
	fprintf(stream, "  -v : verbose output.\n");
	fprintf(stream, "\n");
}

void writeGPIO(char filename[], char value[])
{
	FILE *fd;
	fd = fopen(filename, "w+");
	fprintf(fd, "%s", value);
	fclose(fd);
}

int main(int argc, char *argv[])
{
	program_name = argv[0];

// use next line if last argument needs to be non-option argument.
//	if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') )
	if ( (argc <= 1) || (argv[argc-1] == NULL) )
	{
		fprintf(stderr,"insufficient arguments!\n");
		print_usage(stdout);
		exit(EXIT_FAILURE);
	}

	int c;
	extern char *optarg;
	extern int optind, optopt;

	char *aval = NULL;
	char *nval = NULL;

	int index;
	int verbose = 0;
	int debug = 0;
	int num;
	int action;

	while ( (c = getopt(argc, argv, "a:n:dhv") ) != -1 )
	{
		switch(c)
		{
		case 'a':
			aval = optarg;
			action = E_ACTION_NULL;
			if (!strcmp(optarg, "setup"))
				action = E_ACTION_SETUP;
			if (!strcmp(optarg, "off"))
				action = E_ACTION_OFF;
			if (!strcmp(optarg, "on"))
				action = E_ACTION_ON;
			if (!strcmp(optarg, "flash"))
				action = E_ACTION_FLASH;
				num = 1;
			if (!strcmp(optarg, "error"))
				action = E_ACTION_ERROR;
			if (!strcmp(optarg, "close"))
				action = E_ACTION_CLOSE;
			break;
		case 'n':
			nval = optarg;
			num = atoi(optarg);
			break;
		case 'h':
		case '?':
			print_usage(stdout);
			exit(EXIT_FAILURE);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'd':
			debug = 1;
			break;
		default:
			// getopt prints out "invalid option" message
			print_usage(stdout);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (verbose)
	{
		fprintf(stdout, "option a : %s : %d \n", aval, action);
		fprintf(stdout, "option n : %s : %d \n", nval, num);
		fprintf(stdout, "option d : %s : %d \n", (debug ? "true" : "false"), debug);
		fprintf(stdout, "option v : %s : %d \n", (verbose ? "true" : "false"), verbose);

		for (index = optind; index < argc; index++)
			fprintf(stderr, "non-option argument %s \n", argv[index]);
	}

	switch(action)
	{
	case E_ACTION_SETUP:
		// configure the RED LED
		writeGPIO(GPIO_SYSFS "export", RED_LED);
		writeGPIO(RED_PATH "direction", "out");
		writeGPIO(RED_PATH "value", "0");
		// configure the GREEN LED
		writeGPIO(GPIO_SYSFS "export", GREEN_LED);
		writeGPIO(GREEN_PATH "direction", "out");
		writeGPIO(GREEN_PATH "value", "0");
		// configure the BLUE LED
		writeGPIO(GPIO_SYSFS "export", BLUE_LED);
		writeGPIO(BLUE_PATH "direction", "out");
		writeGPIO(BLUE_PATH "value", "0");
		break;
	case E_ACTION_OFF:
		writeGPIO(RED_PATH "value", "0");
		writeGPIO(GREEN_PATH "value", "0");
		writeGPIO(BLUE_PATH "value", "0");
		break;
	case E_ACTION_ON:
		writeGPIO(RED_PATH "value", "0");
		writeGPIO(GREEN_PATH "value", "1");
		writeGPIO(BLUE_PATH "value", "0");
		break;
	case E_ACTION_FLASH:
		writeGPIO(RED_PATH "value", "0");
		writeGPIO(GREEN_PATH "value", "0");
		writeGPIO(BLUE_PATH "value", "0");
		usleep(100000);
		writeGPIO(BLUE_PATH "value", "1");
		for (index=num-1; index > 0; index--)
		{
			usleep(100000);
			writeGPIO(BLUE_PATH "value", "0");
			usleep(100000);
			writeGPIO(BLUE_PATH "value", "1");
		}
		break;
	case E_ACTION_ERROR:
		writeGPIO(RED_PATH "value", "1");
		writeGPIO(GREEN_PATH "value", "0");
		writeGPIO(BLUE_PATH "value", "0");
		break;
	case E_ACTION_CLOSE:
		writeGPIO(GPIO_SYSFS "unexport", RED_LED);
		writeGPIO(GPIO_SYSFS "unexport", GREEN_LED);
		writeGPIO(GPIO_SYSFS "unexport", BLUE_LED);
		break;
	default:
		break;
	}

	exit(EXIT_SUCCESS);

}
