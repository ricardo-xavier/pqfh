package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.util.Date;
import java.util.Random;

import com.sun.javafx.geom.Rectangle;

import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;

public class Compartilhamento {
	
	private static Socket sock;
	private static int sessao;
	private static int chave;

	public static void main(String[] args) {

		char dados[][] = new char[2][2];
		dados[0][0] = 'a';
		dados[0][1] = 'b';
		dados[1][0] = 'c';
		dados[1][1] = 'd';
		System.out.println(String.valueOf(dados));
	}

	public static String compartilha()  {

		try {

			// gera uma chave aleatória
			Random r = new Random(new Date().getTime());
			chave = r.nextInt(9999);

			// conecta
			String host = TerminalAvanco.getCfg().getServidorCompartilhamento();
			int porta = TerminalAvanco.getCfg().getPortaCompartilhamento();
			sock = new Socket(host, porta);

			// envia a chave
			System.err.println("chave: " + chave);
			sock.getOutputStream().write(String.format("C%04d", chave).getBytes());
			
			// recebe a sessão
			byte[] buf = new byte[4];
			sock.getInputStream().read(buf, 0, 4);
			String s = new String(buf);
			sessao = Integer.parseInt(s);
			System.err.println("conectado: " + sessao);
			
			Alert alert = new Alert(AlertType.INFORMATION);
			alert.setTitle("Informação");
			alert.setHeaderText("Chave para acesso ao compartilhamento:");
			String chaveCompartilhamento = String.format("%04d%04d", sessao, chave);
			alert.setContentText(chaveCompartilhamento);
			alert.show();					

			Terminal terminal = TerminalAvanco.getTerminal();
			char[][] dados = terminal.getDados();
			char[][] frente = terminal.getFrente();
			char[][] fundo = terminal.getFundo();
			int[][] atributos = terminal.getAtributos();

			// envia a tela atual
			for (int lin=0; lin < Terminal.getLinhas(); lin++) {

				String cmd = String.format("%-10s%04d%04d%02d%s%n", 
						"DADOS", sessao, chave, lin, new String(dados[lin]));
				System.out.print(">" + new Date().getTime() + " " + cmd);
				sock.getOutputStream().write(cmd.getBytes("ISO-8859-1"));

				cmd = String.format("%-10s%04d%04d%02d%s%n", 
						"FRENTE", sessao, chave, lin, new String(frente[lin]));
				sock.getOutputStream().write(cmd.getBytes());

				cmd = String.format("%-10s%04d%04d%02d%s%n", 
						"FUNDO", sessao, chave, lin, new String(fundo[lin]));
				sock.getOutputStream().write(cmd.getBytes());

				StringBuilder atrs = new StringBuilder();
				for (int j=0; j<80; j++) {
					atrs.append(String.format("%02d", atributos[lin][j]));
				}
				cmd = String.format("%-10s%04d%04d%02d%s%n", 
						"ATRIBUTOS", sessao, chave, lin, atrs.toString());
				sock.getOutputStream().write(cmd.getBytes());				

			}

			return chaveCompartilhamento;

		} catch (IOException e) {
			e.printStackTrace();
			Alert alert = new Alert(AlertType.ERROR);
			alert.setTitle("Erro");
			alert.setHeaderText("Erro na comunicação com o servidor de compartilhamento");
			alert.setContentText("Verifique se a configuração está correta e se o servidor está ativo");
			alert.showAndWait();		
		}

		return null;
	}

	public static void fecha() {

		if (sock == null) {
			return;
		}
		
		try {
			String cmd = String.format("%-10s%04d%04d%n", "STOP", sessao, chave); 
			System.err.print(cmd);
			sock.getOutputStream().write(cmd.getBytes());
			sock.close();
			
		} catch (IOException e) {
			e.printStackTrace();
		}
		sock = null;
		
	}

	public static Socket getSock() {
		return sock;
	}

	public static void atualiza() throws UnsupportedEncodingException, IOException {

		if (sock == null) {
			return;
		}
		
		Terminal terminal = TerminalAvanco.getTerminal();
		char[][] dados = terminal.getDados();
		char[][] frente = terminal.getFrente();
		char[][] fundo = terminal.getFundo();
		int[][] atributos = terminal.getAtributos();
		Rectangle r = terminal.getR();
		
		int y1 = r.y;
		int y2 = r.y + r.height - 1;
		int x1 = r.x;
		int x2 = r.x + r.width - 1;

		// envia a tela atual
		for (int lin=y1; lin <=y2; lin++) {

			String cmd = String.format("%-10s%04d%04d%02d%02d%02d%s%n", 
					"DADOSR", sessao, chave, lin, x1, x2, new String(dados[lin]).substring(x1, x2+1));
			System.out.print(">" + new Date().getTime() + " " + cmd);
			sock.getOutputStream().write(cmd.getBytes("ISO-8859-1"));

			cmd = String.format("%-10s%04d%04d%02d%02d%02d%s%n", 
					"FRENTER", sessao, chave, lin, x1, x2, new String(frente[lin]).substring(x1, x2+1));
			sock.getOutputStream().write(cmd.getBytes());

			cmd = String.format("%-10s%04d%04d%02d%02d%02d%s%n", 
					"FUNDOR", sessao, chave, lin, x1, x2, new String(fundo[lin]).substring(x1, x2+1));
			sock.getOutputStream().write(cmd.getBytes());

			StringBuilder atrs = new StringBuilder();
			for (int j=x1; j<x2; j++) {
				atrs.append(String.format("%02d", x1, x2, atributos[lin][j]));
			}
			cmd = String.format("%-10s%04d%04d%02d%02d%02d%s%n", 
					"ATRIBUTOSR", sessao, chave, lin, x1, x2, atrs.toString());
			sock.getOutputStream().write(cmd.getBytes());				

		}

		
	}

}
