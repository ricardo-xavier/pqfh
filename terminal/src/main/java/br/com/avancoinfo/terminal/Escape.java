package br.com.avancoinfo.terminal;

public class Escape {

	public static final int A_REVERSE = 0x01;
	public static final int A_ACS = 0x02;
	public static final int A_BOLD = 0x04;
	public static final int A_UNDERLINE = 0x08;
	public static final int A_BLINK = 0x10;
	public static final int A_INVISIBLE = 0x20;

	private Terminal terminal;

	public Escape(Terminal terminal) {
		this.terminal = terminal;
	}

	public void processaSeq(char[] seq, int iseq) {

		String cmd = new String(seq, 0, iseq);

		/*
		if (terminal.getLog() != null) {
			synchronized (terminal.getLog()) {
				terminal.getLog().println("ESCAPE " + cmd);
				terminal.getLog().flush();
			}
		}
		*/

		String prms = "";
		if (seq[0] == '[') {
			prms = new String(seq, 1, iseq - 2);
		} else {
			prms = new String(seq, 0, iseq - 1);
		}

		switch (seq[iseq - 1]) {

		case 'A':
			cursorUp(prms);
			break;

		case 'B':
			cursorDown(prms);
			break;

		case 'C':
			cursorForward(prms);
			break;

		case 'D':
			cursorBackward(prms);
			break;

		case 'H':
			home(prms);
			break;

		case 'J':
			clear(prms);
			break;

		case 'K':
			el();
			break;

		case 'm':
			setaAtributos(cmd);
			break;

		default:
			Debug.grava("ESCAPE COMANDO PENDENTE " + cmd + "\n");

		}
	}

	private void cursorUp(String prms) {
		if (terminal.getLin() > 0) {
			int n = prms.equals("") ? 1 : Integer.parseInt(prms);
			int lin = terminal.getLin();
			if (lin >= n) {
				lin -= n;
			} else {
				lin = 0;
			}
			terminal.setLin(lin);
			terminal.alteraRegiao(-1, lin);
		}
	}

	private void cursorDown(String prms) {
		if (terminal.getLin() < (Terminal.getLinhas() - 1)) {
			int n = prms.equals("") ? 1 : Integer.parseInt(prms);
			terminal.setLin(terminal.getLin() + n);
			terminal.alteraRegiao(-1, terminal.getLin());
		}
	}

	private void cursorBackward(String prms) {
		int n = prms.equals("") ? 1 : Integer.parseInt(prms);
		int col = terminal.getCol();
		if (col >= n) {
			col -= n;
		} else {
			col = 0;
		}
		terminal.setCol(col);
		terminal.alteraRegiao(col, -1);
	}

	private void cursorForward(String prms) {
		int n = prms.equals("") ? 1 : Integer.parseInt(prms);
		terminal.setCol(terminal.getCol() + n);
		terminal.alteraRegiao(terminal.getCol(), -1);
	}

	public void clear(String prms) {

		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		char[][] frente = terminal.getFrente();
		char[][] fundo = terminal.getFundo();
		
		int x1 = terminal.getCol();
		int x2 = Terminal.getColunas() - 1;
		int y2 = Terminal.getLinhas() - 1;
		
		if (prms.equals("2")) {
			home("");
		}
		
		// apaga até o final da linha atual
		int i = terminal.getLin();
		for (int j = terminal.getCol(); j < Terminal.getColunas(); j++) {
			dados[i][j] = ' ';
			atributos[i][j] = terminal.getAtributo();
			frente[i][j] = terminal.getCorFrente();
			fundo[i][j] = terminal.getCorFundo();			
		}
		
		// apaga as linhas até o final
		for (++i; i < Terminal.getLinhas(); i++) {
			x1 = 0;
			for (int j = 0; j < Terminal.getColunas(); j++) {
				dados[i][j] = ' ';
				atributos[i][j] = terminal.getAtributo();
				frente[i][j] = terminal.getCorFrente();
				fundo[i][j] = terminal.getCorFundo();
			}
		}
		
		terminal.alteraRegiao(x1, -1);
		terminal.alteraRegiao(x2, y2);
	}

	private void home(String prms) {
		if (prms.contains(";")) {
			String[] coordenadas = prms.split(";");
			terminal.setLin(Integer.parseInt(coordenadas[0]) - 1);
			terminal.setCol(Integer.parseInt(coordenadas[1]) - 1);
		} else {
			terminal.setLin(0);
			terminal.setCol(0);
		}
		terminal.alteraRegiao(-1, -1);
	}

	public void el() {

		char[][] dados = terminal.getDados();
		int[][] atributos = terminal.getAtributos();
		char[][] frente = terminal.getFrente();
		char[][] fundo = terminal.getFundo();
		
		int i = terminal.getLin();
		for (int j = terminal.getCol(); j < Terminal.getColunas(); j++) {
			dados[i][j] = ' ';
			atributos[i][j] = terminal.getAtributo();
			frente[i][j] = terminal.getCorFrente();
			fundo[i][j] = terminal.getCorFundo();
		}
		terminal.alteraRegiao(Terminal.getColunas()-1, -1);
	}

	private void setaAtributos(String seq) {

		String[] atrs = seq.replace("[", "").replace("m", "").split(";");
		int atributo = terminal.getAtributo();

		for (String atr : atrs) {
			switch (atr) {

			// attributes
			case "":
			case "0": // normal display
				terminal.setAtributo(0);
				terminal.setCorFrente('W');
				terminal.setCorFundo('b');
				break;

			case "1": // bold
				atributo |= A_BOLD;
				terminal.setAtributo(atributo);
				break;

			case "4": // underline
				atributo |= A_UNDERLINE;
				terminal.setAtributo(atributo);
				break;

			case "5": // blink
				atributo |= A_BLINK;
				terminal.setAtributo(atributo);
				break;

			case "7": // reverse
				atributo |= A_REVERSE;
				terminal.setAtributo(atributo);
				break;

			case "8": // nondisplayed
				atributo |= A_INVISIBLE;
				terminal.setAtributo(atributo);
				break;

			case "10": // disable acs
				atributo &= ~A_ACS;
				terminal.setAtributo(atributo);
				break;

			case "12": // enable acs
				atributo |= A_ACS;
				terminal.setAtributo(atributo);
				break;

			// foreground colors
			case "30": // black
				terminal.setCorFrente('b');
				break;

			case "31": // red
				terminal.setCorFrente('R');
				break;

			case "32": // green
				terminal.setCorFrente('G');
				break;

			case "33": // yellow
				terminal.setCorFrente('Y');
				break;

			case "34": // blue
				terminal.setCorFrente('B');
				break;

			case "35": // magenta
				terminal.setCorFrente('M');
				break;

			case "36": // cyan
				terminal.setCorFrente('C');
				break;

			case "37": // white
				terminal.setCorFrente('W');
				break;

			// background colors
			case "40": // black
				terminal.setCorFundo('b');
				break;

			case "41": // red
				terminal.setCorFundo('R');
				break;

			case "42": // green
				terminal.setCorFundo('G');
				break;

			case "43": // yellow
				terminal.setCorFundo('Y');
				break;

			case "44": // blue
				terminal.setCorFundo('B');
				break;

			case "45": // magenta
				terminal.setCorFundo('M');
				break;

			case "46": // cyan
				terminal.setCorFundo('C');
				break;

			case "47": // white
				terminal.setCorFundo('W');
				break;

			default:
				Debug.grava("PENDENTE ATRIBUTO:" + atr + "\n");
			}
		}

	}

}
