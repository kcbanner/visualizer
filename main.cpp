#include "main.hpp"

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int WINDOW_BIT_DEPTH = 24;

SDL_Event event;

GLuint p;
GLuint v;
GLuint f;

FMOD_RESULT result;
FMOD::System *fmod_system;

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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
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


    // FMOD
    // Create the main system object.
    result = FMOD::System_Create(&fmod_system);		
    FMODErrorCheck(result);
    
    // Initialize FMOD.
    result = fmod_system->init(100, FMOD_INIT_NORMAL, 0);	
    FMODErrorCheck(result);

    //fmod_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
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
    GLint amplitude_loc = glGetUniformLocation(p, "amplitude");
    GLint speed_loc = glGetUniformLocation(p, "speed");

    // Scene
    // 
    // Sphere
    GLUquadric* quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricOrientation(quadric, GLU_OUTSIDE);

    // Lights
    GLfloat light_0_ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat light_0_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_0_position[] = {0.0f, 0.0f, 1.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_0_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    
    bool quit = false;
    while (quit == false)
    {
        // OpenGL
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity ();

        // Uniforms for shaders
        glUniform1f(time_loc, (GLfloat)SDL_GetTicks()*0.001);
        glUniform1f(amplitude_loc, 0.5f);
        glUniform1f(speed_loc, 5.0f);

       
        glTranslatef(0.0f, 0.0f, -60.0f);
        glColor4f(1.0f, 0.5f, 0.5f, 0.5f);
        gluSphere(quadric, 10.0f, 32, 32);
        
        glFlush();
        glFinish();
        SDL_GL_SwapBuffers();

        // FMOD
        fmod_system->update();

        // SDL
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
