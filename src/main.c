#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

vec3_t camera_position = { 0, 0, 0 };

float fov_factor = 640;

bool is_running = false;

int previous_frame_time = 0;

triangle_t* triangles_to_render = NULL;

bool show_wireframe = true;
bool show_vertices = true;
bool fill_triangles = false;
bool do_backface_cul = false;

void setup(void)
{
    // allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL Texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width, window_height);

    load_obj_file_data("./assets/f22.obj");
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
            if (event.key.keysym.sym == SDLK_1) {
                show_wireframe = true;
                show_vertices = true;
                fill_triangles = false;
            }
            if (event.key.keysym.sym == SDLK_2) {
                show_wireframe = true;
                show_vertices = false;
                fill_triangles = false;
            }
            if (event.key.keysym.sym == SDLK_3) {
                show_wireframe = false;
                show_vertices = false;
                fill_triangles = true;
            }
            if (event.key.keysym.sym == SDLK_4) {
                show_wireframe = true;
                show_vertices = false;
                fill_triangles = true;
            }
        if (event.key.keysym.sym == SDLK_c)
            do_backface_cul = true;
        if (event.key.keysym.sym == SDLK_d)
            do_backface_cul = false;
        break;
    }
}

////////////////////////////////////////////////////////////////////////
// Function that receives a 3D vector and returns a projected 2D point
////////////////////////////////////////////////////////////////////////
vec2_t project(vec3_t point)
{
    vec2_t projected_point = {
        .x = (point.x * fov_factor) / point.z,
        .y = (point.y * fov_factor) / point.z
    };

    return projected_point;
}

void update(void)
{

    // wait some time until we reach the target frame time in milliseconds
    int time_to_wait = FRAME_TARGET_TIME  - (SDL_GetTicks() - previous_frame_time);

    // only delay execution if we are running too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = SDL_GetTicks();

    triangles_to_render = NULL;
    
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    // loop through all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3] = {0, 0, 0};
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3] = {0, 0, 0};

        // loop all three vertices of this current face and apply the transformation
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            // Rotate the point around the X axis
            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);

            // Rotate the point around the Y axis
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);

            // Rotate the point around the Z axis
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the points away from the camera
            transformed_vertex.z += 5;

            // Save the transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Back-face culling
        vec3_t vector_a = transformed_vertices[0]; /*   A   */
        vec3_t vector_b = transformed_vertices[1]; /*  / \  */
        vec3_t vector_c = transformed_vertices[2]; /* B---C */

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);

        // Compute the normal of the triangle
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Normalize the face normal vector
        vec3_normalize(&normal);

        // Find the vector between point A and the camera
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // bypass the triangles that face away from the camera
        if (vec3_dot(normal, camera_ray) < 0) {
            // only bypass if backface culling enabled
            if (do_backface_cul) continue;
        }
       
        triangle_t projected_triangle = {0, 0, 0};

        // Loop all three vertices to perform the projection
        for (int j = 0; j < 3; j++) {
            // Project the current point
            vec2_t projected_point = project(transformed_vertices[j]);

            // scale and translate the projected point to the center of the screen
            projected_point.x += (window_width / 2.0);
            projected_point.y += (window_height / 2.0);

            projected_triangle.points[j] = projected_point;
        }

        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);

    }
}

void render(void)
{
    draw_grid();

    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        if (show_vertices) {
            draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFF0000);
        }
        
        if (fill_triangles) draw_filled_triangle(
            triangle.points[0].x, triangle.points[0].y,
            triangle.points[1].x, triangle.points[1].y,
            triangle.points[2].x, triangle.points[2].y, 0xFF0000FF);
        if (show_wireframe) draw_triangle(
            triangle.points[0].x, triangle.points[0].y,
            triangle.points[1].x, triangle.points[1].y,
            triangle.points[2].x, triangle.points[2].y, 0xFF00FF00);
    
    }
    
    array_free(triangles_to_render);

    render_color_buffer();
    
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void free_resources(void)
{
    free(color_buffer);
    array_free(mesh.vertices);
    array_free(mesh.faces);
}

int main(int argc, char* argv[])
{

    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
