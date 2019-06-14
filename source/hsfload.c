#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <ogc/cache.h>
#include "hsffile.h"
#include "hsfload.h"
#include "alignedalloc.h"

static HSFHeader header;
static HSFModel model;
static char *orig_string_table;
static char *string_table;
static int *orig_symbol_table;
static void **symbol_table;
static char *file_ptr;
static ObjectData *object_base;
static ObjectFileData *object_file_base;

HSFModel *LoadHsfModel(HSFHeader *hsf_file)
{
	HsfFileInit(hsf_file);
	LoadColorData();
	LoadPaletteData();
	LoadTextureData();
	LoadAttributeData();
	LoadMaterialData();
	LoadVertexData();
	LoadNormalData();
	LoadTexcoordData();
	LoadFaceData();
	LoadObjectData();
	LoadMotionData();
	LoadCenv();
	LoadSkeleton();
	LoadMatrices();
	return HsfFileClose(hsf_file);
}

void HsfFileInit(HSFHeader *hsf_file)
{
	file_ptr = (char *)hsf_file;
	memcpy(&header, hsf_file, sizeof(HSFHeader));
	memset(&model, 0, sizeof(HSFModel));
	orig_string_table = &file_ptr[hsf_file->string_data.data_offset];
	object_base = NULL;
	if(sizeof(void *) != 4)
	{
		string_table = malloc(hsf_file->string_data.count);
		memcpy(string_table, orig_string_table, hsf_file->string_data.count);
	}
	else
	{
		string_table = orig_string_table;
	}
	orig_symbol_table = (int *)&file_ptr[hsf_file->symbol_data.data_offset];
	if(sizeof(void *) != 4)
	{
		symbol_table = malloc(sizeof(void *)*hsf_file->symbol_data.count);
	}
	else
	{
		symbol_table = (void **)orig_symbol_table;
	}
}

HSFModel *HsfFileClose(HSFHeader *hsf_file)
{
	HSFModel *new_model;
	if(sizeof(void *) != 4)
	{
		new_model = malloc(sizeof(HSFModel));
	}
	else
	{
		new_model = (HSFModel *)hsf_file;
	}
	memcpy(model.magic, header.magic, 8);
	model.string = string_table;
	model.magic[7] = 'M'; //Write in Memory Marker
	memcpy(new_model, &model, sizeof(HSFModel));
	if(sizeof(void *) != 4)
	{
		free_aligned(hsf_file);
	}
	return new_model;
}

void LoadColorData()
{
	ComponentData *color;
	void *data;
	int i;
	if(header.color_data.count != 0)
	{
		ComponentFileData *orig_color = (ComponentFileData *)&file_ptr[header.color_data.data_offset];
		int entry_count = header.color_data.count;
		if(sizeof(void *) != 4)
		{
			color = malloc(sizeof(ComponentData)*entry_count);
		}
		else
		{
			color = (ComponentData *)orig_color;
		}
		model.color = color;
		model.color_count = entry_count;
		char *data_base = (char *)&orig_color[entry_count];
		for(i=0; i<entry_count; i++)
		{
			color->name = GetHsfFileString(orig_color->name_offset);
			if(sizeof(void *) != 4)
			{
				data = malloc_aligned(32, orig_color->entries*4);
				memcpy(data, &data_base[orig_color->data_offset], orig_color->entries*4);
				DCInvalidateRange(data, orig_color->entries*4);
			}
			else
			{
				data = (void *)&data_base[orig_color->data_offset];
			}
			color->entries = orig_color->entries;
			color->data = data;
			orig_color++;
			color++;
		}
	}
}

