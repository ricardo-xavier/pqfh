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
			Debug.grava("NAVEGACAO adiciona " + texto + "\n");
			pnlNavegacao.getChildren().add(btnOpcao);
		}
	}

	public static void adiciona(FlowPane pnlNavegacao, Button btn) {
		Debug.grava("NAVEGACAO adiciona " + btn.getId() + "\n");
		pnlNavegacao.getChildren().add(btn);
	}
	
	public static void limpa(FlowPane pnlNavegacao) {
		Debug.grava("NAVEGACAO limpa\n");
		pnlNavegacao.getChildren().clear();
	}

	public static void remove(FlowPane pnlNavegacao, int i) {
		Debug.grava("NAVEGACAO remove " + i + " " + pnlNavegacao.getChildren().get(i).getId() + "\n");
		
		pnlNavegacao.getChildren().remove(i);
		
		if (i == 0) {
			return;
		}
		
		Node ultimo = pnlNavegacao.getChildren().get(pnlNavegacao.getChildren().size()-1);
		Button btn = (Button) ultimo;
		btn.setText(btn.getText().replace(" >>", ""));
	}

	public static void atualiza(FlowPane pnlNavegacao, Button btn, int lin, int col, String id) {
		
		// não atualiza se tiver um com a mesma posição
		for (Node node : pnlNavegacao.getChildren()) {
			Button b = (Button) node;
			if (b.getUserData() == null) {
				continue;
			}
			Ponto p = (Ponto) b.getUserData();
			if ((p.getY() == lin) && (p.getX() == col)) {
				return;
			}
		}
		
		Debug.grava(String.format("NAVEGACAO atualiza %s %d,%d %s%n", btn.getId(), lin, col, id));
		Ponto ponto = new Ponto(lin, col, id);
		btn.setUserData(ponto);
	}

}
