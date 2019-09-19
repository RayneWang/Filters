// modified from CS351 project 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ppmIO.h"

#define USECPP 0

//taken from stack overflow 
//https://stackoverflow.com/questions/5919663/how-does-photoshop-blend-two-images-together/5925219#5925219
#define ChannelBlend_Normal(A,B)     ((int)(A))
#define ChannelBlend_Lighten(A,B)    ((int)((B > A) ? B:A))
#define ChannelBlend_Darken(A,B)     ((int)((B > A) ? A:B))
#define ChannelBlend_Multiply(A,B)   ((int)((A * B) / 255))
#define ChannelBlend_Average(A,B)    ((int)((A + B) / 2))
#define ChannelBlend_Add(A,B)        ((int)(min(255, (A + B))))
#define ChannelBlend_Subtract(A,B)   ((int)((A + B < 255) ? 0:(A + B - 255)))
#define ChannelBlend_Difference(A,B) ((int)(abs(A - B)))
#define ChannelBlend_Negation(A,B)   ((int)(255 - abs(255 - A - B)))
#define ChannelBlend_Screen(A,B)     ((int)(255 - (((255 - A) * (255 - B)) >> 8)))
#define ChannelBlend_Exclusion(A,B)  ((int)(A + B - 2 * A * B / 255))
#define ChannelBlend_Overlay(A,B)    ((int)((B < 128) ? (2 * A * B / 255):(255 - 2 * (255 - A) * (255 - B) / 255)))
#define ChannelBlend_SoftLight(A,B)  ((int)((B < 128)?(2*((A>>1)+64))*((float)B/255):(255-(2*(255-((A>>1)+64))*(float)(255-B)/255))))
#define ChannelBlend_HardLight(A,B)  (ChannelBlend_Overlay(B,A))
#define ChannelBlend_ColorDodge(A,B) ((int)((B == 255) ? B:min(255, ((A << 8 ) / (255 - B)))))
#define ChannelBlend_ColorBurn(A,B)  ((int)((B == 0) ? B:max(0, (255 - ((255 - A) << 8 ) / B))))
#define ChannelBlend_LinearDodge(A,B)(ChannelBlend_Add(A,B))
#define ChannelBlend_LinearBurn(A,B) (ChannelBlend_Subtract(A,B))
#define ChannelBlend_LinearLight(A,B)((int)(B < 128)?ChannelBlend_LinearBurn(A,(2 * B)):ChannelBlend_LinearDodge(A,(2 * (B - 128))))
#define ChannelBlend_VividLight(A,B) ((int)(B < 128)?ChannelBlend_ColorBurn(A,(2 * B)):ChannelBlend_ColorDodge(A,(2 * (B - 128))))
#define ChannelBlend_PinLight(A,B)   ((int)(B < 128)?ChannelBlend_Darken(A,(2 * B)):ChannelBlend_Lighten(A,(2 * (B - 128))))
#define ChannelBlend_HardMix(A,B)    ((int)((ChannelBlend_VividLight(A,B) < 128) ? 0:255))
#define ChannelBlend_Reflect(A,B)    ((int)((B == 255) ? B:min(255, (A * A / (255 - B)))))
#define ChannelBlend_Glow(A,B)       (ChannelBlend_Reflect(B,A))
#define ChannelBlend_Phoenix(A,B)    ((int)(min(A,B) - max(A,B) + 255))
#define ChannelBlend_Alpha(A,B,O)    ((int)(O * A + (1 - O) * B))
#define ChannelBlend_AlphaF(A,B,F,O) (ChannelBlend_Alpha(F(A,B),A,O))


#define min(A, B)   ((A>B) ? B:A)
#define max(A, B)   ((A<B) ? B:A)