void LoadPaletteData()
{
	PaletteData *palette;
	void *data;
	int i;
	if(header.palette_data.count != 0)
	{
		PaletteFileData *orig_palette = (PaletteFileData *)&file_ptr[header.palette_data.data_offset];
		int entry_count = header.palette_data.count;
		if(sizeof(void *) != 4)
		{
			palette = malloc(sizeof(PaletteData)*entry_count);
		}
		else
		{
			palette = (PaletteData *)orig_palette;
		}
		model.palette = palette;
		model.palette_count = entry_count;
		char *data_base = (char *)&orig_palette[entry_count];
		for(i=0; i<entry_count; i++)
		{
			palette->name = GetHsfFileString(orig_palette->name_offset);
			palette->unknown = orig_palette->unknown;
			palette->entries = orig_palette->entries;
			if(sizeof(void *) != 4)
			{
				data = malloc_aligned(32, palette->entries*2);
				memcpy(data, &data_base[orig_palette->palette_offset], palette->entries*2);
				DCInvalidateRange(data, palette->entries*2);
			}
			else
			{
				data = (void *)&data_base[orig_palette->palette_offset];
			}
			palette->palette = data;
			orig_palette++;
			palette++;
		}
	}
}

void LoadTextureData()
{
	HsfTextureData *texture;
	void *data;
	int i;
	if(header.texture_data.count != 0)
	{
		HsfTextureFileData *orig_texture = (HsfTextureFileData *)&file_ptr[header.texture_data.data_offset];
		int entry_count = header.texture_data.count;
		if(sizeof(void *) != 4)
		{
			texture = malloc(sizeof(HsfTextureData)*entry_count);
		}
		else
		{
			texture = (HsfTextureData *)orig_texture;
		}
		model.texture = texture;
		model.texture_count = entry_count;
		char *data_base = (char *)&orig_texture[entry_count];
		for(i=0; i<entry_count; i++)
		{
			texture->name = GetHsfFileString(orig_texture->name_offset);
			texture->max_lod = orig_texture->max_lod;
			texture->format = orig_texture->format;
			texture->bpp = orig_texture->bpp;
			texture->width = orig_texture->width;
			texture->height = orig_texture->height;
			texture->palette_entries = orig_texture->palette_entries;
			PaletteData *palette = GetPalettePtr(orig_texture->palette_index);
			if(palette != NULL)
			{
				texture->palette = palette->palette;
			}
			if(sizeof(void *) != 4)
			{
				int bmp_size = texture->width*texture->height*texture->bpp/8;
				data = malloc_aligned(32, bmp_size);
				memcpy(data, &data_base[orig_texture->texture_offset], bmp_size);
				DCInvalidateRange(data, bmp_size);
			}
			else
			{
				data = (void *)&data_base[orig_texture->texture_offset];
			}
			texture->texture = data;
			orig_texture++;
			texture++;
		}
	}
}

void LoadAttributeData()
{
	AttributeData *attribute;
	int i;
	if(header.attribute_data.count != 0)
	{
		AttributeFileData *orig_attribute = (AttributeFileData *)&file_ptr[header.attribute_data.data_offset];
		int entry_count = header.attribute_data.count;
		if(sizeof(void *) != 4)
		{
			attribute = malloc(sizeof(AttributeData)*entry_count);
		}
		else
		{
			attribute = (AttributeData *)orig_attribute;
		}
		model.attribute = attribute;
		model.attribute_count = entry_count;
		for(i=0; i<entry_count; i++)
		{
			if(orig_attribute->name_offset != -1)
			{
				attribute->name = GetHsfFileString(orig_attribute->name_offset);
			}
			else
			{
				attribute->name = NULL;
			}
			attribute->tex_anim = NULL;
			memmove(&attribute->unk, &orig_attribute->unk, 12);
			attribute->nbt_enable = orig_attribute->nbt_enable;
			attribute->texture_enable = orig_attribute->texture_enable;
			attribute->dontedit = orig_attribute->dontedit;
			memmove(&attribute->tex_anim_start, &orig_attribute->tex_anim_start, sizeof(AttrTransform));
			memmove(&attribute->tex_anim_end, &orig_attribute->tex_anim_end, sizeof(AttrTransform));
			memmove(&attribute->unknowns, &orig_attribute->unknowns, 36);
			attribute->wrap_mode_s = orig_attribute->wrap_mode_s;
			attribute->wrap_mode_t = orig_attribute->wrap_mode_t;
			memmove(&attribute->unknown, &orig_attribute->unknown, 12);
			attribute->mipmap_max_lod = orig_attribute->mipmap_max_lod;
			attribute->texture_flags = orig_attribute->texture_flags;
			attribute->texture = GetTexturePtr(orig_attribute->texture_index);
			orig_attribute++;
			attribute++;
		}
	}
}

