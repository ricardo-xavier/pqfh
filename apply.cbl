      $  set callfh"pqfh"
       IDENTIFICATION DIVISION.
       PROGRAM-ID. apply.


       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
       copy sp01a03.sel.
      *copy sp01a04.sel.

       DATA DIVISION.
       FILE SECTION.
       copy sp01a03.fd.
      *copy sp01a04.fd.

       WORKING-STORAGE SECTION.
       77      ws-sp01a03     pic x(40) value '../../arq/sp01a03'.
       77      status01-a03   pic xx.
       77      ws-sp01a04     pic x(40) value '../../arq/sp01a04'.
       77      status01-a04   pic xx.

       PROCEDURE DIVISION.
           open i-o sp01a03
           call 'apply_changelog'
           close sp01a03

           stop run.
