//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}

	t = tok;

	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if(c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}

		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

//Gerar negativo da imagem GRAY
int vc_grey_negative(IVC *srcdst)
{
	unsigned char *data =(unsigned char *) srcdst->data;
	int width = srcdst->width; //srcdsr-> source destinations
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificação de erros
	if((srcdst->width <=0) || (srcdst->height <=0) || (srcdst->data ==NULL)) return 0;
	if (channels != 1) return 0;

	//Inverter a imagem a GRAY
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
		}
	}

	return 1;
}

//Gerar negativo da imagem RGB
int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data =(unsigned char *) srcdst->data;
	int width = srcdst->width; //srcdsr-> source destinations
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificação de erros
	if((srcdst->width <=0) || (srcdst->height <=0) || (srcdst->data ==NULL)) return 0;
	if (channels != 1) return 0;

	//Inverter a imagem a GRAY
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}
	return 1;
}

// Extrair componente Red da imagem RGB para gray
int vc_rgb_get_red_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst -> data;
    int width = srcdst -> width;
    int height = srcdst -> height;
    int bytesperline = srcdst -> width * srcdst -> channels;
    int channels = srcdst -> channels;
    int x, y;
    long int pos; // pos é o indice do pixel

    // Verificação de erros
    if ((srcdst -> width <= 0) || (srcdst -> height <= 0) || (srcdst -> data == NULL)) return 0;
    if (channels != 3) return 0; // tem que haver 3 canais senão dá erro

    // Extrair a componente Red
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos + 1] = data[pos]; // Green
            data[pos + 2] = data[pos]; // Blue
        }
    }
    return 1;
}

int vc_rgb_get_green_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst -> data;
    int width = srcdst -> width;
    int height = srcdst -> height;
    int bytesperline = srcdst -> width * srcdst -> channels;
    int channels = srcdst -> channels;
    int x, y;
    long int pos; // pos é o indice do pixel

    // Verificação de erros
    if ((srcdst -> width <= 0) || (srcdst -> height <= 0) || (srcdst -> data == NULL)) return 0;
    if (channels != 3) return 0; // tem que haver 3 canais senão dá erro

    // Extrair a componente Red
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos]; // red
            data[pos + 2] = data[pos]; // Blue
        }
    }
    return 1;
}

int vc_rgb_get_blue_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst -> data;
    int width = srcdst -> width;
    int height = srcdst -> height;
    int bytesperline = srcdst -> width * srcdst -> channels;
    int channels = srcdst -> channels;
    int x, y;
    long int pos; // pos é o indice do pixel

    // Verificação de erros
    if ((srcdst -> width <= 0) || (srcdst -> height <= 0) || (srcdst -> data == NULL)) return 0;
    if (channels != 3) return 0; // tem que haver 3 canais senão dá erro

    // Extrair a componente Red
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos]; // red
            data[pos + 1] = data[pos]; // green
        }
    }
    return 1;
}

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst =(unsigned char *) dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//Verificação de erros
	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL )) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 3) || (dst->channels != 1)) return 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float) datasrc[pos_src];
			gf = (float) datasrc[pos_src + 1];
			bf = (float) datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char) ((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;

}

int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst =(unsigned char *) dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;
	float hi, si, v, vmin, s, h;

	//Verificação de erros se as imagens têm o mesmo tamanho
	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL )) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 3) || (dst->channels != 3)) return 0;


	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			r = (float) datasrc[pos_src];
			g = (float) datasrc[pos_src + 1];
			b = (float) datasrc[pos_src + 2];

			//calcular o V
			if(r >= g && r >= b)
			{
				v = r;
			}
			else if(g >= r && g >= b)
			{
				v = g;
			}
			else
			{
				v = b;
			}

			//calcular o Vmin
			if(r <= g && r <= b)
			{
				vmin = r;
			}
			else if(g <= r && g <= b)
			{
				vmin = g;
			}
			else
			{
				vmin = b;
			}


			// calcular o S
			if((v == 0) || (v == vmin))
			{
				s = 0;
				h = 0;
			}
			else
			{
				si = (v - vmin)/v;
			}
			s = si * 255;

			//calcular o H
			if((v==r) && (g>=b))
			{
				hi= 60 * (g-b)/(v-vmin);
			}
			else if((v==r) && (b>g))
			{
				hi = 360 + 60*(g-b)/(v-vmin);
			}
			else if(v==g)
			{
				hi=120 + 60*(b-r)/(v-vmin);
			}
			else
			{
				hi = 240 + 60*(r-g)/(v-vmin);
			}

			h = hi / 360 * 255;


			datadst[pos_dst] = h ;
			datadst[pos_dst + 1] = s;
			datadst[pos_dst + 2] = v;
		}
	}
	return 1;
}

