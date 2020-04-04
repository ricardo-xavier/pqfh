package br.com.avancoinfo.terminal;

import java.net.Socket;

import javafx.application.Application;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.stage.Stage;

public class TerminalAvanco extends Application {
	
	private static Comunicacao com;
	private static String servidor = null;
	private static int porta = 22;
	private static Terminal terminal;
	private static Configuracao cfg;
	private static boolean suporte;
	
	public static void main(String[] args) {
		
		if (args.length > 0) {
			if (args[0].equals("-s")) {
				suporte = true;
				
			} else {
				servidor = args[0];
				porta = Integer.parseInt(args[1]);
			}
		}
		launch(args);
		if (com != null) {
			com.close();
		}
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		
		cfg = new Configuracao();
		cfg.carrega();
		
		Socket sockSuporte = null;
		if (suporte) {
			
			if ((cfg.getServidorCompartilhamento() == null) 
					|| (cfg.getPortaCompartilhamento() == 0)) {
			
				Alert alert = new Alert(AlertType.INFORMATION);
				alert.setTitle("Informação");
				alert.setHeaderText("Servidor/porta de compartilhamento não configurado");
				alert.setContentText("Ajuste a configuração antes de usar o suporte remoto");
				alert.showAndWait();
				return;				
			}
			
			Suporte sup = new Suporte();
			sup.showAndWait();
			sockSuporte = sup.getSock();
			if (sockSuporte == null) {
				return;
			}
		}
		
		if (servidor != null) {
			if (cfg.getServidor() == null) {
				cfg.setServidor(servidor);
				cfg.setPorta(porta);
			}
		}
		while ((cfg.getServidor() == null) || (cfg.getPorta() == 0) || (cfg.getUsuario() == null) || (cfg.getSenha() == null)) {
			cfg.showAndWait();
			if (cfg.isCancelado()) {
				return;
			}
		}
		
		terminal = new Terminal();
		terminal.inicializa();
		terminal.show();
		
		if (sockSuporte != null) {
			terminal.setEstadoLogin(EstadoLogin.OK);
		}
		
		com = new Comunicacao(sockSuporte);
		com.setName("COMUNICACAO");
		com.start();
		
	}

	public void close() {
		if (com.isAlive()) {
			com.close();
		}
	}

	public static Comunicacao getCom() {
		return com;
	}

	public static Terminal getTerminal() {
		return terminal;
	}

	public static Configuracao getCfg() {
		return cfg;
	}

	public static boolean isSuporte() {
		return suporte;
	}

	public static void setSuporte(boolean suporte) {
		TerminalAvanco.suporte = suporte;
	}

}
