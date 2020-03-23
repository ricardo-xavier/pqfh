package br.com.avancoinfo.terminal;

import java.util.ArrayList;
import java.util.List;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;
import javafx.scene.text.TextAlignment;
import javafx.scene.text.TextFlow;
import javafx.stage.Modality;
import javafx.stage.Stage;

/*
 * Menu Principal
 * 
 * abertura
 * ========
 * procura "P  r  i  n  c  i  p  a  l" na linha 6 (se ainda não estiver montado)
 * 
 * se encontrar
 *     se o marcador foi recebido no mesmo bloco, monta o menu
 *     senão seta montarMenu
 *     
 * se o marcador for recebido com montarMenu, então monta o menu
 * 
 * fechamento
 * ==========
 * ao selecionar uma opção, a tecla é enviada
 * 
 * se receber um marcado na coluna 3 ou 4 e o menu estiver aberto, então fecha o menu
 * 
 */

public class Menu extends Stage {

	private static List<String> opcoes;
	private static List<Character> teclas;
	private boolean encerrar;
	private FlowPane pnlNavegacao;
	
	public Menu(char[][] dados, char[][] frente, FlowPane pnlNavegacao) {
		
		encerrar = true;
		this.pnlNavegacao = pnlNavegacao;
		FlowPane pnlMenu = new FlowPane();

		if (pnlNavegacao != null) {
			Button btnPrincipal = new Button("Menu Principal >>");
			btnPrincipal.setId("Menu Principal");
			btnPrincipal.getStyleClass().add("botao");
			FlowPane.setMargin(btnPrincipal, new Insets(4));
		
			BarraNavegacao.limpa(pnlNavegacao);
			BarraNavegacao.adiciona(pnlNavegacao, btnPrincipal);
		}
		
		pnlMenu.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				Teclado.setUltimaTecla(null);
				seleciona(event.getCode());
			}
		});
		
		if (opcoes == null) {
			opcoes = new ArrayList<String>();
			teclas = new ArrayList<Character>();
			for (int i=7; i<=17; i++) {
				String s = new String(dados[i], 35, 40).trim();
				Character letra = ' ';
				for (int j=35; j<75; j++) {
					if (Character.isUpperCase(dados[i][j])) {
						letra = dados[i][j];
						break;
					}
				}
				for (int j=35; j<75; j++) {
					if (frente[i][j] == 'Y') {
						letra = dados[i][j];
						break;						
					}
				}
				opcoes.add(s);
				teclas.add(letra);
			}
		}
		
		for (int o=0; o<opcoes.size(); o++) {
			String opcao = opcoes.get(o);
			char tecla = teclas.get(o);

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
			btnOpcao.getStyleClass().add("botao");
			
			btnOpcao.setOnAction(new EventHandler<ActionEvent>() {
				
				@Override
				public void handle(ActionEvent event) {
					Teclado.setUltimaTecla(null);
					TerminalAvanco.getCom().envia(String.valueOf(tecla));
					if (pnlNavegacao != null) {
						Button btnOpcao = new Button(opcao);
						btnOpcao.setId(opcao.trim());
						btnOpcao.getStyleClass().add("botao");
						FlowPane.setMargin(btnOpcao, new Insets(4, 4, 4, 0));
						pnlNavegacao.getChildren().add(btnOpcao);
					}
				}
			});

			btnOpcao.setOnKeyPressed(new EventHandler<KeyEvent>() {

				@Override
				public void handle(KeyEvent event) {
					Teclado.setUltimaTecla(null);
					seleciona(event.getCode());
					event.consume();
				}
			});
			
			pnl.getChildren().add(btnOpcao);
			
			TextFlow texto = new TextFlow();
			texto.setTextAlignment(TextAlignment.CENTER);
			int p = opcao.indexOf(tecla);
			if (p == 0) {
				Text destaque = new Text(String.valueOf(tecla));
				destaque.setFill(Color.RED);
				Text normal = new Text(opcao.substring(1).trim());
				normal.setFill(Color.BLACK);
				texto.getChildren().addAll(destaque, normal);
				
			} else {
				Text inicio = new Text(opcao.substring(0, p));
				inicio.setFill(Color.BLACK);				
				Text destaque = new Text(String.valueOf(tecla));
				destaque.setFill(Color.RED);
				Text normal = new Text(opcao.substring(p+1).trim());
				normal.setFill(Color.BLACK);
				texto.getChildren().addAll(inicio, destaque, normal);
				
			}
			
			pnl.getChildren().add(texto);
			pnlMenu.getChildren().add(pnl);
			FlowPane.setMargin(pnl, new Insets(10));
			
		}
		
		Scene scene = new Scene(pnlMenu, 700, 480);
		setTitle("Menu Principal");
		setScene(scene);
		initModality(Modality.APPLICATION_MODAL);
		scene.getStylesheets().add(Terminal.class.getResource("avanco.css").toExternalForm());
		
    }

	protected void seleciona(KeyCode code) {
		
		if (code == KeyCode.ESCAPE) {
			close();
		}
		
		for (int o=0; o<opcoes.size(); o++) {
			char tecla = teclas.get(o);
			if (tecla == code.toString().charAt(0)) {
				TerminalAvanco.getCom().envia(String.valueOf(tecla));
				if (pnlNavegacao != null) {
					Button btnOpcao = new Button(opcoes.get(o));
					btnOpcao.setId(opcoes.get(o).trim());
					btnOpcao.getStyleClass().add("botao");
					FlowPane.setMargin(btnOpcao, new Insets(4));
					BarraNavegacao.adiciona(pnlNavegacao, btnOpcao);
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
