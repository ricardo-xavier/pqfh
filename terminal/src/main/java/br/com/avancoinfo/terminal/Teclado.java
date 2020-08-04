package br.com.avancoinfo.terminal;

import javafx.event.EventHandler;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

public class Teclado implements EventHandler<KeyEvent> {

	private Configuracao cfg;
	private static KeyCode ultimaTecla;
	private static char decimalPoint = '?';
	
	public Teclado(Configuracao cfg) {
		this.cfg = cfg;
	}

	@Override
	public void handle(KeyEvent event) {
		
		Comunicacao com = TerminalAvanco.getCom();

		Debug.grava("TECLA " + event.toString() + "\n");
		
		if (MenuInterno.posicionadoMenu()) {
			MenuInterno.verificaTecla(event.getCode());
		}
		
		setUltimaTecla(event.getCode());
			
		switch (event.getCode()) {

		case ESCAPE:
			com.envia("\u001b");
			break;
			
		case BACK_SPACE:
			com.envia("\u0008");
			break;
				
		case ENTER:
			com.envia("\n");
			break;
				
		case INSERT:
			break;
			
		case DELETE:
			com.envia("\u007f");
			break;
			
		case TAB:
			com.envia("\u0009");
			break;
			
		case UP:
			com.envia("\u001b[A");
			break;
				
		case PAGE_UP:
			com.envia("\u001b[I");
			break;
				
		case DOWN:
			com.envia("\u001b[B");
			break;
				
		case PAGE_DOWN:
			com.envia("\u001b[G");
			break;
				
		case RIGHT:
			com.envia("\u001b[C");
			break;
				
		case LEFT:
			com.envia("\u001b[D");
			break;
				
		case F1:
			com.envia("\u001b[M");
			break;				
				
		case F2:
			com.envia("\u001b[N");
			break;				
				
		case F3:
			com.envia("\u001b[O");
			break;				
				
		case F4:
			com.envia("\u001b[P");
			break;				
				
		case F5:
			com.envia("\u001b[Q");
			break;				
				
		case F6:
			com.envia("\u001b[R");
			break;				
				
		case F7:
			com.envia("\u001b[S");
			break;				
				
		case F8:
			com.envia("\u001b[T");
			break;				
				
		case F9:
			com.envia("\u001b[U");
			break;				
				
		case F10:
			com.envia("\u001b[V");
			break;				
				
		case F11:
			com.envia("\u001b[W");
			break;				
				
		case F12:
			com.envia("\u001b[X");
			break;
				
		default:
			
			if (cfg.isPontoVirgula() 
					&& (event.getText().equals(".") || event.getText().equals(","))) {
				
				int col = TerminalAvanco.getTerminal().getCol();
				if (col > 0) {
					int lin = TerminalAvanco.getTerminal().getLin();
					char[][] dados = TerminalAvanco.getTerminal().getDados();
					int[][] atributos = TerminalAvanco.getTerminal().getAtributos();
					if (Character.isDigit(dados[lin][col-1]) || ((atributos[lin][col-1] & Escape.A_ACS) == Escape.A_ACS)) {
						com.envia(getDecimalPoint(event.getText()));
						break;						
					}
				}
			}
			com.envia(event.getText());
			break;

		}

		event.consume();
	}

	private String getDecimalPoint(String tecla) {
		return (!Menu.isIntegral() || (decimalPoint == '?')) ? tecla : String.valueOf(decimalPoint);
	}
	
	public static KeyCode getUltimaTecla() {
		return ultimaTecla;
	}

	public static void setUltimaTecla(KeyCode ultimaTecla) {
		Teclado.ultimaTecla = ultimaTecla;
	}

	public static char getDecimalPoint() {
		return decimalPoint;
	}

	public static void setDecimalPoint(char decimalPoint) {
		Teclado.decimalPoint = decimalPoint;
	}

}
