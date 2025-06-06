// netpbm.c 
// Functions for reading and writing binary PBM, PGM, and PPM image files.
// V2.2 by Marc Pomplun on 10/19/2013
#define _CRT_SECURE_NO_WARNINGS

#include "netpbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>



// Create a new image of the given size and fill it with white pixels.
// When you don't need the image anymore, don't forget to free its memory using deleteImage.
Image createImage(int height, int width)
{
	int i, j;
	Image img;

	img.map = (Pixel **) malloc(sizeof(Pixel *)*height);
	for (i = 0; i < height; i++)
	{
		img.map[i] = (Pixel *) malloc(sizeof(Pixel)*width);
		for (j = 0; j < width; j++)
		{
			img.map[i][j].r = 255;
			img.map[i][j].g = 255;
			img.map[i][j].b = 255;
			img.map[i][j].i = 255;
		}
	}
	img.height = height;
	img.width = width;
	return img;
}

// Delete a previously created image and free its allocated memory on the heap. 
void deleteImage(Image img)
{
	int i;
	
	for (i = 0; i < img.height; i++)
		free(img.map[i]);
	free(img.map);
}

// Create a new matrix of the given size and fill it with zeroes.
// When you don't need the matrix anymore, don't forget to free its memory using deleteMatrix.
Matrix createMatrix(int height, int width)
{
	int i, j;
	Matrix mx;

	mx.map = (double **) malloc(sizeof(double *)*height);
	for (i = 0; i < height; i++)
	{
		mx.map[i] = (double *) malloc(sizeof(double)*width);
		for (j = 0; j < width; j++)
			mx.map[i][j] = 0.0;
	}
	mx.height = height;
	mx.width = width;
	return mx;
}

// Create a new matrix of the given size and fill it with content of 2D double array.
// Call this function with a pointer to the first element of the array, e.g., &a[0][0].
// When you don't need the matrix anymore, don't forget to free its memory using deleteMatrix.
Matrix createMatrixFromArray(double *entry, int height, int width)
{
	int i, j;
	Matrix mx;

	mx.map = (double **) malloc(sizeof(double *)*height);
	for (i = 0; i < height; i++)
	{
		mx.map[i] = (double *) malloc(sizeof(double)*width);
		for (j = 0; j < width; j++)
			mx.map[i][j] = *(entry++);
	}
	mx.height = height;
	mx.width = width;
	return mx;
}

// Delete a previously created matrix and free its allocated memory on the heap. 
void deleteMatrix(Matrix mx)
{
	int m;
	
	for (m = 0; m < mx.height; m++)
		free(mx.map[m]);
	free(mx.map);
}

// Read an image from a file and allocate the required heap memory for it.
// Notice that only binary Netpbm files are supported. Regardless of the
// file type, all fields r, g, b, and i are filled in, with values from 0 to 255. 
Image readImage(char *filename)
{
	FILE *f;
	int i, j, width, height, imax=0, bitsPerPixel, filesize, mapsize, mempos;
	char type[200], line[200];
	unsigned char *temp, output;
	Image img;
	Format filetype;

	f = fopen(filename, "rb");
	if (!f)
	{
		fprintf(stderr, "Can't open input file %s.\n", filename);
		exit(1);
	}

	fscanf(f, "%s", type); 
	if (type[0] != 'P' || type[1] < '4' || type[1] > '6')
	{
		fprintf(stderr, "Error in %s: Only binary PBM, PGM, and PPM files are supported.\n", filename);
		exit(1);
	}
	switch (type[1])
	{
	case '4': 
		filetype = PBM;
		bitsPerPixel = 1;
		break;
	case '5': 
		filetype = PGM;
		bitsPerPixel = 8;
		break;
	default:  
		filetype = PPM;
		bitsPerPixel = 24;
	}

	line[0] = '#';
	while (line[0] == '#' || line[0] == 10 || line[0] == 13)
		fgets(line, 200, f); 
	sscanf(line, "%d %d", &width, &height);
	if (filetype != PBM)
	{
		fgets(line, 200, f); 
		sscanf(line, "%d", &imax);
	}
	if (width <= 0 || height <= 0)
	{
		fprintf(stderr, "Invalid image size in input file %s.\n", filename);
		exit(1);
	}

	// Notice: In PBM files, every row starts with a new byte.
	mapsize = (bitsPerPixel*width + 7)/8*height;
	temp = (unsigned char *) malloc(mapsize);
	// Using fread is much faster than reading byte-by-byte. 
	filesize = (int) fread((void *) temp, 1,mapsize,  f);
	fclose(f);
	if (filesize != mapsize)
	{
		fprintf(stderr, "Data missing in file %s.\n", filename);
		exit(1);
	}

	img = createImage(height, width);
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			mempos = (bitsPerPixel*width + 7)/8*i + (bitsPerPixel*j)/8;
			switch (filetype)
			{
			case PBM: 
				output = 255*(((temp[mempos] & (128 >> j%8)) == 0));
				img.map[i][j].r = output;
				img.map[i][j].g = output;
				img.map[i][j].b = output;
				img.map[i][j].i = output;
				break;
			case PGM: 
				output = (unsigned char) ((int) temp[mempos]*255/imax);
				img.map[i][j].r = output;
				img.map[i][j].g = output;
				img.map[i][j].b = output;
				img.map[i][j].i = output;
				break;
			case PPM:
				img.map[i][j].r = (unsigned char) ((int) temp[mempos]*255/imax);
				img.map[i][j].g = (unsigned char) ((int) temp[mempos + 1]*255/imax);
				img.map[i][j].b = (unsigned char) ((int) temp[mempos + 2]*255/imax);
				img.map[i][j].i = (unsigned char) (((int) temp[mempos] + (int) temp[mempos + 1] + (int) temp[mempos + 2])*255/(3*imax));
                     
			}
		}
	free(temp);
	return img;
}

