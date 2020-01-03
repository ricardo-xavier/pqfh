package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.sql.SQLException;

import javafx.application.Application;
import javafx.stage.Stage;

public class Pendencias extends Application {
	
	private static Connection conn;

	public static void main(String[] args) throws ClassNotFoundException, SQLException {
		conn = BancoDados.conecta();
		launch(args);
		conn.close();
	}

	@Override
	public void start(Stage primaryStage) {
		
		Painel painel = new Painel(conn);
		painel.setMaximized(true);
		painel.show();
		painel.setIconified(true);
		painel.setIconified(false);	
		
	}

}
