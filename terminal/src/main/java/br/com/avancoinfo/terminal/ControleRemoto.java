package br.com.avancoinfo.terminal;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;

public class ControleRemoto extends Thread {
	
	private Socket sock;
	
	public ControleRemoto(Socket sock) {
		this.sock = sock;
	}
	
	@Override
	public void run() {
		
		try {
			
			Comunicacao com = TerminalAvanco.getCom();
			BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream(), "ISO-8859-1"));
			
			while (true) {
				
				String cmd = reader.readLine();
				if (cmd == null) {
					break;
				}
				
				com.envia(cmd);
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}

}
