#ifndef PPMIO_H

#define PPMIO_H

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Pixel;

Pixel *readPPM(int *rows, int *cols, int * colors, char *filename);
void writePPM(Pixel *image, int rows, int cols, int colors, char *filename);

unsigned char *readPGM(int *rows, int *cols, int *intensities, char *filename);
void writePGM(unsigned char *image, long rows, long cols, int intensities, char *filename);

Pixel *edgeDetect(Pixel *image, int rows, int cols);
Pixel *filter(Pixel *image, Pixel *edge, int row, int col);
Pixel *filter2(Pixel *image, Pixel *edge, int row, int col);
Pixel *filter3(Pixel *image, Pixel *edge, int row, int col);
Pixel *filter4(Pixel *image, Pixel *edge, int row, int col, int k);

#endif
