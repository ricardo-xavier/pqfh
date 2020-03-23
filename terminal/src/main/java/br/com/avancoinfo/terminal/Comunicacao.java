package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

import com.jcraft.jsch.ChannelShell;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.JSchException;
import com.jcraft.jsch.Session;

public class Comunicacao extends Thread {

	private static final int TAMBUF = 8192;
	private Session sessao;
	private OutputStream saida;

	@Override
	public void run() {

		byte[] buf = new byte[TAMBUF];
		int pos = 0;
		boolean enviarComando = true;
		Terminal terminal = TerminalAvanco.getTerminal();

		try {
			
			// conecta
			
			Configuracao cfg = TerminalAvanco.getCfg();
			Debug.grava("servidor: " + cfg.getServidor() + "\n");
			Debug.grava("porta: " + cfg.getPorta() + "\n");
			
			JSch jsch = new JSch();
			sessao = jsch.getSession(cfg.getUsuario(), cfg.getServidor(), cfg.getPorta());
			Properties config = new Properties();
			config.setProperty("StrictHostKeyChecking", "no");
			sessao.setConfig(config);
			sessao.setPassword(cfg.getSenha());
			sessao.connect();
			ChannelShell canal = (ChannelShell) sessao.openChannel("shell");
			InputStream entrada = canal.getInputStream();
			saida = canal.getOutputStream();
			canal.connect();
			terminal.setConectado(true, cfg.getServidor(), cfg.getPorta(), cfg.getUsuario());

			// loop para ler entrada
			while (true) {

				if (!sessao.isConnected()) {
					break;
				}
				
				if (canal.getExitStatus() != -1) {
					break;
				}
				
				int n = entrada.read(buf, pos, TAMBUF - pos);
				Debug.grava(String.format("%s READ %d:%d%n", Thread.currentThread().getName(), pos, n));
				if (n < 0) {
					break;
				}
				pos = n;

				while (true) {

					if (!sessao.isConnected()) {
						break;
					}
					
					if (canal.getExitStatus() != -1) {
						break;
					}
					
					if (entrada.available() <= 0) {
						break;
					}
					
					n = entrada.read(buf, pos, TAMBUF - pos);
					Debug.grava(String.format("%s READ %d:%d%n", Thread.currentThread().getName(), pos, n));
					pos += n;
					if (pos == TAMBUF) {
						break;
					}
				}
				
				char ch = '?';
				if (enviarComando) {
					int u;
					for (u=pos-1; u>0 && buf[u] == ' '; u--);
					ch = (char) buf[u];
				}
					
				synchronized (terminal.getFila()) {
					Debug.grava(String.format("%s +FILA %d %s%n", Thread.currentThread().getName(), 
							terminal.getFila().size(), pos, new String(buf, 0, pos)));
					terminal.getFila().add(new Buffer(pos, buf));
				}
				terminal.atualiza();
				pos = 0;
				
				if (enviarComando && (ch == '$')) {
					
					enviarComando = false;
					Debug.grava("> " + cfg.getComando() + "\n");
					
					try {
						saida.write((cfg.getComando() + "\r\n").getBytes());
						saida.flush();
						System.err.println(cfg.getComando());
						
					} catch (IOException e) {
						e.printStackTrace();
					}

				}
				
			}

		} catch (JSchException | IOException e) {
			e.printStackTrace();
			Debug.printStackTrace(e);
			terminal.reconecta(e.getMessage());
		}
		
		System.err.println("exit comunicacao");
		
		terminal.desconecta();

	}
	
	public void envia(String s) {
		try {
			Debug.grava("> " + s + "\n");
			saida.write(s.getBytes());
			saida.flush();
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void close() {
		sessao.disconnect();
		TerminalAvanco.getTerminal().setConectado(false, null, 0, null);
	}

}
