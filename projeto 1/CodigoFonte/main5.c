#include <stdio.h>
#include "vc.h"
// >>>>>>>>>>>>>>>> Converter de um canal para 3 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Abrir imagem, alterar e gravar em novo ficheiro
int main(void)
{
    // apontador do tipo IVC (declarado no vc.h)
    IVC *image;
    IVC *image2;
    int i;

    // abre uma imagem e tons de cinzento
    // chamar a função (tem um argumento - o caminho para a imagem)
    image = vc_read_image("Images/FLIR/flir-04.pgm");
    // vai tentar abrir e o valor que vai guardar vai guardar no apontador
    // se conseguiu ler a imagem mete no apontador senao retorna null e para
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,3,255); // niveis 1 ou 255?

    // chamamos a nossa funçao
    vc_scale_gray_to_rgb(image, image2);



    //função dada, nome de imagem e o respetivo apontador
    vc_write_image("3canais.ppm", image2);


    // devemos de fazer sempre free para libertar a memoria dos pixeis e da estrutura
    vc_image_free(image2);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}