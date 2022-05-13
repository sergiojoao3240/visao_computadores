#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Função filter gaussian <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    int i;


    image = vc_read_image("barbara-sp.pgm");

    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,255);


    vc_gray_lowpass_gaussian_filter(image,image2);

    vc_write_image("img_filter_gaussian.pgm", image2);

    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}