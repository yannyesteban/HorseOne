#include <jni.h>
#include <errno.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/sensor.h>
#include <android/log.h>
//#include "C:\sdk\ndk-bundle\sources\android\native_app_glue\android_native_app_glue.h"
#include <android_native_app_glue.h>
#include <cstring>
#include <malloc.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct engine {
    struct android_app* app;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    int32_t touchX;
    int32_t touchY;
    GLuint programObject;
};

typedef struct
{
// Handle to a program object
    GLuint programObject;
} UserData;

GLuint LoadShader( GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
// Create the shader object
    shader = glCreateShader(type);
    if(shader == 0)
        return 0;
// Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);
// Compile the shader
    glCompileShader(shader);
// Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1)
        {
            char* infoLog = (char*) malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            //esLogMessage("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int Init(struct engine* engine)
{
    //UserData *userData = esContext->userData;
    const char* vShaderStr =
            "attribute vec4 vPosition; \n"
            "void main() \n"
            "{ \n"
            " gl_Position = vPosition; \n"
            "} \n";
    const char* fShaderStr =
            "precision mediump float; \n"
            "void main() \n"
            "{ \n"
            " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
            "} \n";
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;
// Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);
// Create the program object
    programObject = glCreateProgram();
    if(programObject == 0)
        return 0;
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
// Bind vPosition to attribute 0
    glBindAttribLocation(programObject, 0, "vPosition");
// Link the program
    glLinkProgram(programObject);
// Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1)
        {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            //esLogMessage("Error linking program:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteProgram(programObject);
        return false;
    }
// Store the program object
    engine->programObject = programObject;
    glClearColor(0.0f, 0.0f, 0.28f, 1.0f);
    return true;
}

void Draw(struct engine* engine)
{
    //UserData *userData = esContext->userData;
    GLfloat vVertices1[] = {0.0f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                           0.5f, -0.5f, 0.0f};

    GLfloat vVertices2[] = {1.0f, 1.0f, 0.0f,
                           -1.0f, -1.0f, 0.0f,
                           1.0f, -1.0f, 0.0f};

    GLfloat vVertices[] = {-0.5f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,

                           0.0f, -0.5f, 0.0f,

                           0.0f, -0.5f,0.0f,
                           0.5f, -0.5f, 0.0f,
                           0.5f,0.5f, 0.0f




    };


// Set the viewport
    glViewport(0, 0, engine->width, engine->height);
    //glViewport(400,400,400,400);
// Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
// Use the program object
    glUseProgram(engine->programObject);
// Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    eglSwapBuffers(engine->display, engine->surface);
}





/**
 * Initialize an EGL context for the current display.
 * TODO tidy this up, currently it's mostly Google example code
 */
int init_display(struct engine* engine) {

    // Setup OpenGL ES 2
    // http://stackoverflow.com/questions/11478957/how-do-i-create-an-opengl-es-2-context-in-a-native-activity

    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //important
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };

    EGLint attribList[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

    context = eglCreateContext(display, config, NULL, attribList);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    // Grab the width and height of the surface
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    //glViewport(0, 0, w, h);

    return 0;
}

/**
 * Just the current frame in the display.
 */
void draw_frame(struct engine* engine) {
    // No display.
    if (engine->display == NULL) {
        return;
    }

    glClearColor(100,0,0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
Init(engine);
    Draw(engine);
    //eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void terminate_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->touchX = AMotionEvent_getX(event, 0);
        engine->touchY = AMotionEvent_getY(event, 0);
        LOGI("x %d\ty %d\n",engine->touchX,engine->touchY);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                init_display(engine);
                draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            terminate_display(engine);
            break;
        case APP_CMD_LOST_FOCUS:
            draw_frame(engine);
            break;
    }
}

/**
 * Main entry point, handles events
 */
void android_main(struct android_app* state) {
    //app_dummy();

    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    engine.app = state;

    // Read all pending events.
    while (1) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident=ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                terminate_display(&engine);
                return;
            }
        }

        // Draw the current frame
        draw_frame(&engine);
    }
}