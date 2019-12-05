package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/*
     Coluna      |            Tipo             | Collation | Nullable | Default 
-----------------+-----------------------------+-----------+----------+---------
 tipo_pendencia  | numeric(1,0)                |           |          | 
 data_emissao    | timestamp without time zone |           |          | 
 data_inclusao   | timestamp without time zone |           |          | 
 chave_busca     | character varying(100)      |           | not null | 
 cupom           | text                        |           |          | 
 numero_cupom    | numeric(10,0)               |           |          | 
 numero_nota     | numeric(10,0)               |           |          | 
 serie           | numeric(3,0)                |           |          | 
 nfce_substituta | character varying(44)       |           |          | 
 chave_acesso    | character varying(44)       |           |          | 
 xml             | text                        |           |          | 
 codigo_situacao | smallint                    |           |          | 
 situacao        | text                        |           |          | 
 processada      | numeric(1,0)                |           |          | 
 cancelada       | numeric(1,0)                |           |          | 
 inutilizada     | numeric(1,0)                |           |          | 
Índices:
    "pk_pen_chavebusca" PRIMARY KEY, btree (chave_busca)
    
    
  Coluna   |            Tipo             | Collation | Nullable | Default 
-----------+-----------------------------+-----------+----------+---------
 data      | timestamp without time zone |           |          | 
 descricao | text                        |           |          | 


*/

public class BancoDados {
	
	private static String url;
	
	public static Connection conecta() throws ClassNotFoundException, SQLException {
		url = "jdbc:postgresql://192.168.0.218:5432/bd_pendencias?user=postgres";
		String env = System.getenv("BANCO_PENDENCIAS");
		if (env != null) {
			url = env;
		}
		String usuario = System.getenv("USUARIO_BANCO");
		String senha = System.getenv("SENHA_BANCO");
		if (usuario != null) {
			return conecta(url, usuario, senha);
		}
		if (url.contains("?")) {
			return conecta(url);
		} else {
			return conecta(url, "postgres", "postgres");
		}
	}
	
	public static Connection conecta(String url) throws ClassNotFoundException, SQLException {
		String driver = "org.postgresql.Driver";
		Class.forName(driver);
		return DriverManager.getConnection(url);
	}	
	
	public static Connection conecta(String url, String usuario, String senha) throws ClassNotFoundException, SQLException {
		String driver = "org.postgresql.Driver";
		Class.forName(driver);
		return DriverManager.getConnection(url, usuario, senha);
	}	
	
	public static void main(String[] args) throws ClassNotFoundException, SQLException {
		Connection conn;
		if (args.length == 0) {
			conn = BancoDados.conecta();
		} else {
			conn = BancoDados.conecta(args[0]);
		}
		
		String sql = "select table_name from information_schema.tables";
		
		Statement stmt = conn.createStatement();
		ResultSet cursor = stmt.executeQuery(sql);
		
		while (cursor.next()) {
			String name = cursor.getString("table_name");
			System.out.println(name);
		}
		
		cursor.close();
		stmt.close();
		//PendenciaDao.list(conn);
		conn.close();
	}
	
	public static String getIp() {
		int p1 = url.indexOf("//");
		int p2 = url.indexOf(":", p1+2);
		String ip = url.substring(p1+2, p2);
		return ip;
	}

	public static String getUrl() {
		return url;
	}

	public static void setUrl(String url) {
		BancoDados.url = url;
	}

	public static Connection reconecta(Connection conn, String ip) throws SQLException, ClassNotFoundException {
		int p1 = url.indexOf("//");
		int p2 = url.indexOf(":", p1+2);
		url = url.substring(0, p1+2) + ip + url.substring(p2);
		System.out.println(url);
		conn.close();
		return conecta(url);
	}

}
