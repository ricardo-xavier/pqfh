package br.com.avancoinfo.terminal;

import java.util.ArrayList;
import java.util.List;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.Button;
import javafx.scene.layout.GridPane;

public class BotoesFuncao {

	public boolean processa(String linha, GridPane pnlBotoes, Comunicacao com) {
		
		if (pnlBotoes.getUserData() != null) {
			String atual = (String) pnlBotoes.getUserData();
			if (atual.equals("linha")) {
				return true;
			}
		}
		pnlBotoes.setUserData(linha);
		
		List<String> teclas = new ArrayList<String>();
		List<String> descricoes = new ArrayList<String>();

		String teclaAnterior = null;
		String[] partes = linha.split(":");
		for (int p = partes.length - 1; p > 0; p--) {
			String descricao = partes[p].trim();
			String esquerda = partes[p - 1].trim();
			String[] partesEsq = esquerda.split(" ");
			String tecla = partesEsq[partesEsq.length - 1];
			if (teclaAnterior != null) {
				int q = descricao.lastIndexOf(teclaAnterior);
				if (q > 0) {
					descricao = descricao.substring(0, q).trim();
				}
			}
			teclaAnterior = tecla;
			teclas.add(tecla);
			descricoes.add(descricao);
		}

		pnlBotoes.getChildren().clear();
		for (String tecla : teclas) {
			if ((tecla.length() < 2) || (tecla.length() > 3) || !tecla.startsWith("F")
					|| !Character.isDigit(tecla.charAt(1))) {
				return false;
			}
		}

		int lin = 0;
		for (int i = teclas.size() - 1; i >= 0; i--) {
			Button btn = new Button(teclas.get(i) + ":" + descricoes.get(i));
			btn.setId(teclas.get(i));
			btn.setPrefWidth(140);
			btn.setFocusTraversable(false);
			btn.setOnAction(new EventHandler<ActionEvent>() {

				@Override
				public void handle(ActionEvent event) {

					switch (btn.getId()) {

					case "F1":
						com.envia("\u001b[M");
						break;

					case "F2":
						com.envia("\u001b[N");
						break;

					case "F3":
						com.envia("\u001b[O");
						break;

					case "F4":
						com.envia("\u001b[P");
						break;

					case "F5":
						com.envia("\u001b[Q");
						break;

					case "F6":
						com.envia("\u001b[R");
						break;

					case "F7":
						com.envia("\u001b[S");
						break;

					case "F8":
						com.envia("\u001b[T");
						break;

					case "F9":
						com.envia("\u001b[U");
						break;

					case "F10":
						com.envia("\u001b[V");
						break;

					case "F11":
						com.envia("\u001b[W");
						break;

					case "F12":
						com.envia("\u001b[X");
						break;

					}

				}
			});
			pnlBotoes.add(btn, 0, ++lin);
		}

		return true;
	}

}
