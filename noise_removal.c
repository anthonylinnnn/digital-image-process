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
// 定義了與BMP圖像處理相關的結構體。

// 定義BMP文件頭結構體，包含文件類型、大小、保留字和數據偏移等信息。
typedef struct MyBITMAPFILEHEADER_type {
    char            B;
    char            M;
    unsigned int        bfSize;// 文件大小
    unsigned short int    bfReserved1;// 保留,必須設置為0
    unsigned short int    bfReserved2;// 保留,必須設置為0
    unsigned int        bfOffBits;// 從文件頭到像素數據的偏移
    char            buf[14];
} MyBITMAPFILEHEADER;

// 定義BMP信息頭結構體，包含圖像的尺寸、壓縮類型、顏色深度等信息。
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

// 定義調色板結構體，用於存儲256色的調色板信息，每個顏色由4個字節組成（通常是RGBA）。
typedef struct MyRGBQUAD_type {
    unsigned char        palette[256][4];
    char            buf[256*4];
} MyRGBQUAD; /* for color template */

// 定義ImageData為指向unsigned char的指針，用於指向圖像數據。
typedef unsigned char *ImageData;

// 定義MyBITMAP結構體，包含文件頭、信息頭、調色板和圖像數據等，以及圖像數據行的填充字節（bmp_gap）。
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
// 函數原型聲明區域，聲明了一系列操作BMP圖像的函數。

// 聲明加載和保存BMP圖像的函數。
static void load_bmp_image(char filename[], MyBITMAP *bmp);
static void save_bmp_image(char filename[], MyBITMAP *bmp);

// 聲明獲取和設置圖像中特定像素顏色的函數。
static void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b);
static void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b);

// **************************************************
//                  Global Variables
// **************************************************
// 全局變量宣告區域。

// 聲明了一個MyBITMAP類型的全域變數和一個整型變量(資料型態)用於存儲圖像數據行的填充字節。
static MyBITMAP bmp_tmp;
static int bmp_gap = 0;

// **************************************************
// Image Tools
// **************************************************
// 圖像工具函數區域，可能用於進一步的圖像處理。

// **************************************************
// open_bmp
// Open and load a bitmap image file into memory (3 RGB arrays of 1024x1024).
// **************************************************
//void open_bmp(const char *filename[], int **bmp_r[1024][1024], int **bmp_g[1024][1024], int **bmp_b[1024][1024], int *width, int *height)


