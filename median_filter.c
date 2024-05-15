/* ===================================================
     Functions to Read and Write BMP Image Files
=====================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bmp_fixed.h"

// **************************************************
//              System Constant Definition
// **************************************************

// **************************************************
//                Structure Difinitions
// **************************************************

typedef struct MyBITMAPFILEHEADER_type {
    char            B;
    char            M;
    unsigned int        bfSize;// 文件大小
    unsigned short int    bfReserved1;// 保留,必須設置為0
    unsigned short int    bfReserved2;// 保留,必須設置為0
    unsigned int        bfOffBits;// 從文件頭到像素數據的偏移
    char            buf[14];
} MyBITMAPFILEHEADER;

typedef struct MyBITMAPINFOHEADER_type {
    unsigned int        biSize;// 此結構體的大小
    unsigned int        biWidth;// 圖像的寬
    unsigned int        biHeight; // 圖像的高
    unsigned short int    biPlanes;// 表示bmp圖片的平面屬,顯然顯示器只有一個平面,所以恆等於1
    unsigned short int    biBitCount;// 一像素所占的位數,一般為24
    unsigned int        biCompression;// 說明圖象數據壓縮的類型,0為不壓縮。
    unsigned int        biSizeImage;// 像素數據所占大小, 這個值應該等於上面文件頭結構中bfSize-bfOffBits
    unsigned int        biXPelsPerMeter;// 說明水平解析度,用象素/米表示。一般為0
    unsigned int        biYPelsPerMeter;// 說明垂直解析度,用象素/米表示。一般為0
    unsigned int        biClrUsed;// 說明點陣圖實際使用的彩色表中的顏色索引數(設為0的話,則說明使用所有調色板項)。 
    unsigned int        biClrImportant;// 說明對圖象顯示有重要影響的顏色索引的數目,如果是0,表示都重要。
    char            buf[40];
} MyBITMAPINFOHEADER;

typedef struct MyRGBQUAD_type {
    unsigned char        palette[256][4];
    char            buf[256*4];
} MyRGBQUAD; /* for color template */

typedef unsigned char *ImageData;

typedef struct MyBITMAP_type {
    MyBITMAPFILEHEADER    file_header;
    MyBITMAPINFOHEADER    info_header;
    MyRGBQUAD        rgbquad;
    ImageData        image_data;
    int            bmp_gap;
} MyBITMAP;

// **************************************************
//                Prototype Declarations
// **************************************************

static void load_bmp_image(char filename[], MyBITMAP *bmp);
static void save_bmp_image(char filename[], MyBITMAP *bmp);

static void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b);
static void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b);

// **************************************************
//                  Global Variables
// **************************************************

static MyBITMAP bmp_tmp;
static int bmp_gap = 0;

// **************************************************
// Image Tools
// **************************************************

// **************************************************
// open_bmp
// Open and load a bitmap image file into memory (3 RGB arrays of 1024x1024).
// **************************************************
//void open_bmp(const char *filename[], int **bmp_r[1024][1024], int **bmp_g[1024][1024], int **bmp_b[1024][1024], int *width, int *height)

void open_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY], int *width, int *height)
{
    int r, g, b;
    int x, y;

    load_bmp_image((char*)filename, &bmp_tmp);
    
    *width  = bmp_tmp.info_header.biWidth;
    *height = bmp_tmp.info_header.biHeight;
    
    if ((*width > MaxBMPSizeX) || (*height > MaxBMPSizeY)) {
        printf("bmp size too big\n");
        exit(1);
    }

    for (x = 0; x <= *width - 1; x++) {
        for (y = 0; y <= *height - 1; y++) {
            get_bmp_pixel(&bmp_tmp, x, y, &r, &g, &b);
            bmp_r[x][y] = r;
            bmp_g[x][y] = g;
            bmp_b[x][y] = b;
        }
    }

}

// **************************************************
// save_bmp
// Save a bitmap image file from memory (3 RGB arrays of 1024x1024).
// **************************************************

