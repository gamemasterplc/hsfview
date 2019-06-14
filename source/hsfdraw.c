#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include "hsffile.h"
#include "hsfdraw.h"

#define MATRIX_STACK_SIZE 96

static int matrix_stack_pos;
static Mtx matrix_stack[MATRIX_STACK_SIZE];
static HSFModel *model;

void DrawHsfModel(HSFModel *hsf, Mtx *mv)
{
	if(hsf->root_object == NULL)
	{
		return;
	}
	guMtxCopy(*mv, matrix_stack[0]);
	matrix_stack_pos = 1;
	model = hsf;
	CallObject(model->root_object);
}

void CallObject(ObjectData *object)
{
	switch(object->object_type)
	{
		case OBJECT_TYPE_MESH:
			DrawMesh(object);
			break;
		
		case OBJECT_TYPE_ROOT:
			CallChild(object);
			break;
			
		default:
			break;
	}
}

void DrawModelFace(ObjectData *object, ModelFace *face)
{
	int i = 0;
	int tristrip_len = 0;
	int material_idx = (face->tri_face.material_index & 0xFFF);
	MaterialData *material_base = model->material;
	MaterialData *material = &material_base[material_idx];
	int texture_count = material->texture_count;
	int vtx_mode = material->vtx_mode;
	int use_color = vtx_mode&VTXMODE_USE_CLR0;
	int material_flags = (object->render_flags|material->material_flags);
	int pass_flags = (material->alt_flags&PASS_BITS);
	int alpha_flags = material_flags&PUNCHTHROUGH_ALPHA_BITS;
	if(pass_flags != 0||material->transparency != 0)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
	}
	else
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	}
	if(alpha_flags != 0)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetAlphaCompare(GX_GREATER, HALF_ALPHA, GX_AOP_OR, GX_GREATER, HALF_ALPHA);
	}
	else
	{
		GX_SetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_GREATER, 1);
	}
	if(material_flags&DONT_CULL_BACKFACES)
	{
		GX_SetCullMode(GX_CULL_NONE);
	}
	else
	{
		GX_SetCullMode(GX_CULL_BACK);
	}
	if(texture_count == 0)
	{
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetArray(GX_VA_POS, object->vertex->data, sizeof(float)*3);
		GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
		if(model->normal_format == NORMAL_S8)
		{
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
			GX_SetArray(GX_VA_NRM, object->normal->data, sizeof(s8)*3);
		}
		else
		{
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
			GX_SetArray(GX_VA_NRM, object->normal->data, sizeof(float)*3);
		}
		if(use_color)
		{
			GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
			GX_SetArray(GX_VA_CLR0, object->color->data, sizeof(u8)*4);
		}
		GX_SetNumTexGens(0);
		GX_SetNumTevStages(2);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GXColor col;
		col.r = material->ambient_color[0];
		col.g = material->ambient_color[1];
		col.b = material->ambient_color[2];
		col.a = ((1.0f-material->transparency)*255);
		GX_SetTevColor(GX_TEVREG1, col);
		GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_C1, GX_CC_CPREV, GX_CC_ZERO);
		GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_A1, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO);
		GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	else
	{
		GX_InvalidateTexAll();
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetArray(GX_VA_POS, object->vertex->data, sizeof(float)*3);
		GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
		if(model->normal_format == NORMAL_S8)
		{
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
			GX_SetArray(GX_VA_NRM, object->normal->data, sizeof(s8)*3);
		}
		else
		{
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
			GX_SetArray(GX_VA_NRM, object->normal->data, sizeof(float)*3);
		}
		if(use_color)
		{
			GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
			GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
			GX_SetArray(GX_VA_CLR0, object->color->data, sizeof(u8)*4);
		}
		GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetArray(GX_VA_TEX0, object->texcoord->data, sizeof(float)*2);
		AttributeData **attribute_list = material->first_attribute;
		AttributeData *attribute = attribute_list[0];
		if(attribute->texture->format != HSF_TEXTURE_FORMAT_IA8_PALETTE)
		{
			LoadTexture(attribute, attribute->texture, 0);
		}
		else
		{
			LoadTexture(attribute, attribute->texture, USE_INDIRECT_PALETTE);
		}
		GX_SetNumTexGens(1);
		GX_SetNumTevStages(2);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GXColor col;
		col.r = material->ambient_color[0];
		col.g = material->ambient_color[1];
		col.b = material->ambient_color[2];
		col.a = ((1.0f-material->transparency)*255);
		GX_SetTevColor(GX_TEVREG1, col);
		GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_ZERO);
		GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO, GX_CA_ZERO);
		GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	switch(face->tri_face.poly_type)
	{
		case POLYGON_TRIANGLE:
			GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
			GX_Position1x16(face->tri_face.indices[ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->tri_face.indices[8+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[8+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[8+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[8+ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->tri_face.indices[4+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[4+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[4+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[4+ATTR_TEXCOORD_IDX]);
			}
			GX_End();
			break;
			
		case POLYGON_QUAD:
			GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position1x16(face->quad_face.indices[ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->quad_face.indices[ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->quad_face.indices[ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->quad_face.indices[ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->quad_face.indices[8+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->quad_face.indices[8+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->quad_face.indices[8+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->quad_face.indices[8+ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->quad_face.indices[12+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->quad_face.indices[12+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->quad_face.indices[12+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->quad_face.indices[12+ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->quad_face.indices[4+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->quad_face.indices[4+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->quad_face.indices[4+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->quad_face.indices[4+ATTR_TEXCOORD_IDX]);
			}
			GX_End();
			break;
		
		case POLYGON_TRISTRIP:
			tristrip_len = face->tri_face.strip_count;
			GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 3+tristrip_len);
			GX_Position1x16(face->tri_face.indices[ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->tri_face.indices[8+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[8+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[8+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[8+ATTR_TEXCOORD_IDX]);
			}
			GX_Position1x16(face->tri_face.indices[4+ATTR_VERTEX_IDX]);
			GX_Normal1x16(face->tri_face.indices[4+ATTR_NORMAL_IDX]);
			if(use_color)
			{
				GX_Color1x16(face->tri_face.indices[4+ATTR_COLOR_IDX]);
			}
			if(texture_count != 0)
			{
				GX_TexCoord1x16(face->tri_face.indices[4+ATTR_TEXCOORD_IDX]);
			}
			Tristrip *tristrip = face->tri_face.strip;
			for(i=0; i<tristrip_len; i++)
			{
				GX_Position1x16(tristrip->index[ATTR_VERTEX_IDX]);
				GX_Normal1x16(tristrip->index[ATTR_NORMAL_IDX]);
				if(use_color)
				{
					GX_Color1x16(tristrip->index[ATTR_COLOR_IDX]);
				}
				if(texture_count != 0)
				{
					GX_TexCoord1x16(tristrip->index[ATTR_TEXCOORD_IDX]);
				}
				tristrip++;
			}
			GX_End();
			break;
			
	}
}

void DrawObject(ObjectData *object)
{
	Mtx normal;
	int i;
	GX_LoadPosMtxImm(matrix_stack[matrix_stack_pos-1], GX_PNMTX0);
	guMtxInvXpose(matrix_stack[matrix_stack_pos-1], normal);
	GX_LoadNrmMtxImm(normal, GX_PNMTX0);
	GX_SetCullMode(GX_CULL_NONE);
	FaceData *face_head = object->face;
	ModelFace *face = face_head->face;
	int surface_count = face_head->surface_count;
	for(i=0; i<surface_count; i++)
	{
		GX_InvVtxCache();
		DrawModelFace(object, face);
		face++;
	}
}

void DrawMesh(ObjectData *object)
{
	int i = 0;
	PushObjectMatrix(object);
	DrawObject(object);
	for(i=0; i<object->children_count; i++)
	{
		ObjectData **children = object->child_object;
		CallObject(children[i]);
	}
	matrix_stack_pos--;
}

void LoadTexture(AttributeData *attribute, HsfTextureData *texture, int slot_num)
{
	GXTexObj tex_obj;
	GXTlutObj tlut_obj;
	int wrap_x = attribute->wrap_mode_s;
	int wrap_y = attribute->wrap_mode_t;
	int use_mipmap = attribute->texture_flags&ENABLE_MIPMAP;
	use_mipmap >>= MIPMAP_BIT_POS;
	int use_nearest = attribute->texture_flags&ENABLE_NEAREST_FILTER;
	short width = texture->width;
	short height = texture->height;
	void *raw_texture = texture->texture;
	void *palette = texture->palette;

	switch(texture->format)
	{
		case HSF_TEXTURE_FORMAT_I4:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_I4, wrap_x, wrap_y, use_mipmap);
			break;
		
		case HSF_TEXTURE_FORMAT_I8:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_I8, wrap_x, wrap_y, use_mipmap);
			break;
			
		case HSF_TEXTURE_FORMAT_IA4:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_IA4, wrap_x, wrap_y, use_mipmap);
			break;
			
		case HSF_TEXTURE_FORMAT_IA8:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_IA8, wrap_x, wrap_y, use_mipmap);
			break;
		
		case HSF_TEXTURE_FORMAT_RGB565:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_RGB565, wrap_x, wrap_y, use_mipmap);
			break;
		
		case HSF_TEXTURE_FORMAT_RGB5A3:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_RGB5A3, wrap_x, wrap_y, use_mipmap);
			break;
		
		case HSF_TEXTURE_FORMAT_RGBA8:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_RGBA8, wrap_x, wrap_y, use_mipmap);
			break;	
		
		case HSF_TEXTURE_FORMAT_CMPR:
			GX_InitTexObj(&tex_obj, raw_texture, width, height, GX_TF_CMPR, wrap_x, wrap_y, use_mipmap);
			break;		
			
		case HSF_TEXTURE_FORMAT_RGB565_PALETTE:
			if(texture->bpp >= 8)
			{
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_RGB565, texture->palette_entries);
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI8, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			else
			{
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_RGB565, texture->palette_entries);
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI4, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			break;	
			
		case HSF_TEXTURE_FORMAT_RGB5A3_PALETTE:
			if(texture->bpp >= 8)
			{
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_RGB5A3, texture->palette_entries);
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI8, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			else
			{
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_RGB5A3, texture->palette_entries);
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI4, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			break;
			
		case HSF_TEXTURE_FORMAT_IA8_PALETTE:
			if((slot_num&USE_INDIRECT_PALETTE) != 0)
			{
				char *ind_palette = (char *)palette;
				int palette_offset = texture->palette_entries;
				palette_offset += 15;
				palette_offset &= 0xFFF0;
				palette_offset *= 2;
				palette = (void *)(ind_palette+palette_offset);
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_IA8, texture->palette_entries);
			}
			else
			{
				GX_InitTlutObj(&tlut_obj, palette, GX_TL_IA8, texture->palette_entries);
			}
			slot_num &= SLOT_NUM_BITS;
			if(texture->bpp >= 8)
			{
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI8, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			else
			{
				GX_LoadTlut(&tlut_obj, slot_num);
				GX_InitTexObjCI(&tex_obj, raw_texture, width, height, GX_TF_CI4, wrap_x, wrap_y, use_mipmap, slot_num);
			}
			break;
			
		default:
			break;
	}
	
	if(use_nearest != 0)
	{
		GX_InitTexObjLOD(&tex_obj, GX_NEAR, GX_NEAR, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		if(use_mipmap != 0)
		{
			float max_lod = attribute->mipmap_max_lod;
			GX_InitTexObjLOD(&tex_obj, GX_LIN_MIP_LIN, GX_LINEAR, 0, max_lod, 0, 0, 1, 1);
		}
		else
		{
			if(texture->max_lod == 0)
			{
				GX_InitTexObjLOD(&tex_obj, GX_LINEAR, GX_LINEAR, 0, 0, 0, 0, 0, 0);
			}
			else
			{
				float max_lod_level = texture->max_lod;
				max_lod_level -= 1.0f;
				GX_InitTexObjLOD(&tex_obj, GX_LIN_MIP_LIN, GX_LINEAR, 0, max_lod_level, 0, 0, 1, 1);
			}
		}
	}
	GX_LoadTexObj(&tex_obj, slot_num);
}

void CallChild(ObjectData *object)
{
	int i = 0;
	PushObjectMatrix(object);
	for(i=0; i<object->children_count; i++)
	{
		ObjectData **children = object->child_object;
		CallObject(children[i]);
	}
	matrix_stack_pos--;
}

void PushObjectMatrix(ObjectData *object)
{
	Mtx object_mtx;
	Mtx rot_mtx;
	guMtxScale(object_mtx, object->curr_transform.scale.x, object->curr_transform.scale.y, object->curr_transform.scale.z);
	if(object->curr_transform.rot.x != 0)
	{
		guMtxRotDeg(rot_mtx, 'x', object->curr_transform.rot.x);
		guMtxConcat(rot_mtx, object_mtx, object_mtx);
	}
	if(object->curr_transform.rot.y != 0)
	{
		guMtxRotDeg(rot_mtx, 'y', object->curr_transform.rot.y);
		guMtxConcat(rot_mtx, object_mtx, object_mtx);
	}
	if(object->curr_transform.rot.z != 0)
	{
		guMtxRotDeg(rot_mtx, 'z', object->curr_transform.rot.z);
		guMtxConcat(rot_mtx, object_mtx, object_mtx);
	}
	
	guMtxTransApply(object_mtx, object_mtx, object->curr_transform.trans.x, object->curr_transform.trans.y, object->curr_transform.trans.z);
	guMtxConcat(matrix_stack[matrix_stack_pos-1], object_mtx, matrix_stack[matrix_stack_pos]);
	matrix_stack_pos++;
}