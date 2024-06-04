
// **************************************************
// System Constant Definition
// **************************************************

#define MaxBMPSizeX	1024
#define MaxBMPSizeY	1024

// **************************************************
// Structures
// **************************************************

// **************************************************
// Prototypes
// **************************************************

void open_bmp(char filename[], int bmp_r[MaxBMPSizeX][MaxBMPSizeY], int bmp_g[MaxBMPSizeX][MaxBMPSizeY], int bmp_b[MaxBMPSizeX][MaxBMPSizeY], int& width, int& height);
void save_bmp(char filename[], int bmp_r[MaxBMPSizeX][MaxBMPSizeY], int bmp_g[MaxBMPSizeX][MaxBMPSizeY], int bmp_b[MaxBMPSizeX][MaxBMPSizeY]);
void close_bmp();

