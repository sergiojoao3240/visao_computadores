#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Fazer etiquetagem de uma imagem <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    IVC *image3;

    int i;
    OVC *nblobs;

    int nlabels;
    image = vc_read_image("Labelling/labelling-1.pgm");
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,255);
    image3 = vc_image_new(image->width,image->height,1,255);


    // faz a etiquetgaem e diz qunatos objetos encontrou - nlabels
    nblobs = vc_binary_blob_labelling(image, image2, &nlabels);
    printf("%d", nlabels);

   //nlables -> numero de blobs
    vc_mark_blobs(image, image3, nblobs, nlabels);

    vc_binary_blob_info(image2, nblobs, nlabels);

    //nlabels -> nblobs
    for(i=0; i < nlabels; i++)
    {
        printf("\nBlob %d", i);
        printf("\nArea: %d\n", nblobs[i].area);
        printf("Perimetro: %d\n", nblobs[i].perimeter);
        printf("Centro de gravidade: %d, %d\n", nblobs[i].xc , nblobs[i].yc);

    }

    vc_write_image("img_blob.pgm", image2);
    vc_write_image("img_blob2.pgm", image3);


    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}