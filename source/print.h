#ifndef PRINT_H
#define PRINT_H

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

void PrintInit();

float GetStringWidth(float scale, char *string);

void SetTextColor(char r, char g, char b);

void PrintCenteredString(float x, float y, float scale, char *string);

void PrintRightAlignedString(float x, float y, float scale, char *string);

void PrintDropShadowString(float x, float y, float scale, char *string);

void PrintDropShadowCenteredString(float x, float y, float scale, char *string);

void PrintDropShadowRightAlignedString(float x, float y, float scale, char *string);

void PrintRectangle(float x, float y, float width, float height, char r, char g, char b, char a);

void PrintString(float x, float y, float scale, char *string);

#endif