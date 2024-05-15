#include "bmp_31.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


// 定義BMP文件頭結構，包含文件類型、大小、保留字和數據偏移等資訊。
typedef struct MyBITMAPFILEHEADER_type {
    char            B;
    char            M;
    unsigned int        bfSize;// 文件大小
    unsigned short int    bfReserved1;// 保留,必須設置為0
    unsigned short int    bfReserved2;// 保留,必須設置為0
    unsigned int        bfOffBits;// 從文件頭到像素數據的偏移
    char            buf[14];
} MyBITMAPFILEHEADER;

// 定義BMP標頭結構，包含圖像的尺寸、壓縮類型、顏色深度等資訊。
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


// 定義調色板結構，用於存儲256色的調色板資訊，每個顏色由4個位元組成（通常是RGBA）。
typedef struct MyRGBQUAD_type {
    unsigned char        palette[256][4];
    char            buf[256*4];
} MyRGBQUAD; /* for color template */

// 定義ImageData為指向unsigned char的指針，用於指向圖像數據。
typedef unsigned char *ImageData;

// 定義MyBITMAP結構體，包含文件頭、資訊頭、調色板和圖像數據等，以及圖像數據行的填充位元（bmp_gap）。
typedef struct MyBITMAP_type {
    MyBITMAPFILEHEADER    file_header;
    MyBITMAPINFOHEADER    info_header;
    MyRGBQUAD        rgbquad;
    ImageData        image_data;
    int            bmp_gap;
} MyBITMAP;


// 宣告加載和保存BMP圖像的函數。
static void load_bmp_image(char filename[], MyBITMAP *bmp);
static void save_bmp_image(char filename[], MyBITMAP *bmp);

//int* BW=1024;
//int* BH=1024;

//RAW DATA
int RR[MaxBMPSizeX][MaxBMPSizeY]; // MaxBMPSizeX and MaxBMPSizeY are defined in "bmp.h"
int GR[MaxBMPSizeX][MaxBMPSizeY];
int BR[MaxBMPSizeX][MaxBMPSizeY]; // MaxBMPSizeX and MaxBMPSizeY are defined in "bmp.h"

//PROCESSED DATA
int RP[MaxBMPSizeX][MaxBMPSizeY];
int GP[MaxBMPSizeX][MaxBMPSizeY]; // MaxBMPSizeX and MaxBMPSizeY are defined in "bmp.h"
int BP[MaxBMPSizeX][MaxBMPSizeY];

int color_type=0;
int N_W,N_H;

// 宣告獲取和設置圖像中特定像素顏色的函數。
static void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b);
static void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b);

// 宣告了一個MyBITMAP類型的全域變數和一個整型變量(資料型態)用於存儲圖像數據行的填充位元。
static MyBITMAP bmp_tmp;
static int bmp_gap = 0;


