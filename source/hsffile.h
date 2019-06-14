#ifndef HSFFILE_H
#define HSFFILE_H

//Color Defines
#define CHANNEL_RED_IDX 0
#define CHANNEL_GREEN_IDX 1
#define CHANNEL_BLUE_IDX 2
#define CHANNEL_ALPHA_IDX 3
#define MAX_ALPHA 255
#define HALF_ALPHA 128

//Material Defines
#define PASS_BITS 0xF //If pass_flags when ANDed by this value isn't 0 then Z Writes are Disabled
#define VTXMODE_DEFAULT 0x1
#define VTXMODE_USE_NBT 0x2
#define VTXMODE_USE_CLR0 0x4
#define DONT_CULL_BACKFACES 0x2
#define BLEND_MODE_MASK 0x30
#define BLEND_SRCALPHA_ONE 0x10
#define BLEND_ZERO_INVSRCCLR 0x20
#define BLEND_SRCALPHA_INVSRCALPHA 0x0
#define PUNCHTHROUGH_ALPHA_BITS 0x1200
#define MATERIAL_INDEX_MASK 0xFFF
#define HIGHLIGHT_FRAME_MASK 0xF0
#define HIGHLIGHT_ENABLE 0x100

//Attribute Defines
#define WRAP_CLAMP 0
#define WRAP_REPEAT 1
#define ENABLE_NEAREST_FILTER 0x40
#define ENABLE_MIPMAP 0x80
#define MIPMAP_BIT_POS 7

//Polygon Defines
#define POLYGON_TRIANGLE 2
#define POLYGON_QUAD 3
#define POLYGON_TRISTRIP 4
#define ATTR_VERTEX_IDX 0
#define ATTR_NORMAL_IDX 1
#define ATTR_COLOR_IDX 2
#define ATTR_TEXCOORD_IDX 3

//Object Type Defines
#define OBJECT_TYPE_ROOT 0
#define OBJECT_TYPE_MESH 2

//Texture Format Defines
#define HSF_TEXTURE_FORMAT_I4 0
#define HSF_TEXTURE_FORMAT_I8 1
#define HSF_TEXTURE_FORMAT_IA4 2
#define HSF_TEXTURE_FORMAT_IA8 3
#define HSF_TEXTURE_FORMAT_RGB565 4
#define HSF_TEXTURE_FORMAT_RGB5A3 5
#define HSF_TEXTURE_FORMAT_RGBA8 6
#define HSF_TEXTURE_FORMAT_CMPR 7
#define HSF_TEXTURE_FORMAT_RGB565_PALETTE 9
#define HSF_TEXTURE_FORMAT_RGB5A3_PALETTE 10
#define HSF_TEXTURE_FORMAT_IA8_PALETTE 11
#define USE_INDIRECT_PALETTE 0x8000
#define SLOT_NUM_BITS 0x7FFF

//Normal Type Defines
#define NORMAL_S8 0
#define NORMAL_FLOAT 1

//Track Modes
#define TRACK_MODE_NORMAL 2
#define TRACK_MODE_OBJECT 3
#define TRACK_MODE_UNKNOWN 3
#define TRACK_MODE_MATERIAL 9
#define TRACK_MODE_ATTRIBUTE 10

//Interpolation Modes
#define INTERPOLATE_CONSTANT 0
#define INTERPOLATE_LINEAR 1
#define INTERPOLATE_BEZIER 2
#define INTERPOLATE_BITMAP 3
#define INTERPOLATE_NONE 4
#define INTERPOLATE_ZERO 5

typedef struct hsf_section
{
	int data_offset;
	int count;
} HSFSection;

typedef struct vector_2
{
	float x;
	float y;
} Vector2;

typedef struct hsf_header
{
	char magic[8]; //Always HSFV037
	HSFSection fog_data; //Controls Fog Settings
	HSFSection color_data;
	HSFSection material_data;
	HSFSection attribute_data;
	HSFSection vertex_data;
	HSFSection normal_data;
	HSFSection texcoord_data;
	HSFSection face_data; //Makes Shapes out of Vertices
	HSFSection object_data; //Acts like Bones in a Traditional 3D Model
	HSFSection texture_data; //Gives Textures to Map onto Model
	HSFSection palette_data; //Gives Palettes for the Texture to Use
	HSFSection motion_data; //Used for Animating Models
	HSFSection cenv_data;
	HSFSection skeleton_data;
	HSFSection part_data; //Unused
	HSFSection cluster_data; //Unused
	HSFSection shape_data; //Unused
	HSFSection map_attribute_data; //Unused
	HSFSection matrix_data; //Used for Unknown Purpose
	HSFSection symbol_data;
	HSFSection string_data;
} HSFHeader;

