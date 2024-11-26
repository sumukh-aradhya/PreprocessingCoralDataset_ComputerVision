#include "netpbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//funct to generate a ground truth edge map from an input image
Image generateGroundTruth(Image img) {
    Image edgeMap=createImage(img.height, img.width);

    //init edge map to black
    for(int y =0; y < edgeMap.height; y++) {
        for(int x=0; x < edgeMap.width; x++) {
            edgeMap.map[y][x].i=0;  //black background
        }
    }

    //apply edge detection based on intensity differences
    for(int y = 1; y < img.height - 1; y++) {
        for(int x = 1; x < img.width - 1; x++) {
            //compute gradient magnitude using sobel-like filters
            int gx = img.map[y-1][x+1].i-img.map[y-1][x-1].i +
                     2 * img.map[y][x+1].i-2*img.map[y][x-1].i +
                     img.map[y+1][x+1].i-img.map[y+1][x-1].i;

            int gy = img.map[y-1][x-1].i+ 2 *img.map[y-1][x].i+img.map[y-1][x+1].i -
                     img.map[y+1][x-1].i - 2*img.map[y+1][x].i-img.map[y+1][x+1].i;

            int gradientMagnitude = (int)sqrt(gx * gx + gy * gy);

            //threshold the gradient magnitude
            if(gradientMagnitude>128) {
                edgeMap.map[y][x].i = 255;  //mark as edge - white
            }
        }
    }

    return edgeMap;
}

int main(int argc, char **argv) {
    char *inputFile=argv[1];
    char *outputFile=argv[2];
    Image inputImage=readImage(inputFile);

    //generate ground truth edge map
    Image groundTruth=generateGroundTruth(inputImage);
    writeImage(groundTruth, outputFile);

    //clean up
    deleteImage(inputImage);
    deleteImage(groundTruth);

    printf("Ground truth edge map saved to %s\n", outputFile);
    return 0;
}
