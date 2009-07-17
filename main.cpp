#include "main.hpp"

// SDL
int WINDOW_WIDTH = 1024;
int WINDOW_HEIGHT = 768;
int WINDOW_BIT_DEPTH = 24;

bool fullscreen = false;

SDL_Event event;

// OpenGL
GLuint p;
GLuint v;
GLuint f;
GLuint fbo_texture_id;
GLuint fbo_id;
GLuint rbo_id;
const int FBO_TEXTURE_WIDTH = 256;
const int FBO_TEXTURE_HEIGHT = 256;

// Audio
bool record = false;
char* sound_file;

FMOD_RESULT result;
FMOD_CREATESOUNDEXINFO exinfo;
FMOD::System *fmod_system;
FMOD::Sound  *sound   = 0;
FMOD::Channel *channel = 0;

float spectrum[8][256];
float processed_spectrum[256];
float rolling_spectrum[256];
float low_freq_samples[64];
float high_freq_samples[64];
int output_rate = 44100;
float spectrum_freq = (output_rate/2.0f)/256.0f;
Uint32 latency = 30;

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
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

    // Initialize context
    SDL_Surface* drawContext;
    Uint32 flags = SDL_OPENGL | SDL_DOUBLEBUF;
    if(fullscreen)
    {
        flags = flags | SDL_FULLSCREEN;
    }
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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // GLEW
    glewInit();
    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
        printf("GLSL Supported\n");
    else {
        printf("No GLSL! Are you from 1990?");
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
    int num_drivers;
    int count;

    // Create the main system object.
    result = FMOD::System_Create(&fmod_system);
    FMODErrorCheck(result);

    //result = fmod_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
    //FMODErrorCheck(result);

    // Enumerate drivers
    result = fmod_system->getNumDrivers(&num_drivers);
    FMODErrorCheck(result);

    printf("\nOutput drivers: %i\n", num_drivers);
    for (count=0; count < num_drivers; count++)
    {
        char name[256];

        result = fmod_system->getDriverInfo(count, name, 256, 0);
        FMODErrorCheck(result);
        printf("%d : %s\n", count + 1, name);
    }

    result = fmod_system->setDriver(0);
    FMODErrorCheck(result);

    result = fmod_system->getRecordNumDrivers(&num_drivers);
    FMODErrorCheck(result);

    printf("\nRecording drivers: %i\n", num_drivers);
    for (count=0; count < num_drivers; count++)
    {
        char name[256];

        result = fmod_system->getRecordDriverInfo(count, name, 256, 0);
        FMODErrorCheck(result);

        printf("%d : %s\n", count + 1, name);
    }

    result = fmod_system->setDSPBufferSize(512, 4);
    FMODErrorCheck(result);

    // Initialize FMOD.
    result = fmod_system->init(32, FMOD_INIT_NORMAL, 0);
    FMODErrorCheck(result);

    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 1;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = output_rate;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 3;

    if(record == false)
    {
        result = fmod_system->createSound(sound_file, FMOD_2D | FMOD_SOFTWARE, 0, &sound);
        FMODErrorCheck(result);
    }
    else
    {
        result = fmod_system->createSound(0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &sound);
        FMODErrorCheck(result);

        result = fmod_system->recordStart(0, sound, true);
        FMODErrorCheck(result);

        SDL_Delay(latency);
    }

    sound->setMode(FMOD_LOOP_NORMAL);
    result = fmod_system->playSound(FMOD_CHANNEL_REUSE, sound, false, &channel);
    FMODErrorCheck(result);

    if(record)
    {
        result = channel->setMute(true);
        FMODErrorCheck(result);
    }

    // Check latencies
    unsigned int blocksize;
    int numblocks;
    float ms;
    int frequency;

    result = fmod_system->getDSPBufferSize(&blocksize, &numblocks);
    result = fmod_system->getSoftwareFormat(&frequency, 0, 0, 0, 0, 0);

    ms = (float)blocksize * 1000.0f / (float)frequency;

    printf("\nMixer blocksize        = %.02f ms\n", ms);
    printf("Mixer Total buffersize = %.02f ms\n", ms * numblocks);
    printf("Mixer Average Latency  = %.02f ms\n", ms * ((float)numblocks - 1.5f));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return true;
}