//void save_bmp(const char *filename[], int *bmp_r[1024][1024], int *bmp_g[1024][1024], int *bmp_b[1024][1024])
void save_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY])
{
    int width, height;
    int r, g, b;
    int x, y;

    width  = bmp_tmp.info_header.biWidth;
    height = bmp_tmp.info_header.biHeight;

    for (x = 0; x <= width - 1; x++) {
        for (y = 0; y <= height - 1; y++) {
            r = bmp_r[x][y];
            g = bmp_g[x][y];
            b = bmp_b[x][y];
            set_bmp_pixel(&bmp_tmp, x, y, r, g, b);
        }
    }

    save_bmp_image((char*)filename, &bmp_tmp);

}

// **************************************************
// close_bmp
// Close and release memory for a bitmap image.
// **************************************************

void close_bmp()
{

    free(bmp_tmp.image_data);

}

// **************************************************
// load_bmp_image
// Loads a bitmap image file into memory.
// **************************************************

void load_bmp_image(char filename[], MyBITMAP *bmp)
{
    FILE *fp;
    int b0, b1, b2, b3;
    int i;

    if ((fp = fopen("lena_pepper_and_salt_noise10.bmp","rb")) == NULL) {
        printf("Couldn't find file %s.\n",filename);
        exit(1);
    }

    fread(&(bmp->file_header.buf), sizeof(char), 14, fp);

    b0 = (bmp->file_header.buf[0] & 0xff);
    b1 = (bmp->file_header.buf[1] & 0xff);
    bmp->file_header.B = (char)b0;
    bmp->file_header.M = (char)b1;

    b0 = (bmp->file_header.buf[2] & 0xff);
    b1 = (bmp->file_header.buf[3] & 0xff);
    b2 = (bmp->file_header.buf[4] & 0xff);
    b3 = (bmp->file_header.buf[5] & 0xff);
    bmp->file_header.bfSize = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->file_header.buf[6] & 0xff);
    b1 = (bmp->file_header.buf[7] & 0xff);
    bmp->file_header.bfReserved1 = (unsigned short int)((b1<<8) + b0);

    b0 = (bmp->file_header.buf[8] & 0xff);
    b1 = (bmp->file_header.buf[9] & 0xff);
    bmp->file_header.bfReserved2 = (unsigned short int)((b1<<8) + b0);

    b0 = (bmp->file_header.buf[10] & 0xff);
    b1 = (bmp->file_header.buf[11] & 0xff);
    b2 = (bmp->file_header.buf[12] & 0xff);
    b3 = (bmp->file_header.buf[13] & 0xff);
    bmp->file_header.bfOffBits = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    fread(&(bmp->info_header.buf), sizeof(char), 40, fp);

    b0 = (bmp->info_header.buf[0] & 0xff);
    b1 = (bmp->info_header.buf[1] & 0xff);
    b2 = (bmp->info_header.buf[2] & 0xff);
    b3 = (bmp->info_header.buf[3] & 0xff);
    bmp->info_header.biSize = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[4] & 0xff);
    b1 = (bmp->info_header.buf[5] & 0xff);
    b2 = (bmp->info_header.buf[6] & 0xff);
    b3 = (bmp->info_header.buf[7] & 0xff);
    bmp->info_header.biWidth = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);
    
    bmp->bmp_gap = ((int)bmp->info_header.biWidth) % 4;

    b0 = (bmp->info_header.buf[8]  & 0xff);
    b1 = (bmp->info_header.buf[9]  & 0xff);
    b2 = (bmp->info_header.buf[10] & 0xff);
    b3 = (bmp->info_header.buf[11] & 0xff);
    bmp->info_header.biHeight = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[12] & 0xff);
    b1 = (bmp->info_header.buf[13] & 0xff);
    bmp->info_header.biPlanes = (unsigned short int)((b1<<8) + b0);

    b0 = (bmp->info_header.buf[14] & 0xff);
    b1 = (bmp->info_header.buf[15] & 0xff);
    bmp->info_header.biBitCount = (unsigned short int)((b1<<8) + b0);

    b0 = (bmp->info_header.buf[16] & 0xff);
    b1 = (bmp->info_header.buf[17] & 0xff);
    b2 = (bmp->info_header.buf[18] & 0xff);
    b3 = (bmp->info_header.buf[19] & 0xff);
    bmp->info_header.biCompression = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[20] & 0xff);
    b1 = (bmp->info_header.buf[21] & 0xff);
    b2 = (bmp->info_header.buf[22] & 0xff);
    b3 = (bmp->info_header.buf[23] & 0xff);
    bmp->info_header.biSizeImage = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);
    if (bmp->info_header.biSizeImage == 0) {
        if (bmp->info_header.biBitCount == 8) {
            bmp->info_header.biSizeImage = (unsigned int)(bmp->info_header.biWidth * bmp->info_header.biHeight);
        } else if (bmp->info_header.biBitCount == 24) {
            bmp->info_header.biSizeImage = (unsigned int)(bmp->info_header.biWidth * bmp->info_header.biHeight * 3);
        } else {
            printf("Not supported format!\n");
            exit(1);
        }
    }

    b0 = (bmp->info_header.buf[24] & 0xff);
    b1 = (bmp->info_header.buf[25] & 0xff);
    b2 = (bmp->info_header.buf[26] & 0xff);
    b3 = (bmp->info_header.buf[27] & 0xff);
    bmp->info_header.biXPelsPerMeter = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[28] & 0xff);
    b1 = (bmp->info_header.buf[29] & 0xff);
    b2 = (bmp->info_header.buf[30] & 0xff);
    b3 = (bmp->info_header.buf[31] & 0xff);
    bmp->info_header.biYPelsPerMeter = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[32] & 0xff);
    b1 = (bmp->info_header.buf[33] & 0xff);
    b2 = (bmp->info_header.buf[34] & 0xff);
    b3 = (bmp->info_header.buf[35] & 0xff);
    bmp->info_header.biClrUsed = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    b0 = (bmp->info_header.buf[36] & 0xff);
    b1 = (bmp->info_header.buf[37] & 0xff);
    b2 = (bmp->info_header.buf[38] & 0xff);
    b3 = (bmp->info_header.buf[39] & 0xff);
    bmp->info_header.biClrImportant = (unsigned int)((b3<<24) + (b2<<16) + (b1<<8) + b0);

    if (bmp->info_header.biBitCount == 8) {
        fread(&(bmp->rgbquad.buf), sizeof(char), 256*4, fp);
        for (i = 0; i <= 255; i++) {
           b0 = (bmp->rgbquad.buf[i*4]   & 0xff);
           b1 = (bmp->rgbquad.buf[i*4+1] & 0xff);
           b2 = (bmp->rgbquad.buf[i*4+2] & 0xff);
           b3 = (bmp->rgbquad.buf[i*4+3] & 0xff);
           bmp->rgbquad.palette[i][0] = (unsigned char)(b0 & 0xff);
           bmp->rgbquad.palette[i][1] = (unsigned char)(b1 & 0xff);
           bmp->rgbquad.palette[i][2] = (unsigned char)(b2 & 0xff);
           bmp->rgbquad.palette[i][3] = (unsigned char)(b3 & 0xff);
       }
       if ((bmp->image_data = (unsigned char*) malloc(bmp->info_header.biSizeImage)) == NULL) {
           printf("couldn't get memory for file %s.\n",filename);
           exit(1);
       }
       fread(bmp->image_data, sizeof(char), bmp->info_header.biSizeImage, fp);
   } else if (bmp->info_header.biBitCount == 24) {
       if ((bmp->image_data = (unsigned char*) malloc(bmp->info_header.biSizeImage)) == NULL) {
           printf("couldn't get memory for file %s.\n",filename);
           exit(1);
       }
       fread(bmp->image_data, sizeof(char), bmp->info_header.biSizeImage, fp);
   } else {
       printf("Not supported format!\n");
       exit(1);
   }
   
   fclose(fp);

}