// Com esta função vamos selecionar uma cor e transformar em pixeis brancos todos os pixeis que estiverem nesse intervalo
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *datasrc =(unsigned char *) src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst =(unsigned char *) dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s ,v;
	float hmax1, hmin1, smin1, smax1, vmin1, vmax1;

	//converter os hsv minimo e maximo dado para uma escala da 255
	vmin1 = (vmin/100.0) * 255;
	vmax1 = (vmax/100.0) * 255;
	smin1 = (smin/100.0) * 255;
	smax1 = (smax/100.0) * 255;
	hmax1 = hmax / 360.0 * 255;
	hmin1 = hmin / 360.0 * 255;

	//Verificação de erros se as imagens têm o mesmo tamanho
	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL )) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 3) || (dst->channels != 1)) return 0;

	//percorrer todos os pixeis da imagem
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			h = datasrc[pos_src];
			s = datasrc[pos_src + 1];
			v = datasrc[pos_src + 2];


			if (h >= hmin1 && h <= hmax1 && s >= smin1 && s <= smax1 && v <= vmax1 && v >= vmin1)
			{
				datadst[pos_dst] = 255;
			}
			else{
				datadst[pos_dst] = 0;
			}

		}
	}
	return 1;

}

// inverter de um canal para 3
int vc_scale_gray_to_rgb(IVC *src, IVC *dst){
    unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float r, g, b;
    float brilho;
    // float array[3] = {rf, gf, bf}; // array com 3 valores -> vai guardar o red, green e blue

    // a nova imagem tem que ter a mesma resolução que a origem por isso se for <= 0 ou diferentes dá erro
    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 3)) return 0; // rgb e hsv tem o mesmo número de canais logo não podem ser diferentes nem ter nºcanais menor ou maior que 3

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            // imagem em tons de cinzento só tem um canal
            brilho = (float) datasrc[pos_src];

            if(brilho >= 0 && brilho <= 63)
            {
                r = 0;
                g = brilho * 4;
                b = 255;
            }
            else if(brilho >= 64 && brilho <= 127)
            {
                r = 0;
                g = 255;
                b = 255-(brilho - 64) * 4;
            }
            else if(brilho >= 128 && brilho <= 191)
            {
                r = (brilho - 128) * 4;
                g = 255;
                b = 0;
            }
            else if(brilho >= 192 && brilho <= 255)
            {
                r = 255;
                g = 255 - (brilho - 192) * 4;
                b = 0;
            }

            datadst[pos_dst] = r;
            datadst[pos_dst + 1] = g;
            datadst[pos_dst + 2] = b;
        }
    }
    return 1;
}

// segmentação de uma imagem
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold){
    unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho;
    long int pos_src, pos_dst;


    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float) datasrc[pos_src];

			if (brilho > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else if(brilho <= threshold)
			{
				datadst[pos_dst] = 0;
			}
        }
    }
    return 1;
}

int vc_gray_to_binary_global_mean(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, soma=0;
    long int pos_src, pos_dst;
	int resolution;
	float threshold;


    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float) datasrc[pos_src];
			soma = soma + brilho;

		}
	}
	resolution = height * width;

	threshold = soma / resolution;

	for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float) datasrc[pos_src];

			if (brilho > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else if(brilho <= threshold)
			{
				datadst[pos_dst] = 0;
			}
        }
    }
    return 1;
}

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst ,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, vmax, vmin;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float)datasrc[pos_src];
			vmin = 256;
			vmax = 0;

			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho= (float)datasrc[pos];
							if(pos_brilho<vmin)
							{
								vmin = pos_brilho;
							}
							if(pos_brilho>vmax)
							{
								vmax = pos_brilho;
							}
						}
					}
				}
			}
			threshold = (vmin + vmax) / 2;

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float) datasrc[pos_src];

			if (brilho > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else if(brilho <= threshold)
			{
				datadst[pos_dst] = 0;
			}
		}
	}
    return 1;
}

