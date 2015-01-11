
/**
 * RTGlib.h - A C library for parsing .RTG geometry
 * files.
 *
 * --------------------------------------------------
 * This source code is in the public domain. 
 * No warranties - use at your own risk!
 * --------------------------------------------------
 *
 * AUTHOR
 *
 * Gustav Taxén, gustavt@nada.kth.se
 *
 *
 * KNOWN ISSUES
 *
 * - The library is not thread-safe.
 * - The parser is not very robust - it is relatively
 *   easy to create RTG files that will cause it to crash.
 *   Files exported from Maya with the correct
 *   settings (see below) should work, though.
 * - The RTG file parsed must have vertex normals and
 *   texture coordinates. Also, vertex colors, polygon
 *   normals, animation is not supported.
 *
 *
 * THE RTG FORMAT
 *
 * An RTG file consists of a number of node hierarchies and
 * a set of materials. Each hierarchy node keeps 
 * transformation information (translation, rotation, 
 * scaling) and a pointer to a mesh object. Each mesh 
 * object contains lists of geometrical entities
 * (vertices, texture coordinates, normals, etc.) and
 * a pointer to a material.
 *
 *
 * BASIC USAGE
 *
 * To load an RTG file do the following:
 *
 *   RTGFile *file;
 *   file = RTG_Parse("mymodel.rtg", 1);
 *
 * where "mymodel.rtg" is the RTG file to load. The
 * second parameter specifies whether the RTG library
 * should print status information while it is parsing.
 * If it is 1, then information is printed, if 0 the
 * parsing is "silent".
 *
 *
 * ACCESSING INDIVIDUAL NODES
 *
 * If you have a model where the transforms of individual 
 * nodes have to be updated, you have to obtain these from 
 * the RTGFile data structure. Obtain the number of
 * top-level nodes int the hierarchy by saying
 *
 *   unsigned int ntop = file->nnodes;
 *
 * The first top-level node is then
 *
 *   RTGNode *node = file->nodes[0];
 *
 * We can now traverse the node hierarchy, e.g.,
 *
 *   unsigned int nchildren = node->nchildren;
 *   RTGNode *child = node->children[0];
 *
 * and change its transformation information, e.g.,
 *
 *   child->trn.x = 20;
 *   child->trn.y = 10;
 *   child->trn.z = 5;
 *
 *
 * ORDER OF TRANSFORMATIONS OF NODES
 *
 * Each node may contain transformation information. Since
 * scaling and rotation is not commutative operations, the
 * order in which the transformations are applied becomes
 * important. Here is how to apply a node transformation in
 * OpenGL:
 *
 *   RTGNode *node = ...;
 *
 *   glTranslatef(node->trn.x, node->trn.y, node->trn.z);
 *   glRotatef(RADIANS_TO_DEGREES * node->rot.x, 1, 0, 0);
 *   glRotatef(RADIANS_TO_DEGREES * node->rot.y, 0, 1, 0);
 *   glRotatef(RADIANS_TO_DEGREES * node->rot.z, 0, 0, 1);
 *   glScalef(node->scl.x, node->scl.y, node->scl.z);
 *
 * where RADIANS_TO_DEGREES = (180 / PI). Note that each
 * node is transformed with respect to its parent.
 *
 *
 * ACCESSING MATERIAL PARAMETERS
 *
 * If you want to change the material properties of a
 * certain node, you can do this through its mesh object,
 * e.g.
 *
 *   RTGObject *obj = node->obj;
 *   RTGMaterial *mat = obj->mat;
 *   mat->dif[0] = 0.1f;
 *   mat->dif[1] = 0.2f;
 *   mat->dif[2] = 0.3f;
 *
 *
 * EXPORTING FROM MAYA
 *
 * The preferred way of exporting a Maya model to
 * RTG is to use the "Full" or "World" hierarchy
 * export options. The exporter may produce unexpected
 * results if files are exported with the "Flat" 
 * hierarchy option. In this case, the hierarchy 
 * nodes are moved to the top level, but their 
 * transformation information is not forwarded into 
 * the vertex positions (as they do if you choose 
 * the "World" hierarchy option).
 *
 * As would be expected, there is a non-trivial 
 * mapping between Maya shaders and RTG materials. If
 * at all possible, use the Phong material in Maya.
 * Maya makes a decent attempt at exporting Blinn
 * materials, but the mapping is rather unintuitive.
 * Other kinds of materials (such as Lambert) typically
 * produce unexpected results.
 *
 * For materials with a color texture, Maya outputs a 
 * zero diffuse reflectance. Without modification,
 * these materials would be quite useless for
 * rendering with the GL_MODULATE texture environment in
 * OpenGL. Therefore, if a texture is found, the RTG
 * file importer sets the diffuse material reflectance
 * to 1.0 (and thus mimics Maya's way of defining the
 * texture as containing the diffuse reflectance of
 * the material).
 *
 */

