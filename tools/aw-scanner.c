/******************************************************************************
 *                AtomWeb: An embedded web server                             *
 *                                                                            *
 * Copyright (c) 2015 Gong Ke                                                 *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 * 1.Redistributions of source code must retain the above copyright notice,   *
 * this list of conditions and the following disclaimer.                      *
 * 2.Redistributions in binary form must reproduce the above copyright        *
 * notice, this list of conditions and the following disclaimer in the        *
 * documentation and/or other materials provided with the distribution.       *
 * 3.Neither the name of the Gong Ke; nor the names of its contributors may   *
 * be used to endorse or promote products derived from this software without  *
 * specific prior written permission.                                         *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR           *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *****************************************************************************/

#include "aw-tools.h"

typedef struct Node_s Node;
struct Node_s {
	int   is_dir;
	Node *child;
	Node *next;
	char *name;
	char *cname;
	char *dir;
};

static char *dir_name = NULL;
static char *dep_name = NULL;
static char *map_name = NULL;
static Node  root_node;

static const char* usage_str =
"usage: aw-scanner [OPTIONS] DIR\n"
"aw-scanner: Atom Web Scanner\n"
"options:\n"
"\t-m FILE     generate makefile dependencies file\n"
"\t-c FILE     generate atom web map source file\n"
"\t-h          show this message\n"
;

static void
usage (void)
{
	fprintf(stderr, "%s", usage_str);
}

static void
parse_args (int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "m:c:h")) != -1) {
		switch (opt) {
			case 'm':
				dep_name = optarg;
				break;
			case 'c':
				map_name = optarg;
				break;
			case 'h':
				usage();
				break;
			default:
				usage();
				exit(1);
				break;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "no input directory defined\n");
		exit(1);
	}

	dir_name = argv[optind];
}

static Node*
alloc_node (Node *parent, int dir, const char *dname, const char *name)
{
	char *ptr, *nptr;
	Node *n;

	n = (Node*)malloc(sizeof(Node));
	AW_ASSERT(n);

	n->is_dir = dir;
	n->name   = strdup(name);
	n->dir    = strdup(dname);
	n->child  = NULL;
	n->next   = parent->child;
	parent->child = n;

	if (dir) {
		n->cname = NULL;
	} else {
		char cbuf[PATH_MAX];

		strcpy(cbuf, name);

		ptr = cbuf;
		while (1) {
			nptr = strchr(ptr, '.');
			if (nptr) {
				*nptr = '_';
			} else {
				break;
			}
		}

		n->cname = strdup(cbuf);
	}

	return n;
}

static void
clear_node (Node *parent)
{
	Node *c, *n;

	for (c = parent->child; c; c = n) {
		n = c->next;

		clear_node(c);

		free(c->name);
		free(c->dir);

		if (c->cname)
			free(c->cname);

		free(c);
	}
}

static Node*
add_file (Node *parent, const char *dir, const char *name)
{
	AW_INFO(("add file \"%s\"", name));
	return alloc_node(parent, 0, dir, name);
}

static Node*
add_dir (Node *parent, const char *dir, const char *name)
{
	AW_INFO(("add directory \"%s\"", name));
	return alloc_node(parent, 1, dir, name);
}

static void
scan_dir (const char *dname, Node *parent)
{
	DIR *dir;
	struct dirent *ent;
	char path[PATH_MAX];
	struct stat sbuf;
	int r;

	dir = opendir(dname);
	if (!dir) {
		fprintf(stderr, "cannot open directory \"%s\"\n", dname);
		return;
	}

	while (1) {
		ent = readdir(dir);
		if (ent) {
			if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
				continue;

			snprintf(path, sizeof(path), "%s/%s", dname, ent->d_name);

			if ((r = stat(path, &sbuf)) == -1)
				continue;

			if (S_ISREG(sbuf.st_mode)) {
				add_file(parent, dname, ent->d_name);
			} else if (S_ISDIR(sbuf.st_mode)) {
				Node *child;

				child = add_dir(parent, dname, ent->d_name);
				scan_dir(path, child);
			}
		} else {
			break;
		}
	}

	closedir(dir);
}

typedef struct {
	int   *links;
	char **clazz;
	char **name;
	int   *npos;
	int   *child;
	char  *chr;
	int   *next;
	int   *node;
	int    link_size;
	int    link_elem;
	int    node_size;
	int    node_elem;
} Map;

