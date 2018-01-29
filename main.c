// BMP.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

extern unsigned char* window(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth);
extern unsigned char* zoom(unsigned char* image_data, int ogWidth, int ogHeight, int zoomFactor);
extern unsigned char* windowSISD(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth);
extern unsigned char* zoomSISD(unsigned char* image_data, int ogWidth, int ogHeight, int zoomFactor);

#define INPUT_ERROR printf("Incorrect input.\n"); return -1;

static int DEBUG = 0;

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

		if(DEBUG) printf("Buffersize: %d\n", buffersize);

		for (int y = 0; y < h; y++) {
			if (DEBUG) printf("Start to write Row: %d\n", y);
			//Write first line
			fwrite(image_data, w, 3, f);

			// Adjust data
			image_data += w * 3 * sizeof(unsigned char);

			for (int i = 0; i < buffersize; i++) {
				// Fill with buffer
				fwrite(buffer, 1, 1, f);
			}

			if (DEBUG) printf("Write Row: %d\n", y);
		}
		if (DEBUG) printf("All Rows Finished!\n");
	}

	if (DEBUG) printf("Write Finished!\n");

	fclose(f);

	if (DEBUG) printf("File Closed!\n");
}

static unsigned char *image;
static int width, height;

static int readBmp(char *filename)
{
	FILE *file;
	file = fopen(filename, "rb");
	if (file == NULL)
	{
		INPUT_ERROR
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
		return -2;
	}

	// Allocate temporary memory to read widthnew size of data
	//unsigned char* data = (unsigned char *)malloc(widthnew * sizeof(unsigned int));

	fread(image, 3, width*height, file);

	//free(data);
	fclose(file);
	return 0;
}


static unsigned char* windowC(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth) {

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


static unsigned char* zoomC(unsigned char* image_data, int windowWidth, int windowHeight, int zoomFactor) {
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

	// Fill loop
	windowWidth *= zoomFactor;
	windowHeight *= zoomFactor;
	for(int pixelX = 0; pixelX < windowWidth; pixelX++){
		for (int pixelY = 0; pixelY < windowHeight; pixelY++) {
			
			int parentX = pixelX - ((pixelX + 1) % zoomFactor) + 1;
			int parentY = pixelY - ((pixelY + 1) % zoomFactor) + 1;
			
			if(parentX == windowWidth) parentX -= zoomFactor;
			if(parentY == windowHeight) parentY -= zoomFactor;
				
			zoom[pixelX * 3 + pixelY*windowWidth * 3] = zoom[parentX * 3 + parentY*windowWidth * 3];
			zoom[pixelX * 3 + pixelY*windowWidth * 3 + 1] = zoom[parentX * 3 + parentY*windowWidth * 3 + 1];
			zoom[pixelX * 3 + pixelY*windowWidth * 3 + 2] = zoom[parentX * 3 + parentY*windowWidth * 3 + 2];
			
		}
	}
	return zoom;
}


// Zeit in Sekunden
double getTimeSeconds()
{
	struct timespec now;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
	return now.tv_sec + (now.tv_nsec * 1E-9);
}

int main(int argc, char** argv)
{
	// initial values
	char* filename = "lena.bmp";
	int windowWidth = 512;
	int windowHeight = 512;
	int xOffset = 0;
	int yOffset = 0;
	int zoomfactor = 3;
	int mode = 0; // 0 == SIMD, 1 == SISD, 2 == C
	
	// get command line arguments
	// EXAMPLE: -dimen 512 512 -offset 0 0 -file yolo.jpeg -scale 3
	for(int i = 1; i < argc; i++){
		if(0 == strcmp(argv[i], "-file")){
			if(i+1 >= argc){INPUT_ERROR}
			
			filename = argv[++i];
		}
		else if(0 == strcmp(argv[i], "-dimen")){
			if(i+2 >= argc){INPUT_ERROR}
			
			windowWidth = atoi(argv[++i]);
			windowHeight = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-offset")){
			if(i+2 >= argc){INPUT_ERROR}
			
			xOffset = atoi(argv[++i]);
			yOffset = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-scale")){
			if(i+1 >= argc){INPUT_ERROR}
			
			zoomfactor = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-debug")){
			DEBUG = 1;
		}
		else if(0 == strcmp(argv[i], "-mode")){
			mode = atoi(argv[++i]);
		}else{INPUT_ERROR}
	}
	
	// read bitmap and check for incorrect input
	if(-1 == readBmp(filename)) return -1;
	
	// make sure input was valid
	if(windowHeight > height || windowWidth > width){
		
		printf("Invalid input: Dimensions too large!\n");
		return -1;
		
	}else if((windowHeight+yOffset) > height || (windowWidth+xOffset) > width){
		
		printf("Invalid input: Offset or dimensions too large!\n");
		return -1;
		
	}else if(windowHeight<0 ||
		windowWidth<0 ||
		xOffset<0 ||
		yOffset<0 ||
		zoomfactor<0){
			
		printf("Invalid input: Negative input!\n");
		return -1;
	}else if(windowHeight==0 ||
		windowWidth==0 ||
		zoomfactor==0){
			
		printf("Invalid input: 0 is not valid!\n");
		return -1;
	}else if(mode > 2 || mode < 0){
			
		printf("Invalid input: Unknown mode!\n");
		return -1;
	}

	clock_t start, end;

	switch(mode){
		case 0:
			start = clock();
			image = window(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("window: Time taken: SIMD-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 1:
			start = clock();
			image = windowSISD(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("window: Time taken: SISD-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 2:
			start = clock();
			image = windowC(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("window: Time taken: C-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
	}
	
	if(DEBUG)printf("Window finished\n");
	
	if(DEBUG){
		for (int i = 0; i < windowWidth*zoomfactor && i < windowHeight*zoomfactor; i++) {
			printf("Window: %d: %d\n", i, image[i * 3]);
		}
	}
	
	if(DEBUG)printf("Window output finished\n");

	switch(mode){
		case 0:
			start = clock();
			image = zoom(image, windowWidth, windowHeight, zoomfactor);
			end = clock();
			printf("zoom: Time taken: SIMD-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 1:
			start = clock();
			image = zoomSISD(image, windowWidth, windowHeight, zoomfactor);
			end = clock();
			printf("zoom: Time taken: SISD-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 2:
			start = clock();
			image = zoomC(image, windowWidth, windowHeight, zoomfactor);
			end = clock();
			printf("zoom: Time taken: C-Version: %8.8f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
	}
	
	if(DEBUG)printf("Zoom finished\n");
	
	if(DEBUG){
		for (int i = 0; i < windowWidth*zoomfactor && i < windowHeight*zoomfactor; i++) {
				printf("Zoom: %d: %d\n", i, image[i*i * 3]);
		}
	}
	
	if(DEBUG)printf("Zoom output finished\n");
	
	writeBMP(image, windowWidth*zoomfactor, windowHeight*zoomfactor);

	//system("pause");

    return 0;
}


