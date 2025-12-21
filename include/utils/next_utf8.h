#ifndef VOXL_NEXT_UTF8_H
#define VOXL_NEXT_UTF8_H


#include <string>


// alternative à <codecvt> déprécié
// généré par gemini
inline uint32_t NextUTF8(const std::string& str, size_t& i) 
{
  uint8_t c = (uint8_t)str[i++];
  if (c < 0x80) return c; // ASCII standard (1 octet)
  
  if ((c & 0xE0) == 0xC0) { // 2 octets
    return ((c & 0x1F) << 6) | (str[i++] & 0x3F);
  }
  if ((c & 0xF0) == 0xE0) { // 3 octets (ex: €)
    uint32_t res = ((c & 0x0F) << 12) | ((str[i++] & 0x3F) << 6);
    return res | (str[i++] & 0x3F);
  }
  if ((c & 0xF8) == 0xF0) { // 4 octets
    uint32_t res = ((c & 0x07) << 18) | ((str[i++] & 0x3F) << 12);
    res |= ((str[i++] & 0x3F) << 6);
    return res | (str[i++] & 0x3F);
  }
  return '?';
}


#endif // !VOXL_NEXT_UTF8_H