int vc_gray_to_binary_bernsen(IVC *src, IVC *dst ,int kernel, int cmin)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, vmax, vmin;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float)datasrc[pos_src];
			vmin = 256;
			vmax = 0;

			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho= (float)datasrc[pos];
							if(pos_brilho<vmin)
							{
								vmin = pos_brilho;
							}
							if(pos_brilho>vmax)
							{
								vmax = pos_brilho;
							}
						}
					}
				}
			}
		}
	}
    return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, exists;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float)datasrc[pos_src];

			// o brilho do destino é igual ao brilho de origem
			datadst[pos_dst] = datasrc[pos_src];
			exists = 0;
			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							if(pos_brilho > 0)
							{
								exists = 1;
							}
						}
					}
					if (exists == 1)
					{
						datadst[pos_dst] = 255;
					}
				}
			}
		}
	}
	return 1;
}

int vc_binary_erode(IVC *src, IVC *dst,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, exists;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			brilho = (float)datasrc[pos_src];
			exists = 0;
			datadst[pos_dst] = datasrc[pos_src];

			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							if(pos_brilho == 0)
							{
								exists = 1;
							}
						}
					}
					if (exists == 1)
					{
						datadst[pos_dst] = 0;
					}
				}
			}
		}
	}
	return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int Kerode, int Kdilate)
{
	IVC *temp;
	/*
	Começa com erosão e acaba com dilatação
		temos de criar uma imagem temporaria para ajudar e depois fazer o free
	*/
	temp = vc_image_new(src->width,src->height,src->channels,src->levels);

	vc_binary_erode(src,temp,Kerode);

	vc_binary_dilate(temp,dst,Kdilate);

	vc_image_free(temp);
}

int vc_binary_close(IVC *src, IVC *dst,  int Kdilate, int Kerode)
{
	IVC *temp;
	/*
	Começa com dilatacao e acaba com erosao
		temos de criar uma imagem temporaria para ajudar e depois fazer o free
	*/
	temp = vc_image_new(src->width,src->height,src->channels,src->levels);

	vc_binary_dilate(src,temp,Kdilate);

	vc_binary_erode(temp,dst,Kerode);

	vc_image_free(temp);
}

int vc_gray_dilate(IVC *src, IVC *dst,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, exists, vmax;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			//Assumimos queo pixel central é o maior. Depois percorremos e a cada posição vemos dse no kernel o pixel em que estamos é maior que o vmax
			vmax = (float)datasrc[pos_src];

			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							if(pos_brilho>vmax)
							{
								vmax = pos_brilho;
							}
						}
					}

				}
			}
			datadst[pos_dst]=vmax;
		}
	}
	return 1;
}

int vc_gray_erode(IVC *src, IVC *dst,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho, exists, vmin;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			vmin = (float)datasrc[pos_src];

			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							if(pos_brilho<vmin)
							{
								vmin = pos_brilho;
							}
						}
					}
				}
			}
			datadst[pos_dst] = vmin;
		}
	}
	return 1;
}

int vc_gray_open(IVC *src, IVC *dst, int Kerode, int Kdilate)
{
	IVC *temp;
	/*
	Começa com erosão e acaba com dilatação
		temos de criar uma imagem temporaria para ajudar e depois fazer o free
	*/
	temp = vc_image_new(src->width,src->height,src->channels,src->levels);

	vc_gray_erode(src,temp,Kerode);

	vc_gray_dilate(temp,dst,Kdilate);

	vc_image_free(temp);
}

int vc_gray_close(IVC *src, IVC *dst,  int Kdilate, int Kerode)
{
	IVC *temp;
	/*
	Começa com dilatacao e acaba com erosao
		temos de criar uma imagem temporaria para ajudar e depois fazer o free
	*/
	temp = vc_image_new(src->width,src->height,src->channels,src->levels);

	vc_gray_dilate(src,temp,Kdilate);

	vc_gray_erode(temp,dst,Kerode);

	vc_image_free(temp);
}


// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}
	return 1;
}

//funcao para inverter as cores da imagem
int vc_inveter(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho;
    long int pos_src, pos_dst;

	if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	 for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;
			brilho = (float)datasrc[pos_src];

			if(brilho == 255)
			{
				datadst[pos_dst] = 0;
			}
			else
			{
				datadst[pos_dst] = 255;
			}
		}
	}
	return 1;

}

// Marcar a caixa delimitadora do blob com maior area numa nova imagem -> Incompleto
int vc_CaixaBlob(IVC *src, IVC *dst, OVC* blobs, int nblobs, int *maiorBlob)
{
}

