package br.com.avancoinfo.terminal;

import java.util.ArrayList;
import java.util.List;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Menu extends Stage {

	private static List<String> opcoes;
	private Comunicacao com;
	private boolean encerrar;
	
	public Menu(char[][] dados, Comunicacao com) {
		
		encerrar = true;
		this.com = com;
		FlowPane pnlMenu = new FlowPane();

		pnlMenu.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				seleciona(event.getCode());
			}
		});
		
		if (opcoes == null) {
			opcoes = new ArrayList<String>();
			for (int i=7; i<=17; i++) {
				String s = new String(dados[i], 35, 40).trim();
				opcoes.add(s);
			}
		}
		
		for (String opcao : opcoes) {

			VBox pnl = new VBox(5);
			pnl.setMinWidth(200);
			pnl.setMaxWidth(200);
			pnl.setAlignment(Pos.CENTER);

			Image imagem;
			try {
				String nomeImagem = opcao.trim().toLowerCase() + ".png";
				while (nomeImagem.contains(" ")) {
					nomeImagem = nomeImagem.replace(" ", "");
				}
				imagem = new Image(getClass().getResourceAsStream(nomeImagem));
			} catch (Exception e) {
				imagem = new Image(getClass().getResourceAsStream("logo2.png"));
			}
			ImageView iv = new ImageView(imagem);
			Button btnOpcao = new Button("", iv);
			btnOpcao.setId(opcao);
			
			btnOpcao.setOnAction(new EventHandler<ActionEvent>() {
				
				@Override
				public void handle(ActionEvent event) {

					for (int i=0; i<opcao.length(); i++) {
						if (Character.isUpperCase(opcao.charAt(i))) {
							com.envia(String.valueOf(opcao.charAt(i)));
						}
					}

				}
			});

			btnOpcao.setOnKeyPressed(new EventHandler<KeyEvent>() {

				@Override
				public void handle(KeyEvent event) {
					seleciona(event.getCode());
					event.consume();
				}
			});
			
			pnl.getChildren().add(btnOpcao);
			pnl.getChildren().add(new Label(opcao));
			pnlMenu.getChildren().add(pnl);
			FlowPane.setMargin(pnl, new Insets(10));
			
		}
		
		Scene scene = new Scene(pnlMenu, 700, 480);
		setTitle("Menu Principal");
		setScene(scene);
		initModality(Modality.APPLICATION_MODAL);
		
    }

	protected void seleciona(KeyCode code) {
		
		if (code == KeyCode.ESCAPE) {
			close();
		}
		
		for (String opcao : opcoes) {
			for (int i=0; i<opcao.length(); i++) {
				if (Character.isUpperCase(opcao.charAt(i))) {
					if (opcao.charAt(i) == code.toString().charAt(0)) {
						com.envia(String.valueOf(opcao.charAt(i)));
						return;
					}
				}
			}
		}
	}

	public boolean isEncerrar() {
		return encerrar;
	}

	public void setEncerrar(boolean encerrar) {
		this.encerrar = encerrar;
	}

}
