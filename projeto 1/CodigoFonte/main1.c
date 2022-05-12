#include <stdio.h>
#include <stdlib.h>
#include "vc.h"

// Abrir imagem, alterar e gravar em novo ficheiro
int main(void)
{
    // apontador do tipo IVC (declarado no vc.h)
    IVC *image;
    int i;

    // abre uma imagem e tons de cinzento
    // chamar a função (tem um argumento - o caminho para a imagem)
    image = vc_read_image("Images/FLIR/flir-01.pgm");
    // vai tentar abrir e o valor que vai guardar vai guardar no apontador
    // se conseguiu ler a imagem mete no apontador senao retorna null e para
    if(image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found\n");
        getchar();
        return 0;
    }
    
    //Inverter o preto para branco e branco para preto
    vc_grey_negative(image);

    //
    vc_rgb_negative(image);

    // para cada pixel faz o inverso (o preto passa a branco, etc.)
    //! vamos até ao último pixel (image -> bytesperline)(temos a seta e não o ponto porque IVC é uma estrutura (um tipo - typedef struct (tipo de dados compostos e dentro do bloco de codigo tem os dados que o compoe)))
    // sempre que quero chamar uma variavel basta chamar a struct (IVC é um tipo)
    // IVC image -> criava na stack espaço para a struct

    // apartir do endereço de memória tenho que dizer ao que quero aceder (com a seta, ao bytesperline)
    // ao multiplicar sabemos o numero de bytes
    for(i=0; i<image->bytesperline*image->height; i+=image->channels)
    {
        image -> data[i] = 255 - image -> data[i];
    }


    //função dada, nome de imagem e o respetivo apontador
    vc_write_image("vc0001.pgm", image);

    // estes dois códigos apresentam as imagens com o antes e o depois 
    system("cmd /c start FilterGear Images/FLIR/flir-01.pgm");
    system("FilterGear vc0001.pgm"); //output

    // devemos de fazer sempre free para libertar a memoria dos pixeis e da estrutura
    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}