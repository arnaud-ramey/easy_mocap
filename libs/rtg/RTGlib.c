/**
 * RTGlib.c - A C library for parsing .RTG geometry
 * files.
 *
 * AUTHOR: Gustav Tax�n, gustavt@nada.kth.se
 *
 * This source code is in the public domain. 
 * No warranties - use at your own risk!
 *
 * This library is (probably) not thread-safe.
 *
 */

#ifndef WIN32
#include <strings.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "RTGlib.h"

#define RTG_FILE_PARSER_FLAG_TYPES		12

const char *RTGFileParserFlagIDStr[RTG_FILE_PARSER_FLAG_TYPES] = {
		"OUTPUT_VERT_NORMS", "OUTPUT_VERT_COLORS", "OUTPUT_TEX_COORDS",
		"OUTPUT_POLY_NORMS", "OUTPUT_HIERARCHY", "OUTPUT_LOCAL",
		"SHOW_INDEX_COUNTERS", "OUTPUT_MATERIALS", "OUTPUT_ANIMATION",
		"OUTPUT_ALL_NODES", "OUTPUT_DECOMP", "OUTPUT_DEGREES" };

enum RTGFileParserFlagType {
	OUTPUT_VERT_NORMS,
	OUTPUT_VERT_COLORS,
	OUTPUT_TEX_COORDS,
	OUTPUT_POLY_NORMS,
	OUTPUT_HIERARCHY,
	OUTPUT_LOCAL,
	SHOW_INDEX_COUNTERS,
	OUTPUT_MATERIALS,
	OUTPUT_ANIMATION,
	OUTPUT_ALL_NODES,
	OUTPUT_DECOMP,
	OUTPUT_DEGREES
};

struct RTGParseData {
	int verbose;
	char *lines[RTG_MAX_LINES];
	unsigned int nlines;
	int flags[RTG_FILE_PARSER_FLAG_TYPES];
	RTGFile *dest;
};
struct RTGParseData Parser;

int RTG_BeginsWith(const char *line, const char *str) {
	const char *ptr = strstr(line, str);
	if (!ptr) {
		return 0;
	} else {
		return (ptr == line); /* First character */
	}
}

int RTG_BeginsWithIgnoreWhitespace(const char *line, const char *str) {
	const char *pos;
	const char *i;

	pos = strstr(line, str);
	if (!pos) {
		return 0;
	}

	/* Check if the characters up to the
	 search string is whitespace characters */

	for (i = line; i < pos; i++) {
		if (*i != ' ' && *i != '\t') {
			return 0;
		}
	}

	return 1;
}

int RTG_FindNextLineExclEnd(int start, char *lines[], unsigned int nlines,
		const char *str, const char *end) {
	int p = start;
	int s = nlines;

	for (p = start; p < s; p++) {
		if (RTG_BeginsWith(lines[p], str)) {
			return p;
		}
		if (RTG_BeginsWith(lines[p], end)) {
			return -1;
		}
	}

	return -1;
}

int RTG_FindNextLine(int start, char *lines[], unsigned int nlines,
		const char *str) {
	int p = start;
	int s = nlines;

	for (p = start; p < s; p++) {
		if (RTG_BeginsWith(lines[p], str)) {
			return p;
		}
	}

	return -1;
}

/* Stop looking at line 'end' */
int RTG_FindNextLineIgnoreWhitespace(int start, char *lines[],
		unsigned int nlines, const char *str, int end) {
	int p = start;
	const char *i;
	const char *pos;

	for (p = start; p <= end; p++) {
		pos = strstr(lines[p], str);
		if (pos != NULL) {
			for (i = lines[p]; i < pos; i++) {
				if (*i != ' ' && *i != '\t') {
					break;
				}
				return p;
			}
		}
	}

	return -1;
}

int RTG_Seek(const char *line, const char *str) {
	const char *p = strstr(line, str);
	const char *i;
	unsigned int steps = 0;

	if (p == NULL) {
		return -1;
	}

	i = line;
	while (i != p) {
		++i;
		++steps;
	}

	return steps + strlen(str);
}

const char *RTG_EndOfString(const char *str, unsigned int start) {
	return str + start;
}

/* Return true if there only exists whitespace and newline
 characters between start and end (see RTG_TokenizeLines) */
int RTG_EmptyLine(const char *start, const char *end) {
	const char *i = start;
	if (start == end) {
		return 1;
	}
	do {
		if (*i != '\n' && *i != ' ' && *i != '\t' && *i != '\015') {
			return 0;
		}
		++i;
	} while (i != end);
	return 1;
}

