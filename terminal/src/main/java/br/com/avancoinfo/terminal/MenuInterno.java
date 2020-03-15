package br.com.avancoinfo.terminal;

import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;

public class MenuInterno {
	
	private static int y = -1;
	private static int x1;
	private static int x2;
	private static char letraSelecionada = '?';
	private static String texto;
	
	public static boolean dentroMenu(int y, int x, Terminal terminal) {
		
		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		char[][] frente = terminal.getFrente();
		int linhas = Terminal.getLinhas();
		int colunas = Terminal.getColunas();
		int marcador = Terminal.getMARCADOR();
		
		// tem que estar dentro da tela e com pelo menos um espaço de cada lado para as bordas
		if ((y < 1) || (x < 1) || (x >= (colunas - 1)) || ((y >= linhas - 1))) {
			return false;
		}
		
		int x1=-1, x2=-1, y1=-1, y2=-1;
		
		// procura uma linha vertical para a esquerda
		for (int j=x-1; j>=0; j--) {
			if ((dados[y][j] == '3') && ((atributos[y][j] & Escape.A_ACS) == Escape.A_ACS)) {
				x1 = j;
				break;
			}
		}
		if (x1 == -1) {
			return false;
		}
		
		// procura uma linha vertical para a direita
		for (int j=x+1; j<colunas; j++) {
			if ((dados[y][j] == '3') && ((atributos[y][j] & Escape.A_ACS) == Escape.A_ACS)) {
				x2 = j;
				break;
			}
		}
		if (x2 == -1) {
			return false;
		}
		
		// para ser uma linha de menu deverá ter uma letra em destaque ou um marcador
		int destaques = 0;
		char letra = '?';
		for (int j=x1+1; j<x2; j++) {
			
			if (frente[y][j] == 'Y') {
				letra = dados[y][j];
				destaques++;
			}
			
			if (dados[y][j] == marcador) {
				letra = '^';
				destaques = 1;
				break;
			}
			
		}
		if ((letra == '?') || (destaques != 1)) {
			return false;
		}
		letraSelecionada = letra;
		
		// procura os cantos superiores
		for (int i=y; i>=0; i--) {
			
			// tem que seguir com ACS dos dois lados
			if (((atributos[i][x1] & Escape.A_ACS) != Escape.A_ACS)	|| ((atributos[i][x2] & Escape.A_ACS) != Escape.A_ACS)) {
				return false;
			}
			
			if ((dados[i][x1] == 'Z') && (dados[i][x2] == '?')) {
				y1 = i;
				break;
			}
		}
		if (y1 == -1) {
			return false;
		}

		// procura os cantos inferiores
		for (int i=y; i<linhas; i++) {
			
			// tem que seguir com ACS dos dois lados
			if (((atributos[i][x1] & Escape.A_ACS) != Escape.A_ACS) || ((atributos[i][x2] & Escape.A_ACS) != Escape.A_ACS)) {
				return false;
			}
			
			if ((dados[i][x1] == '@') && (dados[i][x2] == 'Y')) {
				y2 = i;
				break;
			}
		}
		if (y2 == -1) {
			return false;
		}
		
		// a moldura está fechada
		
		// verifica se todas as outras linhas tem uma letra em destaque ou um marcador
		for (int i=y1+1; i<y2; i++) {
		
			if (i == y) {
				continue;
			}
			
			destaques = 0;
			letra = '?';
			for (int j=x1+1; j<x2; j++) {
				
				if (frente[y][j] == 'Y') {
					letra = dados[y][j];
					destaques++;
				}
				
				if (dados[y][j] == marcador) {
					letra = '^';
					destaques = 1;
					break;
				}
				
			}
			if ((letra == '?') || (destaques != 1)) {
				return false;
			}
			
		}
		
		MenuInterno.y = y;
		MenuInterno.x1 = x1;
		MenuInterno.x2 = x2;
		return true;
	}

	public static void processa(Terminal terminal, BorderPane tela, MouseEvent event) {

		int y0 = (int) (Terminal.getMargem() + terminal.getPnlNavegacao().getHeight());
		int y = ((int) event.getY() - y0) / terminal.getAltLin();
		int x = ((int) event.getX() - Terminal.getMargem()) / terminal.getLarCar();
		tela.setCursor(javafx.scene.Cursor.DEFAULT);

		// o mouse moveu, mas ainda está na mesma opção do menu
		if ((y == MenuInterno.y) && (x >= MenuInterno.x1) && (x <= MenuInterno.x2)) {
			tela.setCursor(javafx.scene.Cursor.HAND);
			return;
		}

		// desmarca a posição anterior
		if (MenuInterno.y != -1) {
			terminal.alteraRegiao(MenuInterno.x1, MenuInterno.y);
			terminal.alteraRegiao(MenuInterno.x2, MenuInterno.y);
			terminal.mostra();
			MenuInterno.y = -1;
			letraSelecionada = '?';
		}
		
		if (!dentroMenu(y, x, terminal)) {
			return;
		}
		
		// o mouse está em uma opção de um menu interno
		tela.setCursor(javafx.scene.Cursor.HAND);
		texto = new String(terminal.getDados()[y], x1+1, x2-x1-1);
		terminal.mostraHover(y, x1+1, x2-1, texto);
									
	}

	public static char getLetraSelecionada() {
		return letraSelecionada;
	}

	public static void setLetraSelecionada(char letraSelecionada) {
		MenuInterno.letraSelecionada = letraSelecionada;
	}
}
