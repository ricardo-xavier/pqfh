package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.net.Socket;
import java.util.Date;
import java.util.Random;

public class Compartilhamento {

	public static void main(String[] args) {

		char dados[][] = new char[2][2];
		dados[0][0] = 'a';
		dados[0][1] = 'b';
		dados[1][0] = 'c';
		dados[1][1] = 'd';
		System.out.println(String.valueOf(dados));
	}

	public static boolean compartilha()  {

		try {

			// gera uma chave aleatória
			Random r = new Random(new Date().getTime());
			int chave = r.nextInt(9999);

			// conecta
			String host = TerminalAvanco.getCfg().getServidorCompartilhamento();
			int porta = TerminalAvanco.getCfg().getPortaCompartilhamento();
			Socket sock = new Socket(host, porta);

			// envia a chave
			System.err.println("chave: " + chave);
			sock.getOutputStream().write(String.format("%04d", chave).getBytes());

			// recebe a sessão
			byte[] buf = new byte[4];
			sock.getInputStream().read(buf, 0, 4);
			String s = new String(buf);
			int sessao = Integer.parseInt(s);
			System.err.println("conectado: " + sessao);

			Terminal terminal = TerminalAvanco.getTerminal();
			char[][] dados = terminal.getDados();
			char[][] frente = terminal.getFrente();
			char[][] fundo = terminal.getFundo();
			int[][] atributos = terminal.getAtributos();

			// envia a tela atual
			for (int lin=0; lin < Terminal.getLinhas(); lin++) {

				String cmd = String.format("%-10s%04d%04d%02d%s%n", 
						"DADOS", sessao, chave, lin, new String(dados[lin]));
				System.err.print(cmd);
				sock.getOutputStream().write(cmd.getBytes());

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

			sock.close();
			return true;

		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

}
