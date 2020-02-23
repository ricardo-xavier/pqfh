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
	private Terminal terminal;
	private Configuracao cfg;
	private Session sessao;

	public Comunicacao(Terminal terminal, Configuracao cfg) {
		this.terminal = terminal;
		this.cfg = cfg;
	}

	@Override
	public void run() {

		byte[] buf = new byte[TAMBUF];
		int pos = 0;
		boolean enviarComando = true;

		try {
			
			// conecta
			if (terminal.getLog() != null) {
				synchronized (terminal.getLog()) {
					terminal.getLog().println("servidor: " + cfg.getServidor());
					terminal.getLog().println("porta: " + cfg.getPorta());
					terminal.getLog().println();
					terminal.getLog().flush();					
				}
			}
			JSch jsch = new JSch();
			sessao = jsch.getSession(cfg.getUsuario(), cfg.getServidor(), cfg.getPorta());
			Properties config = new Properties();
			config.setProperty("StrictHostKeyChecking", "no");
			sessao.setConfig(config);
			sessao.setPassword(cfg.getSenha());
			sessao.connect();
			ChannelShell canal = (ChannelShell) sessao.openChannel("shell");
			InputStream entrada = canal.getInputStream();
			OutputStream saida = canal.getOutputStream();
			terminal.setSaida(saida);
			canal.connect();
			terminal.setConectado(true, cfg.getServidor(), cfg.getPorta(), cfg.getUsuario());

			// loop para ler entrada
			while (sessao.isConnected()) {

				int n = entrada.read(buf, pos, TAMBUF - pos);
				if (terminal.getLog() != null) {
					synchronized (terminal.getLog()) {
						terminal.getLog().printf("%s %d:%d%n", Thread.currentThread().getName(), pos, n);
						terminal.getLog().flush();						
					}
				}				
				if (n < 0) {
					break;
				}
				pos = n;

				while (entrada.available() > 0) {
					
					n = entrada.read(buf, pos, TAMBUF - pos);
					if (terminal.getLog() != null) {
						synchronized (terminal.getLog()) {
							terminal.getLog().printf("%s %d:%d%n", Thread.currentThread().getName(), pos, n);
							terminal.getLog().flush();							
						}
					}					
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

					if (terminal.getLog() != null) {
						synchronized (terminal.getLog()) {
							terminal.getLog().printf("%s +FILA %d %s%n", Thread.currentThread().getName(), terminal.getFila().size(), pos, new String(buf, 0, pos));
							terminal.getLog().flush();
						}
					}
					
					terminal.getFila().add(new Buffer(pos, buf));
				}
				terminal.atualiza();
				pos = 0;
				
				if (enviarComando && (ch == '$')) {
					
					enviarComando = false;
					
					if (terminal.getLog() != null) {
						synchronized (terminal.getLog()) {
							terminal.getLog().println("> " + cfg.getComando());
							terminal.getLog().flush();
						}
					}
					
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

			if (terminal.getLog() != null) {
				synchronized (terminal.getLog()) {
					e.printStackTrace(terminal.getLog());
					terminal.getLog().flush();
				}
			}
			
			terminal.reconecta(e.getMessage());
		}

	}
	
	public void close() {
		sessao.disconnect();
		terminal.setConectado(false, null, 0, null);
	}

}
