package br.com.avancoinfo.terminal;

import javafx.application.Application;
import javafx.stage.Stage;

public class TerminalAvanco extends Application {
	
	private static Comunicacao com;
	
	public static void main(String[] args) {
		launch(args);
		com.close();
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		
		Terminal terminal = new Terminal();
		terminal.show();
		
		com = new Comunicacao(terminal);
		com.setName("COMUNICACAO");
		com.start();
	}

	public void close() {
		com.close();
	}
}
