package br.com.avancoinfo.terminal;

public class Escape {

	private static final char[] BRANCOS = "                                                                                                                                                                ".toCharArray();

	public static void processaSeq(Terminal terminal, byte[] seq, int iseq) {

		String cmd = new String(seq, 0, iseq);
		
		if (terminal.getLog() != null) {
			terminal.getLog().println("ESCAPE " + cmd);
			terminal.getLog().flush();
		}
		
		switch (cmd) {
		
		case "[H":
			home(terminal);
			break;
		
		case "[J":
			clear(terminal);
			break;
		
		default:
			if (terminal.getLog() != null) {
				terminal.getLog().println("ESCAPE COMANDO PENDENTE " + cmd);
				terminal.getLog().flush();
			}
			
		}
	}

	private static void clear(Terminal terminal) {
		for (int i=0; i<Terminal.getLinhas(); i++) {
			System.arraycopy(BRANCOS, 0, terminal.getDados()[i], 0, Terminal.getColunas());
		}
		terminal.getR().setBounds(0, 0, Terminal.getColunas(), Terminal.getLinhas());
		terminal.mostra();
	}

	private static void home(Terminal terminal) {
		terminal.setLin(0);
		terminal.setCol(0);
	}


}