int RTG_TokenizeLines(char *buf, unsigned int buflen) {
	const char *ptrs, *ptre;

	Parser.nlines = 0;
	ptrs = buf;
	ptre = buf;

	do {
		if (*ptre == '\n' || *ptre == '\015') {
			if (!RTG_EmptyLine(ptrs, ptre)) {
				Parser.lines[Parser.nlines] = (char *) malloc(sizeof(char)
						* (ptre - ptrs) + 1);
				if (!Parser.lines[Parser.nlines]) {
					fprintf(stderr, "Out of memory");
					free(buf);
					return -1;
				}
				memcpy(Parser.lines[Parser.nlines], ptrs, (ptre - ptrs));
				Parser.lines[Parser.nlines][(ptre - ptrs)] = '\0';
				++(Parser.nlines);
			}
			++ptre;
			ptrs = ptre;
		} else {
			++ptre;
		}
	} while (ptrs < (buf + buflen) && ptre < (buf + buflen));

	return Parser.nlines;
}

void RTG_FreeLines(void) {
	unsigned int i;
	for (i = 0; i < Parser.nlines; i++) {
		free(Parser.lines[i]);
	}
}

int RTG_ParseFlag(const char *line) {
	if (strstr(line, "on") != NULL) {
		return 1;
	}
	if (strstr(line, "off") != NULL) {
		return 0;
	}
	fprintf(stderr, "Parse error: Flag '%s' contains no on/off selector\n",
			line);
	return 0;
}

int RTG_ParseFlags(void) {
	int end, i, j;

	if (Parser.verbose) {
		printf("----- FLAGS -----\n");
	}

	/* We'll only parse lines up until 'MATERIAL_LIST' or 'HIERARCHY_LIST' */

	end = RTG_FindNextLine(0, Parser.lines, Parser.nlines, "MATERIAL_LIST");
	if (end == -1) {
		end
				= RTG_FindNextLine(0, Parser.lines, Parser.nlines,
						"HIERARCHY_LIST");
		if (end == -1) {
			fprintf(stderr,
					"Parse error: Identifier 'MATERIAL_LIST' not found\n");
			return 0;
		}
	}

	/* For each line, see if it corresponds to one of the flag identifiers
	 and if so, extract the boolean value */

	for (i = 0; i < end; i++) {
		for (j = 0; j < RTG_FILE_PARSER_FLAG_TYPES; j++) {
			if (RTG_BeginsWith(Parser.lines[i], RTGFileParserFlagIDStr[j])) {
				Parser.flags[j] = RTG_ParseFlag(Parser.lines[i]);
				if (Parser.verbose) {
					printf("%s: %s\n", RTGFileParserFlagIDStr[j],
							(Parser.flags[j]) ? "on" : "off");
				}
			}
		}
	}

	if (!Parser.flags[OUTPUT_VERT_NORMS]) {
		fprintf(stderr,
				"The file does not contain vertex normals, which is required by this parser.\n");
		return 0;
	}
	if (Parser.flags[OUTPUT_VERT_COLORS]) {
		fprintf(stderr,
				"The file contains vertex colors, which is unsupported at this time.\n");
		return 0;
	}
	if (!Parser.flags[OUTPUT_TEX_COORDS]) {
		fprintf(
				stderr,
				"The file does not contain texture coordinates, which is required by this parser.\n");
		return 0;
	}
	if (Parser.flags[OUTPUT_POLY_NORMS]) {
		fprintf(stderr,
				"The file contains polygon normals, which is unsupported at this time.\n");
		return 0;
	}
	if (Parser.flags[SHOW_INDEX_COUNTERS]) {
		fprintf(
				stderr,
				"The file contains polygon index counters, which is unsupported at this time.\n");
		return 0;
	}
	if (Parser.flags[OUTPUT_ANIMATION]) {
		fprintf(stderr,
				"The file contains animation info, which is unsupported at this time.\n");
		return 0;
	}

	return 1;
}

RTGVec3 RTG_ParseVec3(const char *str) {
	char dummy[256];
	RTGVec3 v;
	sscanf(str, "%s %f %f %f", dummy, &v.x, &v.y, &v.z);
	return v;
}

float RTG_ParseFloat(const char *str) {
	float v;
	char dummy[256];
	sscanf(str, "%s %f", dummy, &v);
	return v;
}

int RTG_ParseInteger(const char *str) {
	int v;
	char dummy[256];
	sscanf(str, "%s %d", dummy, &v);
	return v;
}

char *RTG_ParseString(const char *str) {
	char buf[256];
	char dummy[256];
	sscanf(str, "%s %s", dummy, buf);
	return (char *) strdup(buf);
}

