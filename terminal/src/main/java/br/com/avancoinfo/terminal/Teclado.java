package br.com.avancoinfo.terminal;

import java.io.IOException;
import java.io.OutputStream;

import javafx.event.EventHandler;
import javafx.scene.input.KeyEvent;

public class Teclado implements EventHandler<KeyEvent> {

	private OutputStream saida;
	private Terminal terminal;
	private Configuracao cfg;

	public Teclado(Terminal terminal, Configuracao cfg) {
		this.terminal = terminal;
		this.cfg = cfg;
	}

	@Override
	public void handle(KeyEvent event) {

		try {
			
			if (terminal.getLog() != null) {
				synchronized (terminal.getLog()) {
					terminal.getLog().println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " + event.toString());
					terminal.getLog().flush();
				}
			}
			
			switch (event.getCode()) {

			case ESCAPE:
				saida.write("\u001b".getBytes());
				saida.flush();
				break;
			
			case BACK_SPACE:
				saida.write("\u0008".getBytes());
				saida.flush();
				break;
				
			case ENTER:
				saida.write("\n".getBytes());
				saida.flush();
				break;
				
			case INSERT:
				break;
				
			case DELETE:
				saida.write("\u007f".getBytes());
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
				
			case PAGE_UP:
				saida.write("\u001b[I".getBytes());
				saida.flush();
				break;
				
			case DOWN:
				saida.write("\u001b[B".getBytes());
				saida.flush();				
				break;
				
			case PAGE_DOWN:
				saida.write("\u001b[G".getBytes());
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
				
			case F1:
				saida.write("\u001b[M".getBytes());
				saida.flush();				
				break;				
				
			case F2:
				saida.write("\u001b[N".getBytes());
				saida.flush();				
				break;				
				
			case F3:
				saida.write("\u001b[O".getBytes());
				saida.flush();				
				break;				
				
			case F4:
				saida.write("\u001b[P".getBytes());
				saida.flush();				
				break;				
				
			case F5:
				saida.write("\u001b[Q".getBytes());
				saida.flush();				
				break;				
				
			case F6:
				saida.write("\u001b[R".getBytes());
				saida.flush();				
				break;				
				
			case F7:
				saida.write("\u001b[S".getBytes());
				saida.flush();				
				break;				
				
			case F8:
				saida.write("\u001b[T".getBytes());
				saida.flush();				
				break;				
				
			case F9:
				saida.write("\u001b[U".getBytes());
				saida.flush();				
				break;				
				
			case F10:
				saida.write("\u001b[V".getBytes());
				saida.flush();				
				break;				
				
			case F11:
				saida.write("\u001b[W".getBytes());
				saida.flush();				
				break;				
				
			case F12:
				saida.write("\u001b[X".getBytes());
				saida.flush();				
				break;
				
			default:
				
				if (cfg.isPontoVirgula()) {
					if (event.getText().equals(".") || event.getText().equals(",")) {
						if (decimalPoint()) {
							saida.write(".,".getBytes());
							saida.flush();
							break;
						}
					}
				}
				saida.write(event.getText().getBytes());
				saida.flush();
				break;

			}

		} catch (IOException e) {
			e.printStackTrace();
		}

		event.consume();
	}

	private boolean decimalPoint() {
		if (terminal.getCol() == 0) {
			return false;
		}
		if (!Character.isDigit(terminal.getDados()[terminal.getLin()][terminal.getCol()-1])) {
			return false;
		}
		for (int i=0; i<Terminal.getLinhas(); i++) {
			for (int j=0; j<Terminal.getColunas(); j++) {
				if ((terminal.getAtributos()[i][j] & Escape.A_ACS) == Escape.A_ACS) {
					return true;
				}
			}
		}
		return false;
	}

	public void setSaida(OutputStream saida) {
		this.saida = saida;
	}

}
