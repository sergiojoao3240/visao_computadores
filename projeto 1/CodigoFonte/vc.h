//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   		MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	// width dá quantos pixeis tenho numa linha
	int width, height;
	// imagem a cores 3 canais (pretor e branco 1)
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// Funções: espaços de cor
int vc_grey_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);

int vc_rgb_to_gray(IVC *src, IVC *dst);

//Estrair uma cor
int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_get_green_gray(IVC *srcdst);
int vc_rgb_get_blue_gray(IVC *srcdst);



//converter uma imagem rgb para hsv -> tipos de cor usadas em design gráfico
int vc_rgb_to_hsv(IVC *src, IVC *dst);

// fazer uma segmentação de uma imagem hsv
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

// inverter uma imagem de um canal para 3
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);

// fazer a segmentacao de uma imagem binaria
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold);

// segmentacao com media
int vc_gray_to_binary_global_mean(IVC *src, IVC *dst);

// segmentacao utilizando o método midpoint
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst ,int kernel);
//segmentacao utilizando o método bernsen
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst ,int kernel, int cmin);

//fazer a dilatacao de uma imagem
int vc_binary_dilate(IVC *src, IVC *dst,int kernel);
//fazer a erosao de uma imagem
int vc_binary_erode(IVC *src, IVC *dst,int kernel);

// fazer open
int vc_binary_open(IVC *src, IVC *dst, int Kerode, int Kdilate);

// fazer o close
int vc_binary_close(IVC *src, IVC *dst,int Kdilate, int Kerode);
int vc_gray_open(IVC *src, IVC *dst, int Kerode, int Kdilate);

// fazer a dilatação de uma imagem cinzenta
int vc_gray_dilate(IVC *src, IVC *dst,int kernel);
int vc_gray_close(IVC *src, IVC *dst,  int Kdilate, int Kerode);

//fazer a erosao de uma imagem cinzenta
int vc_gray_erode(IVC *src, IVC *dst,int kernel);

//fazer a etiquetagem
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);


//inverter o preto para branco e o branco para preto
int vc_inveter(IVC *src, IVC *dst);

//Desenhar a caixa em todosos objetos
int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs);


//Desenhar caixa no maior blob
int vc_CaixaBlob(IVC *src, IVC *dst, OVC* blobs, int nblobs, int *maiorBlob);

//Fazer o histograma do cinzento de uma imagem
int vc_gray_histogram_show(IVC *src, IVC *dst);

// Fazer o equalize de uma imagem
int vc_gray_histogram_equalize(IVC *src, IVC *dst);

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);

int vc_gray_lowpass_mean_filter(IVC *src, IVC *dst,int kernel);

int vc_gray_lowpass_median_filter(IVC *src, IVC *dst,int kernel);
