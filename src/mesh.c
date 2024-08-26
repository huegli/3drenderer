#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "array.h"
#include "mesh.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { 0, 0, 0 },
    .scale = { 1.0, 1.0, 1.0 },
    .translation = { 0, 0, 0},
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1},
    { .x = -1, .y =  1, .z = -1},
    { .x =  1, .y =  1, .z = -1},
    { .x =  1, .y = -1, .z = -1},
    { .x =  1, .y =  1, .z =  1},
    { .x =  1, .y = -1, .z =  1},
    { .x = -1, .y =  1, .z =  1},
    { .x = -1, .y = -1, .z =  1},
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    {.a = 1, .b = 2, .c = 3, .color = 0xFFFFFFFF},
    {.a = 1, .b = 3, .c = 4, .color = 0xFFFFFFFF},
    // right
    {.a = 4, .b = 3, .c = 5, .color = 0xFFFFFFFF},
    {.a = 4, .b = 5, .c = 6, .color = 0xFFFFFFFF},
    // back
    {.a = 6, .b = 5, .c = 7, .color = 0xFFFFFFFF},
    {.a = 6, .b = 7, .c = 8, .color = 0xFFFFFFFF},
    // left
    {.a = 8, .b = 7, .c = 2, .color = 0xFFFFFFFF},
    {.a = 8, .b = 2, .c = 1, .color = 0xFFFFFFFF},
    // top
    {.a = 2, .b = 7, .c = 5, .color = 0xFFFFFFFF},
    {.a = 2, .b = 5, .c = 3, .color = 0xFFFFFFFF},
    // bottom
    {.a = 6, .b = 8, .c = 1, .color = 0xFFFFFFFF},
    {.a = 6, .b = 1, .c = 4, .color = 0xFFFFFFFF},
};

void load_cube_mesh_data(void)
{
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

void load_obj_file_data(char *filename) {
    FILE *file;
    char line[100];

    // Open the file
    file = fopen(filename, "r");

    // Check if file opened successfully
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // Read each line until end of file is reached
    while (fgets(line, sizeof(line), file) != NULL) {

        char *token = strtok(line, " ");
        if (token != NULL) {
            
            if (strcmp(token, "v") == 0) {
                vec3_t v;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &v.x) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &v.y) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &v.z) != 1) return;
                
                array_push(mesh.vertices, v);
            }
            
            if (strcmp(token, "f") == 0) {
                face_t f = {};

                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%*d/%*d", &f.a) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%*d/%*d", &f.b) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%*d/%*d", &f.c) != 1) return;

		f.color = 0xFFFFFFFF;
		
                array_push(mesh.faces, f);
            }
        }
    }

    // Close the file
    fclose(file);
}
