#ifdef __cplusplus
extern "C" {
#endif

enum imgtyp {DEFAULT=0, P2,P3,P5,P6, PS,
             GREY,RGB,RED,GREEN,BLUE,COLOR};

typedef struct {
  int        sizeX,sizeY;
  GLubyte   *data;
} PPMimage;

typedef struct {
  unsigned char idfield_len;
  unsigned char cmap_type;
  unsigned char image_type;
  unsigned char cmap_spec[5];
  unsigned char x_orig[2];
  unsigned char y_orig[2];
  unsigned char width[2];
  unsigned char height[2];
  unsigned char pixel_size;
  unsigned char image_desc;
} TGAheader;

/* image.c */
PPMimage *loadPPM(const char *imgname, int *type);
int    savePPM(const char *imgname, PPMimage* img, int typimg);
void   saveEPS(pScene sc, const char *imgname, PPMimage* pixels);
int    imgHard(pScene sc,char *data,char key);

#ifdef __cplusplus
}
#endif
