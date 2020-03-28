package br.com.avancoinfo.terminal;

import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Suporte extends Stage {
	
	public Suporte() {
		
		VBox tela = new VBox(10);
		
		Label lblChave = new Label("Chave de Acesso");
		tela.getChildren().add(lblChave);
		
		TextField edtChave = new TextField();
		tela.getChildren().add(edtChave);
		
		HBox botoes = new HBox(10);
		
		Button btnConfirma = new Button("Confirma");
		btnConfirma.getStyleClass().add("botao");
		botoes.getChildren().add(btnConfirma);
		
		Button btnCancela = new Button("Cancela");
		btnCancela.getStyleClass().add("botao");
		botoes.getChildren().add(btnCancela);
		
		tela.getChildren().add(botoes);
		
		setTitle("Suporte Remoto");
		Scene scene = new Scene(tela, 200, 100);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		scene.getStylesheets().add(Terminal.class.getResource("avanco.css").toExternalForm());
		
	}
	

}
