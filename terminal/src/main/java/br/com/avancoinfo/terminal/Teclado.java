package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;

import javafx.event.EventHandler;
import javafx.scene.input.KeyEvent;

public class Teclado implements EventHandler<KeyEvent> {

	private OutputStream saida;
	private PrintStream log;

	public Teclado(PrintStream log) {
		this.log = log;
	}

	@Override
	public void handle(KeyEvent event) {

		try {

			switch (event.getCode()) {

			default:
				
				if (log != null) {
					log.println("> " + event.getText());
					log.flush();
				}
				
				saida.write(event.getText().getBytes());
				saida.flush();
				break;

			}

		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	public void setSaida(OutputStream saida) {
		this.saida = saida;
	}

}