// **************************************************
// save_bmp_image
// Saves memory into a bitmap image file.
// **************************************************

void save_bmp_image(char filename[], MyBITMAP *bmp)
{
   FILE *fp;
   
   if ((fp = fopen(filename,"wb")) == NULL) {
       printf("Couldn't find file %s.\n",filename);
       exit(1);
   }

   fwrite(&(bmp->file_header.buf), sizeof(char), 14, fp);

   fwrite(&(bmp->info_header.buf), sizeof(char), 40, fp);

   if (bmp->info_header.biBitCount == 8) { /* for gray images */
       fwrite(&(bmp->rgbquad.buf), sizeof(char), 256*4, fp); /* color template */
       fwrite(bmp->image_data, sizeof(char), bmp->info_header.biSizeImage, fp);
   } else if (bmp->info_header.biBitCount == 24) { /* for true color images */
       fwrite(bmp->image_data, sizeof(char), bmp->info_header.biSizeImage, fp);
   } else {
       printf("Not supported format!\n");
       exit(1);
   }

   fclose(fp);

}

// **************************************************
// get_bmp_pixel
// (inline code) get pixel colors of bmp image according to (X,Y)
// **************************************************

void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b)
{
   int width, height;
   int offset;

       if (bmp->info_header.biBitCount == 8) {

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;

           *color_b = (int)(*(bmp->image_data+offset+0) & 0xff);    // B
           *color_g = (*color_b);    // G
           *color_r = (*color_b);    // R

       } else if (bmp->info_header.biBitCount == 24) {

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = 3 * (y * width + x) + y * bmp->bmp_gap;

           *color_b = (int)(*(bmp->image_data+offset+0) & 0xff);    // B
           *color_g = (int)(*(bmp->image_data+offset+1) & 0xff);    // G
           *color_r = (int)(*(bmp->image_data+offset+2) & 0xff);    // R

       } else {

           printf("Not supported format!\n");
           exit(1);

       }

}