typedef struct component_file_data
{
	int name_offset; //Relative to Start of String Table
	int entries;
	int data_offset; //Relative to Start of Data after Headers
} ComponentFileData;

typedef struct component_data
{
	char *name;
	int entries;
	void *data;
} ComponentData;

typedef struct palette_file_data
{
	int name_offset; //Relative to Start of String Table
	int unknown;
	int entries;
	int palette_offset; //Relative to Start of Palette Data after Headers
} PaletteFileData;

typedef struct palette_data
{
	char *name;
	int unknown;
	int entries;
	void *palette;
} PaletteData;

typedef struct hsf_texture_file_data
{
	int name_offset; //Relative to Start of String Table
	int max_lod;
	u8 format;
	u8 bpp; //Used for Selecting CI4/CI8
	u16 width;
	u16 height;
	u16 palette_entries;
	char texture_tint[4];
	int palette_index;
	int pad;
	int texture_offset; //Relative to Start of Texture Data after Headers
} HsfTextureFileData;

typedef struct hsf_texture_data
{
	char *name;
	int max_lod;
	u8 format;
	u8 bpp; //Used for Selecting CI4/CI8
	u16 width;
	u16 height;
	u16 palette_entries;
	char texture_tint[4];
	void *palette;
	int pad;
	void *texture;
} HsfTextureData;

typedef struct attr_transform
{
	Vector2 scale;
	Vector2 pos;
} AttrTransform;

typedef struct attribute_file_data
{
	int name_offset; //Relative to Start of String Table
	int tex_anim_offset; //Replaced with Pointer to Texture Animation at Runtime
	char unk[12];
	float nbt_enable; //1.0 for enabled, 0.0 for disabled
	float texture_enable; //1.0 for enabled, 0.0 for disabled
	float dontedit;
	AttrTransform tex_anim_start;
	AttrTransform tex_anim_end;
	char unknowns[36];
	int wrap_mode_s;
	int wrap_mode_t;
	char unknown[12];
	int mipmap_max_lod;
	int texture_flags;
	int texture_index;
} AttributeFileData;

typedef struct attribute_data
{
	char *name;
	void *tex_anim;
	char unk[12];
	float nbt_enable; //1.0 for enabled, 0.0 for disabled
	float texture_enable; //1.0 for enabled, 0.0 for disabled
	float dontedit;
	AttrTransform tex_anim_start;
	AttrTransform tex_anim_end;
	char unknowns[36];
	int wrap_mode_s;
	int wrap_mode_t;
	char unknown[12];
	int mipmap_max_lod;
	int texture_flags;
	HsfTextureData *texture;
} AttributeData;

typedef struct material_file_data
{
	int name_offset; //Relative to Start of String Table
	int unknown;
	short alt_flags; //Alternate flags
	u8 vtx_mode; //Vertex Mode
	char lit_ambient_color[3];
	char ambient_color[3];
	char shadow_color[3];
	float hilite_scale;
	float unk_float;
	float transparency; //Inverted
	float unknowns[2];
	float reflect_brightness;
	float unk;
	int material_flags;
	int texture_count;
	int first_symbol;
} MaterialFileData;

typedef struct material_data
{
	char *name; //Relative to Start of String Table
	int unknown;
	short alt_flags; //Alternate flags
	u8 vtx_mode; //Vertex Mode
	char lit_ambient_color[3];
	char ambient_color[3];
	char shadow_color[3];
	float hilite_scale;
	float unk_float;
	float transparency; //Inverted
	float unknowns[2];
	float reflect_brightness;
	float unk;
	int material_flags;
	int texture_count;
	AttributeData **first_attribute;
} MaterialData;

typedef struct tristrip
{
	u16 index[4];
} Tristrip;

typedef union model_file_face
{
	struct {
		u16 poly_type; //Polygon Type
		u16 material_index; //Only Lower 12 Bits are Used
		u16 indices[12];
		int strip_count;
		int strip_data_index;
		float nbt_data[3];
	} tri_face;
	struct {
		u16 poly_type; //Polygon Type
		u16 material_index; //Only Lower 12 Bits are Used
		u16 indices[16];
		float nbt_data[3];
	} quad_face;
} ModelFileFace;

typedef union model_face
{
	struct {
		u16 poly_type; //Polygon Type
		u16 material_index; //Only Lower 12 Bits are Used
		u16 indices[12];
		int strip_count;
		Tristrip *strip;
		float nbt_data[3];
	} tri_face;
	struct {
		u16 poly_type; //Polygon Type
		u16 material_index; //Only Lower 12 Bits are Used
		u16 indices[16];
		float nbt_data[3];
	} quad_face;
} ModelFace;

typedef struct face_file_data
{
	int name_offset; //Relative to Start of String Table
	int surface_count;
	int data_offset; //Relative to Start of Face Arrays after Face Data Headers
} FaceFileData;