// read in rgb values from the ppm file output by cqcam
Pixel *readPPM(int *rows, int *cols, int * colors, char *filename) {
   char tag[40];
   Pixel *image;
   FILE *fp;
   int read, num[3], curchar;

   if(filename != NULL && strlen(filename))
     fp = fopen(filename, "r");
   else
     fp = stdin;

   if(fp) {
     fscanf(fp, "%s\n", tag);

     // Read the "magic number" at the beginning of the ppm
     if (strncmp(tag, "P6", 40) != 0) {
       fprintf(stderr, "not a ppm!\n");
       exit(1);
     }
     
     // Read the rows, columns, and color depth output by cqcam
     // need to read in three numbers and skip any lines that start with a #
     read = 0;
     while(read < 3) {
       curchar = fgetc(fp);
       if((char)curchar == '#') { // skip this line
	 while(fgetc(fp) != '\n')
	   /* do nothing */;
       }
       else {
	 ungetc(curchar, fp);
	 fscanf(fp, "%d", &(num[read]));
	 read++;
       }
     }
     while(fgetc(fp) != '\n')
       /* pass the last newline character */;

     *cols = num[0];
     *rows = num[1];
     *colors = num[2];

     if(*cols > 0 && *rows > 0) {
#if USECPP
       image = new Pixel[(*rows) * (*cols)];
#else
       image = (Pixel *)malloc(sizeof(Pixel)* (*rows) * (*cols));
#endif
       if(image) {
	 // Read the data
	 fread(image, sizeof(Pixel), (*rows) * (*cols), fp);

	 if(fp != stdin)
	   fclose(fp);

	 return(image);
       }
     }

   }
   
   return(NULL);
     
} // end read_ppm



// Write the modified image out as a ppm in the correct format to be read by 
// read_ppm.  xv will read these properly.
void writePPM(Pixel *image, int rows, int cols, int colors, char *filename)
{
  FILE *fp;

  if(filename != NULL && strlen(filename))
    fp = fopen(filename, "w");
  else
    fp = stdout;

  if(fp) {
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n%d\n", cols, rows, colors);

    fwrite(image, sizeof(Pixel), rows * cols, fp);
  }

  fclose(fp);

} // end write_ppm 


// Write the modified image out as a pgm in the correct format
void writePGM(unsigned char *image, long rows, long cols, int intensities, char *filename)
{
  FILE *fp;

  if(filename != NULL && strlen(filename))
    fp = fopen(filename, "w");
  else
    fp = stdout;

  if(fp) {
    fprintf(fp, "P5\n");
    fprintf(fp, "%ld %ld\n%d\n", cols, rows, intensities);

    fwrite(image, sizeof(unsigned char), rows * cols, fp);
  }

  if(fp != stdout)
    fclose(fp);
} // end write_pgm 


// read in intensity values from the pgm file
unsigned char *readPGM(int *rows, int *cols, int *intensities, char *filename) {
   char tag[40];
   unsigned char *image;
   FILE *fp;
   int read, num[3], curchar;

   if(filename != NULL && strlen(filename))
     fp = fopen(filename, "r");
   else
     fp = stdin;

   if(fp) {
     fscanf(fp, "%s\n", tag);

     // Read the "magic number" at the beginning of the ppm
     if (strncmp(tag, "P5", 40) != 0) {
       fprintf(stderr, "not a ppm!\n");
       exit(1);
     }
     
     // Read the rows, columns, and color depth output by cqcam
     // need to read in three numbers and skip any lines that start with a #
     read = 0;
     while(read < 3) {
       curchar = fgetc(fp);
       if((char)curchar == '#') { // skip this line
	 while(fgetc(fp) != '\n')
	   /* do nothing */;
       }
       else {
	 ungetc(curchar, fp);
	 fscanf(fp, "%d", &(num[read]));
	 read++;
       }
     }
     while(fgetc(fp) != '\n')
       /* pass the last newline character */;

     *cols = num[0];
     *rows = num[1];
     *intensities = num[2];

     if(*intensities != 255) {
       printf("Unable to read this file correctly\n");
       return(NULL);
     }

     if(*cols > 0 && *rows > 0) {
#if USECPP
       image = new unsigned char[(*rows) * (*cols)];
#else
       image = (unsigned char *)malloc(sizeof(unsigned char) * (*rows) * (*cols));
#endif
       if(image) {
	 // Read the data
	 fread(image, sizeof(unsigned char), (*rows) * (*cols), fp);

	 if(fp != stdin)
	   fclose(fp);

	 return(image);
       }
     }

   }
   
   return(NULL);
     
} // end read_pgm

