package br.com.linx.avancoinfo.terminal;

import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.Pane;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Terminal extends Stage {
	
	private GraphicsContext gc;
	
	public Terminal() {
		
		Canvas canvas = new Canvas(750, 500);
		gc = canvas.getGraphicsContext2D();
		gc.setFont(new Font("Courier New", 20));
		
		Pane tela = new Pane();
		tela.getChildren().add(canvas);
		
		setTitle("Terminal Avanço");
		Scene scene = new Scene(tela);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		atualiza();
		
	}
	
	void atualiza() {
		gc.strokeText("Hello Canvas", 150, 100);		
	}

}
