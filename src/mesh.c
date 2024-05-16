#include <stdio.h>
#include <string.h>
#include "array.h"
#include "mesh.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { 0, 0, 0 },
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
    {.a = 1, .b = 2, .c = 3},
    {.a = 1, .b = 3, .c = 4},
    // right
    {.a = 4, .b = 3, .c = 5},
    {.a = 4, .b = 5, .c = 6},
    // back
    {.a = 6, .b = 5, .c = 7},
    {.a = 6, .b = 7, .c = 8},
    // left
    {.a = 8, .b = 7, .c = 2},
    {.a = 8, .b = 2, .c = 1},
    // top
    {.a = 2, .b = 7, .c = 5},
    {.a = 2, .b = 5, .c = 3},
    // bottom
    {.a = 6, .b = 8, .c = 1},
    {.a = 6, .b = 1, .c = 4},
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
                float x, y, z;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &x) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &y) != 1) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%f", &z) != 1) return;
                
                vec3_t v = {
                    x = x,
                    y = y,
                    z = z
                };
                
                array_push(mesh.vertices, v);
                
                // printf("(%f, %f ,%f)\n", x, y, z);
            }
            
            if (strcmp(token, "f") == 0) {
                int a, b, c;
                int dummy1, dummy2;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%d/%d", &a, &dummy1, &dummy2) != 3) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%d/%d", &b, &dummy1, &dummy2) != 3) return;
                
                token = strtok(NULL, " ");
                if (token == NULL) return;
                if (sscanf(token, "%d/%d/%d", &c, &dummy1, &dummy2) != 3) return;
                
                face_t f = {
                    a = a,
                    b = b,
                    c = c
                };
                
                array_push(mesh.faces, f);
                // printf("(%d, %d ,%d)\n", f1, f2, f3);
            }
        }
    }

    // Close the file
    fclose(file);
}
