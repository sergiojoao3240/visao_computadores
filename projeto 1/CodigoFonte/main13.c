#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Função open e close gray<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    IVC *image3;
    int i;


    image = vc_read_image("Images/Special/testpat.pgm");

    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,255);
    image3 = vc_image_new(image->width,image->height,1,255);

    /*
    vc_gray_open(image,image2,3,3);

    vc_binary_close(image2,image3,3,3);

    vc_write_image("img_open_close.pgm", image3);

    */
   vc_gray_dilate(image,image2,3);
   vc_gray_erode(image,image3,3);


       vc_write_image("img_dilate_gray.pgm", image2);
       vc_write_image("img_erode_gray.pgm", image3);


    vc_image_free(image);
    vc_image_free(image2);
    vc_image_free(image3);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}