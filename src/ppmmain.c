
#include <stdio.h>
#include <stdlib.h>
#include "ppmIO.h"


int main(int argc, char *argv[]) {
  Pixel *image, *edge, *output;
  int rows, cols, colors;
  long imagesize;
  long i;
  int intensity;

  if(argc < 3) {
    printf("Usage: ppmmain <input file> <output file>\n");
    exit(-1);
  }

  /* read in the image */
  image = readPPM(&rows, &cols, &colors, argv[1]);
  if(!image) {
    fprintf(stderr, "Unable to read %s\n", argv[1]);
    exit(-1);
  }

  edge = edgeDetect(image, rows, cols);
  
  output = filter4(image, edge, rows, cols, 20);
  

  /* write out the resulting image */
  writePPM(edge, rows, cols, colors, argv[2]);

  /* free the image memory */
  free(image);
  free(edge);
  free(output);
  
  return(0);
}
