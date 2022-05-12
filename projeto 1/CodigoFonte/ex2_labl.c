#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Desenhar uma caixa no maior blob <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    IVC *image3;
    IVC *image4;
    IVC *image5;
    IVC *image6;
    IVC *image7;


    int i;
    OVC *nblobs;

    int nlabels;
    image = vc_read_image("Ex_Labelling/ArrowLeft.ppm");
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    //imagem para ficar a cinzento
    image2 = vc_image_new(image->width,image->height,1,255);
    //imagem para ficar binaria
    image3 = vc_image_new(image2->width,image2->height,1,255);
    // imagem para ser invertida
    image4 = vc_image_new(image3->width,image3->height,1,255);
    //imagem para fazer a dilatacao
    image5 = vc_image_new(image4->width,image4->height,1,255);
    //imagem para os blobs
    image6 = vc_image_new(image5->width, image5->height,1,255);
    //imagem para fazer labelling
    image7 = vc_image_new(image5->width, image5->height,1,255);


    //cinzento
    vc_rgb_to_gray(image,image2);
    //binaria
    vc_gray_to_binary(image2, image3, 127);
    // inverter as cores
    vc_inveter(image3, image4);
    // fazer uma dilatacao da imagem
    vc_binary_dilate(image4,image5,13);

    nblobs = vc_binary_blob_labelling(image5, image6, &nlabels);

    vc_mark_blobs(image5, image7, nblobs, nlabels);


    vc_write_image("img_teste.pgm", image7);

    vc_image_free(image);
    vc_image_free(image2);
    vc_image_free(image3);
    vc_image_free(image4);
    vc_image_free(image5);
    vc_image_free(image6);


    printf("Press any key to exit...\n");
    getchar();

    return 0;
}