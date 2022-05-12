#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Função filter median <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
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


    vc_gray_lowpass_median_filter(image,image2,5);

    vc_write_image("img_filter_mean.pgm", image2);

    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}