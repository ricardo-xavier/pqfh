package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.Button;
import javafx.scene.layout.GridPane;

public class BotoesFuncao {

	public boolean processa(String linha, GridPane pnlBotoes, OutputStream saida) {

		
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

					try {
						switch (btn.getId()) {

						case "F1":
							saida.write("\u001b[M".getBytes());
							saida.flush();
							break;

						case "F2":
							saida.write("\u001b[N".getBytes());
							saida.flush();
							break;

						case "F3":
							saida.write("\u001b[O".getBytes());
							saida.flush();
							break;

						case "F4":
							saida.write("\u001b[P".getBytes());
							saida.flush();
							break;

						case "F5":
							saida.write("\u001b[Q".getBytes());
							saida.flush();
							break;

						case "F6":
							saida.write("\u001b[R".getBytes());
							saida.flush();
							break;

						case "F7":
							saida.write("\u001b[S".getBytes());
							saida.flush();
							break;

						case "F8":
							saida.write("\u001b[T".getBytes());
							saida.flush();
							break;

						case "F9":
							saida.write("\u001b[U".getBytes());
							saida.flush();
							break;

						case "F10":
							saida.write("\u001b[V".getBytes());
							saida.flush();
							break;

						case "F11":
							saida.write("\u001b[W".getBytes());
							saida.flush();
							break;

						case "F12":
							saida.write("\u001b[X".getBytes());
							saida.flush();
							break;

						}
					} catch (IOException e) {
						e.printStackTrace();
					}

				}
			});
			pnlBotoes.add(btn, 0, ++lin);
		}

		return true;
	}

}
