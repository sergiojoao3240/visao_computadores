#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Fazer a semgmentação de uma cor <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Abrir imagem, alterar e gravar em novo ficheiro
int main(void)
{
    // apontador do tipo IVC (declarado no vc.h)
    IVC *image;
    IVC *image2;
    int i;

    // abre uma imagem e tons de cinzento
    // chamar a função (tem um argumento - o caminho para a imagem)
    image = vc_read_image("hsv.ppm");
    // vai tentar abrir e o valor que vai guardar vai guardar no apontador
    // se conseguiu ler a imagem mete no apontador senao retorna null e para
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2=vc_image_new(image->width, image->height,1 ,255);


    vc_hsv_segmentation(image, image2, 20, 80, 50, 100, 50, 100);


    //função dada, nome de imagem e o respetivo apontador
    vc_write_image("hsvSegmentation.ppm", image2);


    // devemos de fazer sempre free para libertar a memoria dos pixeis e da estrutura
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}