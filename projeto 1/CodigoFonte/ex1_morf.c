#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Dilatação de uma imagem binaria <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(void)
{
    IVC *image;
    IVC *image2;
    int i;

    image = vc_read_image("Ex1_Morf/brain.pgm");
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,255); 

    // Etapas do exercicio
    /*
    t= 127
    kernel = 3

    >> 1- Fazer a segmentação da imagem com um threshold muito alto
    >> 2- fazer a diferença das imagens
    >> 3- Fazer a erosão da imagem
    >> 4- Fazer uma dilatação
    >> 5- Passar a imagem para a original
    */
   vc_gray_to_binary(image, image2, 127);

    //vc_binary_dilate(image, image2, 5);

    vc_write_image("img_ex1_morf1.pgm", image2);


    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}