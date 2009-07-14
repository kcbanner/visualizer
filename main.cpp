#include "main.hpp"

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int WINDOW_BIT_DEPTH = 24;

SDL_Event event;

GLuint p;
GLuint v;
GLuint f;

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void shader_init() {
	
    char *vs,*fs;
	
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);	

    vs = textFileRead("vert.glsl");
    fs = textFileRead("frag.glsl");
	
    const char * vv = vs;
    const char * ff = fs;
	
    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);
	
    free(vs);
    free(fs);
	
    glCompileShader(v);
    printShaderInfoLog(v);

    glCompileShader(f);
    printShaderInfoLog(f);
	
    p = glCreateProgram();
	
    glAttachShader(p,v);
    glAttachShader(p,f);
	
    glLinkProgram(p);
    printProgramInfoLog(p);
    
    glUseProgram(p);    
}

bool init()
{
    // SDL Init
    int error;
    error = SDL_Init(SDL_INIT_EVERYTHING);
    if (error == -1)
    {
        return false;
    }

    // Set attributes of the context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // Initialize context
    SDL_Surface* drawContext;
    Uint32 flags = SDL_OPENGL;
    drawContext = SDL_SetVideoMode(WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   WINDOW_BIT_DEPTH,
                                   flags);
    if (drawContext == NULL)
    {
        return false;
    }

    // OpenGL Setup
    glViewport(0, 0, (GLsizei) WINDOW_WIDTH, (GLsizei) WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // GLEW
    glewInit();
    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
        printf("GLSL Supported\n");
    else {
        printf("No GLSL");
        return false;
    }

    if (glewIsSupported("GL_VERSION_2_0"))
			printf("OpenGL 2.0 Supported\n");
    else {
        printf("OpenGL 2.0 not supported\n");
        return false;
    }
    
    // Misc
    SDL_WM_SetCaption( "Lovin it!", NULL );
    
    return true;
}


int main(int argc, char* argv[])
{
    // Intialize SDL, OpenGL context, etc.
    if (init() == false)
    {
        return -1;
    }

    shader_init();


    // Uniform locations
    GLint time_loc = glGetUniformLocation(p, "time");

    // Sphere
    GLUquadric* quadric = gluNewQuadric();
    

    
    bool quit = false;
    while (quit == false)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity ();

        // Uniforms for shaders
        glUniform1f(time_loc, (GLfloat)SDL_GetTicks()*0.001);
                
        glTranslatef(0.0f,0.0f,-6.0f);
        gluSphere(quadric, 1.0f, 20, 20);
        
        glFlush();
        glFinish();
        
        SDL_GL_SwapBuffers();
        
        while (SDL_PollEvent(&event))
        {
            if( event.type == SDL_QUIT )
            {
                quit = true;
            }
        }
    }

    
    SDL_Quit();
    return 0;
}
