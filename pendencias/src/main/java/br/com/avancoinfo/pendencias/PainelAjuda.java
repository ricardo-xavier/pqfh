package br.com.avancoinfo.pendencias;

import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.VBox;
import javafx.scene.web.WebView;
import javafx.stage.Stage;

public class PainelAjuda extends Stage {
	
	public PainelAjuda() {

		setTitle("Ajuda");
        WebView webView = new WebView();

        webView.getEngine().load("http://google.com");

        VBox vBox = new VBox(webView);
        Scene scene = new Scene(vBox, 600, 400);
        
        StringBuilder html = new StringBuilder();
        
        html.append("<html><body>");
        
        html.append("<ul>");
        
        html.append("<li>Use as setas para selecionar uma nota na tabela.");
        
        html.append("<li>Para ver os detalhes de uma nota:");
        
        html.append("<ul>");
        
        html.append("<li>Faça um duplo clique sobre a linha da nota");
        
        html.append("<li>ou selecione a nota e pressione F2 ou ENTER");
        
        html.append("<li>ou selecione e clique no botão F2-detalhes");
        
        html.append("</ul>");
        
        html.append("<li>Para atualizar a lista:");
        
        html.append("<ul>");
        
        html.append("<li>Pressione F5");
        
        html.append("<li>ou clique no botão F5-atualiza");
        
        html.append("</ul>");
        
        html.append("<li>Para executar ...:");
        
        html.append("<ul>");
        
        html.append("<li>Seelecione a nota e pressione F6");
        
        html.append("<li>ou selecione e clique no botão F6-executa");
        
        html.append("</ul>");
        
        html.append("<li>Para filtrar, digite na caixa de pesquisa no abaixo da tabela.");
        
        html.append("<li>Para fechar uma janela clique no x, ou pressione ESC.");
        
        html.append("</ul>");
        
        html.append("</body></html>");
        
		webView.getEngine().loadContent(html.toString());
        
		Stage janela = this;
		
		webView.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				if (event.getCode() == KeyCode.ESCAPE) {
					janela.close();
				}
			}
		});
		
        setScene(scene);
	}
	

}