typedef struct {
	Map  *maps;
	int   size;
	int   elem;
	char *classes;
} MapData;

static int
add_map (MapData *md)
{
	Map *map;

	if (md->elem == md->size) {
		int nsize = md->size * 2;

		if (nsize < 16)
			nsize = 16;

		md->maps = (Map*)realloc(md->maps, nsize * sizeof(Map));
		md->size = nsize;
	}

	map = &md->maps[md->elem];
	memset(map, 0, sizeof(Map));

	return md->elem++;
}

static int
add_node (MapData *md, int map_id)
{
	Map *map = &md->maps[map_id];
	int r;

	if (map->node_size == map->node_elem) {
		int nsize = map->node_size * 2;

		if (nsize < 16)
			nsize = 16;

		map->links = (int*)realloc(map->links, nsize * sizeof(int));
		map->clazz = (char**)realloc(map->clazz, nsize * sizeof(char*));
		map->child = (int*)realloc(map->child, nsize * sizeof(int));
		map->name  = (char**)realloc(map->name, nsize * sizeof(char*));
		map->npos  = (int*)realloc(map->npos, nsize * sizeof(int));

		map->node_size = nsize;
	}

	r = map->node_elem++;

	map->links[r] = -1;
	map->clazz[r] = NULL;
	map->child[r] = -1;
	map->name[r]  = NULL;
	map->npos[r]  = 0;

	return r;
}

static int
add_link (MapData *md, int map_id, int from, int to, int chr)
{
	Map *map = &md->maps[map_id];
	int r;

	if (map->link_size == map->link_elem) {
		int nsize = map->link_size * 2;

		if (nsize < 16)
			nsize = 16;

		map->chr  = (char*)realloc(map->chr, nsize * sizeof(char));
		map->next = (int*)realloc(map->next, nsize * sizeof(int));
		map->node = (int*)realloc(map->node, nsize * sizeof(int));

		map->link_size = nsize;
	}

	r = map->link_elem++;

	map->node[r] = to;
	map->chr[r]  = chr;
	map->next[r] = map->links[from];
	map->links[from] = r;

	return r;
}

static int
gen_node_map (Node *node, MapData *md)
{
	Node *c;
	Map *map;
	int map_id, node_id;

	map_id  = add_map(md);
	map     = &md->maps[map_id];
	node_id = add_node(md, map_id);

	for (c = node->child; c; c = c->next) {
		char *ptr, *end, chr;
		int nn_id;

		ptr = c->name;
		end = ptr + strlen(ptr);

		while (ptr < end) {
			int link_id;

			link_id = map->links[node_id];
			if (link_id == -1) {
				if (map->name[node_id] && map->npos[node_id]) {
					char *name = map->name[node_id];
					int npos = map->npos[node_id];

					chr = name[strlen(name) - npos];
					nn_id = add_node(md, map_id);
					add_link(md, map_id, node_id, nn_id, chr);
					map->name[nn_id] = name;
					map->npos[nn_id] = npos - 1;

					map->name[node_id] = NULL;
					map->npos[node_id] = 0;

					link_id = map->links[node_id];
				}
			}

			if (link_id == -1) {
				if (map->name[node_id]) {
					nn_id = add_node(md, map_id);
					add_link(md, map_id, node_id, nn_id, *ptr++);
					node_id = nn_id;
				}

				break;
			}

			nn_id   = -1;
			while (link_id != -1) {
				if (map->chr[link_id] == *ptr) {
					nn_id = map->node[link_id];
					break;
				}
				link_id = map->next[link_id];
			}

			if (nn_id == -1) {
				nn_id = add_node(md, map_id);
				add_link(md, map_id, node_id, nn_id, *ptr);
			}

			node_id = nn_id;
			ptr ++;
		}

		if (c->is_dir) {
			map->child[node_id] = gen_node_map(c, md);
		} else {
			map->clazz[node_id] = c->cname;

			aw_str_printf(&md->classes, "extern const AW_Class aw_%s_class;\n",
						c->cname);
		}

		map->name[node_id] = c->name;
		map->npos[node_id] = end - ptr;
	}

	return map_id;
}

