#ifndef __FONTS_H
#define __FONTS_H

// font parameters main structure
typedef struct {
  const unsigned char FontWidth;
  const unsigned char FontHeight;
  const unsigned char *fontEn;
  const unsigned char *fontRu;
} FontDef2;

// some available fonts
extern FontDef2 Font_7x9;

#endif

