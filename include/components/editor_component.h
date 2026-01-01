#ifndef VOXL_EDITOR_COMPONENT
#define VOXL_EDITOR_COMPONENT


template<typename T>
struct EditorComponent
{
  static void Register();
  static void Display();
  static void Save();
  static void Load();
};


#endif // !VOXL_EDITOR_COMPONENT