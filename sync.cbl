      $  set callfh"pqfh"
       IDENTIFICATION DIVISION.
       PROGRAM-ID. synchronize.


       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
       copy sp01a03.sel.
       copy sp01a04.sel.

       DATA DIVISION.
       FILE SECTION.
       copy sp01a03.fd.
       copy sp01a04.fd.

       WORKING-STORAGE SECTION.
       77      ws-sp01a03     pic x(40) value '../../arq/sp01a03'.
       77      status01-a03   pic xx.
       77      ws-sp01a04     pic x(40) value '../../arq/sp01a04'.
       77      status01-a04   pic xx.
       77      status04-a10   pic xx.
       77      status04-a12   pic xx.
       77      status04-d12   pic xx.

       PROCEDURE DIVISION.
      * Inicializa o sincronismo     
           call 'sync_start'

      * Abre, adiciona e fecha as tabelas que serao sincronizadas     
      * Essas tabelas precisam ter triggers no banco(ver changelog.cbl)
           open i-o sp01a03
           call 'sync_add_table'
           close sp01a03
           open i-o sp01a04
           call 'sync_add_table'
           close sp01a04

      * Fica em loop sincronizando as tabelas     
           call 'sync_execute'
           stop run.
