package br.com.linx.avancoinfo.terminal;

import com.sun.javafx.geom.Rectangle;
import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.Pane;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Terminal extends Stage {
	
	private static final int LINHAS = 25;
	private static final int COLUNAS = 80;
	private static final int MARGEM = 5;
	private static final String FONTE = "Courier New";
	private static final int TAMFONTE = 20;
	
	private GraphicsContext contexto;
	private int larCar;
	private int altLin;
	
    private char[][] dados;

    private static int lin;
    private static int col;
    
    private Rectangle r = new Rectangle();
	
	public Terminal() {
		
		Font fonte = new Font(FONTE, TAMFONTE);

		// calcula métricas
		FontMetrics fm = Toolkit.getToolkit().getFontLoader().getFontMetrics(fonte);
		altLin = (int) fm.getLineHeight();
		larCar = (int) fm.computeStringWidth("W");
		int altTela = altLin * LINHAS + MARGEM * 2;
		int larTela = larCar * COLUNAS + MARGEM * 2;
		
		// cria um canvas do tamanho da tela
		Canvas canvas = new Canvas(larTela, altTela);
		
		// seta a fonte
		contexto = canvas.getGraphicsContext2D();
		contexto.setFont(fonte);
		
		// cria um painel com o canvas
		Pane tela = new Pane();
		tela.getChildren().add(canvas);
		
		// cria a cena
		setTitle("Terminal Avanço");
		Scene scene = new Scene(tela);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		// inicializa o terminal
        dados = new char[LINHAS][COLUNAS];
        for (int y=0; y<LINHAS; y++) {
        	for (int x=0;x<COLUNAS; x++) {
        		dados[y][x] = ' ';
        	}
        }
        r.setBounds(-1, -1, 0, 0);
		
	}
	
	void atualiza(byte[] buf, int tam) {

		final byte[] comandos = buf;
		
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				for (int i=0; i<comandos.length; i++) {
					
					byte c = comandos[i];
					//System.out.printf("%d %d %d %c %d %d %d %d %n", lin, col, c, (char) c, r.y, r.x, r.height, r.width);
					
					switch (c) {
					
					case '\r':
						
						col = 0;
						if (col <  r.x) {
							int dif = r.x - col;
							r.x = col;
							r.width += dif;
						}						
						break;
					
					case '\n':
						
						if (lin < LINHAS) {
					
							if (++lin >= (r.y + r.height)) {
								r.height = lin - r.y + 1;
							}
							
						} else {
							
							//TODO
							i = comandos.length;
							lin = 0;
						}
						
						break;
					
					default:
						
						if (col < COLUNAS && lin < LINHAS) {
							
							if (r.x == -1) {
								r.x = col;
								r.y = lin;
								r.width = 1;
								r.height = 1;
							}
							
							if (col >= (r.x + r.width)) {
								r.width = col - r.x + 1;
							}
							
							dados[lin][col++] = (char) c;
							
						}
						
						break;
					
					}
				}
				
				int x = r.x * larCar;
				int y = r.y * altLin;
				int lar = r.width * larCar;
				int alt = r.height * altLin; 
				contexto.fillRect(x, y, lar, alt);
				
				contexto.strokeText("X", MARGEM + 0*larCar, MARGEM + altLin);
				contexto.strokeText("X", MARGEM + 0*larCar, MARGEM + LINHAS*altLin);
				contexto.strokeText("X", MARGEM + (COLUNAS-1)*larCar, MARGEM + altLin);
				contexto.strokeText("X", MARGEM + (COLUNAS-1)*larCar, MARGEM + LINHAS*altLin);		
			}
		});
	}

}
