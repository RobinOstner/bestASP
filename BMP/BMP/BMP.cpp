// BMP.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <fstream>
#include <vector>
#include <iterator>

using namespace std;

void writeBMP(unsigned char* image_data, int w, int h) {
	//header und infoheader nach wikipedia definition
	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };

	int sizeData = w*h * 3;
	int filesize = sizeData + sizeof(bmpfileheader) + sizeof(bmpinfoheader);
	
	/* Construct header with filesize part */
	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);
	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	bmpinfoheader[20] = (unsigned char)(sizeData);
	bmpinfoheader[21] = (unsigned char)(sizeData >> 8);
	bmpinfoheader[22] = (unsigned char)(sizeData >> 16);
	bmpinfoheader[23] = (unsigned char)(sizeData >> 24);

	bmpinfoheader[28] = 32;

	FILE *f;
	//tatsächliches öffnen des files
	f = fopen("Copy.bmp", "wb");

	//die beiden header werden zuerst geschrieben
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	//schreiben des Bildes
	fwrite(image_data, w*h, 3, f);
	fclose(f);
}

static unsigned char *image;
static int width, height;

static void readBmp(char *filename)
{
	FILE *file;
	file = fopen(filename, "rb");
	if (file == NULL)
	{
		cout<<"Error: fopen failed\n";
		return;
	}

	unsigned char header[54];

	// Read header
	fread(header, sizeof(unsigned char), 54, file);

	// Capture dimensions
	width = *(int*)&header[18];
	height = *(int*)&header[22];

	int padding = 0;

	// Calculate padding
	while ((width * 3 + padding) % 4 != 0)
	{
		padding++;
	}

	// Compute new width, which includes padding
	int widthnew = width * 3 + padding;

	// Allocate memory to store image data (non-padded)
	image = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	if (image == NULL)
	{
		printf("Error: Malloc failed\n");
		return;
	}

	// Allocate temporary memory to read widthnew size of data
	unsigned char* data = (unsigned char *)malloc(widthnew * sizeof(unsigned int));

	fread(image, 3, width*height, file);

	//// Read row by row of data and remove padded data.
	//for (int i = 0; i<height; i++)
	//{
	//	// Read widthnew length of data
	//	fread(data, sizeof(unsigned char), widthnew, file);

	//	// Retain width length of data, and swizzle RB component.
	//	// BMP stores in BGR format, my usecase needs RGB format
	//	for (int j = 0; j < width * 3; j += 3)
	//	{
	//		int index = (i * width * 3) + (j);
	//		image[index + 0] = data[j + 0];
	//		image[index + 1] = data[j + 1];
	//		image[index + 2] = data[j + 2];
	//	}
	//}

	free(data);
	fclose(file);
}

int main()
{
	char* filename = "Confirm-512.bmp";

	readBmp(filename);

	writeBMP(image, width, height);

	system("pause");

    return 0;
}