void LoadMaterialData()
{
	MaterialData *material;
	int i, j;
	if(header.material_data.count != 0)
	{
		MaterialFileData *orig_material = (MaterialFileData *)&file_ptr[header.material_data.data_offset];
		int entry_count = header.material_data.count;
		if(sizeof(void *) != 4)
		{
			material = malloc(sizeof(MaterialData)*entry_count);
		}
		else
		{
			material = (MaterialData *)orig_material;
		}
		model.material = material;
		model.material_count = entry_count;
		for(i=0; i<entry_count; i++)
		{
			material->name = GetHsfFileString(orig_material->name_offset);
			material->unknown = orig_material->unknown;
			material->alt_flags = orig_material->alt_flags;
			material->vtx_mode = orig_material->vtx_mode;
			material->lit_ambient_color[CHANNEL_RED_IDX] = orig_material->lit_ambient_color[CHANNEL_RED_IDX];
			material->lit_ambient_color[CHANNEL_GREEN_IDX] = orig_material->lit_ambient_color[CHANNEL_GREEN_IDX];
			material->lit_ambient_color[CHANNEL_BLUE_IDX] = orig_material->lit_ambient_color[CHANNEL_BLUE_IDX];
			material->ambient_color[CHANNEL_RED_IDX] = orig_material->ambient_color[CHANNEL_RED_IDX];
			material->ambient_color[CHANNEL_GREEN_IDX] = orig_material->ambient_color[CHANNEL_GREEN_IDX];
			material->ambient_color[CHANNEL_BLUE_IDX] = orig_material->ambient_color[CHANNEL_BLUE_IDX];
			material->shadow_color[CHANNEL_RED_IDX] = orig_material->shadow_color[CHANNEL_RED_IDX];
			material->shadow_color[CHANNEL_GREEN_IDX] = orig_material->shadow_color[CHANNEL_GREEN_IDX];
			material->shadow_color[CHANNEL_BLUE_IDX] = orig_material->shadow_color[CHANNEL_BLUE_IDX];
			material->hilite_scale = orig_material->hilite_scale;
			material->unk_float = orig_material->unk_float;
			material->transparency = orig_material->transparency;
			material->unknowns[0] = orig_material->unknowns[0];
			material->unknowns[1] = orig_material->unknowns[1];
			material->reflect_brightness = orig_material->reflect_brightness;
			material->unk = orig_material->unk;
			material->material_flags = orig_material->material_flags;
			int texture_count = orig_material->texture_count;
			int attr_idx = orig_material->first_symbol;
			material->texture_count = texture_count;
			for(j=0; j<texture_count; j++)
			{
				AttributeData *attr = GetAttributePtr(orig_symbol_table[attr_idx+j]);
				symbol_table[attr_idx+j] = attr;
			}
			material->first_attribute = (AttributeData **)&symbol_table[attr_idx];
			orig_material++;
			material++;
		}
	}
}

void LoadVertexData()
{
	ComponentData *vertex;
	void *data;
	int i;
	if(header.vertex_data.count != 0)
	{
		ComponentFileData *orig_vertex = (ComponentFileData *)&file_ptr[header.vertex_data.data_offset];
		int entry_count = header.vertex_data.count;
		if(sizeof(void *) != 4)
		{
			vertex = malloc(sizeof(ComponentData)*entry_count);
		}
		else
		{
			vertex = (ComponentData *)orig_vertex;
		}
		model.vertex = vertex;
		model.color_count = entry_count;
		char *data_base = (char *)&orig_vertex[entry_count];
		for(i=0; i<entry_count; i++)
		{
			vertex->name = GetHsfFileString(orig_vertex->name_offset);
			if(sizeof(void *) != 4)
			{
				data = malloc_aligned(32, orig_vertex->entries*sizeof(float)*3);
				memcpy(data, &data_base[orig_vertex->data_offset], orig_vertex->entries*sizeof(float)*3);
				DCInvalidateRange(data, orig_vertex->entries*sizeof(float)*3);
			}
			else
			{
				data = (void *)&data_base[orig_vertex->data_offset];
			}
			vertex->entries = orig_vertex->entries;
			vertex->data = data;
			orig_vertex++;
			vertex++;
		}
	}
}

