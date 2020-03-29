package central_compartilhamento;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

public class Central {
	
	private static Map<Integer, Agente> agentes = new HashMap<Integer, Agente>();
	
	public static void main(String[] args) throws IOException {
		
		int porta = Integer.parseInt(args[0]);
		@SuppressWarnings("resource")
		ServerSocket serverSock = new ServerSocket(porta);
		
		while (true) {
			
			// recebe uma conexão
			Socket sock = serverSock.accept();
			System.out.println("central:accept: " + sock.getRemoteSocketAddress() + " " + new Date());
			
			// recebe a origem
			char origem = (char) sock.getInputStream().read();
			
			if (origem == 'C') {
				
				// o cliente iniciou o compartilhamento
			
				// recebe a chave
				byte[] buf = new byte[4];
				sock.getInputStream().read(buf, 0, 4);
				String s = new String(buf);
				int chave = Integer.parseInt(s);
				System.out.println("central:chave: " + chave);
			
				// envia a sessão
				int sessao = agentes.size() + 1;
				System.out.println("central:sessao: " + sessao);			
				sock.getOutputStream().write(String.format("%04d", sessao).getBytes());

				// cria um agente para tratar a sessão
				Agente agente = new Agente(sock, sessao, chave);
				agente.start();
				
				synchronized (agentes) {
					agentes.put(sessao, agente);	
				}
				
			} else {
				
				// o suporte se conectou ao compartilhamento 'S'
				
				// recebe a sessão e a chave
				byte[] buf = new byte[8];
				sock.getInputStream().read(buf, 0, 8);
				String s = new String(buf);
				int sessao = Integer.parseInt(s.substring(0, 4));
				System.out.println("central:suporte:sessao: " + sessao);
				
				// recupera o agente
				Agente agente = agentes.get(sessao);
				if (agente == null) {
					sock.getOutputStream().write("ERRO:Sessao invalida\n".getBytes());
					sock.close();
					continue;
				}
				
				// verifica a chave
				int chave = Integer.parseInt(s.substring(4));
				System.out.println("central:suporte:chave: " + chave);
				if (chave != agente.getChave()) {
					sock.getOutputStream().write("ERRO:Chave invalida\n".getBytes());
					sock.close();
					continue;					
				}

				sock.getOutputStream().write("OK\n".getBytes());
				
			}
		}

	}

	public static Map<Integer, Agente> getAgentes() {
		return agentes;
	}

}
