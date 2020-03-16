package br.com.avancoinfo.atualizador;

import java.io.File;
import java.io.FileFilter;
import java.util.Properties;
import java.util.Vector;

import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.ChannelSftp.LsEntry;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;

public class Atualizador {

	public static void main(String[] args) {

		if (args.length < 1) {
			System.out.println("Argumento <produto> nao informado");
			System.exit(-1);
			return;
		}
		String produto = args[0];
		
		String central = System.getenv("CENTRAL_AVANCO");
		if (central == null) {
			System.out.println("Variavel CENTRAL_AVANCO nao definida");
			System.exit(-1);
			return;			
		}
		
		FileFilter filtro = new FileFilter() {
			
			@Override
			public boolean accept(File pathname) {
				return pathname.getName().startsWith(produto+"-")
						&& pathname.getName().endsWith(".jar");
			}
		};
		
		int versaoInstalada = 0;
		File dir = new File(".");
		for (File arq : dir.listFiles(filtro)) {
			try {
				int v = Integer.parseInt(arq.getName().replace(produto+"-", "").replace(".jar", ""));
				if (v > versaoInstalada) {
					versaoInstalada = v;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		String usuario = "avanco";
		String senha = "Bigu";
		String servidor = central.split(":")[0];
		int porta = Integer.parseInt(central.split(":")[1]);
		
		try {
			
			// conecta
			JSch jsch = new JSch();
			Session sessao = jsch.getSession(usuario, servidor, porta);
			Properties config = new Properties();
			config.setProperty("StrictHostKeyChecking", "no");
			sessao.setConfig(config);
			sessao.setPassword(senha);
			sessao.connect();
			ChannelSftp canal = (ChannelSftp) sessao.openChannel("sftp");
			canal.connect();
			
			// lista arquivos
			int ultimaVersaoPublicada = versaoInstalada;
			String arqDownload = null;
			canal.cd("/u/sist/exec/atualizador");
			Vector<?> arquivos = canal.ls(".");
			for (Object arquivo : arquivos) {
				try {
					String arq = ((LsEntry) arquivo).getFilename();
					if (arq.startsWith(produto+"-") && arq.endsWith(".jar")) {
						int versaoPublicada = Integer.parseInt(arq.replace(produto+"-", "").replace(".jar", ""));
						if (versaoPublicada > ultimaVersaoPublicada) {
							arqDownload = arq;
							ultimaVersaoPublicada = versaoPublicada;
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			
			// baixa a ultima versao
			if (arqDownload != null) {
				try {
					canal.get(arqDownload, arqDownload);
					versaoInstalada = ultimaVersaoPublicada;
				} catch (Exception e) {
					e.printStackTrace();
				}					
			}
			
			// desconecta
			canal.quit();
			
		} catch(Exception e) {
			e.printStackTrace();
		}

		if (versaoInstalada == 0) {
			System.err.println("versaoInstalada=0");
			System.exit(-1);
		}
		
		System.out.println(produto + "-" + versaoInstalada + ".jar");
		System.exit(0);

	}

}
