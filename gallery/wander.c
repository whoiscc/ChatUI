#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>

double since_start()
{
    static struct timeval start = {.tv_sec = 0};
    if (start.tv_sec == 0)
    {
        gettimeofday(&start, NULL);
        return 0.0;
    }
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double)(now.tv_sec - start.tv_sec) +
           ((double)now.tv_usec / 1000000 - (double)start.tv_usec / 1000000);
}

void print_frame(int width, int height)
{
    const int BASE_SPEED = 4;
    const double NOISE_SPEED = 0.4;
    const int RADIUS = 16;

    static double prev_t = 0.0;
    static double x = 0.0, y = 0.0;
    static double r = 0.0, g = 0.0, b = 0.0;

    double t = since_start();
    double t_interval = t - prev_t;
    double step = BASE_SPEED * t_interval;
    x += step * (0.23 + (sin(t * 0.97) + 1) * NOISE_SPEED);
    y += step * (0.31 + (cos(t * 1.17) + 1) * NOISE_SPEED / height * width);
    double cx = cos(x) * (width / 2) + (width / 2);
    double cy = cos(y) * (height / 2) + (height / 2);
    r += step * 60 * 0.5;
    g += step * 60 * 0.7;
    b += step * 60 * 0.4;

    int pr = abs((int)r % 512 - 256);
    int pg = abs((int)g % 512 - 256);
    int pb = abs((int)b % 512 - 256);
    for (int px = (int)cx - RADIUS; px <= (int)cx + RADIUS; px += 1)
    {
        for (int py = (int)cy - RADIUS; py <= (int)cy + RADIUS; py += 1)
        {
            if (hypot(cx - px, cy - py) <= RADIUS)
                printf("%d %d %d %d %d\n", px, py, pr, pg, pb);
        }
    }
    printf("frame\n");

    prev_t = t;
}

int main()
{
    int width = 0, height = 0;

    size_t size;
    char *line = NULL;
    while (getline(&line, &size, stdin) != -1)
    {
        if (strncmp(line, "close", 5) == 0)
            break;

        else if (sscanf(line, "resize %d %d", &width, &height))
        {
            assert(width != 0);
            assert(height != 0);
        }

        else if (strncmp(line, "ready", 5) == 0)
            print_frame(width, height);

        else
            fprintf(stderr, "skip line: %s", line);

        free(line);
        line = NULL;
    }

    free(line);
    return 0;
}