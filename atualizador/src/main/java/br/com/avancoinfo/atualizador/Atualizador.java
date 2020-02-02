package br.com.avancoinfo.atualizador;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;

import br.com.linx.avancoinfo.terminal.TerminalAvanco;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;
import javafx.stage.Stage;

public class Atualizador extends Application {

	public static TerminalAvanco terminal;
	
	public static int getVersaoAtual() throws ClientProtocolException, IOException {

		String url = "http://ricardoxavier.no-ip.org" + "/atualizadorws/versao/terminal";

		CloseableHttpClient httpClient = HttpClients.createDefault();

		HttpGet httpGet = new HttpGet(url);
		HttpResponse httpResponse = httpClient.execute(httpGet);

		InputStream inputStream = httpResponse.getEntity().getContent();
		BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
		String line = "";
		StringBuilder resultStr = new StringBuilder();
		while ((line = bufferedReader.readLine()) != null) {
			resultStr.append(line);
		}
		inputStream.close();

		return Integer.parseInt(resultStr.toString());

	}

	public static void main(String[] args) {

		int versaoInstalada = TerminalAvanco.getVersao();

		try {
			int versaoAtual = 2;//getVersaoAtual();
			if (versaoAtual > versaoInstalada) {
				launch(args);
				if (terminal != null) {
					terminal.close();
				}
			} else {
				TerminalAvanco.main(args);
			}
		} catch (Exception e) {
			TerminalAvanco.main(args);
		}

	}

	@Override
	public void start(Stage primaryStage) throws Exception {

		Alert alert = new Alert(AlertType.CONFIRMATION);
		alert.setTitle("Aviso");
		alert.setHeaderText("Atualização de versão");
		alert.showAndWait();
		ButtonType result = alert.getResult();
		if (result == ButtonType.CANCEL) {
			Platform.runLater(new Runnable() {

				@Override
				public void run() {
					try {
						terminal = new TerminalAvanco();
						terminal.start(null);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			});

		}

	}

}
