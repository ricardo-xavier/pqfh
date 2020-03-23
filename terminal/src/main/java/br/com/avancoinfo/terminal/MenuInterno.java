package br.com.avancoinfo.terminal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javafx.scene.control.Button;
import javafx.scene.input.KeyCode;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;

public class MenuInterno {
	
	private static int y = -1;
	private static int y1 = -1;
	private static int y2;
	private static int x1;
	private static int x2;
	private static char letraSelecionada = '?';
	private static String texto;
	private static Map<String, List<String>> menus = new HashMap<String, List<String>>();
	
	/**
	 * Verifica se uma coordenada está dentro de um menu
	 * 
	 * @param y
	 * @param x
	 * @return
	 */
	public static boolean dentroMenu(int y, int x) {
		
		Terminal terminal = TerminalAvanco.getTerminal();
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
		
		MenuInterno.setY(y);
		MenuInterno.setY1(y1);
		MenuInterno.setY2(y2);
		MenuInterno.setX1(x1);
		MenuInterno.setX2(x2);
		
		// carrega o mapa de menus
		String s = new String(dados[y1+1], x1+1, x2-x1-1).replace((char) marcador, ' ').trim();
		String chave = String.format("%d:%d:%d:%d:%s", x1, y1, x2, y2, s);
		
		if (menus.get(chave) == null) {
			List<String> opcoes = new ArrayList<String>();
			opcoes.add(s);
			for (int i=y1+2; i<y2; i++) {
				s = new String(dados[i], x1+1, x2-x1-1).replace((char) marcador, ' ').trim();
				opcoes.add(s);
			}
			menus.put(chave, opcoes);
			Debug.grava("+MENU " + chave + "\n");
			for (String o : opcoes) {
				Debug.grava("  " + o + "\n");
			}
		}
		
		return true;
	}

	/**
	 * Verifica se o mouse está em uma opção de menu
	 * Se tiver mostra a opção em destaque
	 * 
	 * @param terminal
	 * @param tela
	 * @param event
	 */
	public static void verificaMouse(BorderPane tela, MouseEvent event) {
		
		tela.setCursor(javafx.scene.Cursor.DEFAULT);
		
		if (MenuInterno.y1 == -1) {
			return;
		}
		
		Terminal terminal = TerminalAvanco.getTerminal();
		int y0 = (int) Terminal.getMargem();
		if (terminal.getPnlNavegacao() != null) {
			y0 += terminal.getPnlNavegacao().getHeight();
		}
		int y = ((int) event.getY() - y0) / terminal.getAltLin();
		int x = ((int) event.getX() - Terminal.getMargem()) / terminal.getLarCar();

		if ((y <= MenuInterno.y1) || (y >= MenuInterno.y2) || (x <= MenuInterno.x1) || (x >= MenuInterno.x2)) {

			// desmarca a posição anterior
			if (MenuInterno.y != -1) {
				terminal.alteraRegiao(MenuInterno.x1, MenuInterno.y);
				terminal.alteraRegiao(MenuInterno.x2, MenuInterno.y);
				terminal.mostra();
				MenuInterno.setY(-1);
				setLetraSelecionada('?');
			}
			
			return;
		}

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
			MenuInterno.setY(-1);
			setLetraSelecionada('?');
		}
		