static void
gen_map (void)
{
	MapData md;
	Map *map;
	FILE *fp;
	int i, j;

	if (!map_name)
		return;

	fp = fopen(map_name, "w+");
	if (!fp) {
		fprintf(stderr, "cannot open \"%s\"\n", map_name);
		return;
	}

	memset(&md, 0, sizeof(md));

	gen_node_map(&root_node, &md);

	fprintf(fp, "/*Generated by aw-scanner*/\n\n");
	fprintf(fp, "#include <atomweb.h>\n\n");

	if (md.classes)
		fprintf(fp, "%s\n", md.classes);

	for (i = 0; i < md.elem; i++) {
		map = &md.maps[i];

		fprintf(fp, "static const AW_MapNode aw_map_nodes_%d = {\n", i);
			for (j = 0; j < map->node_elem; j++) {
				fprintf(fp, "\t{%d, %d, ", map->links[j],
							map->child[j]);
				if (map->clazz[j]) {
					fprintf(fp, "&aw_%s_class", map->clazz[j]);
				} else {
					fprintf(fp, "NULL");
				}
				fprintf(fp, "}");
				if (j != map->node_elem -1)
					fprintf(fp, ",");
				fprintf(fp, "\n");
			}
		fprintf(fp, "};\n");
		fprintf(fp, "static const AW_MapLink aw_map_links_%d = {\n", i);
			for (j = 0; j < map->link_elem; j++) {
				fprintf(fp, "\t{%d, %d, %d}", map->chr[j],
							map->next[j],
							map->node[j]);
				if (j != map->link_elem -1)
					fprintf(fp, ",");
				fprintf(fp, "\n");
			}
		fprintf(fp, "};\n");
	}

	fprintf(fp, "const AW_Map aw_map[] = {\n");
	for (i = 0; i < md.elem; i++) {
		fprintf(fp, "\t{aw_map_nodes_%d, aw_map_links_%d}", i, i);
		if (i != md.elem - 1)
			fprintf(fp, ",");
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n\n");

	for (i = 0; i < md.elem; i++) {
		map = &md.maps[i];

		if (map->links)
			free(map->links);
		if (map->clazz)
			free(map->clazz);
		if (map->child)
			free(map->child);
		if (map->chr)
			free(map->chr);
		if (map->next)
			free(map->next);
		if (map->node)
			free(map->node);
		if (map->name)
			free(map->name);
		if (map->npos)
			free(map->npos);
	}

	if (md.maps)
		free(md.maps);
	if (md.classes)
		free(md.classes);
	if (md.maps)
		free(md.maps);

	fclose(fp);

	AW_INFO(("generate map file"));
}

typedef struct {
	char *rules;
	char *srcs;
} DepData;

static void
gen_node_dep (Node *node, DepData *dd)
{
	Node *c;

	for (c = node->child; c; c = c->next) {
		gen_node_dep(c, dd);

		if (!c->is_dir) {
			aw_str_printf(&dd->rules, "%s/%s.c: %s/%s\n",
						c->dir, c->cname, c->dir, c->name);
			aw_str_printf(&dd->rules, "\t$(AW_CONVERTER) -o $@ $<\n\n");
			aw_str_printf(&dd->srcs, "%s/%s.c ",
						c->dir, c->cname);
		}
	}
}

static void
gen_dep (void)
{
	DepData dd;
	FILE *fp;

	if (!dep_name)
		return;

	fp = fopen(dep_name, "w+");
	if (!fp) {
		fprintf(stderr, "cannot open \"%s\"\n", dep_name);
		return;
	}

	fprintf(fp, "#Generated by aw-scanner\n\n");
	fprintf(fp, "ifeq ($(AW_CONVERTER),)\n");
	fprintf(fp, "AW_CONVERTER:=aw-converter\n");
	fprintf(fp, "endif\n\n");

	memset(&dd, 0, sizeof(dd));
	gen_node_dep(&root_node, &dd);

	fprintf(fp, "AW_SRCS:=%s\n\n", dd.srcs ? dd.srcs : "");
	fprintf(fp, "%s", dd.rules ? dd.rules : "");

	if (dd.rules)
		free(dd.rules);
	if (dd.srcs)
		free(dd.srcs);

	fclose(fp);

	AW_INFO(("generate dep file"));
}

int
main (int argc, char **argv)
{
	parse_args(argc, argv);

	memset(&root_node, 0, sizeof(root_node));
	root_node.is_dir = 1;

	scan_dir(dir_name, &root_node);

	gen_map();
	gen_dep();

	clear_node(&root_node);

	return 0;
}