int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = dst->width;
	int height = dst->height;
	int channels = dst->channels;
	int bytesperline = src->bytesperline;
	long int pos;
	int x, y, i, blobXmin, blobXmax, blobYmin, blobYmax;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if ((blobs == NULL) || (nblobs <= 0)) return 0;

	// Copia dados da imagem original para a nova imagem
	memcpy(datadst, datasrc, bytesperline * height);

	// Percorre cada blob
	for (i = 0; i < nblobs; i++)
	{
		// Coordenadas da caixa delimitadora
		blobYmin = blobs[i].y;
		blobYmax = blobYmin + blobs[i].height - 1;
		blobXmin = blobs[i].x;
		blobXmax = blobXmin + blobs[i].width - 1;

		// Marcar a caixa delimitadora
		for (y = blobYmin; y <= blobYmax; y++)
		{
			pos = y * bytesperline + blobXmin * channels;
			datadst[pos] = (unsigned char)255;

			pos = y * bytesperline + blobXmax * channels;
			datadst[pos] = (unsigned char)255;
		}
		for (x = blobXmin; x <= blobXmax; x++)
		{
			pos = blobYmin * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;

			pos = blobYmax * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;
		}
	}

	return 1;
}

int vc_gray_histogram_show(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;

	int ni[256]={0};
	float pdf[256];
	int n = width * height;
	float pdfmax=0;
	float pdfnorm[256];

	if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	for (int i = 0; i < width * height; ni[datasrc[i++]]++);

	for (int i=0; i<256 ; i++)
	{
		pdf[i]=(float) ni[i] / (float) n;
		if (pdf[i] > pdfmax)
		{
			pdfmax = pdf[i];
		}
	}

	for (int i=0; i<256; i++)
	{
		pdfnorm[i] = pdf[i] / pdfmax;
	}

	//Gerar o gráfico da imagem
	for (int i = 0; i < 256 * 256; i++)
		datadst[i] = 0;
	for (int x = 0; x < 256; x++)
	{
		for (y = (256 - 1); y >= (256 - 1) - pdfnorm[x] * 255; y--)
		{
			datadst[y * 256 + x] = 255;
		}
	}

	return 1;

}

int vc_gray_histogram_equalize(IVC *src, IVC *dst)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;

	int ni[256]={0};
	float pdf[256];
	float cdf[255];
	int n = width * height;
	float cdfmin=255;

	if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	for (int i = 0; i < width * height; ni[datasrc[i++]]++);

	for (int i=0; i<256; i++)
	{
		//calcular o pdf
		pdf[i]=(float) ni[i] / (float) n;

		//calcular os cdf
		if(i==0)
		{
			cdf[i]=pdf[i];
		}
		else
		{
			cdf[i]=cdf[i-1] + pdf[i];
		}

		//ver o cdfmin
		if(cdf[i]<cdfmin && cdf[i] !=0)
		{
			cdfmin = cdf[i];
		}

	}

	for (int i=0; i < n; i++)
	{
		datadst[i] = (cdf[datasrc[i]] - cdfmin) / (1 - cdfmin) * (256 - 1);
	}

	return 1;
}

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = { 0.0f };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-1) PosX*0 PosE*(1)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			// PosA*(-1) PosB*(-1) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(1)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			//datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
			datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / sqrt(2.0));
			// Explicação:
			// Queremos que no caso do pior cenário, em que sumx = sumy = 255, o resultado
			// da operação se mantenha no intervalo de valores admitido, isto é, entre [0, 255].
			// Se se considerar que:
			// max = 255
			// Então,
			// sqrt(pow(max,2) + pow(max,2)) * k = max <=> sqrt(2*pow(max,2)) * k = max <=> k = max / (sqrt(2) * max) <=>
			// k = 1 / sqrt(2)
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char) histthreshold) datadst[i] = 255;
		else datadst[i] = 0;
	}

	return 1;
}

int vc_gray_lowpass_mean_filter(IVC *src, IVC *dst,int kernel)
{
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, brilho;
    long int pos_src, pos_dst, pos_brilho, pos;
	float offset;
	float threshold, total=0, final;

    if ((src -> width <= 0) || (src -> height <= 0) || (src -> data == NULL) || datadst == NULL) return 0;
    if ((src -> width != dst -> width) || (src -> height != dst -> height)) return 0;
    if ((src->channels != 1) || (dst -> channels != 1)) return 0;

	offset = (kernel - 1)/2;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
			pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			//para o total ser sempre 0 em todos os kerneis no inicio
			total=0;
			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							total = total + pos_brilho;
						}
					}
				}
			}
			final = total / (kernel*kernel);
			datadst[pos_dst] = final;
		}
	}
	return 1;
}