// Write an image to a file. The file format (binary PBM, PGM, or PPM) is automatically
// chosen based on the given file name. For PBM and PGM files, only the intensity
// (i) information is used, and for PPM files, only r, g, and b are relevant.
void writeImage(Image img, char *filename)
{
	Format filetype;
	FILE *f;
	int i, j, mapsize, bitsPerPixel, bitsum, mempos;
	unsigned char *temp;

	switch (filename[strlen(filename) - 2])
	{
	case 'b':
	case 'B': 
		filetype = PBM;
		bitsPerPixel = 1;
		break;
	case 'g':
	case 'G': 
		filetype = PGM;
		bitsPerPixel = 8;
		break;
	case 'p':
	case 'P': 
		filetype = PPM;
		bitsPerPixel = 24;
		break;
	default:  
		fprintf(stderr, "Invalid output file name: %s.\n", filename);
		exit(1);
	}

	if (img.width <= 0 || img.height <= 0)
	{
		fprintf(stderr, "Invalid image size in output file %s.\n", filename);
		exit(1);
	}

	// Notice: In PBM files, every row starts with a new byte.
	mapsize = (bitsPerPixel*img.width + 7)/8*img.height;
	// Creating linear file data in memory and then using fwrite is much faster than writing byte-by-byte. 
	temp = (unsigned char *) malloc(mapsize);
	mempos = 0;
	bitsum = 0;
	for (i = 0; i < img.height; i++)
		for (j = 0; j < img.width; j++)
			switch (filetype)
			{
			case PBM: 
				if (img.map[i][j].i < 128)
					bitsum += 128>>(j%8);
				if (j%8 == 7 || j == img.width - 1)
				{
					temp[mempos++] = (unsigned char) bitsum;
					bitsum = 0;
				}
				break;
			case PGM: 
				temp[mempos++] = img.map[i][j].i;
				break;
			case PPM: 
				temp[mempos++] = img.map[i][j].r;
				temp[mempos++] = img.map[i][j].g;
				temp[mempos++] = img.map[i][j].b;
		    }

	f = fopen(filename, "wb");
	if (!f)
	{
		fprintf(stderr, "Can't open output file %s.\n", filename);
		exit(1);
	}
	switch (filetype)
	{
	case PBM: 
		fprintf(f, "P4\n# Created by netpbm.c\n%d %d\n", img.width, img.height);
		break;
	case PGM: 
		fprintf(f, "P5\n# Created by netpbm.c\n%d %d\n255\n", img.width, img.height);
		break;
	case PPM: 
		fprintf(f, "P6\n# Created by netpbm.c\n%d %d\n255\n", img.width, img.height);
	}
	fwrite((void *) temp, 1, mapsize, f);
	fclose(f);
	free(temp);
}


// Convert the intensity components of an image into a matrix of identical size.
Matrix image2Matrix(Image img)
{
	int m, n;
	Matrix result = createMatrix(img.height, img.width);

	for (m = 0; m < img.height; m++)
		for (n = 0; n < img.width; n++)
			result.map[m][n] = (double) img.map[m][n].i;
	return result;
}

