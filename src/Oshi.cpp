#define SDL_MAIN_USE_CALLBACKS
#define DEBUG //comment if not debug

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "opengl/Renderer.h"
#include "opengl/IndexBuffer.h"
#include "opengl/VertexBuffer.h"

struct ShaderSource {
    std::string vertex;
    std::string fragment;
};

struct GameState {
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;

    unsigned int VAO;
    unsigned int shaderProgram;
    VertexBuffer *vb;
    IndexBuffer *ib;

    bool isDebug = true;
};

int rectangle(GameState* gameState);
int rectangleSetup(GameState* gameState);


#ifdef DEBUG
void APIENTRY OpenGLDebugCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam)
{
    SDL_Log("** GL ERROR **\nsource : %u\ntype = 0x%X\nid = %u\nseverity = 0x%u\nmessage = %s", 
        source, type, id, severity, message
    );
}

#endif

static ShaderSource parseShader(const std::string& filePath) 
{
    std::ifstream *stream = new std::ifstream(filePath);
    if(stream->fail() || !stream->is_open() || stream->bad())
        SDL_Log("failed to parse Shader");

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while(std::getline(*stream, line)) 
    {
        if (line.find("#shader") != std::string::npos)
        {
            if(line.find("vertex") != std::string::npos) 
                type = ShaderType::VERTEX;
            else if(line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }
    delete stream;
    return { ss[0].str(), ss[1].str()};
}

static int compileShader(unsigned int type, const std::string& source) {
    //vertex shader
    unsigned int id = glCreateShader(type); //creating an empty vertex shader
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr); //attach source to the vertexShader object

    //compilation vertex shader
    glCompileShader(id);

    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success); //check if compilation is successful
    if(!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        SDL_Log("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s", infoLog);
        return GL_FAILURE_NV;
    }

    return id;
}

static int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
    /*keywords in and out are for the inputs and outputs of the shaders
    layout (location = 0) is for the location used for the glVertexAttribPointer function*/
    
    //shader program object
    unsigned int shaderProgram = glCreateProgram(); //return the id of the new program

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(shaderProgram, vs); //link shaders to the program
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram); //link the shaders together
    glValidateProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        SDL_Log("ERROR::SHADER::PROGRAM::LINK_FAILED\n %s", infoLog);
        return GL_FAILURE_NV;
    }

    glDeleteShader(vs); //So we can now delete the shaders
    glDeleteShader(fs);

    return shaderProgram;
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_Log("SDL_AppInit function started");

    GameState* gameState = new GameState();
    
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    SDL_Log("SDL_Init started");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_Log("Erreur d'initialisation de SDL : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL_CreateWindow started"); //trouver a quoi correspondent les flags similaires
    gameState->window = SDL_CreateWindow("OpenGL avec SDL3", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL  /*|  SDL_WINDOW_FULLSCREEN */);
    if (!gameState->window) {
        SDL_Log("Erreur de création de la fenêtre : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL_GL_CreateContext started");
    gameState->glContext = SDL_GL_CreateContext(gameState->window);
    if (!(gameState->glContext)) {
        SDL_Log("Erreur de création du contexte OpenGL : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("glewInit started");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Erreur d'initialisation de GLEW !");
        return SDL_APP_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);
    
#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    SDL_Log("openGL version %s", glGetString(GL_VERSION));
#endif

    rectangleSetup(gameState);

    *appstate = gameState;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    GameState* game = static_cast<GameState*>(appstate);

    //Uint64 start = SDL_GetPerformanceCounter();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // OpenGL
    rectangle(game);


    SDL_GL_SwapWindow(game->window);
    //Uint64 end = SDL_GetPerformanceCounter();
	//float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    // Cap to 60 FPS
    //SDL_Delay(100);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    GameState* game = static_cast<GameState*>(appstate);

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    GameState* game = static_cast<GameState*>(appstate);

    if (game->glContext) {
        delete game->ib;
        delete game->vb; 
        glDeleteVertexArrays(1, &(game->VAO));


        SDL_GL_DestroyContext(game->glContext);
        game->glContext = nullptr;
    }

    if (game->window) {
        SDL_DestroyWindow(game->window);
        game->window = nullptr;
    }

    delete game;

    SDL_Quit();
}


int rectangleSetup(GameState* gameState) {

    ShaderSource shaderSource = parseShader("resources/shaders/basic.shader");
    gameState->shaderProgram = createShader(shaderSource.vertex, shaderSource.fragment);

    //init Data
    float vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f,  0.5f,
        -0.5f, 0.5f
    };

    unsigned int indexes[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &(gameState->VAO));  //VAO Vertex array object
    glBindVertexArray(gameState->VAO); //start configuring VAO
    
    gameState->vb = new VertexBuffer(vertices, sizeof(vertices));

    //specify how OpenGL shader should interpret the positions attributes of the vertices
    glVertexAttribPointer(0/*id in shader*/, 2/*nb of values*/, GL_FLOAT/*type*/, GL_FALSE/*normalized*/, 2 * sizeof(float)/*size of the vertex*/, (const void*)0 /*offset*/);
    glEnableVertexAttribArray(0/*id in shader*/); //activate the line above
    
    gameState->ib = new IndexBuffer(indexes, 6);

    glBindVertexArray(0); //stop configuring VAO

    return GL_SUCCESS_NV;
}

int rectangle(GameState* gameState) {
    glUseProgram(gameState->shaderProgram);//every shader and rendering call after glUseProgram will now use this program object

    float timeValue = SDL_GetTicks() / 100;
    float blueValue = (std::sin(timeValue) / 2.0f) + 0.5f;
    float redValue = (std::cos(timeValue) / 2.0f) + 0.5f;

    int vertexColorationLocation = glGetUniformLocation(gameState->shaderProgram, "ourColor");
    glUniform4f(vertexColorationLocation, redValue, 0.0f, blueValue, 1.0f);

    glBindVertexArray(gameState->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    return GL_SUCCESS_NV;
}