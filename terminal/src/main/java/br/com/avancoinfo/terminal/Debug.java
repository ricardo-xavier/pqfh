package br.com.avancoinfo.terminal;

import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.Date;

public class Debug {
	
	private static PrintStream log;
	private static final boolean DEBUG_CORES = false;
	private static final boolean DEBUG_ATRS = false;

	public static void gravaTela(int n) {
		
		if (log == null) {
			return;
		}
		
		Terminal terminal = TerminalAvanco.getTerminal();

		synchronized (log) {

			log.println();
			log.printf("%s %d,%d%n", Thread.currentThread().getName(), terminal.getLin(), terminal.getCol());
			log.println(n + " " + terminal.getR());

			for (int y = 0; y < Terminal.getLinhas(); y++) {
				log.printf("%02d %s%n", y + 1, new String(terminal.getDados()[y], 0, Terminal.getColunas()));
			}
			log.println();

			if (DEBUG_CORES) {
				
				log.println("Fundo:");
				for (int y = 0; y < Terminal.getLinhas(); y++) {
					log.printf("%02d ", y + 1);
					for (int x = 0; x < Terminal.getColunas(); x++) {
						log.printf("%c", terminal.getFundo()[y][x]);
					}
					log.println();
				}
				log.println();

				log.println("Frente:");
				for (int y = 0; y < Terminal.getLinhas(); y++) {
					log.printf("%02d ", y + 1);
					for (int x = 0; x < Terminal.getColunas(); x++) {
						log.printf("%c", terminal.getFrente()[y][x]);
					}
					log.println();
				}
				log.println();
			}

			if (DEBUG_ATRS) {
				log.println("Atributos:");
				for (int y = 0; y < Terminal.getLinhas(); y++) {
					log.printf("%02d ", y + 1);
					for (int x = 0; x < Terminal.getColunas(); x++) {
						log.printf("%02d ", terminal.getAtributos()[y][x]);
					}
					log.println();
				}
				log.println();
			}
			log.flush();
		}
	}

	public static void open(int versao) {
		if (System.getenv("TERMINAL_DBG") != null) {
			try {
				log = new PrintStream("terminal.log");
				synchronized (log) {
					log.println("terminal v" + versao);
					log.println(new Date());
					log.println();
					log.flush();
				}
			
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
		}
	}

	public static void grava(String s) {
		if (log == null) {
			return;
		}
		synchronized (log) {
			log.print(s);
			log.flush();
		}
	}

	public static void printStackTrace(Exception e) {
		if (log == null) {
			return;
		}
		synchronized (log) {
			e.printStackTrace(log);
			log.flush();
		}
	}

}
