package central_compartilhamento;

import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class Agente extends Thread {

	private Socket sock;
	private int sessao;
	private int chave;
    private String[] dados;
    private String[] frente;
    private String[] fundo;
    private String[] atributos;
    private Socket sockSuporte;    
    private List<String> pendentes;

	public Agente(Socket sock, int sessao, int chave) {
		this.sock = sock;
		this.sessao = sessao;
		this.chave = chave;
		pendentes = new ArrayList<String>();
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
				String cabec = new String(buf, 0, n);
				String cmd = cabec.substring(0, 10).trim();
//				System.err.println(cmd);
				int sessao = Integer.parseInt(cabec.substring(10, 14));
				int chave = Integer.parseInt(cabec.substring(14, 18));
			
				StringBuilder sb = new StringBuilder();
				while (true) {
					int c = sock.getInputStream().read();
					if (c == '\n') {
						break;
					}
					sb.append((char) c);
				}
				String s = sb.toString();
			
				if (sessao != this.sessao) {
					return;
				}
			
				if (chave != this.chave) {
					return;
				}
			
				if (cmd.equals("STOP")) {
					sock.close();
					synchronized (Central.getAgentes()) {
						Central.getAgentes().remove(sessao);
					}
					return;
				}
				
				if (cmd.equals("DADOS")) {
					System.out.println("<" + new Date().getTime() + " " + s);
					int i = Integer.parseInt(s.substring(0, 2));
					dados[i] = s.substring(2);
				}
				
				if (cmd.equals("DADOSR")) {
					System.out.println("=" + new Date().getTime() + " " + s);
					s = cabec.substring(0, 10) + s + "\n";
					if (sockSuporte != null) {
						sockSuporte.getOutputStream().write(s.getBytes());
					} else {
						pendentes.add(s);
					}
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

	public int getChave() {
		return chave;
	}

	public Socket getSock() {
		return sock;
	}

	public String[] getDados() {
		return dados;
	}

	public String[] getFrente() {
		return frente;
	}

	public String[] getFundo() {
		return fundo;
	}

	public String[] getAtributos() {
		return atributos;
	}

	public Socket getSockSuporte() {
		return sockSuporte;
	}

	public void setSockSuporte(Socket sockSuporte) {
		this.sockSuporte = sockSuporte;
		try {
			for (String cmd : pendentes) {
				if (cmd.startsWith("DADOSR")) {
					System.out.print("pendente>" + new Date().getTime() + " " + cmd);
				}
				sockSuporte.getOutputStream().write(cmd.getBytes());
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

}