typedef struct face_data
{
	char *name;
	int surface_count;
	ModelFace *face;
} FaceData;

typedef struct obj_transform
{
	guVector trans;
	guVector rot;
	guVector scale; //Degrees
} ObjTransform;

typedef struct object_const_data
{
	int flags;
	short hook_mdl;
	short pad;
	void *draw_data;
	void *dl_buf;
	Mtx model;
	int unk;
} ObjectConstData;

typedef struct object_file_data
{
	int name_offset; //Relative to Start of String Table
	int object_type; //Used to determine whether Object is a Mesh
	int const_data_offset; //Null in File
	int render_flags; //Combined with material flags to get true material flags
	int global_index; //Value of -1 Means Index is the Root Object
	int children_count; 
	int symbol_index;
	ObjTransform base_transform;
	ObjTransform curr_transform;
	guVector cull_box_min;
	guVector cull_box_max;
	float unk;
	char unknowns[132];
	int face_index;
	int vertex_data_index;
	int normal_data_index;
	int color_data_index;
	int texcoord_data_index;
	int material_data_offset;
	int attribute_index;
	int unknown_int;
	int vtx_child_count;
	int vtx_symbol_index;
	int cluster_count;
	int cluster_symbol_index;
	int hook_flag;
	int cenv_index; //Used for Unknown purpose
	int file_offset[2];
} ObjectFileData;

typedef struct object_data
{
	char *name;
	int object_type; //Used to determine whether Object is a Mesh
	ObjectConstData *const_data;
	int render_flags; //Combined with material flags to get true material flags
	int global_index; //Value of -1 Means Index is the Root Object
	int children_count; 
	struct object_data **child_object;
	ObjTransform base_transform;
	ObjTransform curr_transform;
	guVector cull_box_min;
	guVector cull_box_max;
	float unk;
	char unknowns[132];
	FaceData *face;
	ComponentData *vertex;
	ComponentData *normal;
	ComponentData *color;
	ComponentData *texcoord;
	MaterialData *material;
	AttributeData *attribute;
	int unknown_int;
	int vtx_child_count;
	ComponentData **child_vertices;
	int cluster_count;
	void **cluster;
	int hook_flag;
	void *cenv; //Used for Unknown purpose
	void *file[2];
} ObjectData;

typedef struct track_file_data
{
	char mode;
	char value_idx;
	short idx;
	short type;
	short effect;
	short interpolate_type;
	short keyframe_count;
	int keyframe_offset;
} TrackFileData;

typedef struct bitmap_key
{
	float val;
	HsfTextureData *bitmap;
} BitmapKey;

typedef struct bitmap_file_key
{
	float val;
	int bitmap_index;
} BitmapFileKey;

typedef union keyframe
{
	float *data;
	float val;
} Keyframe;

typedef struct track_data
{
	char mode;
	char value_idx;
	short idx;
	short type;
	short effect;
	short interpolate_type;
	short keyframe_count;
	Keyframe keyframe;
} TrackData;

typedef struct motion_file_data
{
	int name_offset;
	int track_count;
	int track_data_offset;
	float motion_length;
} MotionFileData;

typedef struct motion_data
{
	char *name;
	int track_count;
	TrackData *track;
	float motion_length;
} MotionData;

typedef struct matrix_file_data
{
	int base_matrix_idx;
	int matrix_count;
	int matrix_offset;
} MatrixFileData;

typedef struct matrix_data
{
	int base_matrix_idx;
	int matrix_count;
	Mtx *matrix;
} MatrixData;

typedef struct skeleton_file_data
{
	int name_offset;
	ObjTransform transform;
} SkeletonFileData;

typedef struct skeleton_data
{
	char *name;
	ObjTransform transform;
} SkeletonData;

typedef struct hsf_model
{
	char magic[8]; //Always HSFV037M
	void *scene;
	AttributeData *attribute;
	MaterialData *material;
	ComponentData *vertex;
	ComponentData *normal;
	ComponentData *texcoord;
	ComponentData *color;
	FaceData *face;
	HsfTextureData *texture;
	PaletteData *palette;
	ObjectData *root_object;
	void *cenv;
	SkeletonData *skeleton;
	void *cluster;
	void *part;
	void *shape;
	MotionData *motion;
	ObjectData *object_base;
	void *map_attr;
	MatrixData *matrix;
	char *string;
	short scene_count;
	short attribute_count;
	short material_count;
	short vertex_count;
	short normal_count;
	short texcoord_count;
	short color_count;
	short face_count;
	short texture_count;
	short palette_count;
	short object_count;
	short normal_format;
	short skeleton_count;
	short cluster_count;
	short part_count;
	short shape_count;
	short motion_count;
	short unk_count;
	short map_attr_count;
	short matrix_count;
} HSFModel;

#endif