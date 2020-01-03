//Comando para compile.

/*rm  workspace/JavaCall/teste; gcc workspace/JavaCall/src/JavaCall.c -o workspace/JavaCall/teste -L/usr/lib/jvm/java-1.7.0-openjdk-amd64/include  -L/usr/lib/jvm/java-1.7.0-openjdk-amd64/jre/lib/amd64/server/ -ljvm; workspace/JavaCall/teste */

//compilar java  no 241
// /usr/lib/jvm/java-6-openjdk-amd64/bin$ ./javac /home/vidmar/workspace/WebVendas/src/com/tempore/webvenda/Hello.java -d /u/java/classes/com/tempore/webvenda/


//Script 241 para compilar em 32 bits
 // cd /u/java/; rm JavaCall.c JavaCall.o; scp vidmar@192.168.100.14:/home/vidmar/workspace/JavaCall/src/JavaCall.c . ; gcc -c /u/java/JavaCall.c -L/usr/lib/gcc/i486-slackware-linux/4.2.4/include/   -L/usr/lib/java/lib/i386/server/ -ljvm   ; scp JavaCall.o vidmar@192.168.100.14:/u/java

//Setar essa variavel de sistema com o caminho do jvm.so
//export LD_LIBRARY_PATH=/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <stdlib.h>

#define USER_CLASSPATH "." /* where Prog.class is */

typedef struct javaObjeto{
		int id;
		jobject obj;
		struct javaObjeto* prox;

}javaObjeto;

char concatBuff[1024] = "";
char* versao = "1";

javaObjeto * inserir(javaObjeto* Linicio, javaObjeto* obj);
javaObjeto* removeObj(javaObjeto* inicio, int c);
javaObjeto * liberarec(javaObjeto* inicio);
jobject* busca(javaObjeto* inicio, int c);
void chamaLog(char* texto);
void setDebug();
char* concatOnbuff(char* str1 , char* str2 );

javaObjeto* Linicio = NULL;
JNIEnv *env;
JavaVM *jvm;

//Se  é debug  ou nao
int debug = 0;

void throwJavaException();

char *trim(char *string);

//Pega maquina virtual
void carregaJvm() {

    JavaVMInitArgs vm_args;
    JavaVMOption options[3];

    //.class, não aceita  .jar, pra usar jar tem q especificar um a um, então eu extrai os .jars com  winzip na pasta e classes.
    options[0].optionString = "-Djava.class.path=/u/java/classes"; //Caminho das  classes do usuário
    //No caso bibliotecas nativas ok? Não jars.
    options[1].optionString = "-Djava.library.path=/u/java/libs"; //Caminho das bibliotecas.

    vm_args.version = JNI_VERSION_1_2;
    vm_args.nOptions = 2;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = JNI_TRUE;

    int ret;

    ret = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

    if(ret < 0)
    	printf("\nUnable to Launch JVM\n");

    chamaLog(concatOnbuff("C invoke Java - Versao ", versao));
    chamaLog("JVM CRIADA");

}

void setDebug(){

	debug =1;

}

void new(char *classe, int * id){

	classe = trim(classe);

	if(debug == 1){

		chamaLog("Instanciando ");
		chamaLog(concatOnbuff("Conteudo param classe: ", classe));

	}

	//Lembrando que classe é  o caminho completo dela com pacote e tudo, separado  por barra ex java/lang/String
	jclass cls = (*env)->FindClass(env, classe);
	throwJavaException();

	//Pegando o Id do construtor
	jmethodID methodID = (*env)->GetMethodID(env, cls, "<init>", "()V");
	throwJavaException();

	jobject o = (*env)->NewObject(env, cls, methodID, NULL);

	javaObjeto* aux = (javaObjeto *) malloc(sizeof(javaObjeto));

	aux->obj = o;
	aux->prox = NULL;

	if(Linicio != NULL)
		aux->id = Linicio->id  + 1;
	else
		aux->id = 1;

	Linicio = inserir(Linicio, aux);

	int auxid = aux->id;

	*id = auxid ;

}

