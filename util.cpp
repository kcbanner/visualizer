#include "util.h"

void arrayCopy(int length, float* a, float* b)
{
    int i;
    for(i = 0; i < length; i++)
    {
        b[i] = a[i];
    }
}

float saturate(float x)
{
    if(x > 1.0)
    {
        return 1.0f;
    }
    else if(x < 0.0)
    {
        return 0.0f;
    }
    else
    {
        return x;
    }
}

float normalize(float a, float b)
{
    return abs(a-b)/(a+b);
}

float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = saturate( (x-edge0) / (edge1-edge0));
    // Evaluate polynomial
    return x*x*(3-2*x);
}


void FMODErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

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
