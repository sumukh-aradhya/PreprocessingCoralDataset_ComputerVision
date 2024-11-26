#include "netpbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define BLOCK_SIZE 4

typedef struct {
    float mean;
    float stddev;
    float x;
    float y;
} feature_vec;

//funct to segment textures
Image segment_texture(Image inp_img, int segments) {
    int width=inp_img.width;
    int height=inp_img.height;
    int block_idx= 0;

    int block_col=(width+BLOCK_SIZE-1)/BLOCK_SIZE;
    int block_row=(height+BLOCK_SIZE-1)/BLOCK_SIZE;
    int block_total=block_col*block_row;
    feature_vec* features=(feature_vec*)malloc(block_total*sizeof(feature_vec));

    if(features==NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    //compute features for each block
    int by, bx, m,n;
    for(by=0; by<block_row; by++) {
        for(bx=0; bx<block_col; bx++) {
            int x0=bx*BLOCK_SIZE;
            int y0=by*BLOCK_SIZE;
            double sum=0.0;
            double sum_sq=0.0;
            int count=0;
            for(m=y0; m<y0+BLOCK_SIZE && m<height; m++) {
                for(n=x0; n<x0+BLOCK_SIZE && n<width; n++) {
                    unsigned char pixel=inp_img.map[m][n].i;
                    sum+=pixel;
                    sum_sq+=pixel*pixel;
                    count++;
                }
            }
            double mean=sum/count;
            double variance=(sum_sq/count)-(mean*mean);
            double stddev=sqrt(variance);

            features[block_idx].mean=(float)mean;
            features[block_idx].stddev=(float)stddev;
            features[block_idx].x=(float)(x0+BLOCK_SIZE/2)/width;
            features[block_idx].y=(float)(y0+BLOCK_SIZE/2)/height;
            block_idx++;
        }
    }

    //kmeans clustering
    int k=segments;
    int max_itr=100;

    int* labels=(int*)malloc(block_total*sizeof(int));
    if(labels==NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    feature_vec* centers=(feature_vec*)malloc(k* sizeof(feature_vec));
    if(centers==NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    //init centers randomly
    srand(0);
    int i;
    for(i=0; i<k; i++) {
        int index=rand()%block_total;
        centers[i]=features[index];
    }

    int iter,c;
    for(iter=0; iter<max_itr; iter++) {
        int changes=0;
        //assignment step
        for(i=0; i<block_total; i++) {
            int min_index=-1;
            float min_dist=FLT_MAX;
            for(c=0; c<k; c++) {
                float dx=features[i].mean-centers[c].mean;
                float dy=features[i].stddev-centers[c].stddev;
                float dxx=features[i].x-centers[c].x;
                float dyy=features[i].y-centers[c].y;
                float dist=dx*dx+dy*dy + dxx*dxx+dyy*dyy;
                if(dist<min_dist) {
                    min_dist=dist;
                    min_index=c;
                }
            }
            if(labels[i]!=min_index) {
                labels[i]=min_index;
                changes++;
            }
        }
        //update step
        feature_vec* new_centers=(feature_vec*)calloc(k, sizeof(feature_vec));
        int* counts=(int*)calloc(k, sizeof(int));
        if(new_centers==NULL || counts==NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
        for(i=0; i<block_total; i++) {
            int cluster=labels[i];
            new_centers[cluster].mean+=features[i].mean;
            new_centers[cluster].stddev+=features[i].stddev;
            new_centers[cluster].x+=features[i].x;
            new_centers[cluster].y+=features[i].y;
            counts[cluster]++;
        }
        for(c=0; c<k; c++) {
            if(counts[c]> 0) {
                centers[c].mean= new_centers[c].mean/counts[c];
                centers[c].stddev =new_centers[c].stddev/counts[c];
                centers[c].x=new_centers[c].x/counts[c];
                centers[c].y=new_centers[c].y/counts[c];
            }
        }
        free(new_centers);
        free(counts);
        if(changes== 0)
            break;
    }

    //create output image
    Image op_img=createImage(height, width);

    //generate colors for each cluster
    unsigned char* colors=(unsigned char*)malloc(k*3*sizeof(unsigned char)); //rgb for each cluster
    if(colors==NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    for(i=0; i<k; i++) {
        colors[i*3+0]=rand()%256; //r
        colors[i*3+1]=rand()%256; //g
        colors[i*3+2]=rand()%256; //b
    }

    block_idx=0;
    //assign colors to each block
    for(by=0; by<block_row; by++) {
        for(bx=0; bx < block_col; bx++) {
            int label=labels[block_idx];
            unsigned char r=colors[label*3+0];
            unsigned char g =colors[label*3+1];
            unsigned char b=colors[label*3+2];
            int x0=bx*BLOCK_SIZE;
            int y0=by*BLOCK_SIZE;

            for(m=y0; m<y0+BLOCK_SIZE && m<height; m++) {
                for(n=x0; n<x0+BLOCK_SIZE && n<width; n++) {
                    op_img.map[m][n].r=r;
                    op_img.map[m][n].g=g;
                    op_img.map[m][n].b=b;
                    op_img.map[m][n].i=(r+g+b)/3;
                }
            }
            block_idx++;
        }
    }
    free(features);
    free(labels);
    free(centers);
    free(colors);

    return op_img;
}

int main(int argc, char** argv) {
    char* inp_fname=argv[1];
    int segments=atoi(argv[2]);
    char* op_fname=argv[3];

    //read input image
    Image inp_img=readImage(inp_fname);

    //segment texture
    Image op_img=segment_texture(inp_img, segments);

    //write the output image
    writeImage(op_img, op_fname);

    //clean up
    deleteImage(inp_img);
    deleteImage(op_img);

    printf("Segmentation completed. Output saved as %s\n", op_fname);
    return 0;
}