void freeObj(int * id){

	Linicio = removeObj(Linicio, id);

}

void chamarMetodo(char * metodo, char *retorno, char *assinatura, char * parametro, int* ret, int* objId){

	metodo = trim(metodo);
	assinatura = trim(assinatura);
	parametro = trim(parametro);

	if(debug == 1){

		chamaLog("Entrou Chama Metodo.");
		chamaLog(concatOnbuff("Conteudo param parametro:  ", parametro));
		chamaLog(concatOnbuff("Conteudo assinatura  ", assinatura));
		chamaLog(concatOnbuff("Conteudo param metodo: ", metodo));
		chamaLog(concatOnbuff("Conteudo OBJID: ", objId));
		chamaLog(concatOnbuff("Com retorno: ", (char)ret));

	}

	//Busca  objeto da  lista encadeada.
	jobject o =  busca(Linicio, objId);

	jmethodID metodoId = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, o), metodo, assinatura );
	throwJavaException();

	jstring paramData = (*env)->NewStringUTF(env, parametro);

	jstring jobjRetData;
	jobjRetData = (*env)->CallObjectMethod(env, o, metodoId, paramData);
	throwJavaException();


	if(ret == 1){

		chamaLog("RETORNO: ");
		chamaLog(retorno);

		//Joga pro ponteiro de retorno passado pelo cobol.
		strcpy(retorno, (*env)->GetStringUTFChars(env, jobjRetData, 0));

	}

}

int chamaStatic(char * parametro, char * classe, char * metodo, char * assinatura, char *retorno) {

		parametro = trim(parametro);
		classe = trim(classe);
		metodo = trim(metodo);
		assinatura  = trim(assinatura);

		if(debug){

		chamaLog("Entrou Chama método statico");
		chamaLog(concatOnbuff("Conteudo param parametro:  ", parametro));
		chamaLog(concatOnbuff("Conteudo param classe:  ", classe));
		chamaLog(concatOnbuff("Conteudo assinatura  ", assinatura));
		chamaLog(concatOnbuff("Conteudo metodo: ", metodo));

		}

		//Classe
		jclass idClasse=NULL;

		//Metodo
		jmethodID idMethod = NULL;

		//Obtendo classe da jvm
		idClasse = (*env)->FindClass(env, classe);

		//Exibe  exception do java se ocorrer
		throwJavaException();

		//	Parametros para definir a assinatura dos métodos.
		//		Yes ()Lcom/nativestuff/MyView; is correct. In general:
		//
		//		B = byte
		//		C = char
		//		D = double
		//		F = float
		//		I = int
		//		J = long
		//		S = short
		//		V = voidandroid:drawablePadding
		//		Z = boolean
		//		Lfully-qualified-class = fully qualified class
		//		[type = array of type
		//		(argument types)return type = method type. If no arguments, use empty argument types: ().
		//		If return type is void (or constructor) use (argument types)V.

		//Comando java para ver assinaturas de uma classe javap -s classe

		if(idClasse  == NULL){
			chamaLog("Classe nao encontrada.");
			retorno = "Classe nao encontrada.";
			return 1;
		}

		idMethod = (*env)->GetStaticMethodID(env, idClasse, metodo, assinatura);

		jstring paramData = NULL;
		jstring jobjRetData = NULL;

		paramData = (*env)->NewStringUTF(env, parametro);

		chamaLog("Chamando metodo");

		jobjRetData = (*env)->CallStaticObjectMethod(env, idClasse, idMethod, paramData); //Calling the main method.

		char *pLog;

		pLog = (*env)->GetStringUTFChars(env, jobjRetData, 0);

		//Joga pro ponteiro de retorno passado pelo cobol.
		strcpy(retorno, pLog);

		throwJavaException();

	return 1;
}