void LoadNormalData()
{
	ComponentData *normal;
	void *data;
	int i;
	if(header.normal_data.count != 0)
	{
		ComponentFileData *orig_normal = (ComponentFileData *)&file_ptr[header.normal_data.data_offset];
		int entry_count = header.normal_data.count;
		if(sizeof(void *) != 4)
		{
			normal = malloc(sizeof(ComponentData)*entry_count);
		}
		else
		{
			normal = (ComponentData *)orig_normal;
		}
		model.normal = normal;
		model.color_count = entry_count;
		char *data_base = (char *)&orig_normal[entry_count];
		for(i=0; i<entry_count; i++)
		{
			normal->name = GetHsfFileString(orig_normal->name_offset);
			if(sizeof(void *) != 4)
			{
				data = malloc_aligned(32, orig_normal->entries*sizeof(float)*3);
				memcpy(data, &data_base[orig_normal->data_offset], orig_normal->entries*sizeof(float)*3);
				DCInvalidateRange(data, orig_normal->entries*sizeof(float)*3);
			}
			else
			{
				data = (void *)&data_base[orig_normal->data_offset];
			}
			normal->entries = orig_normal->entries;
			normal->data = data;
			orig_normal++;
			normal++;
		}
	}
}

void LoadTexcoordData()
{
	ComponentData *texcoord;
	void *data;
	int i;
	if(header.texcoord_data.count != 0)
	{
		ComponentFileData *orig_texcoord = (ComponentFileData *)&file_ptr[header.texcoord_data.data_offset];
		int entry_count = header.texcoord_data.count;
		if(sizeof(void *) != 4)
		{
			texcoord = malloc(sizeof(ComponentData)*entry_count);
		}
		else
		{
			texcoord = (ComponentData *)orig_texcoord;
		}
		model.texcoord = texcoord;
		model.color_count = entry_count;
		char *data_base = (char *)&orig_texcoord[entry_count];
		for(i=0; i<entry_count; i++)
		{
			texcoord->name = GetHsfFileString(orig_texcoord->name_offset);
			if(sizeof(void *) != 4)
			{
				data = malloc_aligned(32, orig_texcoord->entries*sizeof(float)*2);
				memcpy(data, &data_base[orig_texcoord->data_offset], orig_texcoord->entries*sizeof(float)*2);
				DCInvalidateRange(data, orig_texcoord->entries*sizeof(float)*2);
			}
			else
			{
				data = (void *)&data_base[orig_texcoord->data_offset];
			}
			texcoord->entries = orig_texcoord->entries;
			texcoord->data = data;
			orig_texcoord++;
			texcoord++;
		}
	}
}

