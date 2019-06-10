#include <jni.h>
#include <errno.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
//#include "C:\sdk\ndk-bundle\sources\android\native_app_glue\android_native_app_glue.h"
#include <android_native_app_glue.h>
#include <cstring>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
void team(struct android_app*);
void team2(struct android_app*);
void launch(struct android_app*);
struct engine {
    struct android_app* app;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    int32_t touchX;
    int32_t touchY;
};

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
    glViewport(0, 0, w, h);

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

    glClearColor(0,100,80, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(engine->display, engine->surface);
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

        team(app);


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
void team2(struct android_app* app){
    jint lResult;

    jint lFlags = 0;


    //EngineAndroid * pEngine = (EngineAndroid*)Engine::InstancePtr();

    JavaVM* lJavaVM = app->activity->vm;    //<---- replace this with your NativeActivity pointer

    JNIEnv *lJNIEnv = 0;


    lResult=lJavaVM->AttachCurrentThread(&lJNIEnv,NULL);

    if (lResult == JNI_ERR) {

        return;

    }

    jobject lNativeActivity =  app->activity->clazz;

    jclass ClassNativeActivity = lJNIEnv->FindClass("android/app/NativeActivity");

    jclass contextClass = lJNIEnv->FindClass("android/content/Context");

    if(contextClass == 0)

        return;

    jmethodID startActivityMethodId = lJNIEnv->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");

    if(startActivityMethodId == 0)

        return;

    jclass intentClass = lJNIEnv->FindClass("android/content/Intent");

    if(intentClass == 0)

        return;

    jmethodID intentConstructorMethodId = lJNIEnv->GetMethodID(intentClass, "<init>", "()V");

    if(intentConstructorMethodId == 0)

        return;

    jmethodID intentSetActionMethodId = lJNIEnv->GetMethodID(intentClass, "setAction", "(Ljava/lang/String;)Landroid/content/Intent;");

    if(intentSetActionMethodId == 0)

        return;

    jmethodID getClassLoader = lJNIEnv->GetMethodID(ClassNativeActivity,"getClassLoader", "()Ljava/lang/ClassLoader;");

    if(getClassLoader == 0)

        return;

    jobject cls = lJNIEnv->CallObjectMethod(lNativeActivity, getClassLoader);

    if(cls == 0)

        return;

    jclass classLoader = lJNIEnv->FindClass("java/lang/ClassLoader");

    if(classLoader == 0)

        return;

    jmethodID findClass = lJNIEnv->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    if(findClass == 0)

        return;

    jstring intentString = lJNIEnv->NewStringUTF("com.stable.horseone.MainActivity");

    if(intentString == 0)

        return;

    jclass marketActivityClass = (jclass)lJNIEnv->CallObjectMethod(cls, findClass, intentString);

    if(marketActivityClass == 0)

        return;

    jobject intentObject = lJNIEnv->NewObject(intentClass,intentConstructorMethodId);

    if(intentObject == 0)

        return;

    lJNIEnv->CallVoidMethod(intentObject, intentSetActionMethodId,intentString);

    lJNIEnv->CallVoidMethod(lNativeActivity, startActivityMethodId, intentObject);

    lJavaVM->DetachCurrentThread();
}
void team(struct android_app* app){

    //struct android_app* app = (struct android_app*) engine->app;
    JNIEnv *env;
    JavaVM* lJavaVM = app->activity->vm;
    app->activity->vm->AttachCurrentThread(&env, NULL);

    jobject lNativeActivity = app->activity->clazz;

    jstring actionString = env->NewStringUTF("com.stable.horseone.pro5");
    jclass intentClass = env->FindClass("android/content/Intent");

    jmethodID newIntent = env->GetMethodID(intentClass, "<init>", "()V");
    jobject intent = env->AllocObject(intentClass);
    //jobject intent = env->NewObject(intentClass, newIntent);

    env->CallVoidMethod(intent, newIntent);
    jmethodID setAction = env->GetMethodID(intentClass, "setAction","(Ljava/lang/String;)Landroid/content/Intent;");
    env->CallObjectMethod(intent, setAction, actionString);
    jclass activityClass = env->FindClass("android/app/Activity");

    jmethodID startActivity = env->GetMethodID(activityClass,"startActivity", "(Landroid/content/Intent;)V");
    //jobject intentObject = env->NewObject(intentClass,newIntent);

    //env->CallVoidMethod(intentObject, setAction,actionString);
    env->CallVoidMethod(lNativeActivity, startActivity, intent);

    app->activity->vm->DetachCurrentThread();
}

void launch(struct android_app* app)
{
    // Attaches the current thread to the JVM.

    jint lResult;

    jint lFlags = 0;


    //EngineAndroid * pEngine = (EngineAndroid*)Engine::InstancePtr();

    JavaVM* lJavaVM = app->activity->vm;    //<---- replace this with your NativeActivity pointer

    JNIEnv *lJNIEnv = 0;


    lResult=lJavaVM->AttachCurrentThread(&lJNIEnv,NULL);

    if (lResult == JNI_ERR) {

        return;

    }

    jobject lNativeActivity =  app->activity->clazz;

    jclass ClassNativeActivity = lJNIEnv->FindClass("android/app/NativeActivity");

    jclass contextClass = lJNIEnv->FindClass("android/content/Context");

    if(contextClass == 0)

        return;

    jmethodID startActivityMethodId = lJNIEnv->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");

    if(startActivityMethodId == 0)

        return;

    jclass intentClass = lJNIEnv->FindClass("android/content/Intent");

    if(intentClass == 0)

        return;

    jmethodID intentConstructorMethodId = lJNIEnv->GetMethodID(intentClass, "<init>", "()V");

    if(intentConstructorMethodId == 0)

        return;

    jmethodID intentSetActionMethodId = lJNIEnv->GetMethodID(intentClass, "setAction", "(Ljava/lang/String;)Landroid/content/Intent;");

    if(intentSetActionMethodId == 0)

        return;

    jmethodID getClassLoader = lJNIEnv->GetMethodID(ClassNativeActivity,"getClassLoader", "()Ljava/lang/ClassLoader;");

    if(getClassLoader == 0)

        return;

    jobject cls = lJNIEnv->CallObjectMethod(lNativeActivity, getClassLoader);

    if(cls == 0)

        return;

    jclass classLoader = lJNIEnv->FindClass("java/lang/ClassLoader");

    if(classLoader == 0)

        return;

    jmethodID findClass = lJNIEnv->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    if(findClass == 0)

        return;

    jstring intentString = lJNIEnv->NewStringUTF("com.stable.horseone.pro");

    if(intentString == 0)

        return;

    jclass marketActivityClass = (jclass)lJNIEnv->CallObjectMethod(cls, findClass, intentString);

    if(marketActivityClass == 0)

        return;

    jobject intentObject = lJNIEnv->NewObject(intentClass,intentConstructorMethodId);

    if(intentObject == 0)

        return;

    lJNIEnv->CallVoidMethod(intentObject, intentSetActionMethodId,intentString);

    lJNIEnv->CallVoidMethod(lNativeActivity, startActivityMethodId, intentObject);

    lJavaVM->DetachCurrentThread();
    return ;
}