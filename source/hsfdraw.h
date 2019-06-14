#ifndef HSFDRAW_H
#define HSFDRAW_H

void DrawHsfModel(HSFModel *hsf, Mtx *mv);

void DrawModelFace(ObjectData *object, ModelFace *face);

void DrawObject(ObjectData *object);

void DrawMesh(ObjectData *object);

void LoadTexture(AttributeData *attribute, HsfTextureData *texture, int slot_num);

void CallObject(ObjectData *object);

void CallChild(ObjectData *object);

void PushObjectMatrix(ObjectData *object);

#endif