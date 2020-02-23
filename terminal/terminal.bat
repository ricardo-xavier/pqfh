@echo off
set CENTRAL_AVANCO=avancoinfo.ddns.com.br:65531
echo Verificando atualizacoes. Aguarde...
java -cp atualizador.jar br.com.avancoinfo.atualizador.Atualizador terminal > jar.tmp
set /p JAR= < jar.tmp
set CLASSPATH=%JAR%;%CLASSPATH%;
echo Executando %JAR%...
java br.com.avancoinfo.terminal.TerminalAvanco