char *RTG_Trim(const char *str) {
	int i = (int) strlen(str) - 1;
	char *ret;

	if (i < 0) {
		return NULL;
	}

	while (str[i] == ' ' || str[i] == '\t') {
		--i;
		if (i < 0) {
			return NULL;
		}
	}

	ret = (char *) malloc(sizeof(char) * (i + 2));
	memcpy(ret, str, i + 1);
	ret[i + 1] = '\0';

	return ret;
}

char *RTG_ParseStringEOL(const char *str) {
	unsigned int i = 0;

	/* First, ignore any initial whitespace characters */

	while (str[i] == ' ' || str[i] == '\t') {
		++i;
		if (i >= strlen(str)) {
			return NULL;
		}
	}

	/* Then, move ahead until we find new whitespace
	 characters */

	while (str[i] != ' ' && str[i] != '\t') {
		++i;
		if (i >= strlen(str)) {
			return NULL;
		}
	}

	/* Now, move ahead until we find new non-whitespace
	 characters */

	while (str[i] == ' ' || str[i] == '\t') {
		++i;
		if (i >= strlen(str)) {
			return NULL;
		}
	}

	/* Return the substring beginning at i and continuing
	 through to the end of the line */

	return RTG_Trim(RTG_EndOfString(str, i));
}

RTGMaterial *RTG_NewMaterial(char *matName, RTGVec3 *amb, RTGVec3 *dif,
		RTGVec3 *spc, RTGVec3 *em, float shn, float trp, char *texName,
		char *texFile) {
	RTGMaterial *mat;

	mat = (RTGMaterial *) malloc(sizeof(RTGMaterial));
	if (!mat) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	mat->name = matName;
	mat->amb[0] = amb->x;
	mat->amb[1] = amb->y;
	mat->amb[2] = amb->z;
	mat->dif[0] = dif->x;
	mat->dif[1] = dif->y;
	mat->dif[2] = dif->z;
	mat->spc[0] = spc->x;
	mat->spc[1] = spc->y;
	mat->spc[2] = spc->z;
	mat->em[0] = em->x;
	mat->em[1] = em->y;
	mat->em[2] = em->z;
	mat->shn = shn;
	mat->trp = trp;
	mat->texName = texName;
	mat->texFile = texFile;

	return mat;
}

int RTG_ParseMaterials(void) {
	int begin, end, nmat, i, mline;
	char tmp[64];
	RTGVec3 amb, dif, spc, em;
	float shn, trp;
	char *texName = NULL, *texFile = NULL, *matName = NULL;

	if (Parser.verbose) {
		printf("----- MATERIALS -----\n");
	}

	begin = RTG_FindNextLine(0, Parser.lines, Parser.nlines, "MATERIAL_LIST");
	end = RTG_FindNextLine(0, Parser.lines, Parser.nlines, "END_MATERIAL_LIST");

	if (begin == -1 || end == -1) {
		fprintf(stderr,
				"Identifiers 'MATERIAL_LIST' and/or 'END_MATERIAL_LIST' not found\n");
		return 0;
	}

	/* Extract the number of materials to expect */
	nmat = atoi(RTG_EndOfString(Parser.lines[begin], RTG_Seek(
			Parser.lines[begin], "MATERIAL_LIST ")));
	if (Parser.verbose) {
		printf("Number of materials: %d\n", nmat);
	}
	if (nmat < 0 || nmat > RTG_MAX_MATERIALS) {
		fprintf(stderr, "Too many materials. Maximum allowed is %d\n",
				RTG_MAX_MATERIALS);
		return 0;
	}
	Parser.dest->nmat = nmat;

	for (i = 0; i < nmat; i++) {
		if (Parser.verbose) {
			printf("Material %d:\n", i);
		}

		sprintf(tmp, "MATERIAL %d", i);
		mline = RTG_FindNextLineIgnoreWhitespace(begin, Parser.lines,
				Parser.nlines, tmp, end);
		if (mline == -1) {
			fprintf(stderr, "Parse error: Identifier '%s' not found.\n", tmp);
			return 0;
		}
		++mline;

		while (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "MATERIAL")
				&& !RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline],
						"END_MATERIAL_LIST")) {

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "NAME")) {
				matName = RTG_ParseString(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Name: %s\n", matName);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "AMBIENT")) {
				amb = RTG_ParseVec3(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Ambient: %f %f %f\n", amb.x, amb.y, amb.z);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "DIFFUSE")) {
				dif = RTG_ParseVec3(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Diffuse: %f %f %f\n", dif.x, dif.y, dif.z);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "SPECULAR")) {
				spc = RTG_ParseVec3(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Specular: %f %f %f\n", spc.x, spc.y, spc.z);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "EMMISION")) {
				em = RTG_ParseVec3(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Emission: %f %f %f\n", em.x, em.y, em.z);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline], "SHININESS")) {
				shn = RTG_ParseFloat(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Shininess: %f\n", shn);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline],
					"TRANSPARENCY")) {
				trp = RTG_ParseFloat(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Transparency: %f\n", trp);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline],
					"TEXTURE_NAME")) {
				texName = RTG_ParseString(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Texture Name: %s\n", texName);
				}
			}

			if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[mline],
					"TEXTURE_FILENAME")) {
				texFile = RTG_ParseStringEOL(Parser.lines[mline]);
				if (Parser.verbose) {
					printf("  Texture Filename: %s\n", texFile);
				}
				/* We've found a valid texture. Maya exports textured
				 surfaces with a diffuse color of 0.0, whereas we
				 want the texture to *modulate* the diffuse color.
				 Therefore, we set the diffuse color to 1.0 if we
				 are using a texture. */
				dif.x = 1.0f;
				dif.y = 1.0f;
				dif.z = 1.0f;
			}

			++mline;

		} /* End while */

		Parser.dest->mat[i] = RTG_NewMaterial(matName, &amb, &dif, &spc, &em,
				shn, trp, texName, texFile);
		texName = NULL;
		texFile = NULL;

	} /* End for */

	return 1;
}

