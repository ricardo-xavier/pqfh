package br.com.avancoinfo.terminal;

import javafx.scene.canvas.GraphicsContext;

// Z D     ?
// 3
// C D     4 
//
// @       Y

public class Acs {
	
	public static final int PROCESSADO = 0x40;
	
	private Terminal terminal;
	private GraphicsContext contexto;

	public Acs(Terminal terminal, GraphicsContext contexto) {
		this.terminal = terminal;
		this.contexto = contexto;
	}

	public void processa(int y, int x) {
		
		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		
		if ((atributos[y][x] & PROCESSADO) == PROCESSADO) {
			atributos[y][x] &= ~PROCESSADO;
		}
		
		atributos[y][x] |= PROCESSADO;
		
		switch (dados[y][x]) {
		
		case 'Z':
			cse(y, x);
			break;
			
		case '?':
			csd(y, x);
			break;
			
		case '@':
			cie(y, x);
			break;
			
		case 'Y':
			cid(y, x);
			break;
			
		case 'C':
			tEsq(y, x);
			break;
			
		case '4':
			tDir(y, x);
			break;
			
		case 'D':
			horizontal(y, x);
			break;
			
		case '3':
			vertical(y, x);
			break;
		
		}
	}
	
	private void cse(int y, int x) {
		
		int x2 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar();
		int y1 = Terminal.getMargem() + y * terminal.getAltLin() + terminal.getAltLin() / 2;
		
		int x1 = x2 - terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y1, x2, y1);
		
		int y2 = y1 + terminal.getAltLin() / 2;
		contexto.strokeLine(x1, y1, x1, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void csd(int y, int x) {
		
		int x1 = Terminal.getMargem() + x * terminal.getLarCar();
		int y1 = Terminal.getMargem() + y * terminal.getAltLin() + terminal.getAltLin() / 2;
		
		int x2 = x1 + terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y1, x2, y1);
		
		int y2 = y1 + terminal.getAltLin() / 2;
		contexto.strokeLine(x2, y1, x2, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void cie(int y, int x) {
		
		int x1 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar() / 2;
		int y1 = Terminal.getMargem() + y * terminal.getAltLin();
		
		int y2 = y1 + terminal.getAltLin() / 2;
		contexto.strokeLine(x1, y1, x1, y2);
		
		int x2 = x1 + terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y2, x2, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void cid(int y, int x) {
		
		int x2 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar() / 2;
		int y1 = Terminal.getMargem() + y * terminal.getAltLin();
		
		int y2 = y1 + terminal.getAltLin() / 2;
		contexto.strokeLine(x2, y1, x2, y2);
		
		int x1 = x2 - terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y2, x2, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void tEsq(int y, int x) {
		
		int x1 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar() / 2;
		int y1 = Terminal.getMargem() + y * terminal.getAltLin();
		
		int y2 = y1 + terminal.getAltLin() / 2;
		int y3 = y2 + terminal.getAltLin() / 2;
		
		contexto.strokeLine(x1, y1, x1, y3);
		
		int x2 = x1 + terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y2, x2, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void tDir(int y, int x) {
		
		int x2 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar() / 2;
		int y1 = Terminal.getMargem() + y * terminal.getAltLin();
		
		int y2 = y1 + terminal.getAltLin() / 2;
		int y3 = y2 + terminal.getAltLin() / 2;
		
		contexto.strokeLine(x2, y1, x2, y3);
		
		int x1 = x2 - terminal.getLarCar() / 2;
		contexto.strokeLine(x1, y2, x2, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void horizontal(int y, int x) {
		
		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		
		int x1 = Terminal.getMargem() + x * terminal.getLarCar();
		int y1 = Terminal.getMargem() + y * terminal.getAltLin() + terminal.getAltLin() / 2;
		
		int x2 = x;
		int j = x + 1;
		while ((j < Terminal.getColunas()) 
				&& (dados[y][j] == 'D') && ((atributos[y][j] & Escape.A_ACS) == Escape.A_ACS)) {
			x2 = j++;
			atributos[y][x2] |= PROCESSADO;
		}
		
		x2 = Terminal.getMargem() + x2 * terminal.getLarCar() + terminal.getLarCar();
		
		contexto.strokeLine(x1, y1, x2, y1);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}
	
	private void vertical(int y, int x) {
		
		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		
		int x1 = Terminal.getMargem() + x * terminal.getLarCar() + terminal.getLarCar() / 2;
		int y1 = Terminal.getMargem() + y * terminal.getAltLin();
		
		int y2 = y;
		int i = y + 1;
		while ((i < Terminal.getColunas()) 
				&& (dados[i][x] == '3') && ((atributos[i][x] & Escape.A_ACS) == Escape.A_ACS)) {
			y2 = i++;
			atributos[y2][x] |= PROCESSADO;
		}
		
		y2 = Terminal.getMargem() + y2 * terminal.getAltLin() + terminal.getAltLin();
		
		contexto.strokeLine(x1, y1, x1, y2);
		
		//System.err.printf("%d %d %d %d %n", x1, x2, y1, y1);
		
	}

}
