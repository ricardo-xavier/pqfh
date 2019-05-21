update materiais.sp01a04 set k0 = lpad(cast(sp0104loja as char(2)), 2, '0') || lpad(cast(sp0104chave as char(6)), 6, '0');
update materiais.sp01a04 set k1 = lpad(cast(sp0104chave as char(6)), 6, '0') || lpad(cast(sp0104loja as char(2)), 2, '0');
select count(*) from materiais.sp01a04 where sp0104loja = 4;