Pixel *edgeDetect(Pixel *img, int row, int col) {
	
	//grayscale first 
	int imagesize = (long)row * (long)col;
	
	Pixel *image = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int i=0;i<imagesize;i++) {

		int val = ((int)img[i].r + (int)img[i].g + (int)img[i].b) / 3;
		image[i].r = val;
		image[i].g = val;
		image[i].b = val;
	}

	//sobel edge detection
	Pixel *output = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	float kernelX[3][3] = {{-1, 0, 1}, 
                       {-2, 0, 2}, 
                       {-1, 0, 1}};
	
	float kernelY[3][3] = {{-1, -2, -1}, 
                        {0,  0,  0}, 
                        {1,  2,  1}};

	for(int x = 1; x < row-1; x++){
		for(int y = 1; y < col-1; y++){            
			int pixel_x = (kernelX[0][0] * image[(x-1)*col + (y-1)].r) + (kernelX[0][1] * image[(x)*col + (y-1)].r) + (kernelX[0][2] * image[(x+1)*col + (y-1)].r) +
              (kernelX[1][0] * image[(x-1)*col + (y)].r)   + (kernelX[1][1] * image[(x)*col + (y)].r)   + (kernelX[1][2] * image[(x+1)*col + (y)].r) +
              (kernelX[2][0] * image[(x-1)*col + (y+1)].r) + (kernelX[2][1] * image[(x)*col + (y+1)].r) + (kernelX[2][2] * image[(x+1)*col + (y+1)].r);

			int pixel_y = (kernelY[0][0] * image[(x-1)*col + (y-1)].r) + (kernelY[0][1] * image[(x)*col + (y-1)].r) + (kernelY[0][2] * image[(x+1)*col + (y-1)].r) +
              (kernelY[1][0] * image[(x-1)*col + (y)].r)   + (kernelY[1][1] * image[(x)*col + (y)].r)   + (kernelY[1][2] * image[(x+1)*col + (y)].r) +
              (kernelY[2][0] * image[(x-1)*col + (y+1)].r) + (kernelY[2][1] * image[(x)*col + (y+1)].r) + (kernelY[2][2] * image[(x+1)*col + (y+1)].r);

			
			int val = sqrt((pixel_x * pixel_x) + (pixel_y * pixel_y));

			output[x*col+y].r = 255-val;
			output[x*col+y].g = 255-val;
			output[x*col+y].b = 255-val;
		}
	 }
	 
	 return(output);
}

Pixel *filter(Pixel *image, Pixel *edge, int row, int col) {
	
	Pixel *output = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int x = 0; x < row; x++){
		for(int y = 0; y < col; y++){ 
			if(edge[x*col+y].r > 100){
				output[x*col+y].r = ChannelBlend_ColorBurn(image[x*col+y].r, edge[x*col+y].r); 
				output[x*col+y].g = ChannelBlend_ColorBurn(image[x*col+y].g, edge[x*col+y].g); 
				output[x*col+y].b = ChannelBlend_ColorBurn(image[x*col+y].b, edge[x*col+y].b); 
			}else{
				output[x*col+y].r = 0;
				output[x*col+y].g = 0;
				output[x*col+y].b = 0;
			}
		}
	}
	
	return(output);
	
}

