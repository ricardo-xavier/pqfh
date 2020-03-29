package central_compartilhamento;

import java.io.IOException;
import java.net.Socket;

public class Agente extends Thread {

	private Socket sock;
	private int sessao;
	private int chave;
    private String[] dados;
    private String[] frente;
    private String[] fundo;
    private String[] atributos;

	public Agente(Socket sock, int sessao, int chave) {
		this.sock = sock;
		this.sessao = sessao;
		this.chave = chave;
	}

	@Override
	public void run() {

		try {

			dados = new String[25];
			frente = new String[25];
			fundo = new String[25];
			atributos = new String[25];

			while (true) {

				// COMANDO SESSAO CHAVE DADOS  \n
				// 10      4      4     (8192)
				byte[] buf = new byte[8192];
				int n = sock.getInputStream().read(buf, 0, 18);
				if (n != 18) {
					return;
				}
				String s = new String(buf);
				System.out.println(s);
				String cmd = s.substring(0, 10).trim();
				int sessao = Integer.parseInt(s.substring(10, 14));
				int chave = Integer.parseInt(s.substring(14, 18));
			
				StringBuilder sb = new StringBuilder();
				while (true) {
					int c = sock.getInputStream().read();
					if (c == '\n') {
						break;
					}
					sb.append((char) c);
				}
				s = sb.toString();
				System.out.println(s);
			
				if (sessao != this.sessao) {
					return;
				}
			
				if (chave != this.chave) {
					return;
				}
			
				if (cmd.equals("STOP")) {
					return;
				}
				
				if (cmd.equals("DADOS")) {
					int i = Integer.parseInt(s.substring(0, 2));
					dados[i] = s.substring(2);
				}
				
				if (cmd.equals("FRENTE")) {
					int i = Integer.parseInt(s.substring(0, 2));
					frente[i] = s.substring(2);
				}
				
				if (cmd.equals("FUNDO")) {
					int i = Integer.parseInt(s.substring(0, 2));
					fundo[i] = s.substring(2);
				}
				
				if (cmd.equals("ATRIBUTOS")) {
					int i = Integer.parseInt(s.substring(0, 2));
					atributos[i] = s.substring(2);
				}
				
			}

		} catch (IOException e) {
			e.printStackTrace();
		}

	}

}
