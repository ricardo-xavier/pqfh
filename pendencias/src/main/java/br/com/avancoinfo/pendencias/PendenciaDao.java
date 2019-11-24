package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class PendenciaDao {
	
	public static List<Pendencia> list(Connection conn) {
		
		List<Pendencia> pendencias = new ArrayList<Pendencia>();
		
		try {
			
			String sql = "select chave_busca, data_emissao,numero_cupom,numero_nota,serie,tipo_pendencia "
					+ "from pen_pendencias order by data_emissao desc";
			
			Statement stmt = conn.createStatement();
			ResultSet cursor = stmt.executeQuery(sql);
			
			DateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
			
			while (cursor.next()) {
				
				String chaveBusca = cursor.getString("chave_busca");
				
				Timestamp dataEmissaoSql = cursor.getTimestamp("data_emissao");
				Date dataEmissao = new Date(dataEmissaoSql.getTime());
				String data = df.format(dataEmissao);
				
				String cupom = cursor.getString("numero_cupom");
				String nota = cursor.getString("numero_nota");
				String serie = cursor.getString("serie");
				
				//TODO traduzir o tipo de pendencia
				
				System.out.printf("%s %s %s %s%n", data, cupom, nota, serie);
				Pendencia pendencia = new Pendencia(chaveBusca, data, cupom, nota, serie, "");
				pendencias.add(pendencia);
				
			}
			
			cursor.close();
			stmt.close();
			
		} catch (SQLException e) {
			e.printStackTrace();
		}
		
		return pendencias;
		
	}
	
	public static Detalhes getDetalhes(Connection conn, String chave) {
		
		Detalhes detalhes = new Detalhes();
		
		try {
			
			String sql = "select chave_acesso, nfce_substituta, convert_from(decode(xml, 'base64'), 'UTF-8') as xml "
					+ "from pen_pendencias where chave_busca = '" + chave + "'";
			
			Statement stmt = conn.createStatement();
			ResultSet cursor = stmt.executeQuery(sql);
			
			while (cursor.next()) {
				
				String chaveAcesso = cursor.getString("chave_acesso");
				String nfceSubstituta = cursor.getString("nfce_substituta");
				String xml = cursor.getString("xml");
				detalhes.setChaveAcesso(chaveAcesso);
				detalhes.setNfceSubstituta(nfceSubstituta);
				detalhes.setXml(xml);
				
			}
			
			cursor.close();
			stmt.close();
			
		} catch (SQLException e) {
			e.printStackTrace();
		}
		
		return detalhes;

	}

}