void open_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY], int *width, int *height)
{//定義了 open_bmp 函數，它接受一個文件名和三個二維整數陣列（分別代表紅、綠、藍色通道的圖像數據），以及兩個指針（指向圖像寬度和高度的整數）作為參數。
    int r, g, b;
    int x, y;

//調用 load_bmp_image 函數，將指定的BMP文件加載到一個全局變量 bmp_tmp 中。這個全局變量包含了圖像的所有信息，包括文件頭、信息頭、調色板和圖像數據。
    load_bmp_image((char*)filename, &bmp_tmp);
    
    //從 bmp_tmp 的信息頭中讀取圖像的寬度和高度，並將這些值賦給通過參數傳入的指針。
    *width  = bmp_tmp.info_header.biWidth;
    *height = bmp_tmp.info_header.biHeight;
    
    //檢查圖像的寬度和高度是否超過了預定義的最大值。如果是，則打印錯誤信息並退出程序。
    if ((*width > MaxBMPSizeX) || (*height > MaxBMPSizeY)) {
        printf("bmp size too big\n");
        exit(1);
    }

//使用兩層for迴圈遍歷圖像的每一個像素。
    for (x = 0; x <= *width - 1; x++) {
        for (y = 0; y <= *height - 1; y++) {
            get_bmp_pixel(&bmp_tmp, x, y, &r, &g, &b);//調用 get_bmp_pixel 函數，獲取當前像素的紅、綠、藍色值。
            bmp_r[x][y] = r;//將獲取到的顏色值分別存儲到三個二維整數陣列中，這些陣列將用於後續的圖像處理。
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
void save_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY])//這行定義了save_bmp函數，它接受一個文件名和三個二維整數陣列（分別代表紅、綠、藍色通道的圖像數據），以及圖像的寬度和高度。
{
    int width, height;
    int r, g, b;
    int x, y;

    width  = bmp_tmp.info_header.biWidth;//這兩行從全域變數bmp_tmp的信息頭部中讀取圖像的寬度和高度，並將它們賦值給width和height變量。
    height = bmp_tmp.info_header.biHeight;

    for (x = 0; x <= width - 1; x++) {//這兩行開始一個雙層for迴圈，用於遍歷圖像的每一個像素。
        for (y = 0; y <= height - 1; y++) {
            r = bmp_r[x][y];//這三行從輸入的三個顏色陣列中讀取當前像素的紅、綠、藍色值。
            g = bmp_g[x][y];
            b = bmp_b[x][y];
            set_bmp_pixel(&bmp_tmp, x, y, r, g, b);//這行調用set_bmp_pixel函數，將當前像素的顏色值寫入到全域變數bmp_tmp的圖像數據中。
        }
    }

    save_bmp_image((char*)filename, &bmp_tmp);//這行調用save_bmp_image函數，將修改後的圖像數據（存儲在bmp_tmp中）保存到指定的文件名中。

}
//總結來說，save_bmp函數的作用是將傳入的RGB圖像數據保存到指定的BMP文件中。它首先讀取圖像的寬度和高度，然後遍歷每一個像素，將其顏色值寫入到全域變數bmp_tmp中，
//最後調用save_bmp_image函數將這些數據寫入到文件中。

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

    if ((fp = fopen(filename,"rb")) == 0) {
        printf("Couldn't find file %s.\n",filename);
        exit(1);
    }

// 讀取BMP文件頭部信息到結構體
    fread(&(bmp->file_header.buf), sizeof(char), 14, fp);

// 解析文件頭部信息，例如文件大小、數據偏移等
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

// 讀取BMP信息頭部信息到結構體
    fread(&(bmp->info_header.buf), sizeof(char), 40, fp);

// 解析信息頭部信息，例如圖像寬度、高度、壓縮類型等
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

// 根據biBitCount判斷是否需要讀取調色板信息
    if (bmp->info_header.biBitCount == 8) {
        fread(&(bmp->rgbquad.buf), sizeof(char), 256*4, fp);
        // 讀取調色板信息
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
       if ((bmp->image_data = (unsigned char*) malloc(bmp->info_header.biSizeImage)) == 0) {
           printf("couldn't get memory for file %s.\n",filename);
           exit(1);
       }
       fread(bmp->image_data, sizeof(char), bmp->info_header.biSizeImage, fp);
    } else if (bmp->info_header.biBitCount == 24) {
       if ((bmp->image_data = (unsigned char*) malloc(bmp->info_header.biSizeImage)) == 0) {
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
   
   if ((fp = fopen(filename,"wb")) == 0) {
       printf("Couldn't find file %s.\n",filename);
       exit(1);
   }

// 寫入BMP文件頭部和信息頭部信息
   fwrite(&(bmp->file_header.buf), sizeof(char), 14, fp);

   fwrite(&(bmp->info_header.buf), sizeof(char), 40, fp);

// 如果有調色板，則寫入調色板信息
   if (bmp->info_header.biBitCount == 8) { /* for gray images */
   // 寫入圖像數據
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
//get_bmp_pixel 函數的目的是獲取 BMP 圖像中指定像素的顏色值。
void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b)
{//定義了一個函數 get_bmp_pixel，它接受一個指向 MyBITMAP 結構體的指針 bmp 和兩個整數 x、y（代表像素的位置），以及三個指向整數的指針 color_r、color_g、color_b（用於存儲獲取的紅、綠、藍顏色值）。
   int width, height;
   int offset;//offset 用於計算像素在圖像數據陣列中的位置。

       if (bmp->info_header.biBitCount == 8) {

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;//計算像素在圖像數據陣列中的位置。bmp->bmp_gap 是每行像素後的填充字節，用於確保每行的字節數是4的倍數。

           *color_b = (int)(*(bmp->image_data+offset+0) & 0xff);    // B //對於灰階圖像，只有一個顏色通道的值，這裡將該值賦給 color_b、color_g 和 color_r，表示灰階值。
           *color_g = (*color_b);    // G
           *color_r = (*color_b);    // R

       } else if (bmp->info_header.biBitCount == 24) {

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = 3 * (y * width + x) + y * bmp->bmp_gap;

        //將計算得到的偏移量用於從圖像數據中獲取藍、綠、紅顏色值，並將這些值存儲到相應的指針變量中。
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
//set_bmp_pixel 函數的目的是設定 BMP 圖像中指定像素的顏色。
void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b)
{//定義了一個函數 set_bmp_pixel，它接受一個指向 MyBITMAP 結構體的指針 bmp 和三個整數 x、y（代表像素的位置）以及 color_r、color_g、color_b（代表紅、綠、藍顏色的值）。
   int width, height;
   int offset;
        //檢查圖像是否為灰階圖像（每個像素8位）。
       if (bmp->info_header.biBitCount == 8) { /* for gray images */

           width  = bmp->info_header.biWidth;//從 bmp 結構體的信息頭中讀取圖像的寬度和高度。
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;//計算像素在圖像數據陣列中的位置。bmp->bmp_gap 是每行像素後的填充字節，用於確保每行的字節數是4的倍數。

           *(bmp->image_data+offset+0) = (unsigned char)(color_b & 0xff);//將藍色值設定到指定像素的位置。對於灰階圖像，只需要設定一個顏色值。

       } else if (bmp->info_header.biBitCount == 24) { /* for true color images */ //檢查圖像是否為真彩色圖像（每個像素24位）。

           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = 3 * (y * width + x) + y * bmp->bmp_gap;//對於真彩色圖像，計算像素在圖像數據陣列中的位置需要乘以3（因為每個像素由3個顏色值組成）。

           *(bmp->image_data+offset+0) = (unsigned char)(color_b & 0xff);//將藍、綠、紅顏色值分別設定到指定像素的位置。
           *(bmp->image_data+offset+1) = (unsigned char)(color_g & 0xff);
           *(bmp->image_data+offset+2) = (unsigned char)(color_r & 0xff);

       } else {

           printf("Not supported format!\n");
           exit(1);

       }

}





void erosion(unsigned char *image_data, int width, int height, int bmp[][MaxBMPSizeY]) {
    int x, y, i, j;
    int min_val;
    int temp[MaxBMPSizeX][MaxBMPSizeY];

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            min_val = 255;
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    int nx = x + i;
                    int ny = y + j;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (bmp[nx][ny] < min_val) min_val = bmp[nx][ny];
                    }
                }
            }
            temp[x][y] = min_val;
        }
    }

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            bmp[x][y] = temp[x][y];
        }
    }
}

void dilation(unsigned char *image_data, int width, int height, int bmp[][MaxBMPSizeY]) {
    int x, y, i, j;
    int max_val;
    int temp[MaxBMPSizeX][MaxBMPSizeY];

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            max_val = 0;
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    int nx = x + i;
                    int ny = y + j;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (bmp[nx][ny] > max_val) max_val = bmp[nx][ny];
                    }
                }
            }
            temp[x][y] = max_val;
        }
    }

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            bmp[x][y] = temp[x][y];
        }
    }
}



int R[1024][1024];
int r[1024][1024];
int G[1024][1024];
int B[1024][1024];
int main(int argc, char *argv[]) {
    int width, height;
    int x, y;

    open_bmp("noisy_rectangle.bmp", r, r, r, &width, &height); // for gray images
    
    for (int i = 0; i < 15; i++)
    {   
        erosion(bmp_tmp.image_data, width, height, r);
        
    }
    for (int i = 0; i < 23; i++)
    {
        dilation(bmp_tmp.image_data, width, height, r);
        
    }
    
    save_bmp("new_noise.bmp", r, r, r); // for gray images

    printf("Job Finished!\n");
    close_bmp();
    system("PAUSE"); /* so that the command window holds a while */

    return 0;
}