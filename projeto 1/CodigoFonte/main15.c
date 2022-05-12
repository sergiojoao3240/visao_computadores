#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Função para desenhar um histograma <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    int i;


    image = vc_read_image("Ex_hist/barbara.pgm");

    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(256,256,1,255);


    vc_gray_histogram_show(image,image2);


    vc_write_image("img_histogram.pgm", image2);


    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}