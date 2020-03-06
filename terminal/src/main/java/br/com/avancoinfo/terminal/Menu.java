package br.com.avancoinfo.terminal;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
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

	private String opcaoSelecionada;

	private String[] opcoes = { "Cadastros basicos" ,
			"administracao Materiais",
			"administracao Financeira",
			"administracao Fiscal",
			"preco por Grupo",
			"vendas - Automacao",
			"mOvimento - caixa",
			"controle de Producao",
			"Exportar/importar arquivos",
			"SPED/EFD - NFe/NFCe",
			"Frota - veiculos"
	};
	
	public Menu() {
		
		FlowPane pnlMenu = new FlowPane(50, 50);

		pnlMenu.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				seleciona(event.getCode());
			}
		});
		
		for (String opcao : opcoes) {
			
			VBox pnl = new VBox(5);
			pnl.setMinWidth(200);
			pnl.setMaxWidth(200);
			pnl.setAlignment(Pos.CENTER);
			
			Image imagem = new Image(getClass().getResourceAsStream("logo2.png"));
			ImageView iv = new ImageView(imagem);
			Button btnOpcao = new Button("", iv);
			
			btnOpcao.setOnAction(new EventHandler<ActionEvent>() {
				
				@Override
				public void handle(ActionEvent event) {
					setOpcaoSelecionada(opcao);
					close();
					
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
			
		}
		
		Scene scene = new Scene(pnlMenu, 730, 400);
		setTitle("Menu Principal");
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
    }

	protected void seleciona(KeyCode code) {
		
		if (code == KeyCode.ESCAPE) {
			close();
		}
		
		for (String opcao : opcoes) {
			for (int i=0; i<opcao.length(); i++) {
				if (Character.isUpperCase(opcao.charAt(i))) {
					if (opcao.charAt(i) == code.toString().charAt(0)) {
						opcaoSelecionada = opcao;	
						close();
						return;
					}
				}
			}
		}
	}

	public String getOpcaoSelecionada() {
		return opcaoSelecionada;
	}

	public void setOpcaoSelecionada(String opcaoSelecionada) {
		this.opcaoSelecionada = opcaoSelecionada;
	}
	

}
