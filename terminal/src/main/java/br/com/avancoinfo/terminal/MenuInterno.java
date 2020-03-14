package br.com.avancoinfo.terminal;

import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;

public class MenuInterno {
	
	private static int y = -1;
	private static int x1;
	private static int x2;
	private static char letraSelecionada = '?';

	public static void processa(Terminal terminal, BorderPane tela, MouseEvent event) {
		
		int y = ((int) event.getY() - Terminal.getMargem()) / terminal.getAltLin();
		int x = ((int) event.getX() - Terminal.getMargem()) / terminal.getLarCar();
		tela.setCursor(javafx.scene.Cursor.DEFAULT);
		
		if ((x >= Terminal.getColunas()) || (y >= Terminal.getLinhas())) {
			return;
		}
		
		if ((y == MenuInterno.y) && (x >= MenuInterno.x1) && (x <= MenuInterno.x2)) {
			tela.setCursor(javafx.scene.Cursor.HAND);
			return;
		}

		if (MenuInterno.y != -1) {
			terminal.alteraRegiao(MenuInterno.x1, MenuInterno.y);
			terminal.alteraRegiao(MenuInterno.x2, MenuInterno.y);
			terminal.mostra();
			MenuInterno.y = -1;
			letraSelecionada = '?';
		}
		
		// procura a linha vertical para a esquerda
		for (int x1=x-1; x1>=0; x1--) {
			if ((terminal.getDados()[y][x1] == '3') 
					&& ((terminal.getAtributos()[y][x1] & Escape.A_ACS) == Escape.A_ACS)) {
				
				// procura a linha vertical para a direita
				for (int x2=x+1; x2<Terminal.getColunas(); x2++) {
					if ((terminal.getDados()[y][x2] == '3') 
							&& ((terminal.getAtributos()[y][x2] & Escape.A_ACS) == Escape.A_ACS)) {

						// verifica se tem um caracter destacado
						int destaque=0;
						boolean todoReverso = true;
						for (x=x1+1; x<x2; x++) {
							if (terminal.getFrente()[y][x] == 'Y') {
								letraSelecionada = terminal.getDados()[y][x];
								destaque++;
							}
							if (((terminal.getAtributos()[y][x] & Escape.A_REVERSE) != Escape.A_REVERSE)
									&& (terminal.getFundo()[y][x] != terminal.getCorFrente())) {
								todoReverso = false;
							}
						}
						if (destaque != 1) {
							if (todoReverso) {
								letraSelecionada = '^';
							} else {
								return;
							}
						}

						// procura os cantos superiores
						for (int y1=y; y1>=0; y1--) {
							
							if (((terminal.getAtributos()[y1][x1] & Escape.A_ACS) != Escape.A_ACS)
								|| ((terminal.getAtributos()[y1][x2] & Escape.A_ACS) != Escape.A_ACS)) {
								return;
							}
							
							if ((terminal.getDados()[y1][x1] == 'Z') 
									&& (terminal.getDados()[y1][x2] == '?')) {

								// procura os cantos inferiores
								for (int y2=y; y2<Terminal.getLinhas(); y2++) {
									
									if (((terminal.getAtributos()[y2][x1] & Escape.A_ACS) != Escape.A_ACS)
										|| ((terminal.getAtributos()[y2][x2] & Escape.A_ACS) != Escape.A_ACS)) {
										return;
									}
									
									if ((terminal.getDados()[y2][x1] == '@') 
											&& (terminal.getDados()[y2][x2] == 'Y')) {
										
										// o mouse está em uma opção de um menu interno
										tela.setCursor(javafx.scene.Cursor.HAND);
										terminal.mostraHover(y, x1+1, x2-1);
										MenuInterno.y = y;
										MenuInterno.x1 = x1;
										MenuInterno.x2 = x2;
									}
									
									if ((terminal.getDados()[y2][x1] != '3') 
											|| (terminal.getDados()[y2][x2] != '3')) {
										return;
									}
									
								}
							}
							
							if ((terminal.getDados()[y1][x1] != '3') 
									|| (terminal.getDados()[y1][x2] != '3')) {
								return;
							}
							
							
						}
						return;
					}
				}
			}
		}
		
	}

	public static int getY() {
		return y;
	}

	public static void setY(int y) {
		MenuInterno.y = y;
	}

	public static int getX1() {
		return x1;
	}

	public static void setX1(int x1) {
		MenuInterno.x1 = x1;
	}

	public static int getX2() {
		return x2;
	}

	public static void setX2(int x2) {
		MenuInterno.x2 = x2;
	}

	public static char getLetraSelecionada() {
		return letraSelecionada;
	}

	public static void setLetraSelecionada(char letraSelecionada) {
		MenuInterno.letraSelecionada = letraSelecionada;
	}

}
