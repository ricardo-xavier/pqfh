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
		com.close();
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		
		Terminal terminal = new Terminal();
		terminal.show();
		
		com = new Comunicacao(terminal);
		if (servidor != null) {
			com.setServidor(servidor);
			com.setPorta(porta);
		}
		com.setName("COMUNICACAO");
		com.start();
		
	}

	public void close() {
		com.close();
	}
}
