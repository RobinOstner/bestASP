// BMP.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

//#include <stdafx.h>
#include <stdlib.h>
#include <stdio.h>
//#include <vector>
//#include <iterator>

//using namespace std;

extern void windowImage(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth);

extern unsigned char* ZoomImage(unsigned char* image_data, int ogWidth, int ogHeight, int zoomFactor);

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

	unsigned char buffer[1] = { 0 };


	if (w % 4 == 0) {
		//schreiben des Bildes
		fwrite(image_data, w*h, 3, f);
	}
	else {
		int buffersize = 4 - (w*3) % 4;

		printf("Buffersize: %d\n", buffersize);

		for (int y = 0; y < h; y++) {

			//Write first line
			fwrite(image_data, w, 3, f);

			// Adjust data
			image_data += w * 3 * sizeof(unsigned char);

			for (int i = 0; i < buffersize; i++) {
				// Fill with buffer
				fwrite(buffer, 1, 1, f);
			}
		}
	}
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
		printf("Error: fopen failed\n");
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

/*
static unsigned char* windowImage(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth) {

	// Allocate memory to store image data (non-padded)
	unsigned char* window = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	
	if (window == NULL)
	{
		printf("Error: Malloc failed\n");
		return image_data;
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			window[(x + y*width) * 3] = image[(xPos+yPos*originalWidth)*3 + (x + y*originalWidth) * 3];
			window[(x + y*width) * 3+1] = image[(xPos + yPos*originalWidth) * 3 + (x + y*originalWidth) * 3+1];
			window[(x + y*width) * 3+2] = image[(xPos + yPos*originalWidth) * 3 + (x + y*originalWidth) * 3+2];
		}
	}

	return window;
}
*/


/*
static unsigned char* zoomImage(unsigned char* image_data, int windowWidth, int windowHeight, int zoomFactor) {
	// Allocate memory to store image data (non-padded)
	unsigned char* zoom = (unsigned char *)malloc(windowHeight*windowWidth * zoomFactor * zoomFactor * 3 * sizeof(unsigned char));
	
	if (zoom == NULL)
	{
		printf("Error: Malloc failed\n");
		return image_data;
	}

	// Loop over all original pixels
	for(int pixelX = 0; pixelX < windowWidth; pixelX++){
		for (int pixelY = 0; pixelY < windowHeight; pixelY++) {

			// The index in the array for the current pixel
			int pixelIndex = pixelX*zoomFactor * 3 + pixelY*zoomFactor*zoomFactor*windowWidth * 3;

			zoom[pixelIndex] = image_data[pixelX * 3 + pixelY*windowWidth * 3];
			zoom[pixelIndex+1] = image_data[pixelX * 3 + pixelY*windowWidth * 3+1];
			zoom[pixelIndex+2] = image_data[pixelX * 3 + pixelY*windowWidth * 3+2];

			// Fill the rest
			for (int x = 0; x < zoomFactor; x++) {
				for (int y = 0; y < zoomFactor; y++) {

					if (x == 0 && y == 0) {
						continue;
					}

					zoom[pixelIndex + x * 3 + y*zoomFactor*windowWidth * 3] = 0;
					zoom[pixelIndex + x * 3 + y*zoomFactor*windowWidth * 3 + 1] = 0;
					zoom[pixelIndex + x * 3 + y*zoomFactor*windowWidth * 3 + 2] = 0;
				}
			}

		}
	}

	for (int i = 0; i < zoomFactor - 1; i++){

		// Loop over all original pixels
		for (int pixelX = 0; pixelX < windowWidth; pixelX++) {
			for (int pixelY = 0; pixelY < windowHeight; pixelY++) {

				// The index in the array for the current pixel
				int pixelIndex = pixelX*zoomFactor * 3 + pixelY*zoomFactor*zoomFactor*windowWidth * 3;

				// Fill the rest
				for (int x = -i; x < i; x++) {
					for (int y = -i; y < i; y++) {
						if (x == 0 && y == 0) {
							continue;
						}

						int fillPixelIndex = pixelIndex + x * 3 + y*zoomFactor*windowWidth * 3;

						if (fillPixelIndex < 0) { continue; }
						if (zoom[fillPixelIndex] != 0) { continue; }

						// Left
						if (fillPixelIndex-3 > 0 && zoom[fillPixelIndex - 3] != 0) {
							zoom[fillPixelIndex] = zoom[fillPixelIndex - 3];
							zoom[fillPixelIndex + 1] = zoom[fillPixelIndex - 2];
							//zoom[fillPixelIndex + 2] = zoom[fillPixelIndex - 1];
						}
						else {
							// Right
							if (fillPixelIndex + 3 < windowWidth*zoomFactor && zoom[fillPixelIndex + 3] != 0) {
								zoom[fillPixelIndex] = zoom[fillPixelIndex - 3];
								zoom[fillPixelIndex + 1] = zoom[fillPixelIndex - 2];
								//zoom[fillPixelIndex + 2] = zoom[fillPixelIndex - 1];
							}
							else {
								// Up
								if (fillPixelIndex + windowWidth*zoomFactor * 3 < windowWidth * windowHeight* zoomFactor * zoomFactor * 3 && zoom[fillPixelIndex + windowWidth*zoomFactor * 3] != 0) {
									zoom[fillPixelIndex] = zoom[fillPixelIndex + windowWidth*zoomFactor * 3];
									zoom[fillPixelIndex + 1] = zoom[fillPixelIndex + windowWidth*zoomFactor * 3 + 1];
									//zoom[fillPixelIndex + 2] = zoom[fillPixelIndex + originalWidth*zoomFactor * 3 + 2];
								}
								else {
									// Down
									if (fillPixelIndex - windowWidth*zoomFactor * 3 >= 0 && zoom[fillPixelIndex - windowWidth*zoomFactor * 3] != 0) {
										zoom[fillPixelIndex] = zoom[fillPixelIndex - windowWidth*zoomFactor * 3];
										zoom[fillPixelIndex + 1] = zoom[fillPixelIndex - windowWidth*zoomFactor * 3 + 1];
										//zoom[fillPixelIndex + 2] = zoom[fillPixelIndex - originalWidth*zoomFactor * 3 + 2];
									}
								}
							}
						}
					}
				}

			}
		}
	}

	return zoom;
}
*/

int main()
{
	char* filename = "Confirm-512.bmp";

	readBmp(filename);

	int windowWidth = 512;
	int windowHeight = 512;
	int xOffset = 0;
	int yOffset = 0;
	int zoomfactor = 3;

	windowImage(image, xOffset, yOffset, windowWidth, windowHeight, width);

	printf("Window:\n");

	for (int i = 0; i < windowWidth && i < windowHeight; i++) {
		printf("%d: %d\n", i, image[i*i * 3]);
	}

	image = ZoomImage(image, windowHeight, windowWidth, zoomfactor);

	printf("Zoom:\n");

	for (int i = 0; i < windowWidth && i < windowHeight; i++) {
			printf("%d: %d\n", i, image[i*i * 3]);
	}

	writeBMP(image, windowWidth*zoomfactor, windowHeight*zoomfactor);

	//system("pause");

    return 0;
}