// Convert a matrix into an image with corresponding, r, g, b, and i components and size.
// If scale == 0 then values remain unchanged but if they are below 0 or above 255, they 
// are set to 0 or 255, respectively.
// If scale != 0 the values are scaled so that minimum value is zero and maximum is 255.
// Setting the gamma value allows for exponential scaling, with gamma == 1.0 enabling
// linear scaling.
Image matrix2Image(Matrix mx, int scale, double gamma)
{
	int m, n, intValue;
	double dblValue, minVal = DBL_MAX, maxVal = -DBL_MAX;
	Image result = createImage(mx.height, mx.width);

	if (scale)
	{
		for (m = 0; m < mx.height; m++)
			for (n = 0; n < mx.width; n++)
			{
				dblValue = mx.map[m][n];
				if (dblValue < minVal)
					minVal = dblValue;
				else if (dblValue > maxVal)
					maxVal = dblValue;
			}
		if (maxVal - minVal < 1e-10)
			maxVal += 1.0;
	}

	//minVal = 0.0;
	for (m = 0; m < mx.height; m++)
		for (n = 0; n < mx.width; n++)
		{
			dblValue = mx.map[m][n];
			if (scale)
				intValue = (int) (255.0*pow((dblValue - minVal)/(maxVal - minVal), gamma) + 0.5);
			else
				intValue = (int) mx.map[m][n];
			if (intValue < 0)
				intValue = 0;
			else if (intValue > 255)
				intValue = 255;
			result.map[m][n].r = result.map[m][n].g = result.map[m][n].b = result.map[m][n].i = intValue;
		}
	return result;
}

// Set color for pixel (vPos, hPos) in image img.
// If r, g, b, or i are set to NO_CHANGE, the corresponding color channels are left unchanged in img.
// If they are set to INVERT, the corresponding channels are inverted, i.e., set to 255 minus their original value
void setPixel(Image img, int vPos, int hPos, int r, int g, int b, int i)
{
	if (vPos >= 0 && vPos < img.height && hPos >= 0 && hPos < img.width && img.map != NULL)
	{
		if (r == INVERT)
			img.map[vPos][hPos].r = 255 - img.map[vPos][hPos].r;
		else if (r >= 0 && r <= 255)
			img.map[vPos][hPos].r = r;
		
		if (g == INVERT)
			img.map[vPos][hPos].g = 255 - img.map[vPos][hPos].g;
		else if (g >= 0 && g <= 255)
			img.map[vPos][hPos].g = g;

		if (b == INVERT)
			img.map[vPos][hPos].b = 255 - img.map[vPos][hPos].b;
		else if (b >= 0 && b <= 255)
			img.map[vPos][hPos].b = b;

		if (i == INVERT)
			img.map[vPos][hPos].i = 255 - img.map[vPos][hPos].i;
		else if (i >= 0 && i <= 255)
			img.map[vPos][hPos].i = i;
	}
}

// Draw filled ellipse in image img centered at (vCenter, hCenter) with radii vRadius and hRadius. 
// Radius (0, 0) will draw an individual pixel. For setting the r, g, b, and i color values, see setPixel function.
void filledEllipse(Image img, int vCenter, int hCenter, int vRadius, int hRadius, int r, int g, int b, int i)
{
	int m, n, hSpan;
	if (vRadius == 0 && hRadius == 0)
		setPixel(img, vCenter, hCenter, r, g, b, i);
	else
	{
		for (m = -vRadius; m <= vRadius; m++)
		{
			if (vRadius == 0)
				hSpan = hRadius;
			else
				hSpan = (int) ((double) hRadius*sqrt(1.0 - SQR((double) m/(double) vRadius)) + 0.5);
			for (n = -hSpan; n <= hSpan; n++)
				setPixel(img, vCenter + m, hCenter + n, r, g, b, i);
		}
	}
}

// Draw filled rectangle in image img with opposite edges (v1, h1) and (v2, h2).
// For setting the r, g, b, and i color values, see setPixel function.
void filledRectangle(Image img, int v1, int h1, int v2, int h2, int r, int g, int b, int i)
{
	int m, n, m1 = v1, n1 = h1, m2 = v2, n2 = h2;
	
	if (v1 > v2)
	{
		m1 = v2;
		m2 = v1;
	}
	if (h1 > h2)
	{
		n1 = h2;
		n2 = h1;
	}
	for (m = m1; m <= m2; m++)
		for (n = n1; n <= n2; n++)
			setPixel(img, m, n, r, g, b, i);
}

