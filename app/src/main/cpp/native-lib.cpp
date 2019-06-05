#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_stable_horseone_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello Horse 444 C++";
    return env->NewStringUTF(hello.c_str());
}
