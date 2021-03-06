package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Properties;

import com.jcraft.jsch.ChannelShell;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.JSchException;
import com.jcraft.jsch.Session;

public class Comunicacao extends Thread {

	private static final int TAMBUF = 8192;
	private Session sessao;
	private OutputStream saida;
	private Socket sockSuporte;
	
	public Comunicacao(Socket sockSuporte) {
		this.sockSuporte = sockSuporte;
	}

	@Override
	public void run() {
		Terminal terminal = TerminalAvanco.getTerminal();
		if (sockSuporte == null) {
			processaSsh(terminal);
		} else {
			processaSuporte(terminal);
		}
	}
	
	private void processaSsh(Terminal terminal) {

		byte[] buf = new byte[TAMBUF];
		int pos = 0;
		boolean enviarComando = true;
		
		try {
			
			// conecta
			
			ChannelShell canal = null;
			InputStream entrada = null;
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
			canal = (ChannelShell) sessao.openChannel("shell");
			entrada = canal.getInputStream();
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
					Debug.grava(String.format("%s +FILA %d %d %s%n", Thread.currentThread().getName(), 
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

	private void processaSuporte(Terminal terminal) {
		
		InputStream entrada = null;
		Configuracao cfg = TerminalAvanco.getCfg();
		
		try {
			terminal.setConectado(true, cfg.getServidorCompartilhamento(), cfg.getPortaCompartilhamento(), null);
			entrada = sockSuporte.getInputStream();
			saida = sockSuporte.getOutputStream();
			
			// loop para ler entrada
			while (true) {
				
				String cmd = "SUPORTE:" + readLine(entrada);
//				System.err.println(cmd);
				
				synchronized (terminal.getFila()) {
					Debug.grava(String.format("%s +FILA %d %d %s%n", Thread.currentThread().getName(), 
							terminal.getFila().size(), cmd.length(), cmd));
					terminal.getFila().add(new Buffer(cmd.length(), cmd.getBytes()));
				}
				terminal.atualiza();
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	
	private String readLine(InputStream entrada) throws IOException {
		StringBuilder sb = new StringBuilder();
		while (true) {
			int c = entrada.read();
			if (c == '\n') {
				break;
			}
			sb.append((char) c);
		}
		return sb.toString();
	}
	
	public void envia(String s) {
		try {
			Debug.grava("> " + s + "\n");
			
			if (sockSuporte == null) {				
				saida.write(s.getBytes());
				saida.flush();
				
			} else {
				s += "\n";
				sockSuporte.getOutputStream().write(s.getBytes());
				sockSuporte.getOutputStream().flush();
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void close() {
		sessao.disconnect();
		TerminalAvanco.getTerminal().setConectado(false, null, 0, null);
	}

}
