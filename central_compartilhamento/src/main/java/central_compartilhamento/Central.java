package central_compartilhamento;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class Central {
	
	private static List<Agente> agentes = new ArrayList<Agente>();
	
	public static void main(String[] args) throws IOException {
		
		int porta = Integer.parseInt(args[0]);
		@SuppressWarnings("resource")
		ServerSocket serverSock = new ServerSocket(porta);
		
		while (true) {
			
			// recebe uma conexão
			Socket sock = serverSock.accept();
			System.out.println("central:accept: " + sock.getRemoteSocketAddress());
			
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
			agentes.add(agente);
			agente.start();
		}

	}

}