int main(int argc, char* argv[])
{
    // Args
    if(argc > 1)
    {
        if(!strcmp(argv[1], "-r"))
        {
            printf("Record mode, will use default recording device.\n\n");
            record = true;
        }
        else
        {
            record = false;
            printf("Going to play %s\n\n", argv[1]);
            sound_file = argv[1];
        }
        if(argc > 2)
        {
            if(!strcmp(argv[2], "-f"))
            {
                fullscreen = true;
            }
            if(argc > 4)
            {
                WINDOW_WIDTH = atoi(argv[3]);
                WINDOW_HEIGHT = atoi(argv[4]);
            }
        }
    }
    else
    {
        printf("Usage: visualizer -r [-f width height]\n       visualizer <filename> [-f width height]\n\n -r: record from default audio source\n  filename: audio file to play\n\n -f width height: fullscreen at width*height resolution.\n", argv[0]);
        return 0;
    }

    // Intialize SDL, OpenGL context, etc.
    if (init() == false)
    {
        return -1;
    }

    printf("\nInitialization complete. Enjoy!\n");

    shader_init();

    // Uniform locations
    GLint time_loc = glGetUniformLocation(p, "time");
    GLint amplitude_loc = glGetUniformLocation(p, "amplitude");
    GLint high_freq_loc = glGetUniformLocation(p, "high_freq");
    GLint low_freq_loc = glGetUniformLocation(p, "low_freq");
    GLint texture_loc = glGetUniformLocation(p, "texture");
    GLint low_freq_max_loc = glGetUniformLocation(p, "low_freq_max");

    // Scene
    //
    // Sphere
    GLUquadric* quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricOrientation(quadric, GLU_OUTSIDE);

    // Lights
    GLfloat light_0_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_0_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_0_position[] = {0.0f, 0.0f, 1.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_0_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    // Textures and FBOs
    glGenTextures(1, &fbo_texture_id);
    glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FBO_TEXTURE_WIDTH, FBO_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffersEXT(1, &fbo_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
    glGenRenderbuffersEXT(1, &rbo_id);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo_id);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, FBO_TEXTURE_WIDTH, FBO_TEXTURE_HEIGHT);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo_texture_id, 0);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


    bool quit = false;

    unsigned int length;
    bool playing = false;
    bool recording  = false;
    bool looping = true;
    unsigned int recordpos = 0;
    unsigned int playpos = 0;

    int i;
    float instant_spectrum[256];
    float instant_spectrum_l[256];
    float instant_spectrum_r[256];

    float rot_angle;

    while (quit == false)
    {
        // Grab spectrums
        result = channel->getSpectrum(instant_spectrum_l, 256, 0, FMOD_DSP_FFT_WINDOW_RECT);
        if(!record)
        {
            result = channel->getSpectrum(instant_spectrum_r, 256, 1, FMOD_DSP_FFT_WINDOW_RECT);
        }
        FMODErrorCheck(result);

        // Merge stereo
        for(i = 0; i < 256; i++)
        {
            if(record)
            {
                instant_spectrum[i] = 10.0f * (float)log10(instant_spectrum_l[i]/2.0f) * 2.0f;
            }
            else
            {
                instant_spectrum[i] = 10.0f * (float)log10((instant_spectrum_l[i] + instant_spectrum_r[i])/2.0f) * 2.0f;
            }
        }
        //instant_spectrum[255] = 0;

        // Rolling spectrum average
        for(i = 7; i >= 1; i = i - 1)
        {
            arrayCopy(256, spectrum[i-1], spectrum[i]);
        }
        arrayCopy(256, instant_spectrum, spectrum[0]);

        int j;
        for(j = 0; j < 256; j++)
        {
            rolling_spectrum[j] = 0;
            for(i = 0; i < 8; i++)
            {
                 rolling_spectrum[j] += spectrum[i][j];
            }
            rolling_spectrum[j] = rolling_spectrum[j] / 8.0f;
        }

        float max = -100.0f;
        int max_index = 0;
        for(i = 0; i < 256; i++)
        {
            if(rolling_spectrum[i] > max)
            {
                    max = rolling_spectrum[i];
                    max_index = i;
            }
        }

         // Normalize
        float low_avg = 0;
        float low_mid_avg = 0;
        float mid_avg = 0;
        float high_avg = 0;

        float low_max = 0;
        float low_mid_max = 0;
        float mid_max = 0;
        float high_max = 0;
        int low_max_index = 0;
        int low_mid_max_index = 0;
        int mid_max_index = 0;
        int high_max_index = 0;

        for(i = 0; i < 256; i++)
        {
            processed_spectrum[i] = (rolling_spectrum[i] + 100)/(100);
            if(i < 64)
            {
                low_avg += processed_spectrum[i];
                if(processed_spectrum[i] > low_max)
                {
                    low_max = processed_spectrum[i];
                    low_max_index = i;
                }
            }
            else if(i < 128 && i >= 64)
            {
                low_mid_avg += processed_spectrum[i];
                if(processed_spectrum[i] > low_mid_max)
                {
                    low_mid_max = processed_spectrum[i];
                    low_mid_max_index = i;
                }
            }
            else if(i < 196 && i >= 128)
            {
                mid_avg += processed_spectrum[i];
                if(processed_spectrum[i] > mid_max)
                {
                    mid_max = processed_spectrum[i];
                    mid_max_index = i;
                }
            }
            else if(i < 256 && i >= 196 )
            {
                high_avg += processed_spectrum[i];
                if(processed_spectrum[i] > high_max)
                {
                    high_max = processed_spectrum[i];
                    high_max_index = i;
                }
            }
        }

        low_avg = low_avg/64.0f;
        low_mid_avg = low_mid_avg/64.0f;
        mid_avg = mid_avg/64.0f;
        high_avg = high_avg/64.0f;

        float high_freq_avg = 0;
        float low_freq_avg  = 0;
        float high_freq_max = spectrum_freq*(low_mid_max_index+1);
        float low_freq_max = spectrum_freq*(low_max_index+1);
        for(i = 63; i >= 1; i = i-1)
        {
            low_freq_samples[i] = low_freq_samples[i-1];
            high_freq_samples[i] = high_freq_samples[i-1];
        }
        high_freq_samples[0] = high_freq_max;
        low_freq_samples[0] = low_freq_max;

        for(i = 0; i < 64; i++)
        {
            high_freq_avg += high_freq_samples[i];
            low_freq_avg += low_freq_samples[i];
        }
        high_freq_avg = high_freq_avg / 64.0f;
        low_freq_avg = low_freq_avg / 64.0f;

        //printf("dominant high freq: %f dominant low freq: %f\n", high_freq_avg, low_freq_avg);

        // OpenGL
        // Uniforms for shaders
        glUniform1f(time_loc, (GLfloat)SDL_GetTicks()*0.001);
        //glUniform1f(amplitude_loc, 8.0f*smoothstep(-1, 1, log(low_mid_max/low_mid_avg)));
        glUniform1f(amplitude_loc, 0.5/normalize(low_avg, low_max));
        glUniform1f(high_freq_loc, high_freq_avg);
        glUniform1f(low_freq_loc, low_freq_avg);
        glUniform1f(low_freq_max_loc, low_freq_max);

        printf("low: %f  high: %f  midmax-mid %f\n", low_freq_avg, high_freq_avg, 1/normalize(low_avg, low_max));

        // Into the FBO
        glViewport(0, 0, FBO_TEXTURE_WIDTH, FBO_TEXTURE_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0f, (float)(FBO_TEXTURE_WIDTH)/FBO_TEXTURE_HEIGHT, 1.0f, 100.0f);
        glMatrixMode(GL_MODELVIEW);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glTranslatef(0.0f, 0.0f, -50.0f);
        glColor4f(0.4f, 0.2f, 1.0f, 1.0f);
        gluSphere(quadric, 8.0f, 64, 64);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
        glGenerateMipmapEXT(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Normal render
        glViewport(0, 0, (GLsizei) WINDOW_WIDTH, (GLsizei) WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0f, (GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity ();

        glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
        glUniform1f(texture_loc, fbo_texture_id);

        rot_angle += 0.9f;

        glTranslatef(0.0f, 0.0f, -40.0f);
        glRotatef(rot_angle,0.0f,1.0f,0.0f);
        glRotatef(20,0.0f,0.0f,1.0f);
        glColor4f(0.4f, 0.2f, 1.0f, 1.0f);
        gluSphere(quadric, 8.0f, 64, 64);



        glFlush();
        glFinish();
        SDL_GL_SwapBuffers();

        // FMOD
        fmod_system->update();

        result = sound->getLength(&length, FMOD_TIMEUNIT_PCM);
        FMODErrorCheck(result);

        result = fmod_system->isRecording(0, &recording);
        FMODErrorCheck(result);

        result = fmod_system->getRecordPosition(0, &recordpos);
        FMODErrorCheck(result);

        if (channel)
        {
            result = channel->isPlaying(&playing);
            FMODErrorCheck(result);

            result = channel->getPosition(&playpos, FMOD_TIMEUNIT_PCM);
            FMODErrorCheck(result);
        }

        //printf("State: %-19s. Record pos = %6d : Play pos = %6d : Loop %-3s\r", recording ? playing ? "Recording / playing" : "Recording" : playing ? "Playing" : "Idle", recordpos, playpos, looping ? "On" : "Off");

        // SDL
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_f:
                        break;
                }
            }
        }

    }

    fmod_system->release();
    SDL_Quit();
    return 0;
}
