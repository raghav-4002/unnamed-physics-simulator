#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "basic.h"
#include "physics.h"


void draw(Point *point);
void move(Point particles[], unsigned size);
void set_values(Point particles[], unsigned size);


int
main(int argc, char *argv[])
{
    if(argc == 1) {
        printf("Usage: %s <number-of-particles>\n", argv[0]);
        exit(1);
    }

    unsigned particle_count = atol(argv[1]);
    Point particles[particle_count];

    srand(time(NULL));
    initialize();

    set_values(particles, particle_count);
    move(particles, particle_count);    

    return 0;
}


void
set_values(Point particles[], unsigned particle_count)
{
    for(unsigned i = 0; i < particle_count; i++) {
        particles[i].position.x    = (rand() % term_attributes.screen_width) + 1;
        particles[i].position.y    = (rand() % term_attributes.screen_length) + 1;

        particles[i].velocity.x    = (rand() % 2) + 1;
        particles[i].velocity.y    = (rand() % 2) + 1;

        particles[i].accelaration.y = 0.00;
        particles[i].accelaration.x = 0.00;
    }
}


void
draw(Point *point)
{
    char buf[32];
    int  len;

    len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", 
            (int)point->position.y, (int)point->position.x);

    write(STDOUT_FILENO, buf, len);
    write(STDOUT_FILENO, "0", 1);

    rebound_from_edges(point);
}


void 
move(Point particles[], unsigned particle_count)
{
    const float req_frame_time = 1.0f / 40.0f;
    struct timespec start, end;
    float elapsed_time, sleep_time;

    while(1) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        write(STDOUT_FILENO, "\x1b[2J", 4);

        for(size_t i = 0; i < particle_count; i++) {
            particles[i].velocity.x += particles[i].accelaration.x;
            particles[i].velocity.y += particles[i].accelaration.y;

            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;

            draw(&particles[i]);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed_time = (end.tv_nsec - start.tv_nsec) / (1.0e9);

        sleep_time   = req_frame_time - elapsed_time;

        if(sleep_time > 0) {
            struct timespec ts = {0, sleep_time * (1.0e9)};

            nanosleep(&ts, NULL);
        }
    }
}