void open_bmp(char filename[], int bmp_r[ MaxBMPSizeX][MaxBMPSizeY], int bmp_g[ MaxBMPSizeX][MaxBMPSizeY], int bmp_b[ MaxBMPSizeX][MaxBMPSizeY], int *width, int *height)
{//定義了 open_bmp 函數，它接受一個文件名和三個二維整數陣列（分別代表紅、綠、藍色通道的圖像數據），以及兩個指針（指向圖像寬度和高度的整數）作為參數。
    int r, g, b;
    int x, y;

    printf("going to convert %s",filename);
//調用 load_bmp_image 函數，將指定的BMP文件加載到一個全局變量 bmp_tmp 中。這個全局變量包含了圖像的所有資訊，包括文件頭、資訊頭、調色板和圖像數據。
    load_bmp_image((char*)filename, &bmp_tmp);
    
    //從 bmp_tmp 的資訊頭中讀取圖像的寬度和高度，並將這些值賦給通過參數傳入的指針。
    *width  = bmp_tmp.info_header.biWidth;
    *height = bmp_tmp.info_header.biHeight;
    
    //檢查圖像的寬度和高度是否超過了預定義的最大值。如果是，則打印錯誤資訊並退出程序。
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

void save_bmp(char filename[], int bmp_r[MaxBMPSizeX][MaxBMPSizeY], int bmp_g[MaxBMPSizeX][MaxBMPSizeY], int bmp_b[MaxBMPSizeX][MaxBMPSizeY])//這行定義了save_bmp函數，它接受一個文件名和三個二維整數陣列（分別代表紅、綠、藍色通道的圖像數據），以及圖像的寬度和高度。
{
    int width, height;
    int r, g, b;
    int x, y;

    width  = bmp_tmp.info_header.biWidth;//這兩行從全域變數bmp_tmp的資訊頭部中讀取圖像的寬度和高度，並將它們賦值給width和height變量。
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

void close_bmp(){free(bmp_tmp.image_data);}

void load_bmp_image(char filename[], MyBITMAP *bmp)
{
    FILE *fp;
    int b0, b1, b2, b3;
    int i;

    if ((fp = fopen(filename,"rb")) == NULL) {
        printf("Couldn't find file %s.\n",filename);
        exit(1);
    }

// 讀取BMP文件頭部資訊到結構體
    fread(&(bmp->file_header.buf), sizeof(char), 14, fp);

// 解析文件頭部資訊，例如文件大小、數據偏移等
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

// 讀取BMP資訊頭部資訊到結構體
    fread(&(bmp->info_header.buf), sizeof(char), 40, fp);

// 解析資訊頭部資訊，例如圖像寬度、高度、壓縮類型等
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
            color_type=0;
            bmp->info_header.biSizeImage = (unsigned int)(bmp->info_header.biWidth * bmp->info_header.biHeight);
        } else if (bmp->info_header.biBitCount == 24) {
            color_type=1;
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

// 根據biBitCount判斷是否需要讀取調色板資訊
    if (bmp->info_header.biBitCount == 8) {
        fread(&(bmp->rgbquad.buf), sizeof(char), 256*4, fp);
        // 讀取調色板資訊
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

void save_bmp_image(char filename[], MyBITMAP *bmp)
{
   FILE *fp;
   
   if ((fp = fopen(filename,"wb")) == NULL) {
       printf("Couldn't find file %s.\n",filename);
       exit(1);
   }

// 寫入BMP文件頭部和資訊頭部資訊
   fwrite(&(bmp->file_header.buf), sizeof(char), 14, fp);

   fwrite(&(bmp->info_header.buf), sizeof(char), 40, fp);

// 如果有調色板，則寫入調色板資訊
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

void get_bmp_pixel(MyBITMAP *bmp, int x, int y, int* color_r, int* color_g, int* color_b)
{//定義了一個函數 get_bmp_pixel，它接受一個指向 MyBITMAP 結構體的指針 bmp 和兩個整數 x、y（代表像素的位置），以及三個指向整數的指針 color_r、color_g、color_b（用於存儲獲取的紅、綠、藍顏色值）。
   int width, height;
   int offset;//offset 用於計算像素在圖像數據陣列中的位置。

       if (bmp->info_header.biBitCount == 8) {
           width  = bmp->info_header.biWidth;
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;//計算像素在圖像數據陣列中的位置。bmp->bmp_gap 是每行像素後的填充位元，用於確保每行的位元數是4的倍數。

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

void set_bmp_pixel(MyBITMAP *bmp, int x, int y, int color_r, int color_g, int color_b)
{//定義了一個函數 set_bmp_pixel，它接受一個指向 MyBITMAP 結構體的指針 bmp 和三個整數 x、y（代表像素的位置）以及 color_r、color_g、color_b（代表紅、綠、藍顏色的值）。
   int width, height;
   int offset;
        //檢查圖像是否為灰階圖像（每個像素8位）。
       if (bmp->info_header.biBitCount == 8) { /* for gray images */

           width  = bmp->info_header.biWidth;//從 bmp 結構體的資訊頭中讀取圖像的寬度和高度。
           height = bmp->info_header.biHeight;

           offset = (y * width + x) + y * bmp->bmp_gap;//計算像素在圖像數據陣列中的位置。bmp->bmp_gap 是每行像素後的填充位元，用於確保每行的位元數是4的倍數。

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

// Helper function for qsort (assuming qsort is available in your environment)
int compare_ints(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}

//void rotation(int* src,double ang,int width,int height,int img_out){


void rotation(int src[MaxBMPSizeX][MaxBMPSizeY], double ang, int width, int height, int dst[1024][1024]) {
    //計算ang對應的三角函數值，以減少浮點數計算量
    double sin_angle = sin(ang * 3.1416 / 180.0);
    double cos_angle = cos(ang * 3.1416 / 180.0);

    int x_h,y_h,new_x_h,new_y_h;//舊圖像中心和新圖像中心
    int x, y,new_x, new_y;//舊圖像索引和新圖像索引

    int N_border_width=abs((int)((cos_angle * width ) + (sin_angle * height)));//新圖寬
    int N_border_height=abs((int)((sin_angle * width ) + (cos_angle * height)));//新圖高

    x_h=width/2;
    y_h=height/2;
    new_x_h=N_border_width/2;
    new_y_h=N_border_height/2;
    
    printf("\n\noriginal border =  %d , %d \nnew border = %d , %d",width,height,N_border_width,N_border_height);
    printf("\n\noriginal certer point = %d,%d \nnew center opint = %d,%d",x_h,y_h,new_x_h,new_y_h);

    double src_x, src_y;//舊圖像對應新圖像的位置
    double color;//顏色值
    int tx=0;//暫存新圖像索引
    int ty=0;//暫存新圖像索引

    //全圖掃描
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            
            int rad=sqrt(pow((x-x_h),2)+pow((y-y_h),2));

            //計算原本[x,y]翻轉後對應的座標
            new_x=x*cos_angle-y*sin_angle-x_h*cos_angle+y_h*sin_angle+x_h;
            new_y=y*cos_angle+x*sin_angle-x_h*sin_angle-y_h*cos_angle+y_h;
            
            //掃描並計算新圖像顏色值
            if (new_x >= 0 && new_x < N_border_width && new_y >= 0 && new_y < N_border_height) {

                src_x = cos_angle * (new_x - width / 2) + sin_angle * (new_y - height / 2) + width/2;
                src_y = -sin_angle * (new_x - width / 2) + cos_angle * (new_y - height / 2) + height/2;
                // Perform bilinear interpolation
                int src_x0 = (int)src_x;
                int src_y0 = (int)src_y;
                int src_x1 = src_x0 + 1;
                int src_y1 = src_y0 + 1;

                double dx = src_x - src_x0;
                double dy = src_y - src_y0;

                //獲得鄰近四點的顏色值
                double c00 = src[src_y0][src_x0];
                double c01 = src[src_y0][src_x1];
                double c10 = src[src_y1][src_x0];
                double c11 = src[src_y1][src_x1];
                    
                //計算顏色值
                color = (1 - dx) * (1 - dy) * c00 + dx * (1 - dy) * c01 + (1 - dx) * dy * c10 + dx * dy * c11;
                //color=sqrt(pow((1-dx),2)+pow((1-dy),2))*c00 + sqrt(pow((dx),2)+pow((1-dy),2))*c01 +sqrt(pow((1-dx),2)+pow((dy),2))*c10 +sqrt(pow((dx),2)+pow((dy),2))*c11 ;

                dst[new_y][new_x] = (int)color;
            }

            //若下方為0
            if (dst[new_y+1][new_x]==0){
                src_x = cos_angle * (new_x - width / 2) + sin_angle * (new_y +1 - height / 2) + width/2;
                src_y = -sin_angle * (new_x - width / 2) + cos_angle * (new_y +1- height / 2) + height/2;
                // Perform bilinear interpolation
                int src_x0 = (int)src_x;
                int src_y0 = (int)src_y;
                int src_x1 = src_x0 + 1;
                int src_y1 = src_y0 + 1;

                double dx = src_x - src_x0;
                double dy = src_y - src_y0;

                double c00 = src[src_y0][src_x0];
                double c01 = src[src_y0][src_x1];
                double c10 = src[src_y1][src_x0];
                double c11 = src[src_y1][src_x1];

                color = (1 - dx) * (1 - dy) * c00 + dx * (1 - dy) * c01 + (1 - dx) * dy * c10 + dx * dy * c11;
                //color=sqrt(pow((1-dx),2)+pow((1-dy),2))*c00 + sqrt(pow((dx),2)+pow((1-dy),2))*c01 +sqrt(pow((1-dx),2)+pow((dy),2))*c10 +sqrt(pow((dx),2)+pow((dy),2))*c11 ;

                dst[new_y+1][new_x] = (int)color;
            }
            //若後方為0
            if (dst[new_y][new_x+1]==0){
                
                src_x = cos_angle * (new_x +1 - width / 2) + sin_angle * (new_y  - height / 2) + width/2;
                src_y = -sin_angle * (new_x +1- width / 2) + cos_angle * (new_y - height / 2) + height/2;
                // Perform bilinear interpolation
                int src_x0 = (int)src_x;
                int src_y0 = (int)src_y;
                int src_x1 = src_x0 + 1;
                int src_y1 = src_y0 + 1;

                double dx = src_x - src_x0;
                double dy = src_y - src_y0;

                double c00 = src[src_y0][src_x0];
                double c01 = src[src_y0][src_x1];
                double c10 = src[src_y1][src_x0];
                double c11 = src[src_y1][src_x1];

                color = (1 - dx) * (1 - dy) * c00 + dx * (1 - dy) * c01 + (1 - dx) * dy * c10 + dx * dy * c11;
                //color=sqrt(pow((1-dx),2)+pow((1-dy),2))*c00 + sqrt(pow((dx),2)+pow((1-dy),2))*c01 +sqrt(pow((1-dx),2)+pow((dy),2))*c10 +sqrt(pow((dx),2)+pow((dy),2))*c11 ;

                dst[new_y][new_x+1] = (int)color;
            }

            tx=new_x;
            ty=new_y;
                        

        }

    }    
    printf("\n\noriginal border =  %d , %d \nnew border = %d , %d",width,height,N_border_width,N_border_height);
    printf("\n\noriginal certer point = %d,%d \nnew center opint = %d,%d",x_h,y_h,new_x_h,new_y_h);
}

//*/

int main(){
    printf("000333\n");//env test
    int width=0;
    int height=0;
    double ang=0.0;
    int modr=0;


    open_bmp("framed_lena_color_256.bmp", RR,GR,BR,&width,&height);
    //if picture is mono color
    if (color_type=0){   
  
        printf("gray image\n");

        printf("\nrotation angle: ");
        scanf("%lf",&ang);
        printf("\nrotate %lf degree\n",ang);

        modr=ang/360;
//        ang=360.0-fmod(ang,360);

        rotation(RR,ang,width,height,RP);

        save_bmp("N_R_M.bmp",RP,RP,RP);
    }
    //if picture is true color
    else if (color_type=1){

        printf("true color image\n");     

        printf("\nrotation angle: ");
        scanf("%lf",&ang);
        printf("\nrotate %lf degree\n",ang); 

        modr=ang/360;
 //       ang=360.0*(modr+1)-ang;        

        rotation(RR,ang,width,height,RP);
        rotation(GR,ang,width,height,GP);
        rotation(BR,ang,width,height,BP);

        save_bmp("N_R_T.bmp",RP,GP,BP);
    }
/*
    rotation(RR,ang,width,height,RP);
    rotation(GR,ang,width,height,GP);
    rotation(BR,ang,width,height,BP);
   
    save_bmp("./N_R.bmp",RP,GP,BP);
    */
    printf("\nimg size = %d ,%d \nrotate %lf degree ",width,height,ang);

    system("pause");
    return(0);
}