void LoadFaceData()
{
	FaceData *face;
	int tristrip_offset = 0;
	ModelFace *mdl_face_base;
	ModelFace *mdl_face;
	ModelFileFace *mdl_file_face;
	
	int i, j;
	if(header.face_data.count != 0)
	{
		FaceFileData *orig_face = (FaceFileData *)&file_ptr[header.face_data.data_offset];
		int entry_count = header.face_data.count;
		if(sizeof(void *) != 4)
		{
			face = malloc(sizeof(FaceData)*entry_count);
		}
		else
		{
			face = (FaceData *)orig_face;
		}
		model.face = face;
		model.color_count = entry_count;
		char *data_base = (char *)&orig_face[entry_count];
		FaceFileData *temp_face = orig_face;
		for(i=0; i<entry_count; i++)
		{
			tristrip_offset = (temp_face->surface_count*48)+temp_face->data_offset;
			temp_face++;
		}
		Tristrip *strip_base = (Tristrip *)&data_base[tristrip_offset];
		for(i=0; i<entry_count; i++)
		{
			face->name = GetHsfFileString(orig_face->name_offset);
			int surface_count = orig_face->surface_count;
			if(sizeof(void *) != 4)
			{
				mdl_face_base = malloc(orig_face->surface_count*sizeof(ModelFace));
				memcpy(mdl_face_base, &data_base[orig_face->data_offset], orig_face->surface_count*sizeof(ModelFace));
			}
			else
			{
				mdl_face_base = (ModelFace *)&data_base[orig_face->data_offset];
			}
			mdl_file_face = (ModelFileFace *)&data_base[orig_face->data_offset];
			mdl_face = mdl_face_base;
			for(j=0; j<surface_count; j++)
			{
				mdl_face->tri_face.poly_type = mdl_file_face->tri_face.poly_type;
				mdl_face->tri_face.material_index = mdl_file_face->tri_face.material_index;
				memmove(&mdl_face->tri_face.indices, &mdl_file_face->tri_face.indices, sizeof(u16)*12);
				if(mdl_file_face->tri_face.poly_type == POLYGON_TRISTRIP)
				{
					int vtx_count = mdl_file_face->tri_face.strip_count;
					Tristrip *strip;
					if(sizeof(void *) != 4)
					{
						strip = malloc(vtx_count*sizeof(Tristrip));
						memcpy(strip, &strip_base[mdl_file_face->tri_face.strip_data_index], vtx_count*sizeof(Tristrip));
					}
					else
					{
						strip = &strip_base[mdl_file_face->tri_face.strip_data_index];
					}
					mdl_face->tri_face.strip = strip;
					mdl_face->tri_face.strip_count = vtx_count;
				}
				if(mdl_file_face->tri_face.poly_type == POLYGON_QUAD)
				{
					memmove(&mdl_face->quad_face.indices[12], &mdl_file_face->quad_face.indices[12], sizeof(u16)*4);
				}
				mdl_face->tri_face.nbt_data[0] = mdl_file_face->tri_face.nbt_data[0];
				mdl_face->tri_face.nbt_data[1] = mdl_file_face->tri_face.nbt_data[1];
				mdl_face->tri_face.nbt_data[2] = mdl_file_face->tri_face.nbt_data[2];
				mdl_file_face++;		
				mdl_face++;
			}
			face->surface_count = surface_count;
			face->face = mdl_face_base;
			orig_face++;
			face++;
		}
	}
}

