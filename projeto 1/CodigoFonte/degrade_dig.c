#include <stdio.h>
#include <stdlib.h>
#include "vc.h"

// neste ficheiro iremos criar uma imagem com um degrade na diagonal
int main(void)
{
     IVC *image;
    int x, y, n;
    long int pos;

    image = vc_image_new(256, 256, 1, 255);
    if (image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    //percorre todos os elementos da linha
    for (x = 0; x < image->width; x++)
    {
        //percorre todos os elementos da coluna
        for (y = 0; y < image->height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;

            image->data[pos] = (y + x)/2;   
     
 
            //temos apenas a linha diagonal desenhada  
           /* if(x=y)
            {
                image->data[pos] = x;
            }  */ 
             
        }
    } 


    vc_write_image("degradeD.pgm", image);

    system("cmd /c start FilterGear degradeD.pgm");
    system("FilterGear degradeD.pgm"); //output

    vc_image_free(image);
    printf("Press any key to exit...\n");
    getchar();
    return 0;

}