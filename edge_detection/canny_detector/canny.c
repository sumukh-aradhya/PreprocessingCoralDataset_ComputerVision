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

//funct for canny edge detection
Image canny(Image img) {
    //step 1: smoothing using 3x3 Gaussian filter
    double gaussdata[3][3]={{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    Matrix gaussfilter=createMatrixFromArray(&gaussdata[0][0], 3, 3);
    
    Matrix img_matrix=image2Matrix(img); //convert image to a matrix of intensity values
    Matrix smooth_matrix=convolve(img_matrix, gaussfilter); //apply guass filter using convolution - smoothing
    
    //step 2: Sobel edge detection on the smoothed image
    Image sobel_img=sobel(matrix2Image(smooth_matrix, 0, 1.0));

    Matrix gradx, grady;
    double sobelx[3][3]={{-1,0,1}, {-2,0,2}, {-1, 0,1}};
    double sobely[3][3]={{-1, -2,-1}, {0,0,0}, {1,2,1}};

    gradx=convolve(smooth_matrix, createMatrixFromArray(&sobelx[0][0], 3,3)); //gradient in x direction
    grady=convolve(smooth_matrix, createMatrixFromArray(&sobely[0][0], 3,3)); //gradient in y direction

    //store gradient magnitude and direction
    Matrix gradientMagnitude=createMatrix(img.height, img.width);
    Matrix gradientDirection =createMatrix(img.height, img.width);

    for(int i=0; i<img.height; i++) {
        for(int j=0; j<img.width; j++) {
            gradientMagnitude.map[i][j]=sqrt(pow(gradx.map[i][j], 2)+pow(grady.map[i][j], 2));
            gradientDirection.map[i][j]=atan2(grady.map[i][j], gradx.map[i][j]);
        }
    }

    //step 3: non-maximum suppression
    Matrix nonmaxsuppressed=createMatrix(img.height, img.width);
    for(int i=1; i<img.height-1; i++) { //ignoring border pixels
        for(int j=1; j<img.width-1; j++) {
            double angle = gradientDirection.map[i][j]*180.0/PI;
            angle=fmod(angle+180.0, 180.0);

            double magnitude=gradientMagnitude.map[i][j];
            double q=0, r=0;
            
            //check gradient direction and compare current pixel with its neighbors along this direction
            if((angle>=0 && angle<22.5)||(angle>= 157.5 && angle<180)) {
                q=gradientMagnitude.map[i][j+1];
                r=gradientMagnitude.map[i][j-1];
            } else if(angle>=22.5 && angle<67.5) {
                q=gradientMagnitude.map[i+1][j-1];
                r=gradientMagnitude.map[i-1][j+1];
            } else if(angle>=67.5 && angle<112.5) {
                q=gradientMagnitude.map[i+1][j];
                r=gradientMagnitude.map[i-1][j];
            } else if(angle>=112.5 && angle<157.5) {
                q=gradientMagnitude.map[i-1][j-1];
                r=gradientMagnitude.map[i+1][j+1];
            }

            //suppress minimum points
            if(magnitude>=q && magnitude>=r) {
                nonmaxsuppressed.map[i][j]=magnitude;
            } else{
                nonmaxsuppressed.map[i][j]= 0;
            }
        }
    }

    //step 4: hysteresis thresholding
    double low_threshold=2000;
    double high_threshold=2400;

    Matrix thresholded=createMatrix(img.height, img.width);

    for(int i=0; i<img.height; i++) {
        for(int j=0; j<img.width; j++) {
            if(nonmaxsuppressed.map[i][j]>=high_threshold) {
                thresholded.map[i][j] = 255; //edge is strong
            } else if(nonmaxsuppressed.map[i][j]>=low_threshold) {
                thresholded.map[i][j] = 128; //edge is weak
            } else {
                thresholded.map[i][j]=0; //not an edge
            }
        }
    }

    for(int i=1; i<img.height-1; i++) {
        for(int j=1; j<img.width-1; j++) {
            if(thresholded.map[i][j]==128) {
                if(thresholded.map[i+1][j]==255 || thresholded.map[i-1][j] ==255 ||
                    thresholded.map[i][j+1]==255 || thresholded.map[i][j-1] ==255 ||
                    thresholded.map[i+1][j+1]==255 || thresholded.map[i-1][j-1] ==255 ||
                    thresholded.map[i+1][j-1]==255 || thresholded.map[i-1][j+1] ==255) {
                    thresholded.map[i][j] =255; //connected so - strong
                } else {
                    thresholded.map[i][j]=0; //not connected so - suppress
                }
            }
        }
    }

    //create final binary image
    Image res = matrix2Image(thresholded,0,1.0);

    deleteMatrix(gaussfilter);
    deleteMatrix(smooth_matrix);
    deleteMatrix(gradx);
    deleteMatrix(grady);
    deleteMatrix(gradientMagnitude);
    deleteMatrix(gradientDirection);
    deleteMatrix(nonmaxsuppressed);
    deleteMatrix(thresholded);

    return res;
}


//edge detection function as per question
void edgeDetection(char *inputFilename, char *cannyFilename) {
    Image img=readImage(inputFilename);
    
    //call canny
    Image canny_img=canny(img);
    writeImage(canny_img, cannyFilename);
    
    //clean up
    deleteImage(img);
    deleteImage(canny_img);
}

int main() {
    char *inputFile = "/Users/sumukharadhya/Downloads/CV/TermProject/edge_detection/canny_detector/inputs/6.ppm";
    char *cannyFile = "/Users/sumukharadhya/Downloads/CV/TermProject/edge_detection/canny_detector/outputs/color/6_op.ppm";

    edgeDetection(inputFile, cannyFile);
    return 0;
}