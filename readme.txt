https://www.postgresql.org/docs/9.4/libpq-exec.html

update materiais.sp01a04 set sp0104key0 = lpad(cast(sp0104loja as char(2)), 2, '0') || lpad(cast(sp0104chave as char(6)), 6, '0');
update materiais.sp01a04 set sp0104key1 = lpad(cast(sp0104chave as char(6)), 6, '0') || lpad(cast(sp0104loja as char(2)), 2, '0');
select count(*) from materiais.sp01a04 where sp0104loja = 4;
