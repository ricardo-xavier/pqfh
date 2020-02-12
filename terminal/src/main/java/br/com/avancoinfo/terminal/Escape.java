package br.com.avancoinfo.terminal;

public class Escape {

	private static final char[] BRANCOS = "                                                                                                                                                                "
			.toCharArray();
	
    private static final int A_REVERSE = 0x01;
    private static final int A_ACS = 0x02;
    private static final int A_BOLD = 0x04;
    private static final int A_UNDERLINE = 0x08;
    private static final int A_BLINK = 0x10;
    private static final int A_INVISIBLE = 0x20;
	
	private Terminal terminal;

	public void processaSeq(Terminal terminal, byte[] seq, int iseq) {

		this.terminal = terminal;
		String cmd = new String(seq, 0, iseq);

		if (terminal.getLog() != null) {
			terminal.getLog().println("ESCAPE " + cmd);
			terminal.getLog().flush();
		}

		switch (seq[iseq - 1]) {

		case 'H':
			home();
			break;

		case 'J':
			clear();
			break;

		case 'm':
			setaAtributos(cmd);
			break;

		default:
			if (terminal.getLog() != null) {
				terminal.getLog().println("ESCAPE COMANDO PENDENTE " + cmd);
				terminal.getLog().flush();
			}

		}
	}

	private void clear() {
		for (int i = 0; i < Terminal.getLinhas(); i++) {
			System.arraycopy(BRANCOS, 0, terminal.getDados()[i], 0, Terminal.getColunas());
		}
		terminal.getR().setBounds(0, 0, Terminal.getColunas(), Terminal.getLinhas());
		terminal.mostra();
	}

	private void home() {
		terminal.setLin(0);
		terminal.setCol(0);
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
				if (terminal.getLog() != null) {
					terminal.getLog().println("PENDENTE ATRIBUTO:" + atr);
					terminal.getLog().flush();
				}
			}
		}

	}

}
