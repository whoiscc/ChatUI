## ChatUI 叉忒

一个非常非常简单的UI框架，可以让非常非常简单的程序也轻松获得UI！用聊天的方式！

不限语言，不限环境，只需要：
* 从标准输入流读入用户事件，如：
    * `resize 800 600` 窗口大小变化为宽800像素长600像素
    * `click 100 200` 点击横坐标100纵坐标200
    * `ready` 可以输出下一帧画面，每个`frame`（见下）必须在读取到`ready`以后才可以输出
* 向标准输出流写入窗口画面：
    * `300 400 11 22 33` 在横坐标100纵坐标200的位置上添加一个红11绿22蓝33的像素
    * `frame` 将之前每一行表示的像素绘制在窗口中

以交换文本的方式来监听和更新图形界面。

本项目中包含一个Rust库ShimUI，作为ChatUI的宿主。只要将ChatUI应用和ShimUI使用双向管道连接起来（使用`run.sh`），就可以让ChatUI应用使用浏览器作为界面了。

用例`gallery/wander.c`。使用了C语言编写以演示ChatUI用起来的方便程度（以及C语言的不方便程度）：

```C
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
```

在`print_frame`中，通过反复将像素点`printf`到输出中来创建下一帧画面中的更新；在`main`中，通过不断`getline`获取下一个用户事件。

直接在终端中执行（以`<`开头的行是输入）：

```
$ cc -o wander gallery/wander.c -lm -O3
$ ./wander
< resize 800 600
< ready
784 600 256 256 256
785 595 256 256 256
... 省略很多像素点
816 600 256 256 256
frame
< ready
669 68 177 44 12
669 69 177 44 12
...
700 75 177 44 12
frame
< (EOF)
```

由ShimUI托管执行：

```
$ ./run.sh wander
```

![wander preview](wander.gif)