// Draw straight line in image img between (v1, h1) and (v2, h2) with a given width, dash pattern, and color. 
// Width 0 indicates single-pixel width. The inputs dash and gap determine the length in pixels of the dashes 
// and the gaps between them, resp. Use 0 for either input to draw a solid line.
// For setting the r, g, b, and i color values, see setPixel function.
void line(Image img, int v1, int h1, int v2, int h2, int width, int dash, int gap, int r, int g, int b, int i)
{
	int h, v, direction, distance = (int) sqrt((double) SQR(v2 - v1) + SQR(h2 - h1)), distanceCovered;
	double slope;

	if (v1 == v2 && h1 == h2)
	{
		filledEllipse(img, v1, h1, width, width, r, g, b, i);
		return;
	}

	if (abs(h2 - h1) > abs(v2 - v1))
	{
		slope = (double) (v2 - v1)/(double) (h2 - h1);
		direction = (h2 > h1)? 1:-1;
		for (h = h1; h != h2 + direction; h += direction)
		{
			v = v1 + (int) (slope*(double) (h - h1) + 0.5);
			distanceCovered = (int) sqrt((double) (SQR(h - h1) + SQR(v - v1)));
			if (dash*gap == 0 || distanceCovered%(dash + gap) < dash || (distance%(dash + gap) >= dash && distanceCovered > distance - distance%(dash + gap)))
				filledEllipse(img, v, h, width, width, r, g, b, i);
		}
	}
	else
	{
		slope = (double) (h2 - h1)/(double) (v2 - v1);
		direction = (v2 > v1)? 1:-1;
		for (v = v1; v != v2 + direction; v += direction)
		{
			h = h1 + (int) (slope*(double) (v - v1) + 0.5);
			distanceCovered = (int) sqrt((double) (SQR(h - h1) + SQR(v - v1)));
			if (dash*gap == 0 || distanceCovered%(dash + gap) < dash || (distance%(dash + gap) >= dash && distanceCovered > distance - distance%(dash + gap)))
				filledEllipse(img, v, h, width, width, r, g, b, i);
		}
	}
}

// Draw rectangle in image img with opposite corners (v1, h1) and (v2, h2) with a given width, dash pattern, and color. 
// Inputs are otherwise identical to the line function.
void rectangle(Image img, int v1, int h1, int v2, int h2, int width, int dash, int gap, int r, int g, int b, int i)
{
	line(img, v1, h1, v1, h2, width, dash, gap, r, g, b, i);
	line(img, v1, h2, v2, h2, width, dash, gap, r, g, b, i);
	line(img, v2, h2, v2, h1, width, dash, gap, r, g, b, i);
	line(img, v2, h1, v1, h1, width, dash, gap, r, g, b, i);
}

// Draw ellipse in image img centered at (vCenter, hCenter) and radii (vRadius, hRadius) with a given width, dash pattern, and color. 
// Width 0 indicates single-pixel width. The inputs dash and gap determine the length in pixels of the dashes 
// and the gaps between them, resp. Use 0 for either input to draw a solid line.
// For setting the r, g, b, and i color values, see setPixel function.
void ellipse(Image img, int vCenter, int hCenter, int vRadius, int hRadius, int width, int dash, int gap, int r, int g, int b, int i)
{
	int v, h, last_v = -100, last_h = -100, secondlast_v = -100, secondlast_h = -100, last_shown = 0, distanceCovered = 0;
	double alpha, stepsize = PI/2.0/(double) (vRadius + hRadius);

	for (alpha = 0.0; alpha < 2.0*PI; alpha += stepsize)
	{
		v = vCenter + (int) ((double) vRadius*sin(alpha));
		h = hCenter + (int) ((double) hRadius*cos(alpha));
		if (v != last_v || h != last_h)
		{
			if (abs(v - secondlast_v) <= 1 && abs(h - secondlast_h) <= 1)
			{
				if (dash*gap == 0 || distanceCovered%(dash + gap) < dash)
					filledEllipse(img, v, h, width, width, r, g, b, i);
				secondlast_v = -1;
				secondlast_h = -1;
				last_shown = 1;
				distanceCovered++;
			}
			else
			{
				if (!last_shown)
				{
					if ((dash*gap == 0 || distanceCovered%(dash + gap) < dash) && last_v > -100)
						filledEllipse(img, last_v, last_h, width, width, r, g, b, i);
					distanceCovered++;
				}
				secondlast_v = last_v;
				secondlast_h = last_h;
				last_shown = 0;
			}
			last_v = v;
			last_h = h;
		}
	}
}




