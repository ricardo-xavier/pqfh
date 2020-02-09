package br.com.avancoinfo.terminal;

import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.Date;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import com.sun.javafx.geom.Rectangle;
import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Terminal extends Stage {
	
	private static final int VERSAO = 3;
	private static final int LINHAS = 25;
	private static final int COLUNAS = 80;
	private static final int MARGEM = 5;
	private static final String FONTE = "Courier New";
	private static final int TAMFONTE = 20;
	private static final int ESC = 27;
	private static final int ESTADO_INICIAL = 0;
	
	private GraphicsContext contexto;
	private int larCar;
	private int altLin;
	private Font fonte;
	
    private char[][] dados;
    private BlockingQueue<Buffer> fila;

    private int estado = ESTADO_INICIAL;
    private int lin;
    private int col;
    private byte[] seq;
    private int iseq;
    
    private Rectangle r = new Rectangle();
    
    private PrintStream log;
    
    private Terminal terminal;
	
	public Terminal() {
		
		terminal = this;
		try {
			log = new PrintStream("terminal.log");
			log.println("terminal v" + VERSAO);
			log.println(new Date());
			log.println();
			log.flush();
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		
		fila = new LinkedBlockingQueue<>();
		fonte = new Font(FONTE, TAMFONTE);
		seq = new byte[32];

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
		setTitle("Terminal Avanço v" + VERSAO);
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
	
	void atualiza() {

		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				Buffer buf = null;
				synchronized (fila) {
					buf = fila.poll();
				}
				
				int tam = buf.getTam();
				byte[] comandos = buf.getDados();
				
				//System.err.println(tam);
				/*
				if (terminal.getLog() != null) {
					terminal.getLog().printf("%s %d %s%n", Thread.currentThread().getName(), tam, new String(comandos, 0, tam));
					terminal.getLog().flush();
				}
				*/
				
				for (int i=0; i<tam; i++) {
					
					byte c = comandos[i];
					//System.out.printf("%d %d %d %c %d %d %d %d %n", lin, col, c, (char) c, r.y, r.x, r.height, r.width);
					
					switch (estado) {
					
					case ESTADO_INICIAL:

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
						
						case ESC:
							estado = ESC;
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
						break;
					
					case ESC:
						seq[iseq++] = c;
						if (Character.isAlphabetic(c)) {
							Escape.processaSeq(terminal, seq, iseq);
							iseq = 0;
							estado = ESTADO_INICIAL;
						}
						break;
						
					}
						
				}
				
				if (log != null) {
					gravaTela(tam);
				}
				
				mostra();
			}
		});
	}
	
	public void mostra() {
		contexto.setFill(Color.WHITE);
		int x = r.x * larCar;
		int y = r.y * altLin;
		int lar = r.width * larCar;
		int alt = r.height * altLin; 
		contexto.fillRect(x, y, lar, alt);
		contexto.setFill(Color.BLACK);
		
		for (int i=r.y; i<r.y+r.height; i++) {
			if (i >= 25) {
				break;
			}
			String s = new String(dados[i], r.x, r.x+r.width);
			//System.out.println(i + " " + s);
			contexto.strokeText(s, MARGEM + r.x*larCar, MARGEM + i * altLin + altLin);
		}		
	}
	
	private void gravaTela(int n) {
		log.println();
		log.printf("%s %d,%d%n", Thread.currentThread().getName(), lin, col);
		log.println(n + " " + r);
		for (int y=0; y<LINHAS; y++) {
			log.printf("%02d %s%n", y+1, new String(dados[y], 0, COLUNAS));
		}
		log.println();
		log.flush();
	}

	public PrintStream getLog() {
		return log;
	}

	public int getLin() {
		return lin;
	}

	public void setLin(int lin) {
		this.lin = lin;
	}

	public int getCol() {
		return col;
	}

	public void setCol(int col) {
		this.col = col;
	}

	public char[][] getDados() {
		return dados;
	}

	public void setDados(char[][] dados) {
		this.dados = dados;
	}

	public Rectangle getR() {
		return r;
	}

	public void setR(Rectangle r) {
		this.r = r;
	}

	public static int getLinhas() {
		return LINHAS;
	}

	public static int getColunas() {
		return COLUNAS;
	}

	public BlockingQueue<Buffer> getFila() {
		return fila;
	}

	public void setFila(BlockingQueue<Buffer> fila) {
		this.fila = fila;
	}

}