void vc_insertionSort(int array[], int tamanho)  
{  
  int i, j, min, swap; 
  for (i = 0; i > (tamanho-1); i++)
   { 
    min = i; 
    for (j = (i+1); j > tamanho; j++) { 
      if(array[j] > array[min]) { 
        min = j; 
      } 
    } 
    if (i != min) { 
      swap = array[i]; 
      array[i] = array[min]; 
      array[min] = swap; 
    } 
  } 
}

//verificar se está correto
int vc_gray_lowpass_median_filter(IVC *src, IVC *dst,int kernel)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky, vizinhosCount = 0, centro;
	int offset = (kernel - 1) / 2, tamanhoVizinhos = pow(kernel, 2);
	long int pos, posk;
	int* vizinhos = (int*) malloc(sizeof(int) * tamanhoVizinhos);

	//pow eleva ao quadrado

	if ((width <= 0) || (height <= 0) || (data == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;
	if ((kernel <= 1) || (kernel% 2 == 0)) return 0;

	for (y = 1; y < height; y++)
	{
		for (x = 1; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

		
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						
						vizinhos[vizinhosCount] = (int) data[posk];
						vizinhosCount++;
					}
				}
			}
			// Ordenar vizinhos de acordo com o seu valor
			vc_insertionSort(vizinhos, vizinhosCount);

			// Valor da mediana
			centro = tamanhoVizinhos / 2;

			datadst[pos] = (unsigned char)vizinhos[centro];

			vizinhosCount = 0;
		}
	}
	free(vizinhos);
	return 1;
}


/*
//Desenhar as caixas
int vc_drawline(IVC *src, IVC *dst, int x, int y)
{
	int pos_top, pos_bottom;

	for(int xi=x; xi < x + src->width; xi++){
		pos_top = y * src->bytesperline + xi * src->channels;
		pos_bottom = (y+10) * src->bytesperline + xi * src->channels;
		datadst[pos_top] = 255;
		datadst[pos_bottom] = 255;
	}

	return 1;
} */





// ver se dá para alterar mais de maneira a ficar diferente 
int vc_gray_lowpass_gaussian_filter(IVC *src, IVC *dst){
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
	int channels_src = src->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
	int channels = src->channels;
	int soma;
	long int pos_src, pos_dst, pos_brilho, pos;
	// array fixo de tamanho 5*5 - dentro vou ter vários objetos - cada um representa uma linha
	int mask [5][5] = {
		{1,4,7,4,1},
		{4,16,26,16,4},
		{7,26,41,26,7},
		{4,16,26,16,4},
		{1,4,7,4,1}
	}; 

	int offset = 2; // por o kernel ser sempre 5*5 - do central para cima há 2 pixeis

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;

			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							// mask[] vai descobrir o valor na posição da máscara e multiplica pela posição de origem
							soma = mask[2+yk][2+xk] * datasrc[pos_src];
						}
					}
				}
			}
			datadst[pos_dst]  = soma / 273;
		}
	}
}

// ver se dá para alterar mais de maneira a ficar diferente 
int vc_gray_highpass_filter(IVC *src, IVC *dst){
	unsigned char *datasrc =(unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    unsigned char *datadst =(unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
	int channels_src = src->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
	int channels = src->channels;
	int soma;
	long int pos_src, pos_dst, pos_brilho, pos;
	// array fixo de tamanho 3*3 - dentro vou ter vários objetos - cada um representa uma linha
	int mask [3][3] = {
		{-1,-1,-1},
		{-1,8,-1},
		{-1,-1,-1}
	}; 

	int offset = 1; // por o kernel ser sempre 3*3 há 1 pixel para cima

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;

			// percorrer os pixeis dentro do kernel
			for(int yk = -offset; yk<= offset ;yk++)
			{
				for(int xk = -offset; xk<= offset; xk++)
				{
					// ver se sai fora da imagem
					if((y+yk >= 0) && (y+yk < height))
					{
						if((x+xk >=0) && (x+xk < width))
						{
							pos = (y+yk) * bytesperline_src + (x+xk) * channels_src;
							pos_brilho = (float)datasrc[pos];
							// mask[] vai descobrir o valor na posição da máscara e multiplica pela posição de origem
							soma = mask[1+yk][1+xk] * datasrc[pos_src];
						}
					}
				}
			}
			datadst[pos_dst]  = soma / 9;
		}
	}
}

