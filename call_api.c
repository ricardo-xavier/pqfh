#include <jni.h>
#include <stdlib.h>
#include <string.h>

JavaVM *jvm=NULL;
JNIEnv *env=NULL;

extern int dbg;

void create_vm() {

    JavaVMInitArgs vm_args;
    JavaVMOption options; 

    char *java = getenv("PQFH_JAVA");
    if (java == NULL) {
        options.optionString = "-Djava.class.path=/u/java"; // path to java classes
    } else {
        sprintf(options.optionString, "-Djava.class.path=%s", java);
    }
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

void api_get_all(char *operacao, char *arquivo) {

    char *bearer, *endereco, aux[4097], *p;

    bearer = getenv("PQFH_BEARER");
    endereco = getenv("PQFH_API");
        
    if (jvm == NULL) {
        create_vm();
    }

    jclass class = (*env)->FindClass(env, "br/com/avancoinfo/callapi/CallApi");
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
