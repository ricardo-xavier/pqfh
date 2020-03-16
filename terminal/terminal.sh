export JAVA_HOME=/home/ricardo/jdk1.8.0_231
PATH=$JAVA_HOME/bin:$PATH
export CENTRAL_AVANCO=avancoinfo.ddns.com.br:65531
echo "Verificando atualizacoes. Aguarde..."
JAR=`java -cp atualizador.jar br.com.avancoinfo.atualizador.Atualizador terminal`
if [ $? == 0 ]
then
        export CLASSPATH=$JAR:$CLASSPATH
        echo "Executando $JAR..."
        java br.com.avancoinfo.terminal.TerminalAvanco
fi