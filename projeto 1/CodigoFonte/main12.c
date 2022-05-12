#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Função open e close binary <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    IVC *image3;
    IVC *image4;
    int i;


    image = vc_read_image("flir-01.pgm");

    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,255);
    image3 = vc_image_new(image->width,image->height,1,255);
    image4 = vc_image_new(image->width,image->height,1,255);

    vc_gray_to_binary(image,image2,127);

    vc_binary_open(image2,image3,3,3);

    vc_binary_close(image3,image4,3,3);

    vc_write_image("img_open_close.pgm", image4);


    vc_image_free(image);
    vc_image_free(image2);
    vc_image_free(image3);
    vc_image_free(image4);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}