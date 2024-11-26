#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "netpbm.h"

//funct to load a binary ground truth edge map
Image loadGroundTruth(char *filename) {
    return readImage(filename);
}

//funt to calculate evaluation metrics
void evaluateEdgeDetection(Image groundTruth, Image detectedEdges, double *precision, double *recall, double *fMeasure) {
    int tp=0, fp=0, fn=0;

    for(int y=0; y<groundTruth.height; y++) {
        for(int x=0; x<groundTruth.width; x++) {
            int gt=groundTruth.map[y][x].i>128;      //ground truth (binary)
            int det=detectedEdges.map[y][x].i>128;  //detected edges (binary)

            if (gt && det) tp++;  //true positive
            if (!gt && det) fp++; //false positive
            if (gt && !det) fn++; //false negative
        }
    }

    *precision=(double)tp/(tp + fp);
    *recall=(double)tp/(tp + fn);
    *fMeasure=2*(*precision * *recall) / (*precision + *recall);
}

int main(int argc, char **argv) {
    char *groundTruthFile = argv[1];
    char *sobelFile = argv[2];
    char *cannyFile = argv[3];

    Image groundTruth=loadGroundTruth(groundTruthFile);
    Image sobelEdges=readImage(sobelFile);
    Image cannyEdges=readImage(cannyFile);

    //evaluate Sobel
    double precisionSobel, recallSobel, fMeasureSobel;
    evaluateEdgeDetection(groundTruth, sobelEdges, &precisionSobel, &recallSobel, &fMeasureSobel);

    printf("Sobel Evaluation:\n");
    printf("  Precision: %.3f\n", precisionSobel);
    printf("  Recall: %.3f\n", recallSobel);
    printf("  F-Measure: %.3f\n", fMeasureSobel);

    //evaluate Canny
    double precisionCanny, recallCanny, fMeasureCanny;
    evaluateEdgeDetection(groundTruth, cannyEdges, &precisionCanny, &recallCanny, &fMeasureCanny);

    printf("Canny Evaluation:\n");
    printf("  Precision: %.3f\n", precisionCanny);
    printf("  Recall: %.3f\n", recallCanny);
    printf("  F-Measure: %.3f\n", fMeasureCanny);

    //clean up
    deleteImage(groundTruth);
    deleteImage(sobelEdges);
    deleteImage(cannyEdges);

    return 0;
}