void RTG_ClearNode(RTGNode *node) {
	node->nchildren = 0;
	node->obj = NULL;
	node->objName = NULL;
	node->rot.x = 0;
	node->rot.y = 0;
	node->rot.z = 0;
	node->trn.x = 0;
	node->trn.y = 0;
	node->trn.z = 0;
	node->scl.x = 0;
	node->scl.y = 0;
	node->scl.z = 0;
}

void RTG_AddChild(RTGNode *parent, RTGNode *child) {
	if (parent->nchildren == RTG_MAX_CHILDREN) {
		fprintf(stderr, "Node cannot have more than %d children: ignored.\n",
				RTG_MAX_CHILDREN);
		return;
	}
	parent->children[parent->nchildren] = child;
	parent->nchildren++;
}

int RTG_ChildrenAvailableToRead(int start, int level) {
	int lvl = 0;
	if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[start],
			"END_HIERARCHY_LIST")) {
		return 0;
	}
	sscanf(Parser.lines[start], "%d", &lvl);
	if (lvl > level) {
		return 1;
	}
	return 0;
}

int RTG_ParseTransformsAndPivots(RTGNode *dest, int start, int transforms,
		int pivots, int level) {
	int p = start;
	int i;
	RTGVec3 v;

	if (transforms) {
		v = RTG_ParseVec3(Parser.lines[start]);
		if (Parser.flags[OUTPUT_HIERARCHY]) {
			if (Parser.verbose) {
				for (i = 0; i < level; i++) {
					printf("  ");
				}
				printf("Translation: %f %f %f\n", v.x, v.y, v.z);
			}
			dest->trn.x = v.x;
			dest->trn.y = v.y;
			dest->trn.z = v.z;
		} else {
			dest->trn.x = 0;
			dest->trn.y = 0;
			dest->trn.z = 0;
		}
		++p;

		v = RTG_ParseVec3(Parser.lines[start + 1]);
		if (Parser.flags[OUTPUT_HIERARCHY]) {
			if (Parser.verbose) {
				for (i = 0; i < level; i++) {
					printf("  ");
				}
				printf("Rotation:    %f %f %f\n", v.x, v.y, v.z);
			}
			dest->rot.x = v.x;
			dest->rot.y = v.y;
			dest->rot.z = v.z;
		} else {
			dest->rot.x = 0;
			dest->rot.y = 0;
			dest->rot.z = 0;
		}
		++p;

		v = RTG_ParseVec3(Parser.lines[start + 2]);
		if (Parser.flags[OUTPUT_HIERARCHY]) {
			if (Parser.verbose) {
				for (i = 0; i < level; i++) {
					printf("  ");
				}
				printf("Scaling:     %f %f %f\n", v.x, v.y, v.z);
			}
			dest->scl.x = v.x;
			dest->scl.y = v.y;
			dest->scl.z = v.z;
		} else {
			dest->scl.x = 1;
			dest->scl.y = 1;
			dest->scl.z = 1;
		}
		++p;

	} else {
		dest->trn.x = 0;
		dest->trn.y = 0;
		dest->trn.z = 0;
		dest->rot.x = 0;
		dest->rot.y = 0;
		dest->rot.z = 0;
		dest->scl.x = 1;
		dest->scl.y = 1;
		dest->scl.z = 1;
	}
	if (pivots) {
		p += 2;
	}
	return p;
}

