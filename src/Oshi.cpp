#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <GLEW/glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderSource {
    std::string vertex;
    std::string fragment;
};

static ShaderSource parseShader(const std::string& filePath) 
{
    std::ifstream stream(filePath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while(getline(stream, line)) 
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

    return { ss[((int)ShaderType::VERTEX)].str(), ss[((int)ShaderType::FRAGMENT)].str()};
}


SDL_Window *window = nullptr;
SDL_GLContext glContext = nullptr;

//Screen Constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;

int rectangle();
int rectangleSetup();
int shaderSetup();

unsigned int VAO;
unsigned int shaderProgram;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_Log("Erreur d'initialisation de SDL : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //trouver a quoi correspondent les flags similaires
    window = SDL_CreateWindow("OpenGL avec SDL3", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL /*|  SDL_WINDOW_FULLSCREEN */);
    if (!window) {
        SDL_Log("Erreur de création de la fenêtre : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_Log("Erreur de création du contexte OpenGL : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Erreur d'initialisation de GLEW !");
        return SDL_APP_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);
    std::cout << glGetString(GL_VERSION) << std::endl;
    rectangleSetup();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // OpenGL
    rectangle();


    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (glContext) {
        glDeleteVertexArrays(1, &VAO);


        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

int shaderSetup() {

    ShaderSource shaderSource = parseShader("/resources/shaders/basic.shader");
    std::cout << shaderSource.vertex << "\n" << shaderSource.fragment;

/* 
    //vertex shader
    unsigned int vertexShaderId = glCreateShader(GL_VERTEX_SHADER); //creating an empty vertex shader
    glShaderSource(vertexShaderId, 1, &(shaderSource.vertex), NULL); //attach source to the vertexShader object

    //compilation vertex shader
    glCompileShader(vertexShaderId);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success); //check if compilation is successful
    if(!success) {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return GL_FAILURE_NV;
    }

    //fragment shader
    unsigned int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &(shaderSource.fragment), NULL);

    glCompileShader(fragmentShaderId);

    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success); //iv means it needs Integer and Vector(the success pointer)
    if(!success) {
        glGetShaderInfoLog(fragmentShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return GL_FAILURE_NV;
        //glGetError is not very helpful. Insted use glDebugMessageCallback
    }

    //shader program object
    shaderProgram = glCreateProgram(); //return the id of the new program

    glAttachShader(shaderProgram, vertexShaderId); //link shaders to the program
    glAttachShader(shaderProgram, fragmentShaderId);
    glLinkProgram(shaderProgram); //link the shaders together

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        return GL_FAILURE_NV;
    }

    glDeleteShader(vertexShaderId); //So we can now delete the shaders
    glDeleteShader(fragmentShaderId);
 */
    return GL_SUCCESS_NV;
}

int rectangleSetup() {

    shaderSetup();

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

    //Vertex input
    unsigned int VBO; //stand for Vertex Buffer Object
    unsigned int EBO;
    //VAO Vertex array object
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); //start configuring VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind empty VBO to the GL_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //copy data from vertices in GL_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
    //data are now stored in the GPU and are managed by the VBO and widely by the VAO.

    //specify how OpenGL should interpret the positions attributes of the vertex

    /*keywords in and out are for the inputs and outputs of the shaders
    layout (location = 0) is for the location used for the glVertexAttribPointer function*/
    glVertexAttribPointer(0/*id*/, 2/*nb of values*/, GL_FLOAT/*type*/, GL_FALSE/*normalized*/, 2 * sizeof(float)/*size of the vertex*/, (const void*)0 /*offset*/);
    glEnableVertexAttribArray(0/*id*/); //activate the line above

    //glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); //stop configuring VAO

    return GL_SUCCESS_NV;
}

int rectangle() {
    glUseProgram(shaderProgram);//every shader and rendering call after glUseProgram will now use this program object

    float timeValue = SDL_GetTicks() / 100;
    float blueValue = (std::sin(timeValue) / 2.0f) + 0.5f;
    float redValue = (std::cos(timeValue) / 2.0f) + 0.5f;

    int vertexColorationLocation = glGetUniformLocation(shaderProgram, "ourColor");
    glUniform4f(vertexColorationLocation, redValue, 0.0f, blueValue, 1.0f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    return GL_SUCCESS_NV;
}