void ParseObject(ObjectFileData *object, ObjectData *dest)
{
	int i;
	int object_idx;
	int children_count;
	int symbol_idx;
	int object_offset;
	dest->object_type = object->object_type;
	dest->const_data = NULL;
	dest->render_flags = object->render_flags;
	switch(dest->object_type)
	{
		case OBJECT_TYPE_MESH:
			dest->global_index = object->global_index;
			children_count = object->children_count;
			dest->children_count = children_count;
			symbol_idx = object->symbol_index;
			for(i=0; i<children_count; i++)
			{
				symbol_table[symbol_idx+i] = GetObjectPtr(orig_symbol_table[symbol_idx+i]);
			}
			dest->child_object = (ObjectData **)&symbol_table[symbol_idx];
			if(model.root_object == NULL)
			{
				model.root_object = dest;
			}
			dest->vertex = GetVertexPtr(object->vertex_data_index);
			dest->normal = GetNormalPtr(object->normal_data_index);
			dest->color = GetColorPtr(object->color_data_index);
			dest->texcoord = GetTexcoordPtr(object->texcoord_data_index);
			dest->face = GetFacePtr(object->face_index);
			dest->vtx_child_count = 0;
			dest->child_vertices = NULL;
			dest->cluster_count = 0;
			dest->cluster = NULL;
			dest->cenv = NULL;
			dest->material = model.material;
			dest->attribute = model.attribute;
			dest->file[0] = &file_ptr[object->file_offset[0]];
			dest->file[1] = &file_ptr[object->file_offset[1]];
			memmove(&dest->base_transform, &object->base_transform, sizeof(ObjTransform));
			memcpy(&dest->curr_transform, &dest->base_transform, sizeof(ObjTransform));
			memmove(&dest->cull_box_min, &object->cull_box_min, sizeof(guVector));
			memmove(&dest->cull_box_max, &object->cull_box_max, sizeof(guVector));
			dest->unk = object->unk;
			memmove(&dest->unknowns, &object->unknowns, 132);
			for(i=0; i<children_count; i++)
			{
				object_offset = (int)((char *)symbol_table[symbol_idx+i]-(char *)object_base);
				object_idx = object_offset/sizeof(ObjectData);
				ParseObject(&object_file_base[object_idx], GetObjectPtr(object_idx));
			}
			break;
		
		case OBJECT_TYPE_ROOT:
			dest->global_index = object->global_index;
			children_count = object->children_count;
			dest->children_count = children_count;
			symbol_idx = object->symbol_index;
			for(i=0; i<children_count; i++)
			{
				symbol_table[symbol_idx+i] = GetObjectPtr(orig_symbol_table[symbol_idx+i]);
			}
			dest->child_object = (ObjectData **)&symbol_table[symbol_idx];
			memmove(&dest->base_transform, &object->base_transform, sizeof(ObjTransform));
			memmove(&dest->curr_transform, &dest->base_transform, sizeof(ObjTransform));
			dest->unk = object->unk;
			if(model.root_object == NULL)
			{
				model.root_object = dest;
			}
			for(i=0; i<children_count; i++)
			{
				object_offset = (int)((char *)symbol_table[symbol_idx+i]-(char *)object_base);
				object_idx = object_offset/sizeof(ObjectData);
				ParseObject(&object_file_base[object_idx], GetObjectPtr(object_idx));
			}
			break;
		
		default:
			break;
	}
}

void LoadObjectData()
{
	int i;
	ObjectFileData *root;
	if(header.object_data.count != 0)
	{
		ObjectFileData *obj_file = (ObjectFileData *)&file_ptr[header.object_data.data_offset];
		object_file_base = obj_file;
		if(sizeof(void *) != 4)
		{
			object_base = malloc(sizeof(ObjectData)*header.object_data.count);
		}
		else
		{
			object_base = (ObjectData *)&file_ptr[header.object_data.data_offset];
		}
		int obj_count = header.object_data.count;
		
		for(i=0; i<obj_count; i++)
		{
			ObjectData *curr_obj = &object_base[i];
			curr_obj->name = GetHsfFileString(obj_file->name_offset);
			obj_file++;
		}
		obj_file = object_file_base;
		for(i=0; i<obj_count; i++)
		{
			if(obj_file->global_index == -1)
			{
				break;
			}
			obj_file++;
		}
		root = obj_file;
		ParseObject(root, &object_base[i]);
		model.root_object = &object_base[i];
		model.object_count = header.object_data.count;
		model.object_base = object_base;
	}
}

