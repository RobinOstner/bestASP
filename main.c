#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Define external functions and their arguments
extern unsigned char* window(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth);
extern unsigned char* zoom(unsigned char* image_data, int ogWidth, int ogHeight, int factor);
extern unsigned char* windowSISD(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth);
extern unsigned char* zoomSISD(unsigned char* image_data, int ogWidth, int ogHeight, int factor);

#define INPUT_ERROR printf("Incorrect input.\n"); return -1;

// Debug output 
enum DEBUG_MODE {NONE=0, ALL, READ, WRITE, WINDOW, ZOOM, STEPS};
static int DEBUG = NONE;

void writeBMP(unsigned char* image_data, int w, int h) {
	// Header and Infoheader according to Wikipedia
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};

	int sizeData = w * h * 3;
	int filesize = sizeData + sizeof(bmpfileheader) + sizeof(bmpinfoheader);
	
	// Write information to header
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
	// Loading the actual file
	f = fopen("Copy.bmp", "wb");

	// Write headers
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	unsigned char buffer[3] = {0, 0, 0};

	if (w % 4 == 0) {
		// Writing the image if width is multiple of 4 pixels
		if(DEBUG == ALL || DEBUG == WRITE) printf("Width is multiple of 4. No buffer needed");
		fwrite(image_data, w * h, 3, f);
	}
	else {
		// Padding necessary to get to multiple of 4 pixels
		int buffersize = 4 - (w * 3) % 4;

		if(DEBUG == ALL || DEBUG == WRITE) printf("Buffersize: %d\n", buffersize);

		// Loop over image vertically and write linewise
		for (int y = 0; y < h; y++) {
			if (DEBUG == ALL || DEBUG == WRITE) printf("Write Row: %d\n", y);
			
			// Write first line
			fwrite(image_data, 3, w, f);

			// Increment pointer to next line
			image_data += w * 3 * sizeof(unsigned char);
			
			// Fill with buffer
			fwrite(buffer, 1, buffersize, f);
		}
		if (DEBUG == ALL || DEBUG == WRITE) printf("All Rows Finished!\n");
	}

	if (DEBUG == ALL || DEBUG == WRITE) printf("Write Finished!\n");

	// Close filestream
	fclose(f);

	if (DEBUG == ALL || DEBUG == WRITE) printf("File Closed!\n");
}

static unsigned char *image;
static int width, height;

static int readBmp(char *filename)
{
	// Open the filestream
	FILE *file;
	file = fopen(filename, "rb");
	// Stop if file couldn't be opened
	if (file == NULL)
	{
		if(DEBUG == ALL || DEBUG == READ) printf("Couldn't read file.");
		INPUT_ERROR
	}

	unsigned char header[54];

	// Read header
	fread(header, sizeof(unsigned char), 54, file);

	// Capture dimensions
	width = * (int*) &header[18];
	height = * (int*) &header[22];

	if(DEBUG == ALL || DEBUG == READ){
		printf("Input image dimensions: %d : %d\n", width, height);
	}
	
	int padding = 0;

	// Calculate padding
	while ((width * 3 + padding) % 4 != 0)
	{
		padding++;
	}

	// Compute new width, which includes padding
	int widthnew = width * 3 + padding;

	// Allocate memory to store image data (non-padded)
	image = (unsigned char *) malloc(width * height * 3 * sizeof(unsigned char));
	// Stop if malloc failed
	if (image == NULL)
	{
		printf("Error: Malloc failed\n");
		return -2;
	}

	// Read actual image
	fread(image, 3, width * height, file);

	// Close stream
	fclose(file);
	return 0;
}


