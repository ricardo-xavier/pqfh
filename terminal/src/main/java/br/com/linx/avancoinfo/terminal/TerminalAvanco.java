package br.com.linx.avancoinfo.terminal;

import javafx.application.Application;
import javafx.stage.Stage;

public class TerminalAvanco extends Application {
	
	private static final int VERSAO = 1;
	private static Comunicacao com;
	
	public static int getVersao() {
		return VERSAO;
	}

	public static void main(String[] args) {
		launch(args);
		com.close();
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		
		Terminal terminal = new Terminal();
		terminal.show();
		
		com = new Comunicacao(terminal);
		com.start();
	}

	public void close() {
		com.close();
	}
}