// **************************************************
// set_bmp_pixel
// (inline code) set pixel colors of bmp image according to (X,Y)
// **************************************************

void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b)
{
   int width, height;
   int offset;

       if (bmp->info_header.biBitCount == 8) { /* for gray images */

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;

           *(bmp->image_data+offset+0) = (unsigned char)(color_b & 0xff);

       } else if (bmp->info_header.biBitCount == 24) { /* for true color images */

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = 3 * (y * width + x) + y * bmp->bmp_gap;

           *(bmp->image_data+offset+0) = (unsigned char)(color_b & 0xff);
           *(bmp->image_data+offset+1) = (unsigned char)(color_g & 0xff);
           *(bmp->image_data+offset+2) = (unsigned char)(color_r & 0xff);

       } else {

           printf("Not supported format!\n");
           exit(1);

       }

}


int compare_ints(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}
void median_filter(unsigned char *image_data, int width, int height, int filter_size) {
    int window_size = filter_size * filter_size;
    int window_radius = filter_size / 2;
    int *neighbor_values = (int *)malloc(window_size * sizeof(int));
    unsigned char *filtered_data = (unsigned char *)malloc(width * height * sizeof(unsigned char));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
        int neighbor_count = 0;
            for (int dy = -window_radius; dy <= window_radius; dy++) {
                for (int dx = -window_radius; dx <= window_radius; dx++) {
                    int neighbor_x = x + dx;
                    int neighbor_y = y + dy;

          if (neighbor_x >= 0 && neighbor_x < width && neighbor_y >= 0 && neighbor_y < height) {
            neighbor_values[neighbor_count++] = image_data[neighbor_y * width + neighbor_x];
          }
        }
      }


      qsort(neighbor_values, neighbor_count, sizeof(int), compare_ints);

      int median_index = neighbor_count / 2;
      filtered_data[y * width + x] = (unsigned char)neighbor_values[median_index];
    }
  }

    memcpy(image_data, filtered_data, width * height * sizeof(unsigned char));
    free(filtered_data);
    free(neighbor_values);
}


int R[MaxBMPSizeX][MaxBMPSizeY]; // MaxBMPSizeX and MaxBMPSizeY are defined in "bmp.h"
int r[MaxBMPSizeX][MaxBMPSizeY];

int main(int argc, char *argv[]) {
    int width, height;
    int x, y;

    // open_bmp("lena_color_512.bmp", R, G, B, width, height); // for true color images
    open_bmp("lena_pepper_and_salt_noise10.bmp", R, R, R, &width, &height); // for gray images

    int filter_size = 5;  // Adjust filter size as needed
    median_filter(bmp_tmp.image_data, width, height, filter_size);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            
            R[x][y] = bmp_tmp.image_data[y * width + x];
        }
    }
    // save_bmp("lenna_new1.bmp", r, g, b); // for true color images
    save_bmp("lena_new5.bmp", R, R, R); // for gray images

    printf("Job Finished!\n");
    close_bmp();
    system("PAUSE"); /* so that the command window holds a while */

    return 0;
}