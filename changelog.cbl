       IDENTIFICATION DIVISION.
       PROGRAM-ID. changelog.

       DATA DIVISION.

       WORKING-STORAGE SECTION.

       PROCEDURE DIVISION.
           call 'create_changelog_table'

      *    adicionar tabelas aqui     
           call 'add_changelog_trigger' using "materiais ", "sp01a04 "
           call 'add_changelog_trigger' using "basicos ", "sp01a03 "
           stop run.