int RTG_ParseNode(RTGNode *dest, int start, int transforms, int pivots) {
	int level;
	char type[16];
	char name[256];
	int pos, i;
	RTGNode *node;

	/* Read data for this node */

	sscanf(Parser.lines[start], "%d %s %s", &level, type, name);
	if (Parser.verbose) {
		for (i = 0; i < level; i++) {
			printf("  ");
		}
		printf("NODE: %s\n", name);
	}

	dest->objName = (char *) strdup(name);

	pos = RTG_ParseTransformsAndPivots(dest, start + 1, transforms, pivots,
			level);

	while (RTG_ChildrenAvailableToRead(pos, level)) {
		node = (RTGNode *) malloc(sizeof(RTGNode));
		RTG_ClearNode(node);
		RTG_AddChild(dest, node);
		pos = RTG_ParseNode(node, pos, transforms, pivots);
	}

	return pos;
}

int RTG_ParseHierarchy(void) {
	int begin, end;
	int ntop;
	int xforms;
	int pivots;
	int l, i;
	RTGNode *node;

	if (Parser.verbose) {
		printf("----- HIERARCHY -----\n");
	}

	begin = RTG_FindNextLine(0, Parser.lines, Parser.nlines, "HIERARCHY_LIST");
	end
			= RTG_FindNextLine(0, Parser.lines, Parser.nlines,
					"END_HIERARCHY_LIST");

	if (begin == -1 || end == -1) {
		fprintf(
				stderr,
				"Parse error: Identifiers 'HIERARCHY_LIST' and/or 'END_HIERARCHY_LIST' not found\n");
		return 0;
	}

	/* Identify the hierarchy type */

	/* Maya doesn't output the number of top-level objects correctly,
	 so we have to count all level-0 objects instead. */

	ntop = 0;
	for (i = begin + 1; i < end; i++) {
		if (Parser.lines[i][0] == '0') {
			++ntop;
		}
	}

	if (ntop > RTG_MAX_TOP_LEVEL_NODES) {
		fprintf(stderr, "Too many top-level nodes. Maximum allowed is %d\n",
				RTG_MAX_TOP_LEVEL_NODES);
	}
	Parser.dest->nnodes = ntop;

	/* Maya doesn't use the H, HX or HXP identifiers as specified
	 in the RTG documentation, so we have to figure out if we
	 have transforms and/or pivots */

	xforms = 0;
	pivots = 0;
	for (i = begin + 1; i < end; i++) {
		if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[i], "tran")) {
			xforms = 1;
		}
		if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[i], "sPiv")) {
			pivots = 1;
		}
	}

	if (Parser.verbose) {
		printf("Number of top level objects: %d\n", ntop);
		printf("Transforms: %s\n", (xforms) ? "on" : "off");
		printf("Pivots: %s\n\n", (pivots) ? "on" : "off");
	}

	/* Parse the top-level objects */

	l = begin + 1;

	for (i = 0; i < ntop; i++) {
		if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[l],
				"END_HIERARCHY_LIST") || l >= end) {
			fprintf(stderr, "Parse error: Malformed hierarchy description\n");
			return 0;
		}

		node = (RTGNode *) malloc(sizeof(RTGNode));
		if (!node) {
			fprintf(stderr, "Out of memory\n");
			return 0;
		}
		RTG_ClearNode(node);
		l = RTG_ParseNode(node, l, xforms, pivots);
		Parser.dest->nodes[i] = node;
	}

	return 1;
}

RTGMaterial *RTG_GetMaterial(RTGFile *file, unsigned int index) {
	if (index >= file->nmat) {
		return NULL;
	}
	return file->mat[index];
}

int RTG_CountVertices(const char *line) {
	unsigned int i = 0;
	unsigned int l = (unsigned int) strlen(line);
	int nvert = 0;

	/* Skip over any initial whitespace characters */
	while ((line[i] == ' ' || line[i] == '\t') && i < l) {
		++i;
	}

	/* If we don't have a 'v' here, the line is malformed */
	if (line[i] != 'v') {
		fprintf(stderr,
				"Parse error: Malformed polygon description, line does not begin with 'v'\n");
		return -1;
	}

	/* Skip ahead to the first numeral */
	++i;
	++i;

	/* Step forward until we reach the 'n' and
	 count the spaces */
	while (line[i] != 'n') {
		if (i >= l) {
			fprintf(stderr,
					"Parse error: Malformed polygon description, line does not contain a 'n'\n");
			return -1;
		}
		if (line[i] == ' ') {
			++nvert;
		}
		++i;
	}
	return nvert;
}

