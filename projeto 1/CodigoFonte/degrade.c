#include <stdio.h>
#include <stdlib.h>
#include "vc.h"

int main(void)
{
    IVC *image;
    int x, y;
    long int pos;

    image = vc_image_new(256, 256, 1, 255);
    if (image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    for (x = 0; x < image->width; x++)
    {
        for (y = 0; y < image->height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;
            // image->data[pos]=y; obtemos o degrade de baixo para cima, ou seja o branco em baixo e o preto em cima

            // desta forma obtemos o degrade da esquerda para cima, tendo o branco do lado direito e o preto no lado esquedo
            image->data[pos]=x;

        }
    }


    vc_write_image("degrade.pgm", image);

    system("FilterGear degrade.pgm");

    vc_image_free(image);
    printf("Press any key to exit...\n");
    getchar();
    return 0;

}