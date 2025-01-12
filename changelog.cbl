       IDENTIFICATION DIVISION.
       PROGRAM-ID. changelog.

       DATA DIVISION.

       WORKING-STORAGE SECTION.

       PROCEDURE DIVISION.
           call 'create_changelog_function'
           call 'create_changelog_table'

      *    adicionar tabelas aqui     
           call 'add_changelog_trigger' using "materiais", "sp01a04"
           stop run.
