package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

public class PendenciaDao {
	
	public static List<Pendencia> list(Connection conn,
			boolean processadasTodas,
			boolean processadasSim,
			boolean processadasNao,
			boolean canceladasTodas,
			boolean canceladasSim,
			boolean canceladasNao,
			boolean inutilizadasTodas,
			boolean inutilizadasSim,
			boolean inutilizadasNao,
			boolean autorizadasTodas,
			boolean autorizadasSim,
			boolean autorizadasNao,			
			LocalDate emissaoIni,
			LocalDate emissaoFim,
			String cnpj) {
		
		List<Pendencia> pendencias = new ArrayList<Pendencia>();
		
		try {
			
			String where = "";
			
			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd"); 
			
			if (emissaoIni != null) {
				if (where.equals("")) {
					where = String.format("where data_emissao >= '%s'", formatter.format(emissaoIni));
				} else {
					where += String.format(" and data_emissao >= '%s'", formatter.format(emissaoIni));
				}
			}
			
			if (emissaoFim != null) {
				if (where.equals("")) {
					where = String.format("where data_emissao <= '%s'", formatter.format(emissaoFim));
				} else {
					where += String.format(" and data_emissao <= '%s'", formatter.format(emissaoFim));
				}
			}
			
			if (!processadasTodas) {
				if (where.equals("")) {
					where = "where processada = ";
				} else {
					where += " and processada = ";
				}
				where += processadasSim ? "1" : "0";
			}
			
			if (!canceladasTodas) {
				if (where.equals("")) {
					where = "where cancelada = ";
				} else {
					where += " and cancelada = ";
				}
				where += canceladasSim ? "1" : "0";
			}
			
			if (!inutilizadasTodas) {
				if (where.equals("")) {
					where = "where inutilizada = ";
				} else {
					where += " and inutilizada = ";
				}
				where += inutilizadasSim ? "1" : "0";
			}
			
			if (!autorizadasTodas) {
				if (where.equals("")) {
					where = "where codigo_situacao ";
				} else {
					where += " and codigo_situacao ";
				}
				where += autorizadasSim ? "= 100" : "!= 100";
			}
			
			if (cnpj != null) {
				if (where.equals("")) {
					where = "where chave_acesso like ";
				} else {
					where += " and chave_acesso like ";
				}
				where += "'%" + cnpj.trim() + "%'";
			}
			
			String sql = "select chave_busca,data_emissao,data_inclusao,numero_cupom,numero_nota,serie,tipo_pendencia,codigo_situacao,situacao,processada,cancelada,inutilizada "
					+ "from pen_pendencias "
					+ where 
					+ " order by data_emissao desc limit 100";
			
			Statement stmt = conn.createStatement();
			ResultSet cursor = stmt.executeQuery(sql);
			
			DateFormat df = new SimpleDateFormat("dd/MM/yyyy HH:mm:ss");
			
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
				if (tipo.equals("1")) {
					tipo = "Contingência";
				} else if (tipo.equals("0")) {
					tipo = "Chaves pendentes";
				}
				String situacao = cursor.getString("codigo_situacao");
				String descricao = cursor.getString("situacao");
				switch (situacao) {
				case "100":
					descricao = "Autorizada";
					break;
				case "135":
					descricao = "Cancelada";
					break;
				case "150":
					descricao = "Autorizada fora do prazo";
					break;
				default:
					if ((descricao != null) && (descricao.trim().length() > 30)) {
						descricao = descricao.substring(0, 30);
					}
					break;
				}
				
				boolean processada = cursor.getInt("processada") == 1;
				boolean cancelada = cursor.getInt("cancelada") == 1;
				boolean inutilizada = cursor.getInt("inutilizada") == 1;
				
				Pendencia pendencia = new Pendencia(chaveBusca, data, cupom, nota, serie, tipo, situacao, descricao, 
						processada, cancelada, inutilizada, dataInclusao);
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
			
			String sql = "select chave_acesso, nfce_substituta, convert_from(decode(xml, 'base64'), 'UTF-8') as xml, situacao "
					+ "from pen_pendencias "
					+ "where chave_busca = '" + chave + "'";
			
			Statement stmt = conn.createStatement();
			ResultSet cursor = stmt.executeQuery(sql);
			
			while (cursor.next()) {
				
				String chaveAcesso = cursor.getString("chave_acesso");
				String nfceSubstituta = cursor.getString("nfce_substituta");
				String xml = cursor.getString("xml");
				String situacao = cursor.getString("situacao");
				
				if ((situacao != null) && situacao.trim().startsWith("{") && situacao.trim().endsWith("}")) {
					Gson gson = new GsonBuilder().setPrettyPrinting().create();
					JsonParser jp = new JsonParser();
					JsonElement je = jp.parse(situacao);
					situacao = gson.toJson(je);					
				}
				
				detalhes.setChaveAcesso(chaveAcesso);
				detalhes.setNfceSubstituta(nfceSubstituta);
				detalhes.setXml(xml);
				detalhes.setSituacao(situacao);
				
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
