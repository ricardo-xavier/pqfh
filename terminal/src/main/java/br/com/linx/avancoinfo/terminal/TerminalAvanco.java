package br.com.linx.avancoinfo.terminal;

import javafx.application.Application;
import javafx.stage.Stage;

public class TerminalAvanco extends Application {

	public static void main(String[] args) {
		launch(args);
	}

	@Override
	public void start(Stage primaryStage) throws Exception {
		
		Comunicacao com = new Comunicacao();
		com.start();
		
		Terminal terminal = new Terminal();
		terminal.show();
	}

}
