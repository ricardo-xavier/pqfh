#include <jni.h>
#include <stdlib.h>
#include <string.h>

JavaVM *jvm=NULL;
JNIEnv *env=NULL;
JNIEnv *thread_env=NULL;

extern int dbg;

static void throwJavaException() {

    jthrowable e = (*thread_env)->ExceptionOccurred(thread_env);
    if (e != NULL) {
        (*thread_env)->ExceptionDescribe(thread_env);
    }

}

void create_vm() {

    JavaVMInitArgs vm_args;
    JavaVMOption options; 

    if (jvm != NULL) {
        return;
    }

    char *java = getenv("PQFH_JAVA");
    if (java == NULL) {
        options.optionString = "-Djava.class.path=/u/java"; // path to java classes
    } else {
        sprintf(options.optionString, "-Djava.class.path=%s", java);
    }
    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = JNI_TRUE;

    JNI_CreateJavaVM(&jvm, (void**) &env, &vm_args);
}

void attach_vm() {

    if (jvm == NULL) {
        return;
    }

    (*jvm)->AttachCurrentThread(jvm, (void **) &thread_env, NULL);
}

void api_get_all(char *operacao, char *arquivo) {

/*
    char *bearer, *endereco, aux[4097], *p;

    bearer = getenv("PQFH_BEARER");
    endereco = getenv("PQFH_API");
        
    if (jvm == NULL) {
        create_vm();
    }

    jclass class = (*thread_env)->FindClass(thread_env, "br/com/avancoinfo/callapi/CallApi");
    throwJavaException();

    jmethodID method;
    strcpy(aux, operacao);
    if ((p = strchr(aux, ' ')) != NULL) *p = 0;
    if (dbg > 0) {
        fprintf(stderr, "api_get_all [%s]\n", aux);
    }
    jstring j_operacao = (*env)->NewStringUTF(env, aux);
    strcpy(aux, arquivo);
    if ((p = strchr(aux, ' ')) != NULL) *p = 0;
    if (dbg > 0) {
        fprintf(stderr, "api_get_all [%s]\n", aux);
    }
    jstring j_arquivo = (*env)->NewStringUTF(env, aux);

    if (bearer != NULL) {
        jstring j_bearer = (*env)->NewStringUTF(env, bearer);
        method = (*env)->GetStaticMethodID(env, class, "setBearer", "(Ljava/lang/String;)V");
        throwJavaException();
        (*env)->CallStaticVoidMethod(env, class, method, j_bearer);
        throwJavaException();
    }

    if (endereco != NULL) {
        jstring j_endereco = (*env)->NewStringUTF(env, endereco);
        method = (*env)->GetStaticMethodID(env, class, "setEndereco", "(Ljava/lang/String;)V");
        throwJavaException();
        (*env)->CallStaticVoidMethod(env, class, method, j_endereco);
        throwJavaException();
    }

    method = (*env)->GetStaticMethodID(env, class, "getAll", "(Ljava/lang/String;Ljava/lang/String;)V");
    throwJavaException();

    (*env)->CallStaticVoidMethod(env, class, method, j_operacao, j_arquivo);
    throwJavaException();
*/

}

void pqfh_call_java(char *endereco, char *operacao, char *metodo, char *json, char *bearer) {

    if (dbg > 0) {
        fprintf(stderr, "pqfh_call_java [%s] [%s] [%s] [%s]\n", endereco, operacao, metodo, json);
    }

    attach_vm();

    jclass class = (*thread_env)->FindClass(thread_env, "br/com/avancoinfo/callapi/CallApi");
    throwJavaException();
    if (dbg > 1) {
        fprintf(stderr, "classe carregada\n");
    }

    jstring j_endereco = (*thread_env)->NewStringUTF(thread_env, endereco);
    jstring j_bearer = (*thread_env)->NewStringUTF(thread_env, bearer);
    jstring j_operacao = (*thread_env)->NewStringUTF(thread_env, operacao);
    jstring j_metodo = (*thread_env)->NewStringUTF(thread_env, metodo);
    jstring j_json = (*thread_env)->NewStringUTF(thread_env, json);

    if (dbg > 1) {
        fprintf(stderr, "setBearer\n");
    }
    jmethodID method = (*thread_env)->GetStaticMethodID(thread_env, class, "setBearer", "(Ljava/lang/String;)V");
    throwJavaException();

    (*thread_env)->CallStaticVoidMethod(thread_env, class, method, j_bearer);
    throwJavaException();

    if (dbg > 1) {
        fprintf(stderr, "setEndereco\n");
    }
    method = (*thread_env)->GetStaticMethodID(thread_env, class, "setEndereco", "(Ljava/lang/String;)V");
    throwJavaException();

    (*thread_env)->CallStaticVoidMethod(thread_env, class, method, j_endereco);
    throwJavaException();

    if (dbg > 1) {
        fprintf(stderr, "call\n");
    }
    method = (*thread_env)->GetStaticMethodID(thread_env, class, "call", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    throwJavaException();

    (*thread_env)->CallStaticVoidMethod(thread_env, class, method, j_operacao, j_metodo, j_json);
    throwJavaException();

    if (dbg > 1) {
        fprintf(stderr, "fim thread\n");
    }

}
