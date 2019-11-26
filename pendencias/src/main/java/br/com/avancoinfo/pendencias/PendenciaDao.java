package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.sql.PreparedStatement;
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
			
			String sql = "select chave_busca,data_emissao,data_inclusao,numero_cupom,numero_nota,serie,tipo_pendencia,codigo_situacao,processada,cancelada,inutilizada "
					+ "from pen_pendencias "
					+ "order by data_emissao desc";
			
			Statement stmt = conn.createStatement();
			ResultSet cursor = stmt.executeQuery(sql);
			
			DateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
			
			while (cursor.next()) {
				
				String chaveBusca = cursor.getString("chave_busca");
				Timestamp dataEmissaoSql = cursor.getTimestamp("data_emissao");
				Date dataEmissao = new Date(dataEmissaoSql.getTime());
				Timestamp dataInclusaoSql = cursor.getTimestamp("data_inclusao");
				Date dataInclusao = new Date(dataInclusaoSql.getTime());				
				String data = df.format(dataEmissao);
				String cupom = cursor.getString("numero_cupom");
				String nota = cursor.getString("numero_nota");
				String serie = cursor.getString("serie");
				String tipo = cursor.getString("tipo_pendencia");
				String situacao = cursor.getString("codigo_situacao");
				
				int processada = cursor.getInt("processada");
				int cancelada = cursor.getInt("cancelada");
				int inutilizada = cursor.getInt("inutilizada");
				String status = "";
				if (processada == 1) {
					status = "Processada";
				} else if (cancelada == 1) {
					status = "Cancelada";
				} else if (inutilizada == 1) {
					status = "Inutilizada";
				}
				
				Pendencia pendencia = new Pendencia(chaveBusca, data, cupom, nota, serie, tipo, situacao, status, dataInclusao);
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
					+ "from pen_pendencias "
					+ "where chave_busca = '" + chave + "'";
			
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

	public static List<Log> log(Connection conn, Date dataInclusao) {
		
		List<Log> logs = new ArrayList<Log>();
		
		try {
			
			String sql = "select descricao "
					+ "from pen_logs "
					+ "where data=? ";
			
			PreparedStatement stmt = conn.prepareStatement(sql);
			System.out.println(dataInclusao);
			stmt.setTimestamp(1, new java.sql.Timestamp(dataInclusao.getTime()));
			ResultSet cursor = stmt.executeQuery();
			
			DateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
			
			while (cursor.next()) {
				
				String descricao = cursor.getString("descricao");
				
				Log log = new Log(df.format(dataInclusao), descricao);
				logs.add(log);
				
			}
			
			cursor.close();
			stmt.close();
			
		} catch (SQLException e) {
			e.printStackTrace();
		}
		
		return logs;
	}

}