void LoadMotionData()
{
	MotionData *motion;
	TrackData *track;
	TrackFileData *track_file;
	ObjectData *temp_obj;
	AttributeData *temp_attr;
	char *motion_name;
	char *key_file;
	float *key_new;
	int num_tracks;
	int i;
	int j;

	if(header.motion_data.count != 0)
	{
		model.motion_count = header.motion_data.count;
		MotionFileData *motion_file = (MotionFileData *)&file_ptr[header.motion_data.data_offset];
		if(sizeof(void *) != 4)
		{
			motion = malloc(sizeof(MotionData));
		}
		else
		{
			motion = (MotionData *)&file_ptr[header.motion_data.data_offset];
		}
		model.motion = motion;
		motion->name = GetHsfFileString(motion_file->name_offset);
		motion->track_count = motion_file->track_count;
		num_tracks = motion->track_count;
		if(sizeof(void *) != 4)
		{
			track = malloc(num_tracks*sizeof(TrackData));
		}
		else
		{
			track = (TrackData *)&file_ptr[header.motion_data.data_offset+16+motion_file->track_data_offset];
		}
		track_file = (TrackFileData *)&file_ptr[header.motion_data.data_offset+16+motion_file->track_data_offset];
		motion->track = track;
		motion->motion_length = motion_file->motion_length;
		key_file = (char *)&track_file[num_tracks];
		for(i=0; i<num_tracks; i++)
		{
			track->mode = track_file->mode;
			track->value_idx = track_file->value_idx;
			track->idx = track_file->idx;
			track->type = track_file->type;
			track->effect = track_file->effect;
			track->interpolate_type = track_file->interpolate_type;
			track->keyframe_count = track_file->keyframe_count;
			if(object_base != NULL)
			{
				switch(track->mode)
				{
					case TRACK_MODE_NORMAL:
					case TRACK_MODE_OBJECT:
						motion_name = GetHsfFileString(track->idx);
						temp_obj = object_base;
						track->idx = -1;
						for(j=0; j<header.object_data.count; j++)
						{
							if(strcmp(temp_obj->name, motion_name) == 0)
							{
								track->idx = j;
								break;
							}
							temp_obj++;
						}
						break;
						
					case TRACK_MODE_ATTRIBUTE:
						if(track->idx != -1)
						{
							motion_name = GetHsfFileString(track->idx);
							temp_attr = model.attribute;
							track->idx = -1;
							for(j=0; j<header.attribute_data.count; j++)
							{
								if(temp_attr->name != NULL)
								{
									if(strcmp(temp_attr->name, motion_name) == 0)
									{
										track->idx = j;
										break;
									}
								}
								temp_attr++;
							}
						}
						break;
						
					default:
						break;
				}
			}
			switch(track->interpolate_type)
			{
				case INTERPOLATE_CONSTANT:
					if(sizeof(void *) != 4)
					{
						key_new = malloc(sizeof(float)*2*track->keyframe_count);
					}
					else
					{
						key_new = (float *)&key_file[track_file->keyframe_offset];
					}
					memmove(key_new, &key_file[track_file->keyframe_offset], sizeof(float)*2*track->keyframe_count);
					track->keyframe.data = key_new;
					break;
					
				case INTERPOLATE_LINEAR:
					if(sizeof(void *) != 4)
					{
						key_new = malloc(sizeof(float)*2*track->keyframe_count);
					}
					else
					{
						key_new = (float *)&key_file[track_file->keyframe_offset];
					}
					memmove(key_new, &key_file[track_file->keyframe_offset], sizeof(float)*2*track->keyframe_count);
					track->keyframe.data = key_new;
					break;
					
				case INTERPOLATE_BEZIER:
					if(sizeof(void *) != 4)
					{
						key_new = malloc(sizeof(float)*4*track->keyframe_count);
					}
					else
					{
						key_new = (float *)&key_file[track_file->keyframe_offset];
					}
					memmove(key_new, &key_file[track_file->keyframe_offset], sizeof(float)*4*track->keyframe_count);
					track->keyframe.data = key_new;
					break;
					
				case INTERPOLATE_BITMAP:
					if(sizeof(void *) != 4)
					{
						key_new = malloc(sizeof(BitmapKey)*2*track->keyframe_count);
					}
					else
					{
						key_new = (float *)&key_file[track_file->keyframe_offset];
					}
					BitmapKey *key = (BitmapKey *)key_new;
					BitmapFileKey *file_bmp = (BitmapFileKey *)&key_file[track_file->keyframe_offset/sizeof(float)];
					for(j=0; j<track->keyframe_count; j++)
					{
						key->val = file_bmp->val;
						key->bitmap = GetTexturePtr(file_bmp->bitmap_index);
						file_bmp++;
						key++;
					}
					memmove(key_new, &key_file[track_file->keyframe_offset], sizeof(float)*2*track->keyframe_count);
					track->keyframe.data = key_new;
					break;
					
				default:
					track->keyframe.val = track_file->keyframe_offset;
					break;
			}
			track++;
			track_file++;
		}
	}
}

void LoadCenv()
{
	model.normal_format = header.object_data.count;
}

