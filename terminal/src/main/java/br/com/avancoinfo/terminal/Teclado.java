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

			if (log != null) {
				log.println("> " + event.toString());
				log.flush();
			}
			
			switch (event.getCode()) {
			
			case BACK_SPACE:
				saida.write("\u0008".getBytes());
				saida.flush();
				break;
			
			case TAB:
				saida.write("\u0009".getBytes());
				saida.flush();
				break;
			
			case UP:
				saida.write("\u001b[A".getBytes());
				saida.flush();
				break;
				
			case DOWN:
				saida.write("\u001b[B".getBytes());
				saida.flush();				
				break;
				
			case RIGHT:
				saida.write("\u001b[C".getBytes());
				saida.flush();				
				break;
				
			case LEFT:
				saida.write("\u001b[D".getBytes());
				saida.flush();				
				break;

			default:
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
