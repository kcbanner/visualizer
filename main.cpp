#include "main.hpp"

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int WINDOW_BIT_DEPTH = 24;

SDL_Event event;

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

    gluPerspective(60.0f, (GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
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

    bool quit = false;
    while (quit == false)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity ();

        glTranslatef(-1.5f,0.0f,-6.0f);
        glBegin(GL_TRIANGLES);
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
        glEnd();

        glTranslatef(3.0f,0.0f,0.0f);
        glBegin(GL_QUADS);						
		glVertex3f(-1.0f, 1.0f, 0.0f);			
		glVertex3f( 1.0f, 1.0f, 0.0f);			
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
        glEnd();
        
        glFlush();
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