void LoadSkeleton()
{
	SkeletonFileData *skeleton_file;
	SkeletonData *skeleton;
	int i;
	if(header.skeleton_data.count != 0)
	{
		model.skeleton_count = header.skeleton_data.count;
		skeleton_file = (SkeletonFileData *)&file_ptr[header.skeleton_data.data_offset];
		if(sizeof(void *) != 4)
		{
			skeleton = malloc(model.skeleton_count*sizeof(SkeletonData));
		}
		else
		{
			skeleton = (SkeletonData *)&file_ptr[header.skeleton_data.data_offset];
		}
		model.skeleton = skeleton;
		for(i=0; i<model.skeleton_count; i++)
		{
			skeleton->name = GetHsfFileString(skeleton_file->name_offset);
			memmove(&skeleton->transform, &skeleton_file->transform, sizeof(ObjTransform));
			skeleton++;
			skeleton_file++;
		}
	}
}

void LoadMatrices()
{
	MatrixData *matrix;
	Mtx *mtx_data;
	char *mtx;
	if(header.matrix_data.count != 0)
	{
		model.matrix_count = header.matrix_data.count;
		MatrixFileData *matrix_file = (MatrixFileData *)&file_ptr[header.matrix_data.data_offset];
		if(sizeof(void *) != 4)
		{
			matrix = malloc(sizeof(MatrixData));
		}
		else
		{
			matrix = (MatrixData *)&file_ptr[header.matrix_data.data_offset];
		}
		model.matrix = matrix;
		matrix->base_matrix_idx = matrix_file->base_matrix_idx;
		matrix->matrix_count = matrix_file->base_matrix_idx;
		mtx = (char *)matrix;
		if(sizeof(void *) != 4)
		{
			mtx_data = malloc(sizeof(Mtx)*(matrix->base_matrix_idx+matrix->matrix_count));
			memset(mtx_data, 0, sizeof(Mtx)*(matrix->base_matrix_idx+matrix->matrix_count));
		}
		else
		{
			mtx_data = (Mtx *)&mtx[sizeof(MatrixData)+matrix_file->matrix_offset];
		}
	}
}

PaletteData *GetPalettePtr(int palette_idx)
{
	if(palette_idx == -1)
	{
		return NULL;
	}
	PaletteData *palette_base = model.palette;
	return &palette_base[palette_idx];
}

HsfTextureData *GetTexturePtr(int texture_idx)
{
	if(texture_idx == -1)
	{
		return NULL;
	}
	HsfTextureData *texture_base = model.texture;
	return &texture_base[texture_idx];
}

AttributeData *GetAttributePtr(int attribute_idx)
{
	if(attribute_idx == -1)
	{
		return NULL;
	}
	AttributeData *attribute_base = model.attribute;
	return &attribute_base[attribute_idx];
}

ObjectData *GetObjectPtr(int object_idx)
{
	if(object_idx == -1)
	{
		return NULL;
	}
	return &object_base[object_idx];
}

ComponentData *GetVertexPtr(int vertex_idx)
{
	if(vertex_idx == -1)
	{
		return NULL;
	}
	ComponentData *vertex_base = model.vertex;
	return &vertex_base[vertex_idx];
}

ComponentData *GetNormalPtr(int normal_idx)
{
	if(normal_idx == -1)
	{
		return NULL;
	}
	ComponentData *normal_base = model.normal;
	return &normal_base[normal_idx];
}

ComponentData *GetTexcoordPtr(int texcoord_idx)
{
	if(texcoord_idx == -1)
	{
		return NULL;
	}
	ComponentData *texcoord_base = model.texcoord;
	return &texcoord_base[texcoord_idx];
}

ComponentData *GetColorPtr(int color_idx)
{
	if(color_idx == -1)
	{
		return NULL;
	}
	ComponentData *color_base = model.color;
	return &color_base[color_idx];
}

FaceData *GetFacePtr(int face_idx)
{
	if(face_idx == -1)
	{
		return NULL;
	}
	FaceData *face_base = model.face;
	return &face_base[face_idx];
}

char *GetHsfFileString(int offset)
{
	return &string_table[offset];
}