void  destroiJvm(){

		chamaLog("Destruindo jvm");

		liberarec(Linicio);

		(*jvm)->DestroyJavaVM(jvm);

	}


//Main metodo para teste.
//NÂO ESQUECA DE COMENTAR
//int main(void) {
//
//	char * retorno = (char *) malloc(100);
//
//	carregaJvm();
//
//	destroiJvm();
//
//	//int  id = 0;
//
//	//new("com/tempore/webvenda/Hello", &id);
//
//	//chamaLog(" ID %d", id);
//
//	//chamarMetodo("setSoma", retorno, "(Ljava/lang/String;)V", "jose" , 0 , id);
//
//	//chamarMetodo("getSoma", retorno, "()Ljava/lang/String;", "jose", 1, id );
//
//	//chamaLog("%s ", retorno);
//
////	char * retorno = (char *) malloc(100);
////
////	carregaJvm();
////
////	chamaJava("JOSE","Hello", "retornaString", retorno);
////
////	printf("%s \n", retorno);
////
////	destroiJvm();
//
//	return EXIT_SUCCESS;
//
//}

void throwJavaException(){
	jthrowable e = (*env)->ExceptionOccurred(env);

	if (e) {

		(*env)->ExceptionDescribe(env);

	}

}

//FUNCOES AUXILIARES

int vazia (javaObjeto* inicio){
	return !inicio;
}

javaObjeto * inserir(javaObjeto* Linicio, javaObjeto* obj) {

	obj->prox = NULL;
	//Inserir na lista

	if (Linicio == NULL) {
		Linicio = obj;

	} else {
		obj->prox = Linicio;
		Linicio = obj;
	}

	return Linicio;
}

jobject* busca(javaObjeto* inicio, int c) {

	if (inicio == NULL)
		return NULL;

	if (inicio->id == c)
		return inicio->obj;

	else
		return busca(inicio->prox, c);

}

javaObjeto * liberarec(javaObjeto* inicio){

	if(inicio){
	    liberarec(inicio->prox);
	    free(inicio);
	}
	return NULL;
}

javaObjeto* removeObj(javaObjeto* inicio, int c) {

	javaObjeto* aux = NULL;
	javaObjeto* aux2 = NULL;

	//if (vazia(inicio)) return inicio;
	aux = inicio;
	if (aux->id == c) {

		inicio = inicio->prox;
		free(aux);

	} else {

		while (aux->prox) {
			if (aux->prox->id == c) {
				aux2 = aux->prox;
				aux->prox = aux->prox->prox;
				free(aux2);
				break;
			}
			aux = aux->prox;

		}

	}
	return inicio;
}

//Chama log java do avanco utilitarios nao esquecer de colocar a classe ArquivoLog no path.
void chamaLog(char* texto){

		//Classe
		jclass idClasse=NULL;

		//Metodo
		jmethodID idMethod = NULL;

		jfieldID idCaminhoLog = NULL;
		jfieldID idPastaLog = NULL;
		jfieldID idLogConsole = NULL;

		//Obtendo classe da jvm
		idClasse = (*env)->FindClass(env, "avanco/utilitarios/log/AvancoLog");

		idCaminhoLog = (*env)->GetStaticFieldID(env, idClasse,"local", "Ljava/lang/String;");
		idPastaLog = (*env)->GetStaticFieldID(env, idClasse,"pastaLog", "Ljava/lang/String;");
		idLogConsole = (*env)->GetStaticFieldID(env, idClasse,"logConsole", "Z");
		throwJavaException();

		//Setando atributos estáticos da classe AvancoLog, já disse q ela tem que estar no build path?
		(*env)->SetStaticObjectField(env, idClasse, idCaminhoLog, (*env)->NewStringUTF(env, "/u/java/log"));
		throwJavaException();
		(*env)->SetStaticObjectField(env, idClasse, idPastaLog, (*env)->NewStringUTF(env, "logjavacall"));
		(*env)->SetStaticBooleanField(env, idClasse, idLogConsole, JNI_FALSE);

		idMethod = (*env)->GetStaticMethodID(env, idClasse, "escreveLog", "(Ljava/lang/String;Ljava/lang/String;)Z");
		jstring paramTag = NULL;
		jstring paramMsg = NULL;

		paramTag = (*env)->NewStringUTF(env, "NATIVE");
		paramMsg = (*env)->NewStringUTF(env, texto);

		(*env)->CallStaticObjectMethod(env, idClasse, idMethod, paramTag, paramMsg ); //Calling the main method.

		throwJavaException();

}

