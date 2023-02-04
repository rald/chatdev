#ifndef GUI__H
#define GUI__H
void init_gui(void);
void drawWindows(void);
void printInputW(char *message);
void printOutputW(char *user, char *text);
void sigHandler(int sig);
void exit_window(char *message);
char* handle_input(void);
#endif
