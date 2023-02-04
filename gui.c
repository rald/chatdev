#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include "gui.h"

#define STANDARD_PAIR 1
#define GREEN_PAIR 2
#define RED_PAIR 3
#define ESC_KEY 27
#define DEL_KEY 127
#define BUFFERSIZE 1024

WINDOW *OutputWindow,*InputWindow;
char *greeting = "--Enter message--";
char input[BUFFERSIZE];
int pos = 0;
    
/*
This function initializes the window and sets color pairs. 
*/
void init_gui(void){
	//initialize	
	initscr();
  noecho();
	cbreak();
	curs_set(FALSE);
	start_color();
	signal(SIGINT, sigHandler);
	signal(SIGWINCH, sigHandler);

	//check if colors are supported	
	if (has_colors() == FALSE)
	  exit_window("Your terminal does not support color");

	/*
	//check terminal width	
	if(getmaxx(stdscr) < 50)
		exit_window("Your terminal width is to small");
	
	//check terminald height
	if(getmaxy(stdscr)< 15)
		exit_window("Your terminal height is to small");
	*/
	
	//define color pairs
	init_pair(STANDARD_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(RED_PAIR, COLOR_RED, COLOR_BLACK);
	
	//create windows 
  OutputWindow = newwin(getmaxy(stdscr)-3,getmaxx(stdscr),0,0);
  InputWindow = newwin(3, getmaxx(stdscr), getmaxy(stdscr) - 3, 0);  
	
	//config windows	
	nodelay(stdscr, TRUE);	
	nodelay(InputWindow, TRUE);
	nodelay(OutputWindow, TRUE);
  scrollok(OutputWindow, TRUE);
}



/*
This is a simple function that outputs message onto InputWindow.
The message gets printed in green and the windows gets 
refreshed afterwards.
*/
void printInputW(char *message){
    wclear(InputWindow);
    box(InputWindow, 0, 0);
    wattron(InputWindow, COLOR_PAIR(GREEN_PAIR));
    mvwprintw(InputWindow, getmaxy(InputWindow)-2, 1, "%s", message);
    wattroff(InputWindow, COLOR_PAIR(GREEN_PAIR));
    wrefresh(InputWindow);
}

/*
This is a simple function that outputs user and text onto OutputWindow
and keeps track of the id. The the window gets refreshed.
*/
void printOutputW(char *user, char *text){
	if(strstr(text, "Client")!=NULL){
		wattron(OutputWindow, COLOR_PAIR(STANDARD_PAIR));
		wprintw(OutputWindow," %s", text);	
		wattroff(OutputWindow, COLOR_PAIR(STANDARD_PAIR));
	}else if(strcmp(user, "")==0){
		wattron(OutputWindow, COLOR_PAIR(RED_PAIR));
		wprintw(OutputWindow," %s", text);	
		wattroff(OutputWindow, COLOR_PAIR(RED_PAIR));
	}else{
		wattron(OutputWindow, COLOR_PAIR(GREEN_PAIR));
		wprintw(OutputWindow," %s:", user);
		wattroff(OutputWindow, COLOR_PAIR(GREEN_PAIR));
		wprintw(OutputWindow," %s",text);
	}
	box(OutputWindow, 0, 0);
  wrefresh(OutputWindow);
}

/*
This function clears and refreshes the screen. It then draws
all the windows and the content of the windows. And it starts
the input handler thread to manage key input.
*/
void drawWindows(void){
  clear();
	refresh();	
		
	printOutputW("","\n");
	strcpy(input, greeting);
  printInputW(input);	
	
	box(OutputWindow, 0, 0);
	box(InputWindow, 0, 0);
    
  wrefresh(InputWindow);
	wrefresh(OutputWindow);
	refresh();
    
	sleep(1);
}

/*
This function handles the input. The first thing you can see in
the input window is the greeting. If you press ESC the greeting 
will be displayed on the screen. If you press delete you can 
delete the last character. If you press enter the message gets written
onto the Output Window and send to the client.With the /exit command
you can close the client.
*/
char* handle_input(void){
    int c = getch();
    //send message and clear input window	
    if(c=='\n' && strcmp(input, greeting)!=0 && strcmp(input, "")!=0){
        if(strcmp(input, "/exit")==0)
        	exit_window("typed exit command");
        input[pos]=c;
        wclear(InputWindow);
        box(InputWindow, 0, 0);
        wrefresh(InputWindow);
        printOutputW("Me",input);
			
				char *ret = (char *) malloc(BUFFERSIZE);
        strcpy(ret, input);
        strncpy(input, " ",sizeof(input));
        pos = 0;
        
        return ret;
    //delete last character
    }else if(c==DEL_KEY && pos!=0){
        pos--;
        input[pos]='\0';
        printInputW(input);
    //display greeting 
    }else if(c==ESC_KEY){
        strncpy(input, greeting,sizeof(input));
        printInputW(input);
        pos = 0;
    //print detected key
    }else if(c==KEY_RESIZE){
      drawWindows();
    }else if(c!=ERR && pos!=getmaxx(stdscr)-2){
        if(strcmp(input, greeting)==0)
            strncpy(input, "\0",sizeof(input));
        input[pos]=c;
        printInputW(input);
        pos++;
    }
    char c2=(char)c;
		char *ret = &c2;
    return ret;
}


/*
 This function closes window and exits programm.
 And prints a message if given.
*/
void exit_window(char *message){
	int err = endwin();
	if(strcmp(message,"")!=0)
		printf("Error: %s\n",message);
	if(err==OK)
		printf("Successfully closed window\n");
	else if(err==ERR)
		printf("Couldnt end window\n");
	exit(1);
}


/*
This function handels signals. It then cleans
everything up and exits. Before exiting it leaves
a message depending on the signal type. 
*/
void sigHandler(int sig){
	if(sig==SIGINT)
  	exit_window("control-c key detected");
	else if(sig == SIGWINCH) {
    drawWindows();
	}
}