#ifndef RTGLIB_H
#define RTGLIB_H


/*
 * GLOBAL DEFINITIONS
 *
 */


/*
 * Maximum number of allowed lines in an RTG file.
 * Increase and recompile if necessary.
 *
 */
#define	RTG_MAX_LINES				20000

/*
 * Maximum number of children for one RTG hierarchy node.
 * Increase and recompile if necessary.
 */
#define RTG_MAX_CHILDREN			16

/*
 * Maximum number of materials for one RTG file.
 * Increase and recompile if necessary.
 */
#define RTG_MAX_MATERIALS			128

/*
 * Maximum number of top level nodes for one RTG file.
 * Increase and recompile if necessary.
 */
#define RTG_MAX_TOP_LEVEL_NODES		256

/*
 * Maximum number of vertices in one polygon.
 * Increase and recompile if necessary.
 */
#define RTG_MAX_POLY_VERT			256


/*
 * DATA STRUCTURES
 *
 */

/**
 * A three-dimensional vector.
 *
 */
struct RTGVec3 {
	float x;
	float y;
	float z;
};
typedef struct RTGVec3 RTGVec3;

/**
 * A two-dimensional vector.
 *
 */
struct RTGVec2 {
	float x;
	float y;
};
typedef struct RTGVec2 RTGVec2;


/**
 * A polygon vertex with indices to
 * its position, normal and tex coord
 * data.
 *
 */
struct RTGVertex {
	unsigned int p;		/* Position index */
	unsigned int n;		/* Normal index */
	unsigned int t;		/* Texture coordinate index */
};
typedef struct RTGVertex RTGVertex;


/**
 * A polygon. Contains a list of vertices.
 *
 */
struct RTGPolygon {
	RTGVertex *v;		/* Vertices */
	unsigned int nvert;	/* Number of vertices */
};
typedef struct RTGPolygon RTGPolygon;


/**
 * A RTG file material.
 *
 */
struct RTGMaterial {
	char *name;			/* Name of the material */
	float amb[3];		/* Ambient reflectance */
	float dif[3];		/* Diffuse reflectance */
	float spc[3];		/* Specular reflectance */
	float shn;			/* Shininess exponent (a la Phong) */
	float em[3];		/* Emission */
	float trp;			/* Transparency */

	char *texName;		/* Name of texture */
	char *texFile;		/* Name of file in which texture resides */
};
typedef struct RTGMaterial RTGMaterial;


/**
 * A mesh object.
 *
 */
struct RTGObject {
	char *name;			/* Name of the object */
	
	unsigned int mat;	/* Index of the material used by this object */

	RTGVec3 *pos;		/* List of vertex positions */
	unsigned int npos;	/* Number of positions */

	RTGVec3	*norm;		/* List of vertex normals */
	unsigned int nnorm;	/* Number of normals */

	RTGVec2 *tex;		/* Texture coordniates */
	unsigned int ntex;	/* Number of texture coordinates */

	RTGPolygon **poly;	/* Polygons */
	unsigned int npoly;	/* Number of polygons */
};
typedef struct RTGObject RTGObject;


/**
 * A hierarchy node.
 *
 */
struct RTGNode;
struct RTGNode {
	struct RTGNode *children[RTG_MAX_CHILDREN];	/* List of children of this node */
	unsigned int nchildren;

	RTGObject *obj;		/* Object associated with this node */
	char *objName;		/* Name of object associated with this node */

	RTGVec3 trn;		/* Translation */
	RTGVec3 rot;		/* Rotation */
	RTGVec3 scl;		/* Scaling */
};
typedef struct RTGNode RTGNode;


/**
 * A representation of a RTG file.
 *
 */
struct RTGFile {
	RTGNode *nodes[RTG_MAX_TOP_LEVEL_NODES];	/* Top level nodes */
	unsigned int nnodes;						/* Number of top level nodes */
	RTGMaterial *mat[RTG_MAX_MATERIALS];		/* List of materials */
	unsigned int nmat;							/* Number of materials */
};
typedef struct RTGFile RTGFile;


/**
 * Parse a RTG file, and allocate memory for and fill in
 * a RTGFile struct. When you are ready with the RTGFile
 * struct, you should free its memory by calling
 * RTG_Free().
 *
 * @see RTG_Free()
 *
 * @param verbose If 1, then print parsing info as file
 * is being parse. If 0, then the parsing is silent.
 * @return Pointer to a memory location where the RTGFile
 * struct has been created if successful, NULL otherwise.
 *
 */
RTGFile *RTG_Parse(const char *filename, int verbose);

/**
 * Release the memory held by the specified RTGFile
 * struct. If the pointer is NULL, this function does
 * nothing.
 *
 * @see RTG_Parse()
 *
 */
void RTG_Free(RTGFile *rtgfile);


#endif /* RTGLIB_H */
