package br.com.avancoinfo.terminal;

public class Cursor extends Thread {
	
	private Terminal terminal;

	public Cursor(Terminal terminal) {
		this.terminal = terminal;
		setName("CURSOR");
	}
	
	
	@Override
	public void run() {
		
		while (terminal.isConectado()) {
			
			terminal.mostraCursor(false);
			
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
		}
	}

}
