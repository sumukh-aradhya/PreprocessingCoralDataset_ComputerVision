#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "netpbm.h"

//funct for convolution
Matrix convolve(Matrix m1, Matrix m2) {
    int i,j,x,y, fheight, fwidth;
    Matrix res=createMatrix(m1.height, m1.width);
    
    //find center of filter
    fheight=m2.height/2;
    fwidth=m2.width/2;
    
    //convolve
    for(i=fheight; i<m1.height-fheight; i++) {
        for(j=fwidth; j<m1.width-fwidth; j++) {
            double sum= 0.0;
            for(x=0; x<m2.height; x++) {
                for(y=0; y<m2.width; y++) {
                    sum+=m1.map[i-fheight+x][j-fwidth+y]*m2.map[x][y];
                }
            }
            res.map[i][j]=sum;
        }
    }
    return res;
}

//funct for sobel edge detection
Image sobel(Image img) {
    double sobelx[3][3]={{-1,0,1}, {-2,0,2}, {-1,0,1}}; //for horizontal detection
    double sobely[3][3]={{-1,-2,-1}, {0,0,0}, {1,2,1}}; //for vertical detection
    
    Matrix sobelX=createMatrixFromArray(&sobelx[0][0], 3, 3);
    Matrix sobelY=createMatrixFromArray(&sobely[0][0], 3, 3);
    
    Matrix img_matrix=image2Matrix(img); //convert image to a matrix of intensity values
    Matrix resx=convolve(img_matrix, sobelX); //calculate horizontal gradients
    Matrix resy=convolve(img_matrix, sobelY); //calculate vertical gradients
    
    Matrix sobelres=createMatrix(img.height, img.width);
    
    //to track max and min values in result
    double maxval=-DBL_MAX;
    double minval=DBL_MAX;
    
    for(int i=0; i<img.height; i++) {
        for(int j=0; j<img.width; j++) {
            sobelres.map[i][j]=sqrt(pow(resx.map[i][j], 2) + pow(resy.map[i][j], 2));
            if(sobelres.map[i][j]>maxval) 
                maxval = sobelres.map[i][j];
            if(sobelres.map[i][j]<minval) 
                minval = sobelres.map[i][j];
        }
    }
    
    //scale to 0-255
    Image res=matrix2Image(sobelres, 1, 1.0);
    
    deleteMatrix(sobelX);
    deleteMatrix(sobelY);
    deleteMatrix(img_matrix);
    deleteMatrix(resx);
    deleteMatrix(resy);
    deleteMatrix(sobelres);
    
    return res;
}

//edge detection function as per question
void edgeDetection(char *inputFilename, char *sobelFilename) {
    Image img=readImage(inputFilename);
    
    //call sobel
    Image sobel_img=sobel(img);
    writeImage(sobel_img, sobelFilename);
    
    //clean up
    deleteImage(img);
    deleteImage(sobel_img);
}

int main() {
    char *inputFile = "/Users/sumukharadhya/Downloads/CV/TermProject/edge_detection/sobel_detector/inputs/6.ppm";
    char *sobelFile = "/Users/sumukharadhya/Downloads/CV/TermProject/edge_detection/sobel_detector/outputs/color/6_op.ppm";

    edgeDetection(inputFile, sobelFile);
    return 0;
}