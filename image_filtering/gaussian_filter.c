#include "netpbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define KERNEL_SIZE 5
#define SIGMA 1.0

//func to generate a Gaussian kernel
void generateGaussianKernel(double kernel[KERNEL_SIZE][KERNEL_SIZE], double sigma) {
    int halfSize=KERNEL_SIZE/2;
    double sum=0.0;

    for(int i=0; i<KERNEL_SIZE; i++) {
        for(int j=0; j<KERNEL_SIZE; j++) {
            int x=i-halfSize;
            int y=j-halfSize;
            kernel[i][j]=exp(-(x*x+y*y)/(2*sigma*sigma))/(2*M_PI*sigma*sigma);
            sum+=kernel[i][j];
        }
    }

    //normalize kernel
    for(int i=0; i<KERNEL_SIZE; i++) {
        for(int j=0; j<KERNEL_SIZE; j++) {
            kernel[i][j]/=sum;
        }
    }
}

//funct to apply Gaussian filter
Image applyGaussianFilter(Image img, double kernel[KERNEL_SIZE][KERNEL_SIZE]) {
    Image result=createImage(img.height, img.width);
    int halfSize=KERNEL_SIZE/2;

    for(int y=0; y<img.height; y++) {
        for(int x=0; x<img.width; x++) {
            double sum=0.0;

            for(int ky=-halfSize; ky<=halfSize; ky++) {
                for(int kx=-halfSize; kx<=halfSize; kx++) {
                    int ny=y+ky;
                    int nx=x+kx;

                    if(ny>=0 && ny<img.height && nx>=0 && nx<img.width) {
                        sum+=img.map[ny][nx].i*kernel[ky+halfSize][kx+halfSize];
                    }
                }
            }
            result.map[y][x].r=result.map[y][x].g=result.map[y][x].b=result.map[y][x].i=(int)sum;
        }
    }
    return result;
}

int main(int argc, char **argv) {
    char *inputFilename=argv[1];
    char *outputFilename=argv[2];
    Image img=readImage(inputFilename);

    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generateGaussianKernel(kernel, SIGMA);

    Image filteredImg=applyGaussianFilter(img, kernel);

    writeImage(filteredImg, outputFilename);

    //cleanup
    deleteImage(img);
    deleteImage(filteredImg);

    printf("Gaussian filtering completed. Output saved as %s\n", outputFilename);
    return 0;
}
