
// **************************************************
// System Constant Definition
// **************************************************

#define MaxBMPSizeX 1024    // can be changed
#define MaxBMPSizeY 1024    // can be changed

// **************************************************
// Structures
// **************************************************

// **************************************************
// Prototypes
// **************************************************

void open_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY], int *width, int *height);
void save_bmp(const char *filename, int bmp_r[][MaxBMPSizeY], int bmp_g[][MaxBMPSizeY], int bmp_b[][MaxBMPSizeY]);
void close_bmp();