int RTG_TokenizePoly(const char *line, char tokens[RTG_MAX_POLY_VERT * 3][32],
		unsigned int *ntok) {
	size_t l = strlen(line);
	unsigned int t = 0;
	unsigned int i, p, pt;

	/* Skip over any initial whitespace characters */
	i = 0;
	while ((line[i] == ' ' || line[i] == '\t') && i < l) {
		++i;
	}

	/* Split into tokens */
	p = i;
	pt = 0;
	*ntok = 0;
	while (i < l) {
		if (line[i] == ' ' || line[i] == '\t') {
			/* We've found the space between tokens */
			tokens[t][pt] = '\0';
			++t;
			pt = 0;
			*ntok += 1;
			while ((line[i + 1] == ' ' || line[i + 1] == '\t') && i < l) {
				++i;
			}
		} else {
			tokens[t][pt] = line[i];
			++pt;
		}
		++i;
	}

	return 1;
}

int RTG_ParseObject(RTGObject *obj, int start) {
	/* Parse name, number of vertices, normals, tex coords and polys
	 Assume that we have vertex normals and texture coordinates, if
	 not, the logic that deals with flags should report an error. */

	char dummy[64], nameStr[256], vertStr[16], normStr[16], tcStr[16],
			polyStr[16], matStr[256];
	int matIndex;
	unsigned int i, nvert, nnorm, ntc, npoly;
	int p = start;
	float x, y, z;
	char tokens[RTG_MAX_POLY_VERT * 3][32];
	unsigned int ntok;
	RTGPolygon *poly = NULL;
	unsigned int j;

	sscanf(Parser.lines[p], "%s %s %s %s %s %s", dummy, nameStr, vertStr,
			normStr, tcStr, polyStr);
	nvert = atoi(&vertStr[1]);
	nnorm = atoi(&normStr[1]);
	ntc = atoi(&tcStr[1]);
	npoly = atoi(&polyStr[1]);
	if (Parser.verbose) {
		printf("   Name:           %s\n", nameStr);
		printf("   Positions:      %d\n", nvert);
		printf("   Normals:        %d\n", nnorm);
		printf("   Texture coords: %d\n", ntc);
		printf("   Polygons:       %d\n", npoly);
	}
	++p;

	obj->name = (char *) strdup(nameStr);

	/* Allocate memory for vertex positions, normals and
	 texture coordinates */

	obj->pos = (RTGVec3 *) malloc(sizeof(RTGVec3) * nvert);
	if (!obj->pos) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	obj->npos = nvert;

	if (nnorm > 0) {
		obj->norm = (RTGVec3 *) malloc(sizeof(RTGVec3) * nnorm);
		if (!obj->norm) {
			fprintf(stderr, "Out of memory\n");
			return -1;
		}
		obj->nnorm = nnorm;
	} else {
		obj->norm = NULL;
		obj->nnorm = 0;
	}

	if (ntc > 0) {
		obj->tex = (RTGVec2 *) malloc(sizeof(RTGVec2) * ntc);
		if (!obj->norm) {
			fprintf(stderr, "Out of memory\n");
			return -1;
		}
		obj->ntex = ntc;
	} else {
		obj->tex = NULL;
		obj->ntex = 0;
	}

	obj->poly = (RTGPolygon **) malloc(sizeof(RTGPolygon *) * npoly);
	if (!obj->poly) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	obj->npoly = npoly;

	/* Read material identifier */

	if (Parser.flags[OUTPUT_MATERIALS]) {
		if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "USES_MATERIAL")) {
			fprintf(stderr,
					"Parse error: Identifier 'USES_MATERIAL' not found.\n");
			return -1;
		}
		sscanf(Parser.lines[p], "%s %d %s", dummy, &matIndex, matStr);
		if (Parser.verbose) {
			printf("   Material:   %s (index %d)\n", matStr, matIndex);
		}
		++p;

		obj->mat = matIndex;
	}

	/* Read vertices */

	if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "VERTEX")) {
		fprintf(stderr, "Parse error: Identifier 'VERTEX' not found\n");
		return -1;
	}
	++p;
	for (i = 0; i < nvert; i++) {
		sscanf(Parser.lines[p], "%f %f %f", &x, &y, &z);
		obj->pos[i].x = x;
		obj->pos[i].y = y;
		obj->pos[i].z = z;
		++p;
	}

	/* Read normals */

	if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "NORMAL")) {
		fprintf(stderr, "Parse error: Identifier 'NORMAL' not found\n");
		return -1;
	}
	++p;
	for (i = 0; i < nnorm; i++) {
		sscanf(Parser.lines[p], "%f %f %f", &x, &y, &z);
		obj->norm[i].x = x;
		obj->norm[i].y = y;
		obj->norm[i].z = z;
		++p;
	}

	/* Read tex coords */

	if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "TEXCOORD")) {
		fprintf(stderr, "Parse error: Identifier 'TEXCOORD' not found\n");
		return -1;
	}
	++p;
	for (i = 0; i < ntc; i++) {
		sscanf(Parser.lines[p], "%f %f", &x, &y);
		obj->tex[i].x = x;
		obj->tex[i].y = y;
		++p;
	}

	/* Read polygon indices */

	if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "POLYGON")) {
		fprintf(stderr, "Parse error: Identifier 'POLYGON' not found\n");
		return -1;
	}
	++p;
	for (i = 0; i < npoly; i++) {
		/* Figure out how many vertices we have */

		nvert = RTG_CountVertices(Parser.lines[p]);
		if (nvert > RTG_MAX_POLY_VERT) {
			fprintf(stderr,
					"Polygon has %d vertices, maximum number allowed is %d\n",
					nvert, RTG_MAX_POLY_VERT);
			return -1;
		}

		/* Split line into tokens */

		RTG_TokenizePoly(Parser.lines[p], tokens, &ntok);

		/* Read the vertices, normals and tex coords */

		poly = (RTGPolygon *) malloc(sizeof(RTGPolygon));
		if (!poly) {
			fprintf(stderr, "Out of memory\n");
			return -1;
		}
		poly->v = (RTGVertex *) malloc(sizeof(RTGVertex) * nvert);
		if (!poly->v) {
			fprintf(stderr, "Out of memory\n");
			return -1;
		}
		poly->nvert = nvert;

		obj->poly[i] = poly;

		for (j = 0; j < nvert; j++) {
			poly->v[j].p = atoi(tokens[j + 1]);
			poly->v[j].n = atoi(tokens[(nvert + 1) + j + 1]);
			if (ntc > 0) {
				poly->v[j].t = atoi(tokens[(2 * nvert + 2) + j + 1]);
			} else {
				poly->v[j].t = 0;
			}
		}

		++p;
	}

	if (Parser.verbose) {
		printf("Number of polygons: %d\n", npoly);
	}

	return p + 1;
}