int STR_tamanho(const char *string)
{
  if (string)
    {
      return strlen(string);
    }

  return 0;
}

int STR_desloca_bloco(char *string,
		      int posicao_inicial_bloco_deslocar,
		      int posicao_final_bloco_deslocar,
		      int deslocamento)
{
  int numero_caracteres_deslocar = 0;
  /* Posição onde o próximo caracter a ser deslocado está. */
  char *posicao_origem = NULL;
  /* Posição para onde será copiado o próximo caracter a ser
   * deslocado
   */
  char *posicao_destino = NULL;

  /* ###### Verificação dos parâmetros ###### */

  /* Se deslocamento for 0 (zero), então não precisa nem
   * continuar.
   */
  if (!deslocamento)
    {
      return 0;
    }

  /* Se 'string' for NULL então não há condições de prosseguir . */
  if (!string)
    {
      return 1;
    }

  /* Se a posição final for menor que a posição inicial
   * da parte a ser deslocada, então o programa assume
   * como posição final o fim da string 'string'. Se mesmo
   * assim, posicao final for menor que posicao inicial,
   * então é melhor terminar retornando diferente de 0 (zero),
   * o que indica que algo deu errado.
   */
  if (posicao_final_bloco_deslocar < posicao_inicial_bloco_deslocar)
    {
      posicao_final_bloco_deslocar = STR_tamanho(string);

      /* Se 'posicao_final_bloco_deslocar' ainda for menor que
       * 'posicao_inicial_bloco_deslocar' então o melhor a fazer é terminar
       * a função retornando diferente de 0 (zero).
       */
      if (posicao_final_bloco_deslocar < posicao_inicial_bloco_deslocar)
	{
	  return 1;
	}
    }

  /* ###### Parâmetros OK, pode continuar... ###### */

  /* 'posicao_origem' é inicializado para o endereço do primeiro caracter
   * do bloco que se quer deslocar.
   * 'posicao_destino' é inicializado para o endereço do caracter que está
   * na deslocado de 'deslocamento' em relação ao primeiro caracter do bloco
   * que se quer deslocar.
   *
   * Note que esta função funciona para valores negativos para as variáveis
   * 'deslocamento', 'posicao_inicial_bloco_deslocar' e
   * 'posicao_final_bloco_deslocar'.
   */
  posicao_origem = &(string[posicao_inicial_bloco_deslocar]);
  posicao_destino = &(string[posicao_inicial_bloco_deslocar + deslocamento]);

  /* Armazena em 'numero_caracteres_deslocar' o número total de caracteres que
   * terão de ser deslocados.
   */
  numero_caracteres_deslocar = (posicao_final_bloco_deslocar
				- posicao_inicial_bloco_deslocar
				+ 1);

  if (deslocamento > 0)
    {
      /* Neste caso o deslocamento é positivo (para a direita). */

      /* No caso do deslocamento ser positivo (para a direita),
       * é melhor fazer o deslocamento da direita para a esquerda,
       * a fim de evitar problemas devido a sobreposição de
       * caracteres que estão sendo deslocados sobre caracteres que terão
       * de ser deslocados e ainda não foram.
       */

      /* Esta aritmética com ponteiros é válida em C.
       * Mas não brinque com isso se não entender bem o que se passa aqui!
       */
      posicao_origem += numero_caracteres_deslocar - 1;
      posicao_destino += numero_caracteres_deslocar - 1;

      /* Vai decrementando 'numero_caracteres_deslocar' pois a cada
       * iteração deste loop 'for', um caracter é deslocado.
       * Este 'for' acaba quando 'numero_caracteres_deslocar' for 0 (zero).
       */
      for (; numero_caracteres_deslocar; numero_caracteres_deslocar--)
	{
	  /* Como estamos deslocando da direita para a esquerda,
	   * os apontadores são decrementados a cada iteração.
	   */
	  /* O caracter que é apontado por 'posicao_origem' é copiado
	   * para a posição 'posicao_destino', e ambos os apontadores
	   * são decrementados.
	   */
	  *posicao_destino-- = *posicao_origem--;
	}
    }
  else
    {
      /* Neste caso o deslocamento é negativo (para a esquerda). */

      /* Vai decrementando 'numero_caracteres_deslocar' pois a cada
       * iteração deste loop 'for', um caracter é deslocado.
       * Este 'for' acaba quando 'numero_caracteres_deslocar' for 0 (zero).
       */
      for (; numero_caracteres_deslocar; numero_caracteres_deslocar--)
	{
	  /* Comom estamos deslocando da esquerda para a direita, os
	   * apontadores são incrementados a cada iteração.
	   */
	  /* O caracter que é apontado por 'posicao_origem' é copiado
	   * para a posição 'posicao_destino', e ambos os apontadores
	   * são incrementados até que todo o bloco tenha sido deslocado.
	   */
	  *posicao_destino++ = *posicao_origem++;
	}
    }

  /* Função executou com sucesso! Retorna 0 (zero). */
  return 0;
}

