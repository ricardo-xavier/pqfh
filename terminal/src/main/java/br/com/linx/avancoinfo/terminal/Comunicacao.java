package br.com.linx.avancoinfo.terminal;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import com.jcraft.jsch.ChannelShell;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.JSchException;
import com.jcraft.jsch.Session;

public class Comunicacao extends Thread {

	private static final int TAMBUF = 8192;
	private Terminal terminal;
	private Session sessao;

	public Comunicacao(Terminal terminal) {
		this.terminal = terminal;
	}

	@Override
	public void run() {

		String usuario = "avanco";
		String senha = "Bigu";
		String servidor = "avancoinfo.ddns.com.br";
		int porta = 65531;

		byte[] buf = new byte[TAMBUF];
		int pos = 0;

		try {
			
			// conecta
			JSch jsch = new JSch();
			sessao = jsch.getSession(usuario, servidor, porta);
			Properties config = new Properties();
			config.setProperty("StrictHostKeyChecking", "no");
			sessao.setConfig(config);
			sessao.setPassword(senha);
			sessao.connect();
			ChannelShell canal = (ChannelShell) sessao.openChannel("shell");
			InputStream entrada = canal.getInputStream();
			canal.connect();

			// loop para ler entrada
			while (sessao.isConnected()) {

				int n = entrada.read(buf, pos, TAMBUF - pos);
				if (n < 0) {
					break;
				}
				pos = n;

				while (entrada.available() > 0) {
					
					n = entrada.read(buf, pos, TAMBUF - pos);
					pos += n;
					if (pos == TAMBUF) {
						break;
					}
				}

				terminal.atualiza(buf, pos);
			}

		} catch (JSchException | IOException e) {
			e.printStackTrace();
		}

	}
	
	public void close() {
		sessao.disconnect();
	}

}
