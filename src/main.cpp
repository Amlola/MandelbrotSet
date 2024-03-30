#include "../inc/mandlebrot.h"


int main(int argc, char* argv[])
    {
    MandelBrot set = {};

    MandelBrotInit(&set);

    if (argc == 1)
        {
        printf("You didn't specify the mode\n");
        return -1;
        }

    else if (argc > 1) 
        {
        sscanf(argv[1], "%d", (int*)&set.mode);

        if (set.mode > 2 || set.mode < 0)
            {
            printf("You specified the wrong mode\n");
            return -2;
            }

        if (argc == 2) GetMandelBrotSet(&set);

        else ChooseRunTest(&set);
        }

    free(set.pixels_array);

    return 0;
    }