		char[][] dados = terminal.getDados();
		char[][] frente = terminal.getFrente();
		int marcador = Terminal.getMARCADOR();

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
			return;
		}
		
		// o mouse está em uma opção de um menu interno
		setLetraSelecionada(letra);
		tela.setCursor(javafx.scene.Cursor.HAND);
		texto = new String(terminal.getDados()[y], x1+1, x2-x1-1);
		terminal.mostraHover(y, x1+1, x2-1, texto);
		setY(y);
									
	}

	/**
	 * Verifica se ainda tem um menu na posição marcada
	 * Se não for revalidado, o menu não será mais considerado na verificação da posição do mouse
	 * 
	 */
	public static void revalida() {
		
		Terminal terminal = TerminalAvanco.getTerminal();
		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		
		if (y1 == -1) {
			return;
		}
		
		if (((atributos[y1][x1] & Escape.A_ACS) != Escape.A_ACS)
			|| ((atributos[y1][x2] & Escape.A_ACS) != Escape.A_ACS)
			|| ((atributos[y2][x1] & Escape.A_ACS) != Escape.A_ACS)
			|| ((atributos[y2][x2] & Escape.A_ACS) != Escape.A_ACS)
			|| (dados[y1][x1] != 'Z')
			|| (dados[y1][x2] != '?')
			|| (dados[y2][x1] != '@')
			|| (dados[y2][x2] != 'Y')) {
				setY1(-1);
		}
		
	}

	/**
	 * Verifica se o cursor esta posicionado em um menu
	 * O cursor pode ficar no fim da linha do menu em cima da borda, ou no fim da tela 23,79
	 * 
	 * @return
	 */
	public static boolean posicionadoMenu() {

		Terminal terminal = TerminalAvanco.getTerminal();
		int lin = terminal.getLin();
		int col = terminal.getCol();
		char ch = terminal.getDados()[lin][col];
		int atr = terminal.getAtributos()[lin][col];
		
		boolean menu = false;
		if ((lin == (Terminal.getLinhas() - 2)) && (col == (Terminal.getColunas() - 1))) {
			menu = true;
		} else if ((ch == '3') && ((atr & Escape.A_ACS) == Escape.A_ACS)) {
			menu = true;
		}
		
		return menu;
		
	}
	
	/**
	 * Verifica se o marcador do painel de navegação ainda existe
	 * Se não existir, o botão será removido da navegação
	 * 
	 */
	public static void verificaRemocao() {
		
		Terminal terminal = TerminalAvanco.getTerminal();
		int n = terminal.getPnlNavegacao().getChildren().size();
		if (n == 0) {
			return;
		}
		
		// só remover se estiver posicionado em um menu 
		if (!posicionadoMenu()) {
			return;
		}
		
		Button btn = (Button) terminal.getPnlNavegacao().getChildren().get(n-1);
		if (btn.getUserData() != null) {
			Ponto p = (Ponto) btn.getUserData();
			char ch = terminal.getDados()[p.getY()][p.getX()];
			if (ch != Terminal.getMARCADOR()) {
				BarraNavegacao.remove(terminal.getPnlNavegacao(), n-1);
				verificaRemocao();
			}
		} else {
			
			// saida de uma tela não menu
			// a última tecla é ESC e o último menu está com ponto nulo
			if (Teclado.getUltimaTecla() == KeyCode.ESCAPE) {
				BarraNavegacao.remove(terminal.getPnlNavegacao(), n-1);
			}
			
		}
	}
	
	public static char getLetraSelecionada() {
		return letraSelecionada;
	}

	public static void setLetraSelecionada(char letraSelecionada) {
		MenuInterno.letraSelecionada = letraSelecionada;
	}

	public static int getY() {
		return y;
	}

	public static void setY(int y) {
		MenuInterno.y = y;
	}

	public static int getY1() {
		return y1;
	}

	public static void setY1(int y1) {
		MenuInterno.y1 = y1;
	}

	public static int getY2() {
		return y2;
	}

	public static void setY2(int y2) {
		MenuInterno.y2 = y2;
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

	public static String getTexto() {
		return texto;
	}

	/**
	 * Verifica se uma tecla pressionada vai selecionar uma opção do último menu aberto
	 * 
	 * @param code
	 */
	public static void verificaTecla(KeyCode code) {

		int colMax = -1;
		String chaveSelecionada = null;
		Terminal terminal = TerminalAvanco.getTerminal();
		
		String codeS = code.toString();
		char c = '?';
		if (codeS.length() == 1) {
			c = codeS.charAt(0);
		}
		if ((code != KeyCode.ENTER) && !Character.isAlphabetic(c)) {
			return;
		}

		// seleciona o menu mais a direita que esteja visivel
		for (String chave : menus.keySet()) {
			String[] partes = chave.split(":");
			int x1 = Integer.parseInt(partes[0]);
			int y1 = Integer.parseInt(partes[1]);
			int x2 = Integer.parseInt(partes[2]);
			int y2 = Integer.parseInt(partes[3]);
			if (((terminal.getAtributos()[y1][x1] & Escape.A_ACS) == Escape.A_ACS)
					&& ((terminal.getAtributos()[y1][x2] & Escape.A_ACS) == Escape.A_ACS)
					&& ((terminal.getAtributos()[y2][x1] & Escape.A_ACS) == Escape.A_ACS)
					&& ((terminal.getAtributos()[y2][x2] & Escape.A_ACS) == Escape.A_ACS)) {
				if (x1 > colMax) {
					colMax = x1;
					chaveSelecionada = chave;
				}
			}
		}

		if (chaveSelecionada == null) {
			return;
		}

		String[] partes = chaveSelecionada.split(":");
		int x1 = Integer.parseInt(partes[0]);
		int y1 = Integer.parseInt(partes[1]);
		int x2 = Integer.parseInt(partes[2]);
		int y2 = Integer.parseInt(partes[3]);

		String semDestaque = null;
		
		for (int y=y1+1; y<y2; y++) {
			
			String s = new String(terminal.getDados()[y]).substring(x1+1, x2).trim();

			int p = s.indexOf(Terminal.getMARCADOR());
			if (p >= 0) {
				s = s.substring(p+1).trim();
				if (code == KeyCode.ENTER) {
    				if (terminal.getPnlNavegacao() != null) {
    					BarraNavegacao.adiciona(terminal.getPnlNavegacao(), s);
    				}
					return;
				}
			}

			// procura letra destacada
			boolean temDestaque = false;
			for (int x=x1+1; x<x2; x++) {
				char cor = terminal.getFrente()[y][x];
				if (cor == 'Y')  {
					temDestaque = true;
					char ch = terminal.getDados()[y][x];
					if (ch == code.toString().charAt(0)) {
	    				if (terminal.getPnlNavegacao() != null) {
	    					BarraNavegacao.adiciona(terminal.getPnlNavegacao(), s);
	    				}
	    				return;
					}
				}
			}
			if (!temDestaque) {
				semDestaque = s;
			}

			// procura letra destacada
			for (int x=x1+1; x<x2; x++) {
				char ch = terminal.getDados()[y][x];
				if (Character.isUpperCase(ch) && (ch == code.toString().toUpperCase().charAt(0))) {
    				if (terminal.getPnlNavegacao() != null) {
    					BarraNavegacao.adiciona(terminal.getPnlNavegacao(), s);
    				}
    				break;
				}
			}

		}
		
		if (semDestaque != null) {
			if (terminal.getPnlNavegacao() != null) {
				BarraNavegacao.adiciona(terminal.getPnlNavegacao(), semDestaque);
			}			
		}

	}
	
}
