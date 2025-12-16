#ifndef VOXL_GLYPH_H
#define VOXL_GLYPH_H


struct Glyph
{
  // coordonnées du QUAD
  float planeLeft;
  float planeBottom;
  float planeRight;
  float planeTop;

  // coordonnées UV
  float uvLeft;
  float uvBottom;
  float uvRight;
  float uvTop;

  // de combien déplacer le curseur X pour la prochaine lettre
  float advance;
};


#endif // !VOXL_GLYPH_H