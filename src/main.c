#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "light.h"
#include "matrix.h"

vec3_t camera_position = { 0, 0, 0 };

mat4_t proj_matrix;

bool is_running = false;

int previous_frame_time = 0;

triangle_t* triangles_to_render = NULL;

bool show_wireframe = false;
bool show_vertices = false;
bool fill_triangles = true;
bool do_backface_cul = true;

// Sorting triangle helper function
int comp(const void *elem1, const void *elem2) {
    triangle_t t1 = *((triangle_t*)elem1);
    triangle_t t2 = *((triangle_t*)elem2);
    
    if (t1.avg_depth < t2.avg_depth)
        return 1;
    else
        return -1;
}

void setup(void)
{
    // allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL Texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width, window_height);

    // Initialize the perspective projection matrix
    float fov = M_PI / 3.0; // the same as 180/3 or 60 degrees
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);
    
    load_obj_file_data("./assets/f22.obj");
    // load_cube_mesh_data();
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

    // scale/rotate the mesh per animation frame
    mesh.rotation.x += (float)0.01;
    mesh.rotation.z += (float)0.01;
    mesh.rotation.y += (float)0.01;
    // Translate the points away from the camera
    mesh.translation.z = 5.0;
    
    // create a scale matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z );
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    // loop through all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3] = {0};
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3] = {0};

        // loop all three vertices of this current face and apply the transformation
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Create a World Matrix combining scale, rotation and translation matrices
            mat4_t world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiply the world matrix by the original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
            
            // Save the transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Back-face culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* B---C */

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);

        // Compute the normal of the triangle
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Normalize the face normal vector
        vec3_normalize(&normal);

        // Find the vector between point A and the camera
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

	// Calculate how aligned the camper ray is with the face normal
	float dot_normal_camera = vec3_dot(normal, camera_ray);

	if (do_backface_cul) {
	    // bypass the triangles that face away from the camera
	    if (dot_normal_camera < 0) {
		continue;
	    }
	}
	
        vec4_t projected_points[3] = {0};
        
        // Loop all three vertices to perform the projection
        for (int j = 0; j < 3; j++) {
            // Project the current point
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // Scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // Invert the y values to accoutn for flipped screen y coordinate
            projected_points[j].y *= -1;
            
            // translate the projected point to the center of the screen
            projected_points[j].x += ((float)window_width / 2.0);
            projected_points[j].y += ((float)window_height / 2.0);

        }
        
        // Calculate the average depth for each face based on the vectices after tranformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

	// calculate the triangle color based on the light angle
	float light_intensity_factor = -vec3_dot(normal, light.direction);
	
	// Calculate the shade intensity
	uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
	
        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y },
            },
            .color = triangle_color,
            .avg_depth = avg_depth
        };

        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);

    }
    
    // Sort the triangles to render by their avg depth
    qsort(triangles_to_render, array_length(triangles_to_render), sizeof(triangle_t), comp);
}

void render(void)
{
    draw_grid();

    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        if (show_vertices) {
            draw_rect((int)triangle.points[0].x, (int)triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect((int)triangle.points[1].x, (int)triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect((int)triangle.points[2].x, (int)triangle.points[2].y, 3, 3, 0xFFFF0000);
        }

        if (fill_triangles) draw_filled_triangle(
            (int)triangle.points[0].x, (int)triangle.points[0].y,
            (int)triangle.points[1].x, (int)triangle.points[1].y,
            (int)triangle.points[2].x, (int)triangle.points[2].y, triangle.color);
        if (show_wireframe) draw_triangle(
            (int)triangle.points[0].x, (int)triangle.points[0].y,
            (int)triangle.points[1].x, (int)triangle.points[1].y,
            (int)triangle.points[2].x, (int)triangle.points[2].y, 0xFF00FF00);
    

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