static unsigned char* windowC(unsigned char* image_data, int xPos, int yPos, int width, int height, int originalWidth) {

	// Allocate memory to store image data (non-padded)
	int size = width * height * 3;
	unsigned char* window = (unsigned char *) malloc(size * sizeof(unsigned char));
	
	// Stop if malloc failed
	if (window == NULL)
	{
		printf("Error: Malloc failed\n");
		return image_data;
	}
	
	// Add the offset to the pointer right away
	image_data += ((xPos + originalWidth * yPos) * 3);
	// Line to read from
	unsigned char* currentLine = image_data;
	// Pointer to write to
	unsigned char* currentWindowPointer = window;
	// Last pixel of the new image
	unsigned char* end = window + size;
	
	// Loop over all pixels of new image
	for(; currentWindowPointer < end;){
		
		// Loop over the current line and copy all bytes to new image
		for (int loopX = 0; loopX < width; ++loopX){
			*(currentWindowPointer++) = *(currentLine++);
			*(currentWindowPointer++) = *(currentLine++);
			*(currentWindowPointer++) = *(currentLine++);
		}
		// set currentLine to the next line
		image_data += originalWidth * 3;
		currentLine = image_data;
	}

	return window;
}


static unsigned char* zoomC(unsigned char* image_data, int windowWidth, int windowHeight, int factor) {
	// Allocate memory to store image data (non-padded)
	unsigned char* zoom = (unsigned char *) malloc(windowHeight * windowWidth * factor * factor * 3 * sizeof(unsigned char));
	
	// Stop if malloc failed
	if (zoom == NULL)
	{
		printf("Error: Malloc failed\n");
		return image_data;
	}
	
	// Last pointer of old image
	unsigned char* end = image_data + windowWidth * windowHeight * 3;
	// Increment between pixels in new image
	unsigned int increment = factor * 3;
	// Increment between lines in new image
	unsigned int lineIncrement = increment * windowWidth * (factor - 1);
	
	unsigned char* pointerOld = image_data;
	unsigned char* pointerNew = zoom;
	
	for(int counter = 0; pointerOld < end;){
		if(DEBUG == ALL || DEBUG == ZOOM)
			printf("Zoom copy loop: \tCounter: %d, \twindowWidth: %d, \tindex old: %d, \tindex new: %d\n",
				counter, windowWidth, pointerOld - image_data, pointerNew - zoom);
				
		// Write pixel and increment pointerOld
		*(pointerNew) = *(pointerOld++);
		*(pointerNew+1) = *(pointerOld++);
		*(pointerNew+2) = *(pointerOld++);
		
		// increment pointerNew
		pointerNew += increment;
		++counter;
		
		// if counter exceeds windowWidth, jump to next line
		if(counter >= windowWidth){
			pointerNew += lineIncrement;
			counter = 0;
		}
	}

	// Fill loop
	windowWidth *= factor;
	windowHeight *= factor;
	for(int pixelX = 0; pixelX < windowWidth; pixelX++){
		for (int pixelY = 0; pixelY < windowHeight; pixelY++) {
		
			// Calculate coordinates of pixel to take color from
			// Coordinates used are offset by +1 / +1, in order to calculate according to Nearest Neighbor algorithm described in the assignment.
			int parentX = pixelX + 1 - ((pixelX + 1) % factor);
			int parentY = pixelY + 1 - ((pixelY + 1) % factor);
			
			// Edge handling
			if(parentX == windowWidth) parentX -= factor;
			if(parentY == windowHeight) parentY -= factor;

			// The index in the array for the current pixel
			// Index of current pixel
			int currentIndex = (pixelX + pixelY * windowWidth) * 3;
			// Index of pixel to take color from
			int parentIndex = (parentX + parentY * windowWidth) * 3;
			
			// Write 3 bytes. One per color channel
			zoom[currentIndex] = zoom[parentIndex];
			zoom[currentIndex + 1] = zoom[parentIndex + 1];
			zoom[currentIndex + 2] = zoom[parentIndex + 2];
			
		}
	}
	return zoom;
}

