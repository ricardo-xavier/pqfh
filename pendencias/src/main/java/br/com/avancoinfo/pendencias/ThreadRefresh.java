package br.com.avancoinfo.pendencias;

import javafx.application.Platform;

public class ThreadRefresh extends Thread {
	
	Painel contexto;
	
	public ThreadRefresh(Painel contexto) {
		this.contexto = contexto;
	}

	@Override
	public void run() {

		while (true) {
			try {
				Thread.sleep(30000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			Platform.runLater(new Runnable() {

				@Override
				public void run() {
					contexto.atualiza();
				}
			});
		}
	}

}
