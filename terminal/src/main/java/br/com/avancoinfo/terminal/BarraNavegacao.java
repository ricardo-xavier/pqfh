package br.com.avancoinfo.terminal;

import javafx.geometry.Insets;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.layout.FlowPane;

public class BarraNavegacao {

	public static void adiciona(FlowPane pnlNavegacao, String texto) {

		int pos = texto.indexOf(Terminal.getMARCADOR());
		if (pos >= 0) {
			texto = texto.substring(pos+1);
		}
		texto = texto.trim();
		Node ultimo = pnlNavegacao.getChildren().get(pnlNavegacao.getChildren().size()-1);
		if (!ultimo.getId().equals(texto)) {
			Button btn = (Button) ultimo;
			if (!btn.getText().contains(" >>")) {
				btn.setText(btn.getText() + " >>");
			}
			
			Button btnOpcao = new Button(texto);
			btnOpcao.setId(texto);
			btnOpcao.getStyleClass().add("botao");
			FlowPane.setMargin(btnOpcao, new Insets(4, 4, 4, 0));
			pnlNavegacao.getChildren().add(btnOpcao);
			
		}
	
	}

	public static void removeEsc(FlowPane pnlNavegacao, Terminal terminal) {
		
		Node ultimo = pnlNavegacao.getChildren().get(pnlNavegacao.getChildren().size()-1);
		
		if (MenuInterno.getY1() == -1) {
			return;
		}
		
		char[][] dados = terminal.getDados();
		for (int y=MenuInterno.getY1()+1; y<MenuInterno.getY2(); y++) {
			String texto = new String(dados[y], MenuInterno.getX1()+1, MenuInterno.getX2()-MenuInterno.getX1()-1).trim();
			int pos = texto.indexOf(Terminal.getMARCADOR());
			if (pos >= 0) {
				texto = texto.substring(pos+1);
			}
			texto = texto.trim();
			if (ultimo.getId().equals(texto)) {
				pnlNavegacao.getChildren().remove(ultimo);
				ultimo = pnlNavegacao.getChildren().get(pnlNavegacao.getChildren().size()-1);
				Button btn = (Button) ultimo;
				btn.setText(btn.getText().replace(" >>", ""));
				break;
			}
		}
	}

}
