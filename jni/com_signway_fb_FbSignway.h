/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_signway_fb_FbSignway */

#ifndef _Included_com_signway_fb_FbSignway
#define _Included_com_signway_fb_FbSignway
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_signway_fb_FbSignway
 * Method:    FbOpen
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbOpen
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_signway_fb_FbSignway
 * Method:    FbClose
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbClose
  (JNIEnv *, jobject);

/*
 * Class:     com_signway_fb_FbSignway
 * Method:    FbRead
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbRead
  (JNIEnv *, jobject, jbyteArray, jint);

/*
 * Class:     com_signway_fb_FbSignway
 * Method:    FbWrite
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbWrite
  (JNIEnv *, jobject, jbyteArray, jint);

/*
 * Class:     com_signway_fb_FbSignway
 * Method:    FbIoctl
 * Signature: (ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbIoctl
  (JNIEnv *, jobject, jint, jintArray, jintArray, jintArray);

#ifdef __cplusplus
}
#endif
#endif