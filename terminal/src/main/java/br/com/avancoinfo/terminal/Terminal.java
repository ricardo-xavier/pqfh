package br.com.avancoinfo.terminal;

import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.Date;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import com.sun.javafx.geom.Rectangle;
import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.VPos;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.BorderPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

public class Terminal extends Stage {
	
	private static final int VERSAO = 10;
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
    private int[][] atributos;
    private char[][] frente;
    private char[][] fundo;
    private BlockingQueue<Buffer> fila;

    private int estado = ESTADO_INICIAL;
    private int lin;
    private int col;
    private byte[] seq;
    private int iseq;
    
    private Rectangle r = new Rectangle();
    
    private PrintStream log;
    
    private Terminal terminal;
    private Teclado teclado;
    private Escape escape;
    private Acs acs;
    
    private boolean cursorReverso;
    private boolean conectado;
    
	private int atributo = 0;
	private char corFrente = 'b';
	private char corFundo = 'W';
	
	private Label lblStatus;
	
	public Terminal(Configuracao cfg) {
		
		terminal = this;
		if (System.getenv("TERMINAL_DBG") != null) {
			try {
				log = new PrintStream("terminal.log");
				log.println("terminal v" + VERSAO);
				log.println(new Date());
				log.println();
				log.flush();
			
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
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
		contexto.setTextBaseline(VPos.TOP);  
		contexto.setFont(fonte);
		
		// cria um painel com o canvas
		BorderPane tela = new BorderPane();
		tela.setPadding(new Insets(0));
		tela.setCenter(canvas);
	
		// barra de status
		BorderPane statusBar = new BorderPane();
		statusBar.setMinHeight(48);
		statusBar.setMaxHeight(48);
		statusBar.setPadding(new Insets(0, 5, 0, 0));
		statusBar.setId("statusbar");
		tela.setBottom(statusBar);
		
		Image imageOk = new Image(getClass().getResourceAsStream("config.png"));
		Button btnConfig = new Button("", new ImageView(imageOk));
		statusBar.setRight(btnConfig);
		btnConfig.setOnAction(new EventHandler<ActionEvent>() {
			
			@Override
			public void handle(ActionEvent event) {
				cfg.showAndWait();
				r.setBounds(0, 0, COLUNAS, LINHAS);
				mostra();
			}
		});

		lblStatus = new Label("Conectando ...");
		statusBar.setCenter(lblStatus);
		
		// cria a cena
		setTitle("Terminal Avanço v" + VERSAO);
		Scene scene = new Scene(tela);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		// inicializa o terminal
        dados = new char[LINHAS][COLUNAS];
        atributos = new int[LINHAS][COLUNAS];
        frente = new char[LINHAS][COLUNAS];
        fundo = new char[LINHAS][COLUNAS];

        escape = new Escape(terminal);
        escape.clear("2");
        acs = new Acs(terminal, contexto);
        
        teclado = new Teclado(log);
        canvas.setFocusTraversable(true);

        tela.setOnKeyPressed(teclado);
        
        setOnCloseRequest(new EventHandler<WindowEvent>() {
			
			@Override
			public void handle(WindowEvent event) {
				System.err.println("close");
			}
		});
    }
	
	void atualiza() {

		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				Buffer buf = null;
				int tam = 0;
				byte[] comandos = null;
				
				synchronized (fila) {

					buf = fila.poll();
				
					tam = buf.getTam();
					comandos = buf.getDados();
				
					if (terminal.getLog() != null) {
						terminal.getLog().printf("%s -FILA %d %d %s%n", Thread.currentThread().getName(), fila.size(), tam, new String(comandos, 0, tam));
						terminal.getLog().flush();
					}
				}
				
				r.x = col;
				r.y = lin;
				r.width = 1;
				r.height = 1;

				mostraCursor(true);
				
				for (int i=0; i<tam; i++) {
					
					byte c = comandos[i];
					//log.printf("%d %d %d %c %d %d %d %d %n", lin, col, c, (char) c, r.y, r.x, r.height, r.width); log.flush(); 
					
					switch (estado) {
					
					case ESTADO_INICIAL:

						switch (c) {
					
						case '\r':
							col = 0;
							alteraRegiao(0, -1);
							break;
					
						case '\n':
						
							if (lin < (LINHAS - 1)) {
								lin++;
								alteraRegiao(-1, lin);
							
							} else {
					
								int aux = lin;
								lin = 0;
								escape.el();
								lin = aux;
								alteraRegiao(-1, -1);
							}
						
							break;
							
						case '\b':
							if (col > 0) {
								col--;
								dados[lin][col] = ' ';
								alteraRegiao(col, -1);
							}
							break;
						
						case ESC:
							estado = ESC;
							break;
					
						default:
					
							dados[lin][col] = (char) c;
							atributos[lin][col] = atributo;
							frente[lin][col] = corFrente;
							fundo[lin][col] = corFundo;
							
							if (col < (COLUNAS - 1)) {
								col++;
								alteraRegiao(col, -1);
								
							} else {
								col = 0;
								lin++;
								alteraRegiao(col, lin);
							}
						
							break;
						}
						break;
					
					case ESC:
						seq[iseq++] = c;
						if (Character.isAlphabetic(c)) {
							escape.processaSeq(seq, iseq);
							iseq = 0;
							estado = ESTADO_INICIAL;
						}
						break;
						
					}
						
				}
				
				if (log != null) {
					Debug.gravaTela(terminal, tam);
				}
				
				mostra();
				
			}
		});
		
	}
	
	public void mostra() {

		// pinta o fundo
		for (int i=r.y; i<r.y+r.height; i++) {
			if (i >= 25) {
				break;
			}
			
			for (int j=r.x; j<r.x+r.width; j++) {
				int x1 = j;
				char corX1 = (atributos[i][x1] & Escape.A_REVERSE) == Escape.A_REVERSE ? frente[i][x1] : fundo[i][x1];
				int x2 = x1;
				for (; x2<r.x+r.width; x2++) {
					char corX2 = (atributos[i][x2] & Escape.A_REVERSE) == Escape.A_REVERSE ? frente[i][x2] : fundo[i][x2];
					if (corX2 != corX1) {
						break;
					}
				}
				if (x2 == (r.x+r.width)) {
					x2--;
				}
				contexto.setFill(converteCor(corX1));
				int x = MARGEM + x1 * larCar;
				int y = MARGEM + i * altLin;
				int lar = (x2 - x1 + 1) * larCar;
				contexto.fillRect(x, y, lar, altLin);				
				j = x2;
			}
		}
		
		// mostra a frente
		char ultimaCor = '?';
		for (int i=r.y; i<r.y+r.height; i++) {
			if (i >= 25) {
				break;
			}
			for (int j=r.x; j<r.x+r.width; j++) {

				char cor = (atributos[i][j] & Escape.A_REVERSE) == Escape.A_REVERSE ? fundo[i][j] : frente[i][j];
				if (cor != ultimaCor) {
					contexto.setStroke(converteCor(cor));
					ultimaCor = cor;
				}
				
				if ((atributos[i][j] & Escape.A_ACS) == Escape.A_ACS) {
					acs.processa(i, j);
					continue;
				}
				
				String s = String.valueOf(dados[i][j]);
				contexto.strokeText(s, MARGEM + j*larCar, MARGEM + i*altLin);
			}
		}		
		
	}
	
	public Color converteCor(char cor) {
		switch (cor) { 
		case 'W': return Configuracao.getWhite();
		case 'b': return Configuracao.getBlack();
		case 'R': return Configuracao.getRed();
		case 'G': return Configuracao.getGreen();
		case 'Y': return Configuracao.getYellow();
		case 'B': return Configuracao.getBlue();
		case 'M': return Configuracao.getMagenta();
		case 'C': return Configuracao.getCyan();
		}
		return Color.WHITE;
	}
	
	public void mostraCursor(boolean remove) {
		if (!fila.isEmpty() && !remove) {
			return;
		}
		if (remove) {
			cursorReverso = true;
		}
		
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				if (cursorReverso && ((atributo & Escape.A_REVERSE) != Escape.A_REVERSE)) {
					contexto.setFill(converteCor(corFrente));
				} else {
					contexto.setFill(converteCor(corFundo));
				}
				int x = MARGEM + col * larCar;
				int y = MARGEM + lin * altLin;
				contexto.fillRect(x, y, larCar, altLin);
				if (dados[lin][col] != ' ') {
					if (cursorReverso && ((atributo & Escape.A_REVERSE) != Escape.A_REVERSE)) {
						contexto.setStroke(converteCor(corFundo));
					} else {
						contexto.setStroke(converteCor(corFrente));
					}					
					String s = String.valueOf(dados[lin][col]);
					if ((atributos[lin][col] & Escape.A_ACS) != Escape.A_ACS) {
						contexto.strokeText(s, x, y);
					} else {
						x += terminal.getLarCar() / 2;
						int y2 = y + terminal.getAltLin();
						contexto.strokeLine(x, y, x, y2);

					}
				}
				cursorReverso = !cursorReverso;
				
			}
		});
		
	}
	
	public void alteraRegiao(int x, int y) {
		
		int dif;
		
		if ((x == -1) && (x == -1)) {
			r.setBounds(0, 0, COLUNAS, LINHAS);
			return;
		}
		
		if (x != -1) {
			
			if (x < r.x) {
				dif = r.x - x;
				r.x -= dif;
				r.width += dif;
			}
		
			if (x >= (r.x + r.width)) {
				dif = x - r.x;
				r.width = dif + 1;
			}
			
		}
		
		if (y != -1) {
			
			if (y < r.y) {
				dif = r.y - y;
				r.y -= dif;
				r.width += dif;
			}
		
			if (y >= (r.y + r.height)) {
				dif = y - r.y;
				r.height = dif + 1;
			}
			
		}
		
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

	public void setSaida(OutputStream saida) {
		teclado.setSaida(saida);
	}

	public boolean isConectado() {
		return conectado;
	}

	public void setConectado(boolean conectado, String servidor, int porta, String usuario) {
		this.conectado = conectado;
		if (conectado) {
			new Cursor(this).start();
			Platform.runLater(() -> lblStatus.setText("Conectado a: " + servidor + ":" + porta + "  Usuário: " + usuario));
		}

	}

	public int getAtributo() {
		return atributo;
	}

	public void setAtributo(int atributo) {
		this.atributo = atributo;
	}

	public char getCorFrente() {
		return corFrente;
	}

	public void setCorFrente(char corFrente) {
		this.corFrente = corFrente;
	}

	public char getCorFundo() {
		return corFundo;
	}

	public void setCorFundo(char corFundo) {
		this.corFundo = corFundo;
	}

	public int[][] getAtributos() {
		return atributos;
	}

	public void setAtributos(int[][] atributos) {
		this.atributos = atributos;
	}

	public char[][] getFrente() {
		return frente;
	}

	public void setFrente(char[][] frente) {
		this.frente = frente;
	}

	public char[][] getFundo() {
		return fundo;
	}

	public void setFundo(char[][] fundo) {
		this.fundo = fundo;
	}

	public static int getMargem() {
		return MARGEM;
	}

	public int getLarCar() {
		return larCar;
	}

	public int getAltLin() {
		return altLin;
	}

}