Pixel *filter2(Pixel *image, Pixel *edge, int row, int col) {
	
	Pixel *output = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int x = 1; x < row-1; x++){
		for(int y = 1; y < col-1; y++){ 
			if(edge[x*col+y].r > 200){
				output[x*col+y].r = (image[(x-1)*col + (y-1)].r + image[(x)*col + (y-1)].r + image[(x+1)*col + (y-1)].r +
				  image[(x-1)*col + (y)].r  + image[(x)*col + (y)].r   + image[(x+1)*col + (y)].r +
				  image[(x-1)*col + (y+1)].r + image[(x)*col + (y+1)].r + image[(x+1)*col + (y+1)].r)/9;

				output[x*col+y].g = (image[(x-1)*col + (y-1)].g + image[(x)*col + (y-1)].g + image[(x+1)*col + (y-1)].g +
				  image[(x-1)*col + (y)].g  + image[(x)*col + (y)].g   + image[(x+1)*col + (y)].g +
				  image[(x-1)*col + (y+1)].g + image[(x)*col + (y+1)].g + image[(x+1)*col + (y+1)].g)/9;
				  
				output[x*col+y].b = (image[(x-1)*col + (y-1)].b + image[(x)*col + (y-1)].b + image[(x+1)*col + (y-1)].b +
				  image[(x-1)*col + (y)].b  + image[(x)*col + (y)].b   + image[(x+1)*col + (y)].b +
				  image[(x-1)*col + (y+1)].b + image[(x)*col + (y+1)].b + image[(x+1)*col + (y+1)].b)/9;
			}else{
				output[x*col+y].r = 0;
				output[x*col+y].g = 0;
				output[x*col+y].b = 0;
			}
		}
	}
	
	return(output);
}
	
Pixel *filter3(Pixel *image, Pixel *edge, int row, int col) {
	
	Pixel *output = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int x = 0; x < row; x++){
		for(int y = 0; y < col; y++){ 
			if(edge[x*col+y].r > 100){
				int val = ((int)image[x*col+y].r + (int)image[x*col+y].g + (int)image[x*col+y].b) / 3;
				output[x*col+y].r = ChannelBlend_ColorDodge((255-val), val); 
				output[x*col+y].g = output[x*col+y].r;
				output[x*col+y].b = output[x*col+y].r;
			}
		}
	}
	
	Pixel *output2 = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int x = 0; x < row; x++){
		for(int y = 0; y < col; y++){ 
		
			float alpha = (output[x*col+y].r)/255.0;
			if(alpha == 0){
				alpha = 0.05;
			}
			output2[x*col+y].r = image[x*col+y].r * alpha; 
			output2[x*col+y].g = image[x*col+y].g * alpha; 
			output2[x*col+y].b = image[x*col+y].b * alpha; 
			
		}
	}
	
	return(output2);
	
}

Pixel *filter4(Pixel *image, Pixel *edge, int row, int col, int k) {
	
	Pixel *output = (Pixel *)malloc(sizeof(Pixel)* (row) * (col));
	
	for(int x = 0; x < row-k; x+=k){
		for(int y = 0; y < col-k; y+=k){
				
			int sum_r = 0; 
			int sum_g = 0; 
			int sum_b = 0; 
			int total = 0;
			
			for(int i=0; i<k; i++){
				for(int j=0; j<k; j++){
					
					if(edge[(x+i)*col + (y+j)].r > 200){
						sum_r += image[(x+i)*col + (y+j)].r; 
						sum_g += image[(x+i)*col + (y+j)].g; 
						sum_b += image[(x+i)*col + (y+j)].b; 
						total += 1;
						// 
// 						printf("%d \n",sum_r/total);
						
						output[(x+i)*col + (y+j)].r = sum_r/total;
						output[(x+i)*col + (y+j)].g = sum_g/total;
						output[(x+i)*col + (y+j)].b = sum_b/total;
					}else{
						output[(x+i)*col + (y+j)].r = 0;
						output[(x+i)*col + (y+j)].g = 0;
						output[(x+i)*col + (y+j)].b = 0;
					}
				}
			} 
		}
	}
	
	return(output);
}
