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
            printf("You specified the wrong mode of rendering\n");
            return -2;
            }

        if (argc == 2) GetMandelBrotSet(&set);

        else 
            {
            int mode_measure = 0;
            sscanf(argv[2], "%d", &mode_measure);

            if (mode_measure < 0 || mode_measure > 1) 
                {
                printf("You specified the wrong mode of measure\n");
                return -3;
                }

            ChooseRunTest(&set, mode_measure);
            }
        }

    free(set.pixels_array);

    return 0;
    }