int main(int argc, char** argv)
{
	// initial values
	char* filename = "lena.bmp";
	int windowWidth = -1;
	int windowHeight = -1;
	int xOffset = 0;
	int yOffset = 0;
	int factor = 3;
	int mode = 0;	// 0 == SIMD, 1 == SISD, 2 == C
	
	// get command line arguments
	// use -help in commandline to get a description of every command
	for(int i = 1; i < argc; i++){
		if(0 == strcmp(argv[i], "-file")){
			// Check if there is a parameter following
			if(i+1 >= argc){INPUT_ERROR}
			
			filename = argv[++i];
		}
		else if(0 == strcmp(argv[i], "-dimen")){
			// Check if there are two parameters following
			if(i+2 >= argc){INPUT_ERROR}
			
			windowWidth = atoi(argv[++i]);
			windowHeight = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-offset")){
			// Check if there are two parameters following
			if(i+2 >= argc){INPUT_ERROR}
			
			xOffset = atoi(argv[++i]);
			yOffset = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-scale")){
			// Check if there is a parameter following
			if(i+1 >= argc){INPUT_ERROR}
			
			factor = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-debug")){
			// Default to ALL if no parameter is following
			if(++i >= argc){
				DEBUG = ALL;
			}else if(0 == strcmp(argv[i], "read")){
				DEBUG = READ;
			}else if(0 == strcmp(argv[i], "write")){
				DEBUG = WRITE;
			}else if(0 == strcmp(argv[i], "window")){
				DEBUG = WINDOW;
			}else if(0 == strcmp(argv[i], "zoom")){
				DEBUG = ZOOM;
			}else if(0 == strcmp(argv[i], "steps")){
				DEBUG = STEPS;
			}else if(0 == strcmp(argv[i], "all")){
				DEBUG = ALL;
			}else{
				// Default to ALL if following argument is none of the above
				DEBUG = ALL;
				// --i, because following argument is likely to be a different parameter
				--i;
			}
		}
		else if(0 == strcmp(argv[i], "-mode")){
			mode = atoi(argv[++i]);
		}
		else if(0 == strcmp(argv[i], "-help")){
			printf("\nADDITIONAL PARAMETERS:\n\n");
			
			printf("-file FILENAME\n");
			printf("\tstring FILENAME: Name of the input image to be loaded.\n");
			printf("\t\t(No quotation marks needed. Filename extension required.)\n");
			printf("\t\t(Default: lena.bmp)\n");
			printf("\t\t(Currently supported formats: BMP.)\n\n");
			
			printf("-dimen WIDTH HEIGHT\n");
			printf("\tint WIDTH: Width of the cutout to be taken from input picture.\n");
			printf("\t\t(Default: -1. -1 will be set to width of input image.)\n");
			printf("\tint HEIGHT: Height of the cutout to be taken from input picture.\n");
			printf("\t\t(Default: -1. -1 will be set to height of input image.)\n");
			printf("\t\t(Dimensions have to be <= dimensions of input image.)\n\n");
			
			printf("-offset X Y\n");
			printf("\tint X: Horizontal offset of cutout. Default: 0\n");
			printf("\tint Y: Vertical offset of cutout. Default: 0\n");
			printf("\t\t(Cutout dimensions + Offset have to fit inside input image.)\n\n");
			
			printf("-scale SCALE\n");
			printf("\tint SCALE: Scale factor to be applied to width AND height of cutout.\n");
			printf("\t\t(Has to be >0)\n\n");
			
			printf("-mode MODE\n");
			printf("\tint MODE: Execution mode.\n");
			printf("\t\t0: ASM SIMD (Default)\n");
			printf("\t\t1: ASM SISD\n");
			printf("\t\t2: C\n\n");
			
			printf("-debug {MODE}\n");
			printf("\tstring MODE: Debug mode. Optional.\n");
			printf("\t\t(No quotation marks needed.)\n");
			printf("\t\tall: All debug modes simultaneously. (Default)\n");
			printf("\t\tread: Debug loading of input image.\n");
			printf("\t\twrite: Debug writing of output image.\n");
			printf("\t\twindow: Debug window function. (cutout from input image)\n");
			printf("\t\tzoom: Debug zoom function. (upscaling of cutout)\n");
			printf("\t\tsteps: Display execution steps.\n");
			printf("\t\t(Some debug modes may not give any output. This is for development only.)\n");
			return 0;
		}
		// If parameter was not recognized
		else{INPUT_ERROR}
	}
	
	if(DEBUG == ALL || DEBUG == STEPS) printf("Input handling finished\n");
	
	// Read bitmap and check for incorrect input
	if(0 != readBmp(filename)) return -1;
	if(DEBUG == ALL || DEBUG == STEPS) printf("Read finished\n");
	
	// Default width and height are equal to input's
	if(windowHeight == -1) windowHeight = height;
	if(windowWidth == -1) windowWidth = width;
	
	// Make sure input was valid
	if(windowHeight > height || windowWidth > width){
		
		printf("Invalid input: Dimensions too large!\n");
		return -1;
		
	}else if((windowHeight + yOffset) > height || (windowWidth + xOffset) > width){
		
		printf("Invalid input: Offset or dimensions too large!\n");
		return -1;
		
	}else if(windowHeight < 0 ||
		windowWidth < 0 ||
		xOffset < 0 ||
		yOffset < 0 ||
		factor < 0){
			
		printf("Invalid input: Negative input!\n");
		return -1;
		
	}else if(windowHeight == 0 ||
		windowWidth == 0 ||
		factor == 0){
			
		printf("Invalid input: 0 is not valid!\n");
		return -1;
		
	}else if(mode > 2 || mode < 0){
			
		printf("Invalid input: Unknown mode!\n");
		return -1;
		
	}
	
	if(DEBUG == ALL || DEBUG == STEPS) printf("Input checks finished\n");

	// Declare variables for time measuring
	clock_t start, end;

	// WINDOW
	// Switch to mode defined by input parameter
	// Capture time taken by execution of function and print.
	if(DEBUG == ALL || DEBUG == WINDOW)
		printf("Calling window with following parameters:\nOffset: %d : %d\nDimensions: %d : %d\nogWidth: %d\n", xOffset, yOffset, windowWidth, windowHeight, width);
	switch(mode){
		case 0:
			start = clock();
			image = window(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("Window: Time taken: SIMD-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 1:
			start = clock();
			image = windowSISD(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("Window: Time taken: SISD-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 2:
			start = clock();
			image = windowC(image, xOffset, yOffset, windowWidth, windowHeight, width);
			end = clock();
			printf("Window: Time taken: C-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
	}
	
	if(DEBUG == ALL || DEBUG == STEPS) printf("Window finished\n");

	
	if(DEBUG == ALL || DEBUG == WINDOW){
		// Write output image to disc
		writeBMP(image, windowWidth, windowHeight);
		return 0;
	}
	
	// ZOOM
	// Switch to mode defined by input parameter
	// Capture time taken by execution of function and print.
	switch(mode){
		case 0:
			start = clock();
			image = zoom(image, windowWidth, windowHeight, factor);
			end = clock();
			printf("Zoom: Time taken: SIMD-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 1:
			start = clock();
			image = zoomSISD(image, windowWidth, windowHeight, factor);
			end = clock();
			printf("Zoom: Time taken: SISD-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
		case 2:
			start = clock();
			image = zoomC(image, windowWidth, windowHeight, factor);
			end = clock();
			printf("Zoom: Time taken: C-Version: %6.6f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
			break;
	}
	
	if(DEBUG == ALL || DEBUG == STEPS) printf("Zoom finished\n");
	
	// Write output image to disc
	writeBMP(image, windowWidth * factor, windowHeight * factor);

	if(DEBUG == ALL || DEBUG == STEPS) printf("Write finished\n");
    return 0;
}


