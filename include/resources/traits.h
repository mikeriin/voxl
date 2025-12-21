#ifndef VOXL_RESOURCE_TRAITS_H
#define VOXL_RESOURCE_TRAITS_H


#include "loaders/font_loader.h"
#include "loaders/obj_loader.h"


template<typename T> struct ResourceTraits {};

template<> struct ResourceTraits<Font> { using Loader = FontLoader; };
template<> struct ResourceTraits<Mesh> { using Loader = OBJLoader; };


#endif // !VOXL_RESOURCE_TRAITS_H