RTGNode *RTG_FindNode(RTGNode *node, const char *name) {
	RTGNode *res;
	unsigned int i;

	if (strcmp(node->objName, name) == 0) {
		return node;
	}

	for (i = 0; i < node->nchildren; i++) {
		res = RTG_FindNode(node->children[i], name);
		if (res != NULL) {
			return res;
		}
	}

	return NULL;
}

RTGNode *RTG_FindNodeInFile(RTGFile *file, const char *name) {
	unsigned int i;
	RTGNode *res;

	for (i = 0; i < file->nnodes; i++) {
		res = RTG_FindNode(file->nodes[i], name);
		if (res != NULL) {
			return res;
		}
	}

	return NULL;
}

int RTG_ParseObjects(unsigned int nobj) {
	unsigned int i;
	int p;
	RTGObject *obj = NULL;
	RTGNode *node;

	if (Parser.verbose) {
		printf("----- OBJECTS -----\n");
		printf("Number of objects: %d\n", nobj);
	}

	/* Read the objects */

	p = RTG_FindNextLine(0, Parser.lines, Parser.nlines - 1, "OBJECT_START");

	for (i = 0; i < nobj; i++) {
		if (!RTG_BeginsWithIgnoreWhitespace(Parser.lines[p], "OBJECT_START")
				|| p >= (int) Parser.nlines) {
			fprintf(stderr,
					"Parse error: Identifier 'OBJECT_START' not found\n");
			return 0;
		}
		if (Parser.verbose) {
			printf("Object %d\n", i);
		}
		obj = (RTGObject *) malloc(sizeof(RTGObject));
		if (!obj) {
			fprintf(stderr, "Out of memory\n");
			return 0;
		}
		p = RTG_ParseObject(obj, p);
		if (p == -1) {
			free(obj);
			return 0;
		}

		node = RTG_FindNodeInFile(Parser.dest, obj->name);
		if (!node) {
			fprintf(
					stderr,
					"Parse error: There is no node with name '%s' in the hierarchy\n",
					obj->name);
			free(obj);
			return 0;
		}
		node->obj = obj;
	}

	return 1;
}

