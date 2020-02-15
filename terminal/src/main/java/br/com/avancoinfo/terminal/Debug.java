package br.com.avancoinfo.terminal;

public class Debug {

	public static void gravaTela(Terminal terminal, int n) {
		terminal.getLog().println();
		terminal.getLog().printf("%s %d,%d%n", Thread.currentThread().getName(), terminal.getLin(), terminal.getCol());
		terminal.getLog().println(n + " " + terminal.getR());

		for (int y=0; y<Terminal.getLinhas(); y++) {
			terminal.getLog().printf("%02d %s%n", y+1, new String(terminal.getDados()[y], 0, Terminal.getColunas()));
		}
		terminal.getLog().println();
		
		terminal.getLog().println("Fundo:");
		for (int y=0; y<Terminal.getLinhas(); y++) {
			terminal.getLog().printf("%02d ", y+1);
			for (int x=0; x<Terminal.getColunas(); x++) {
				terminal.getLog().printf("%c", terminal.getFundo()[y][x]);
			}
			terminal.getLog().println();
		}
		terminal.getLog().println();
		
		terminal.getLog().println("Frente:");
		for (int y=0; y<Terminal.getLinhas(); y++) {
			terminal.getLog().printf("%02d ", y+1);
			for (int x=0; x<Terminal.getColunas(); x++) {
				terminal.getLog().printf("%c", terminal.getFrente()[y][x]);
			}
			terminal.getLog().println();
		}
		terminal.getLog().println();
		
		terminal.getLog().println("Atributos:");
		for (int y=0; y<Terminal.getLinhas(); y++) {
			terminal.getLog().printf("%02d ", y+1);
			for (int x=0; x<Terminal.getColunas(); x++) {
				
				if (terminal.getAtributos()[y][x] == 0) {
					terminal.getLog().printf("   ", terminal.getAtributos()[y][x]);
				} else {
					terminal.getLog().printf("%02x ", terminal.getAtributos()[y][x]);
				}
			}
			terminal.getLog().println();
		}
		terminal.getLog().println();
		
		terminal.getLog().flush();
	}
	

}