/* Remove os espacos em branco do começo e final de `string'.
 * Se `string' consistir de espaços em branco apenas, retorna NULL.
 * Atenção: `string' poderá ter seu conteúdo alterado.
 */
char *trim(char *string)
{
  int i = 0;
  int tamanho_string = 0;
  int encontrado_caracter_nao_espaco = 0;
  int posicao_primeiro_caracter_nao_espaco = 0;

  if (!string)
    {
      return NULL;
    }

  /* Obtém o tamanho de 'string'. */
  tamanho_string = STR_tamanho(string);

  for(i = 0; i < tamanho_string; i++)
    {
      if (!isspace((int)string[i]))
	{
	  int j;
	  /* Encontrou caracter que não seja espaço. */
	  encontrado_caracter_nao_espaco = 1;
	  /* Guarda a posição deste caracter */
	  posicao_primeiro_caracter_nao_espaco = i;

	  /*O laço 'for' a seguir garante que no final da string não haja
	   * nenhum caracter de espaço.
	   */
	  for (j = (tamanho_string - 1); isspace((int)string[j]); j--);

	  string[j + 1] = '\0';

	  /* O laço for inicial deve ser interrompido, pois um caracter
	   * não espaço já foi encontrado.
	   */
	  break;
	}
    }

  /* Se a string for constituída inteiramente por espaços, retorna NULL */
  if (!encontrado_caracter_nao_espaco)
    {
      return NULL;
    }

  /* Se havia caracteres espaço no começo da string... */
  if (posicao_primeiro_caracter_nao_espaco)
    {
      /* Desloca 'string' para a esquerda, para remover os espaços iniciais. */
      if (STR_desloca_bloco(string, \
			    posicao_primeiro_caracter_nao_espaco, \
			    tamanho_string, \
			    (posicao_primeiro_caracter_nao_espaco * (-1))))
	  {
	    /* Ocorreu erro em STR_desloca_bloco(). */
	  }
    }

  return string;
}

//concatena em um buff de 256
char* concatOnbuff(char* str1, char* str2) {

	strcat(concatBuff, str1);
	strcat(concatBuff, str2);

	return concatBuff;
}