RTGFile *RTG_Parse(const char *filename, int verbose) {
	char *buf;
	FILE *in;
	long nbytes;
	unsigned int buflen;
	unsigned int nobj, i;

	Parser.verbose = verbose;

	/* Open the file and split it into lines */

	if (Parser.verbose) {
		printf("--- PARSING '%s' ---\n", filename);
	}
	in = fopen(filename, "r");
	if (!in) {
		printf("Couldn't open file '%s'\n", filename);
		return NULL;
	}

	fseek(in, 0, SEEK_END);
	nbytes = ftell(in);
	fseek(in, 0, SEEK_SET);

	if (Parser.verbose) {
		printf("File length: %d bytes\n", nbytes);
	}

	buf = (char *) malloc(sizeof(char) * nbytes);
	if (!buf) {
		fclose(in);
		fprintf(stderr, "Out of memory");
		return NULL;
	}
	memset(buf, 0, sizeof(char) * nbytes);
	buflen = (unsigned int) fread(buf, sizeof(char), nbytes, in);
	fclose(in);
	if (RTG_TokenizeLines(buf, buflen) == -1) {
		return NULL;
	}
	free(buf);

	/* Parse the file.

	 Since some versions of Maya doesn't export the
	 correct number of objects when hierarchies are
	 disabled, we need to count the number of occurrences
	 of the string "OBJECT_START" in the file. */

	nobj = 0;
	for (i = 0; i < Parser.nlines; i++) {
		if (RTG_BeginsWithIgnoreWhitespace(Parser.lines[i], "OBJECT_START")) {
			++nobj;
		}
	}

	if (nobj == 0) {
		fprintf(stderr, "No objects to parse");
		RTG_FreeLines();
		return NULL;
	}

	if (Parser.verbose) {
		printf("Number of objects: %d\n", nobj);
	}

	if (!RTG_ParseFlags()) {
		RTG_FreeLines();
		return NULL;
	}

	Parser.dest = (RTGFile *) malloc(sizeof(RTGFile));
	if (!Parser.dest) {
		fprintf(stderr, "Out of memory");
		RTG_FreeLines();
		return NULL;
	}
	Parser.dest->nmat = 0;

	if (Parser.flags[OUTPUT_MATERIALS]) {
		if (!RTG_ParseMaterials()) {
			RTG_FreeLines();
			RTG_Free(Parser.dest);
			return NULL;
		}
	}

	if (!RTG_ParseHierarchy()) {
		RTG_FreeLines();
		RTG_Free(Parser.dest);
		return NULL;
	}

	if (!RTG_ParseObjects(nobj)) {
		RTG_FreeLines();
		RTG_Free(Parser.dest);
		return NULL;
	}

	if (verbose) {
		printf("--- END PARSE ---\n");
	}

	RTG_FreeLines();
	return Parser.dest;
}

void RTG_FreeObj(RTGObject *obj) {
	unsigned int i;

	if (obj->name) {
		free(obj->name);
	}
	if (obj->pos) {
		free(obj->pos);
	}
	if (obj->norm) {
		free(obj->norm);
	}
	if (obj->tex) {
		free(obj->tex);
	}

	if (obj->poly) {
		for (i = 0; i < obj->npoly; i++) {
			if (obj->poly[i]) {
				if (obj->poly[i]->v) {
					free(obj->poly[i]->v);
				}
			}
		}
		free(obj->poly);
	}

	free(obj);
}

void RTG_FreeNode(RTGNode *node) {
	unsigned int i;

	if (node->obj) {
		RTG_FreeObj(node->obj);
	}
	if (node->objName) {
		free(node->objName);
	}

	for (i = 0; i < node->nchildren; i++) {
		if (node->children[i]) {
			RTG_FreeNode(node->children[i]);
		}
	}

	free(node);
}

void RTG_Free(RTGFile *file) {
	unsigned int i;

	if (!file) {
		return;
	}

	if (file->mat) {
		for (i = 0; i < file->nmat; i++) {
			if (file->mat[i]) {
				free(file->mat[i]->name);
				free(file->mat[i]->texName);
				free(file->mat[i]->texFile);
			}
		}
	}

	if (file->nodes) {
		for (i = 0; i < file->nnodes; i++) {
			if (file->nodes[i]) {
				RTG_FreeNode(file->nodes[i]);
			}
		}
	}

	free(file);
}
