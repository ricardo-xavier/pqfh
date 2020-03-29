package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.net.Socket;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Suporte extends Stage {
	
	private Socket sock;
	
	public Suporte() {
		
		VBox tela = new VBox(10);
		
		Label lblChave = new Label("Chave de Acesso");
		tela.getChildren().add(lblChave);
		
		TextField edtChave = new TextField();
		tela.getChildren().add(edtChave);
		
		HBox botoes = new HBox(10);
		
		Button btnConfirma = new Button("Confirma");
		btnConfirma.getStyleClass().add("botao");
		btnConfirma.setOnAction(new EventHandler<ActionEvent>() {
			
			@Override
			public void handle(ActionEvent event) {
				
				// conecta
				String host = TerminalAvanco.getCfg().getServidorCompartilhamento();
				int porta = TerminalAvanco.getCfg().getPortaCompartilhamento();
				try {
					sock = new Socket(host, porta);
					
					String chave = edtChave.getText();
					
					String cmd = String.format("S%-8s%n", chave);
					System.err.print(cmd);
					sock.getOutputStream().write(cmd.getBytes());
					
					String resp = readLine(sock);
					if (resp.startsWith("ERRO:")) {
						Alert alert = new Alert(AlertType.ERROR);
						alert.setTitle("Erro");
						alert.setHeaderText(resp.substring(5));
						alert.showAndWait();							
					} else {
						Alert alert = new Alert(AlertType.INFORMATION);
						alert.setTitle("Informação");
						alert.setHeaderText("Conectado ao terminal remoto");
						alert.setContentText("Em desenvolvimento... \n O terminal será fechado.");
						alert.showAndWait();													
					}
					sock.close();
					
				} catch (IOException e) {
					e.printStackTrace();
					Alert alert = new Alert(AlertType.ERROR);
					alert.setTitle("Erro");
					alert.setHeaderText("Erro na comunicação com o servidor de compartilhamento");
					alert.setContentText("Verifique se a configuração está correta e se o servidor está ativo");
					alert.showAndWait();							
				}
				
				close();
				System.exit(0);
			}
		});
		botoes.getChildren().add(btnConfirma);
		
		Button btnCancela = new Button("Cancela");
		btnCancela.setOnAction(new EventHandler<ActionEvent>() {
			
			@Override
			public void handle(ActionEvent event) {
				close();
				
			}
		});		
		btnCancela.getStyleClass().add("botao");
		botoes.getChildren().add(btnCancela);
		
		tela.getChildren().add(botoes);
		
		setTitle("Suporte Remoto");
		Scene scene = new Scene(tela, 200, 100);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		scene.getStylesheets().add(Terminal.class.getResource("avanco.css").toExternalForm());
		
	}
	
	private String readLine(Socket sock) throws IOException {
		StringBuilder sb = new StringBuilder();
		while (true) {
			int c = sock.getInputStream().read();
			if (c == '\n') {
				break;
			}
			sb.append((char) c);
		}
		return sb.toString();
	}

	public Socket getSock() {
		return sock;
	}

	public void setSock(Socket sock) {
		this.sock = sock;
	}
	

}
