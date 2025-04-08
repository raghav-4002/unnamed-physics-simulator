#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>


/*
 * for smoother experience only use values of velocity from 0 to 3
 * And the values of accelaration from 0 to 1 (floats are ok)
 */
#define X_VELOCITY     0
#define Y_VELOCITY     0

#define X_ACCELARATION 0
#define Y_ACCELARATION 0

#define MASS           2.5

#define X_FORCE        0
#define Y_FORCE        0

#define OBJECT        "O"


struct {
    struct termios orig_termios;
    int            screen_length, screen_width;
} term_attributes;

typedef struct {
    float x;
    float y;
} Vector2;

typedef float Some_mass_unit_idk;

typedef struct {

    Some_mass_unit_idk mass;
    Vector2            position;
    Vector2            velocity;
    Vector2            accelaration;
    char               symbol[8];
} Point;


void init(void);
void draw(Point *point);
void move(Point *point);
void apply_force(Point *point, Vector2 force);
void get_window_size(void);
void enable_raw_mode(void);
void disable_raw_mode(void);
void kill(const char *message);


int
main(void)
{
    init();

    Point point = {MASS,
    {0, 0},
    {X_VELOCITY, Y_VELOCITY},
    {X_ACCELARATION, Y_ACCELARATION}, 
    OBJECT};

    Vector2 force = {0.1, 0};

    apply_force(&point, force);

    return 0;
}


void
init(void)
{
    enable_raw_mode();
    get_window_size();

    /* hide cursor */
    write(STDOUT_FILENO, "\x1b[?25l", 6);
}


void
draw(Point *point)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    char buf[32];
    int  len;

    len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (int)point->position.y, (int)point->position.x);

    write(STDOUT_FILENO, buf, len);

    write(STDOUT_FILENO, point->symbol, sizeof(point->symbol));
}


void 
move(Point *point)
{
    struct timespec  ts = {0, 63000000};
    struct timespec *n = NULL;

    while(1) {
        point->velocity.x += point->accelaration.x;
        point->velocity.y += point->accelaration.y;
        point->position.x += point->velocity.x;
        point->position.y += point->velocity.y;

        draw(point);

        nanosleep(&ts, n);
    }
}


void
apply_force(Point *point, Vector2 force)
{
    point->accelaration.x = (force.x / point->mass);
    point->accelaration.y = (force.y / point->mass);

    move(point);
}


void
get_window_size(void)
{
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) kill("ioctl");

    term_attributes.screen_length = ws.ws_row;
    term_attributes.screen_width = ws.ws_col;
}


void
enable_raw_mode(void)
{
    if(tcgetattr(STDIN_FILENO, &term_attributes.orig_termios) == -1) kill("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = term_attributes.orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | CS8);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) kill("tcsetattr");
}


void
disable_raw_mode(void)
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attributes.orig_termios) == -1) 
        kill("tcsetattr");

    /* unhide cursor */
    write(STDOUT_FILENO, "\x1b[?25h", 6);
}


void
kill(const char *message)
{
    perror(message);

    exit(1);
}