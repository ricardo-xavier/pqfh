#include <jni.h>

JavaVM *jvm=NULL;
JNIEnv *env=NULL;

void create_vm() {

    JavaVMInitArgs vm_args;
    JavaVMOption options; 

    options.optionString = "-Djava.class.path=/u/sist/exec/ricardo"; // path to java classes
    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = 0;

    JNI_CreateJavaVM(&jvm, (void**) &env, &vm_args);
}

static void throwJavaException(){

    jthrowable e = (*env)->ExceptionOccurred(env);
    if (e != NULL) {
        (*env)->ExceptionDescribe(env);
    }

}

void pqfh_call_java(char *rota, char *metodo, char *json, char *bearer) {

    fprintf(stderr, "pqfh_call_java [%s] [%s] [%s]\n", rota, metodo, json);
        
    if (jvm == NULL) {
        create_vm();
    }

    //jclass class = (*env)->FindClass(env, "avanco/HelloJava");
    jclass class = (*env)->FindClass(env, "br/com/avancoinfo/callapi/CallApi");
    throwJavaException();

    jstring j_rota = (*env)->NewStringUTF(env, rota);
    jstring j_bearer = (*env)->NewStringUTF(env, bearer);
    jstring j_metodo = (*env)->NewStringUTF(env, metodo);
    jstring j_json = (*env)->NewStringUTF(env, json);

    jmethodID method = (*env)->GetStaticMethodID(env, class, "setBearer", "(Ljava/lang/String;)V");
    throwJavaException();

    (*env)->CallStaticVoidMethod(env, class, method, j_bearer);
    throwJavaException();

    method = (*env)->GetStaticMethodID(env, class, "setRota", "(Ljava/lang/String;)V");
    throwJavaException();

    (*env)->CallStaticVoidMethod(env, class, method, j_rota);
    throwJavaException();

    method = (*env)->GetStaticMethodID(env, class, "call", "(Ljava/lang/String;Ljava/lang/String;)V");
    throwJavaException();

    (*env)->CallStaticVoidMethod(env, class, method, j_metodo, j_json);
    throwJavaException();

}
