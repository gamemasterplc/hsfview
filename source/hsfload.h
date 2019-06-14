#ifndef HSFLOAD_H
#define HSFLOAD_H

HSFModel *LoadHsfModel(HSFHeader *hsf_file);

void HsfFileInit(HSFHeader *hsf_file);

HSFModel *HsfFileClose(HSFHeader *hsf_file);

void LoadColorData();

void LoadPaletteData();

void LoadTextureData();

void LoadAttributeData();

void LoadMaterialData();

void LoadVertexData();

void LoadNormalData();

void LoadTexcoordData();

void LoadFaceData();

void ParseObject(ObjectFileData *object, ObjectData *dest);

void LoadObjectData();

void LoadMotionData();

void LoadCenv();

void LoadSkeleton();

void LoadMatrices();

PaletteData *GetPalettePtr(int palette_idx);

HsfTextureData *GetTexturePtr(int texture_idx);

AttributeData *GetAttributePtr(int attribute_idx);

ObjectData *GetObjectPtr(int object_idx);

ComponentData *GetVertexPtr(int vertex_idx);

ComponentData *GetNormalPtr(int normal_idx);

ComponentData *GetTexcoordPtr(int texcoord_idx);

ComponentData *GetColorPtr(int color_idx);

FaceData *GetFacePtr(int face_idx);

char *GetHsfFileString(int offset);

#endif