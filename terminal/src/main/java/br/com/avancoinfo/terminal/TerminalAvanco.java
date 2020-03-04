package br.com.avancoinfo.terminal;

import javafx.application.Application;
import javafx.stage.Stage;

public class TerminalAvanco extends Application {
	
	private static Comunicacao com;
	private static String servidor = null;
	private static int porta = 22;
	
	public static void main(String[] args) {
		
		if (args.length > 1) {
			servidor = args[0];
			porta = Integer.parseInt(args[1]);
		}
		launch(args);
		if (com != null) {
			com.close();
		}
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		
		Configuracao cfg = new Configuracao();
		cfg.carrega();
		
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
		
		Menu menu = new Menu();
		menu.showAndWait();
		
		Terminal terminal = new Terminal(cfg);
		terminal.show();
		
		com = new Comunicacao(terminal, cfg);
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

	public static void setCom(Comunicacao com) {
		TerminalAvanco.com = com;
	}
}
