package br.com.avancoinfo.terminal;

import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Menu extends Stage {
	
	public Menu() {
		
		FlowPane pnlMenu = new FlowPane(50, 50);
		
		String[] opcoes = { "Cadastros basicos" ,
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
		
		for (String opcao : opcoes) {
			
			VBox pnl = new VBox(5);
			pnl.setMinWidth(200);
			pnl.setMaxWidth(200);
			pnl.setAlignment(Pos.CENTER);
			Image imagem = new Image(getClass().getResourceAsStream("logo2.png"));
			ImageView iv = new ImageView(imagem);
			pnl.getChildren().add(iv);
			pnl.getChildren().add(new Label(opcao));
			pnlMenu.getChildren().add(pnl);
			
		}
		
		Scene scene = new Scene(pnlMenu, 730, 400);
		setTitle("Menu Principal");
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
    }
	

}
