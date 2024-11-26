#include "netpbm.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAX_RADIUS 100  //max radius for circle detection
#define MIN_RADIUS 20   //min radius for circle detection
#define THRESHOLD_SCALE 0.6  //scale for dynamic threshold
#define THETA_STEP 10   //step size for theta in degrees

typedef struct {
    int ***votes;
    int height;
    int width;
    int maxRadius;
} HoughSpace;

HoughSpace createHoughSpace(int height, int width, int maxRadius);
void freeHoughSpace(HoughSpace hough);
void houghTransformCircles(Image edgeImage, HoughSpace *hough);
void findHoughMaxima(HoughSpace hough, Image edgeImage, Image houghMaxima);
void markDetectedCircles(Image img, int yCenter, int xCenter, int radius);
void writeHoughSpaceAsImage(HoughSpace hough, const char *filename);

//funct to create 3D hough space for circle detection
HoughSpace createHoughSpace(int height, int width, int maxRadius) {
    HoughSpace hough;
    hough.height=height;
    hough.width=width;
    hough.maxRadius=maxRadius;

    hough.votes=(int ***)malloc(height * sizeof(int **));
    for(int i=0; i<height; i++) {
        hough.votes[i]=(int **)malloc(width * sizeof(int *));
        for(int j=0; j<width; j++) {
            hough.votes[i][j]=(int *)calloc(maxRadius, sizeof(int));
        }
    }
    return hough;
}

void freeHoughSpace(HoughSpace hough) {
    for(int i=0; i<hough.height; i++) {
        for(int j=0; j<hough.width; j++) {
            free(hough.votes[i][j]);
        }
        free(hough.votes[i]);
    }
    free(hough.votes);
}

//perform Hough Transform for circles on an edge image
void houghTransformCircles(Image edgeImage, HoughSpace *hough) {
    printf("Performing Hough Transform...\n");

    for(int y=0; y<edgeImage.height; y++) {
        for(int x=0; x<edgeImage.width; x++) {
            if(edgeImage.map[y][x].i>200) {  //edge pixel - threshold
                for(int r=MIN_RADIUS; r<hough->maxRadius; r++) {
                    for(int theta=0; theta<360; theta+=THETA_STEP) {
                        int x0=x-(int)(r*cos(theta*M_PI/180.0));
                        int y0=y-(int)(r*sin(theta*M_PI/180.0));
                        if(x0>=0 && x0<hough->width && y0>=0 && y0<hough->height) {
                            hough->votes[y0][x0][r]++;
                        }
                    }
                }
            }
        }
    }
    printf("Hough Transform completed.\n");
}

//write Hough space as an image for debugging
void writeHoughSpaceAsImage(HoughSpace hough, const char *filename) {
    Image houghImage=createImage(hough.height, hough.width);
    int maxVotes=0;
    //find max votes
    for(int y=0; y<hough.height; y++) {
        for(int x=0; x<hough.width; x++) {
            for(int r=MIN_RADIUS; r<hough.maxRadius; r++) {
                if(hough.votes[y][x][r]>maxVotes) {
                    maxVotes=hough.votes[y][x][r];
                }
            }
        }
    }

    //normalize votes into a grayscale image
    for(int y=0; y<hough.height; y++) {
        for(int x=0; x<hough.width; x++) {
            int totalVotes=0;
            for(int r=MIN_RADIUS; r<hough.maxRadius; r++) {
                totalVotes+=hough.votes[y][x][r];
            }
            houghImage.map[y][x].i=(int)(255.0*totalVotes/ maxVotes);
        }
    }
    writeImage(houghImage, filename);
    deleteImage(houghImage);
}

//find maxima in Hough space and mark detected circles
void findHoughMaxima(HoughSpace hough, Image edgeImage, Image houghMaxima) {
    int maxVotes=0;

    //find max votes in the hough space
    for(int y=0; y<hough.height; y++) {
        for(int x=0; x<hough.width; x++) {
            for(int r=MIN_RADIUS; r<hough.maxRadius; r++) {
                if(hough.votes[y][x][r]>maxVotes) {
                    maxVotes=hough.votes[y][x][r];
                }
            }
        }
    }

    //set a dynamic threshold
    int dynamicThreshold=(int)(maxVotes* THRESHOLD_SCALE);
    printf("Max Votes: %d, Dynamic Threshold: %d\n", maxVotes, dynamicThreshold);

    //detect and mark circles based on the dynamic threshold
    for(int y= 0; y<hough.height; y++) {
        for(int x=0; x<hough.width; x++) {
            for(int r=MIN_RADIUS; r<hough.maxRadius; r++) {
                if(hough.votes[y][x][r]>dynamicThreshold) {
                    markDetectedCircles(edgeImage, y, x, r);
                    houghMaxima.map[y][x].i = 255; //mark maxima
                }
            }
        }
    }
}

//funct to mark detected circles in the edge image
void markDetectedCircles(Image img, int yCenter, int xCenter, int radius) {
    for(int theta=0; theta<360; theta+=THETA_STEP) {
        int x=xCenter+(int)(radius*cos(theta*M_PI/ 180.0));
        int y=yCenter+(int)(radius *sin(theta*M_PI/180.0));
        if(x>=0 && x<img.width && y>= 0 && y < img.height) {
            img.map[y][x].i=255;  //mark circle edge
        }
    }
}

int main(int argc, char *argv[]) {
    char *inputEdgeFile=argv[1];
    char *outputEdgeFile=argv[2];
    char *outputHoughFile=argv[3];
    Image edgeImage = readImage(inputEdgeFile);

    Image outputEdges = createImage(edgeImage.height, edgeImage.width);

    //init to black
    for(int y=0; y<outputEdges.height; y++) {
        for(int x=0; x<outputEdges.width; x++) {
            outputEdges.map[y][x].i=0;
        }
    }

    //create an output image for Hough maxima
    Image houghMaxima=createImage(edgeImage.height, edgeImage.width);

    for(int y=0; y<houghMaxima.height; y++) {
        for(int x=0; x<houghMaxima.width; x++) {
            houghMaxima.map[y][x].i=0;
        }
    }

    HoughSpace hough = createHoughSpace(edgeImage.height, edgeImage.width, MAX_RADIUS);

    houghTransformCircles(edgeImage, &hough);
    writeHoughSpaceAsImage(hough, "hough_space_debug.pgm");
    findHoughMaxima(hough, outputEdges, houghMaxima);

    writeImage(outputEdges, outputEdgeFile);
    writeImage(houghMaxima, outputHoughFile);

    //clean up
    freeHoughSpace(hough);
    deleteImage(edgeImage);
    deleteImage(outputEdges);
    deleteImage(houghMaxima);

    printf("Hough transformation completed. Results saved to %s and %s\n", outputEdgeFile, outputHoughFile);
    return 0;
}
