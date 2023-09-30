/**
* Author: Raymond Lin
* Assignment: Simple 2D Scene
* Date due: 2023-09-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.7411f,
            BG_BLUE = 0.2235f,
            BG_GREEN = 0.2666f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const float MILLISECONDS_IN_SECOND = 1000.0;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char JETT_SPRITE[] = "assets/jett.png";
const char KNIFE_SPRITE[] = "assets/knife.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

SDL_Window* g_display_window;

bool g_game_is_running = true;
bool g_is_growing = true;
float g_previous_ticks = 0.0f;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
          g_knife1_matrix,
          g_knife2_matrix,
          g_knife3_matrix,
          g_jett_matrix,
          g_projection_matrix;

GLuint g_jett_texture_id,
       g_knife_texture_id;

// Jett movement
glm::vec3 g_jett_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_jett_movement = glm::vec3(0.0f, 0.0f, 0.0f);


// Transformation variables

float g_jett_speed = 5.0f;

const float RADIUS = 2.0f;
const float ROT_SPEED = 1.0f;
const float PI = 3.14159f;

    // Knife 1
float g_angle1 = 0.0f,
      g_x_coord1 = RADIUS, 
      g_y_coord1 = 0.0f;

    // Knife 2
float g_angle2 = 0.0f,
      g_x_coord2 = RADIUS,
      g_y_coord2 = 0.0f;

    // Knife 3
float g_angle3 = 0.0f,
      g_x_coord3 = RADIUS,
      g_y_coord3 = 0.0f;


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Simple 2D Scene",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_knife1_matrix = glm::mat4(1.0f);
    g_knife2_matrix = glm::mat4(1.0f);
    g_knife3_matrix = glm::mat4(1.0f);
    g_jett_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);


    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_knife_texture_id = load_texture(KNIFE_SPRITE);
    g_jett_texture_id = load_texture(JETT_SPRITE);


    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_jett_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                g_jett_movement.x = 1.0f;
                break;

            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_jett_movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_jett_movement.x = 1.0f;
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        g_jett_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_jett_movement.y = -1.0f;
    }

    // This makes sure that the player can't "cheat" their way into moving faster
    if (glm::length(g_jett_movement) > 1.0f)
    {
        g_jett_movement = glm::normalize(g_jett_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;


    // Knife1 orbit
    g_angle1 += ROT_SPEED * delta_time * 1.0f;

    g_x_coord1 = RADIUS * glm::cos(g_angle1);
    g_y_coord1 = RADIUS * glm::sin(g_angle1);

    g_knife1_matrix = glm::mat4(1.0f);
    g_knife1_matrix = glm::translate(g_jett_matrix, glm::vec3(g_x_coord1, g_y_coord1, 0.0f));

    // Knife2 orbit
    g_angle2 += ROT_SPEED * delta_time * 1.0f;

    g_x_coord2 = RADIUS * glm::cos(g_angle2 + (2 * PI / 3));
    g_y_coord2 = RADIUS * glm::sin(g_angle2 + (2 * PI / 3));

    g_knife2_matrix = glm::mat4(1.0f);
    g_knife2_matrix = glm::translate(g_jett_matrix, glm::vec3(g_x_coord2, g_y_coord2, 0.0f));

    // Knife3 orbit
    g_angle3 += ROT_SPEED * delta_time * 1.0f;

    g_x_coord3 = RADIUS * glm::cos(g_angle3 + (4 * PI / 3));
    g_y_coord3 = RADIUS * glm::sin(g_angle3 + (4 * PI / 3));

    g_knife3_matrix = glm::mat4(1.0f);
    g_knife3_matrix = glm::translate(g_jett_matrix, glm::vec3(g_x_coord3, g_y_coord3, 0.0f));

    // Jett movement
    g_jett_position += g_jett_movement * g_jett_speed * delta_time;

    g_jett_matrix = glm::mat4(1.0f);
    g_jett_matrix = glm::translate(g_jett_matrix, g_jett_position);
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    g_shader_program.set_model_matrix(g_knife1_matrix);

    // Vertices
    float knife_vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   
    };

    float jett_vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    // Textures
    float knife_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    float jett_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, knife_vertices);
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, jett_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, knife_texture_coordinates);
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, jett_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_knife1_matrix, g_knife_texture_id);
    draw_object(g_knife2_matrix, g_knife_texture_id);
    draw_object(g_knife3_matrix, g_knife_texture_id);
    draw_object(g_jett_matrix, g_jett_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
