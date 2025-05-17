      $ set callfh"pqfh"
       IDENTIFICATION DIVISION.
       PROGRAM-ID.    list.
       ENVIRONMENT    DIVISION.
       CONFIGURATION  SECTION.
       SPECIAL-NAMES.
           DECIMAL-POINT IS COMMA.
       INPUT-OUTPUT   SECTION.
       FILE-CONTROL.
       copy sp01a03.sel.
       DATA DIVISION.
       FILE SECTION.
       copy sp01a03.fd.
       WORKING-STORAGE SECTION.
       77      ws-sp01a03     pic x(40) value '../../arq/sp01a03'.
       77      status01-a03   pic xx.
       PROCEDURE DIVISION.
       INICIO.
           open input sp01a03
           move 'TESTE-CHANGELOG' to sp0103desc 
           move 'CERVEJA LTA ANTARC 350ML'
                to sp0103desc
           move zeros to sp0103chave
           start sp01a03 key is >= sp0103key3
           read sp01a03 next ignore lock
           perform until status01-a03 <> '00'  
              or sp0103desc <> 'CERVEJA LTA ANTARC 350ML'
      *       or sp0103desc <> 'TESTE-CHANGELOG'     
              display sp0103chave
              read sp01a03 next ignore lock
           end-perform           
           close sp01a03
           stop run.
