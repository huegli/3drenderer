#include "display.h"
#include "vector.h"

////////////////////////////////////////////////////////////////////
// Declare an array of vectors/points
////////////////////////////////////////////////////////////////////

#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];

bool is_running = false;

void setup(void)
{
    // allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL Texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width, window_height);

    int point_count = 0;

    // Initialize cube points
    for (float x = -1.0; x <= 1.0; x += 0.25) {
        for (float y = -1.0; y <= 1.0; y += 0.25) {
            for (float z = -1.0; z <= 1.0; z += 0.25) {
                vec3_t new_point = { .x = x, .y = y, .z = z };
                cube_points[point_count++] = new_point;
            }
        }
    }
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
        break;
    }
}

void update(void)
{
    // TODO
}

void render(void)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid();

    draw_rect(100, 100, 300, 200, 0xFFFF0000);
    draw_pixel(20, 20, 0xFFFFFF00); 

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{

    is_running = initialize_window();

    vec3_t v = { 0.0f, 0.0f, 0.0f };

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
