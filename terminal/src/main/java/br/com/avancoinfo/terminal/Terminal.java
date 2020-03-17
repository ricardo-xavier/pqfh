package br.com.avancoinfo.terminal;

import java.io.FileNotFoundException;
import java.io.IOException;
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
import javafx.geometry.Pos;
import javafx.geometry.VPos;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;


enum EstadoLogin {
	OK,
	AGUARDANDO_PROMPT,
	AGUARDANDO_RESPOSTA
}

public class Terminal extends Stage {
	
	private static final int VERSAO = 23;
	private static final int LINHAS = 25;
	private static final int COLUNAS = 80;
	private static final int MARGEM = 5;
	private static final String FONTE = "Courier New";
	private static final int ESC = 27;
	private static final int BELL = 7;
	private static final int ESTADO_INICIAL = 0;
	private static final int ESTADO_ESC = 1;
	private static final int ESTADO_EXEC = 2;
	
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
    private char[] seq;
    private int iseq;
    
    private Rectangle r = new Rectangle();
    
    private PrintStream log;
    
    private Terminal terminal;
    private Teclado teclado;
    private Escape escape;
    private Acs acs;
    private Configuracao cfg;
    
    private boolean conectado;
    
	private int atributo = 0;
	private char corFrente = 'b';
	private char corFundo = 'W';
	
	private Label lblStatus;
	
	private Canvas canvas;
	
	// salva posição do cursor reverso para restaurar quando mudar de posição
	private boolean cursorReverso;
	private int linCursor = -1;
	private int colCursor;
	
	private EstadoLogin estadoLogin;
	
	private Menu menu;
	private boolean montarMenu;
	private boolean marcadorRecebido;
	private int linMarcador = -1;
	private int colMarcador = -1;
	private boolean enviarSenha;
	
	private Comunicacao com;
	private SelecaoFilial selecao;
	
	private static char MARCADOR = 65533;

	private GridPane pnlBotoes;
	private FlowPane pnlSocial;
	private FlowPane pnlNavegacao;
	
	public Terminal(Configuracao cfg) {
		
		this.cfg = cfg;
		terminal = this;
		if (System.getenv("TERMINAL_DBG") != null) {
			try {
				log = new PrintStream("terminal.log");
				synchronized (log) {
					log.println("terminal v" + VERSAO);
					log.println(new Date());
					log.println();
					log.flush();
				}
			
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
		}
		
		fila = new LinkedBlockingQueue<>();
		fonte = new Font(FONTE, cfg.getTamFonte());
		seq = new char[256];

		// calcula métricas
		FontMetrics fm = Toolkit.getToolkit().getFontLoader().getFontMetrics(fonte);
		altLin = (int) fm.getLineHeight();
		larCar = (int) fm.computeStringWidth("W");
		int altTela = altLin * LINHAS + MARGEM * 2;
		int larTela = larCar * COLUNAS + MARGEM * 2;
		
		// cria um canvas do tamanho da tela
		canvas = new Canvas(larTela, altTela);
		
		// seta a fonte
		contexto = canvas.getGraphicsContext2D();
		contexto.setTextBaseline(VPos.TOP);  
		contexto.setFont(fonte);
		
		// cria o painel principal
		BorderPane tela = new BorderPane();
		tela.setPadding(new Insets(0));
		tela.setCenter(canvas);
		
		// barra de navegação
		if (cfg.isBarraNavegacao()) {
			pnlNavegacao = new FlowPane();
			tela.setTop(pnlNavegacao);
			pnlNavegacao.setId("pnlNavegacao");
			pnlNavegacao.setVisible(false);
		}
	
		// barra de status
		BorderPane statusBar = new BorderPane();
		statusBar.setMinHeight(48);
		statusBar.setMaxHeight(48);
		statusBar.setPadding(new Insets(0, 5, 0, 0));
		statusBar.setId("statusbar");
		tela.setBottom(statusBar);
		
		pnlSocial = new FlowPane();
		statusBar.setLeft(pnlSocial);
		pnlSocial.setMaxWidth(100);
		pnlSocial.setVisible(false);
		pnlSocial.setAlignment(Pos.CENTER);

		Image imageSite = new Image(getClass().getResourceAsStream("a.png"));//, 32, 32, false, false);
		ImageView ivSite = new ImageView(imageSite);
		ivSite.getStyleClass().add("botao");
		pnlSocial.getChildren().add(ivSite);
		ivSite.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://avancoinfo.com.br/";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});

		Image imageFace = new Image(getClass().getResourceAsStream("face.png"));//, 48, 48, false, false);
		ImageView ivFace = new ImageView(imageFace);
		ivFace.getStyleClass().add("botao");
		pnlSocial.getChildren().add(ivFace);
		ivFace.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://facebook.com/avancoinfo/";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});

		Image imageLinkedin = new Image(getClass().getResourceAsStream("in.png"));//, 32, 32, false, false);
		ImageView ivLinkedin = new ImageView(imageLinkedin);
		ivLinkedin.getStyleClass().add("botao");
		pnlSocial.getChildren().add(ivLinkedin);
		ivLinkedin.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://pt.linkedin.com/company/avan-o-inform-tica";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		
		FlowPane.setMargin(ivSite, new Insets(4));
		FlowPane.setMargin(ivFace, new Insets(4));
		FlowPane.setMargin(ivLinkedin, new Insets(4));
		
		Image imageOk = new Image(getClass().getResourceAsStream("config.png"));
		Button btnConfig = new Button("", new ImageView(imageOk));
		btnConfig.getStyleClass().add("botao");
		btnConfig.setFocusTraversable(false);
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
		
		if (cfg.isBotoesFuncao()) {
			pnlBotoes = new GridPane();
			pnlBotoes.setVgap(5);
			pnlBotoes.setMinWidth(150);
			pnlBotoes.setMaxWidth(150);
			tela.setRight(pnlBotoes);
		}
		
		// cria a cena
		setTitle("Terminal Avanço v" + VERSAO);
		Scene scene = new Scene(tela);
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		scene.getStylesheets().add(Terminal.class.getResource("avanco.css").toExternalForm());
		
		// inicializa o terminal
        dados = new char[LINHAS][COLUNAS];
        atributos = new int[LINHAS][COLUNAS];
        frente = new char[LINHAS][COLUNAS];
        fundo = new char[LINHAS][COLUNAS];

        escape = new Escape(terminal);
        escape.clear("2");
        acs = new Acs(terminal, contexto);
        
        teclado = new Teclado(terminal, cfg);
        canvas.setFocusTraversable(true);

        tela.setOnKeyPressed(teclado);
        
        if (cfg.isMouseMenus()) {
        	tela.setOnMouseMoved(new EventHandler<MouseEvent>() {

        		@Override
        		public void handle(MouseEvent event) {
        			MenuInterno.processa(terminal, tela, event);
        		}
        	});
        
        	tela.setOnMouseClicked(new EventHandler<MouseEvent>() {

        		@Override
        		public void handle(MouseEvent event) {
        			if (MenuInterno.getLetraSelecionada() != '?') {
        				com.envia(MenuInterno.getLetraSelecionada() == '^' ? "\n" : String.valueOf(MenuInterno.getLetraSelecionada()));
        				MenuInterno.setLetraSelecionada('?');
        				if (pnlNavegacao != null) {
        					BarraNavegacao.adiciona(pnlNavegacao, MenuInterno.getTexto());
        				}
        			}
        		}
        	
        	});
        }
        canvas.requestFocus();
        
        setOnCloseRequest(new EventHandler<WindowEvent>() {
			
			@Override
			public void handle(WindowEvent event) {
				System.err.println("close terminal");
			}
		});
        
		estadoLogin = EstadoLogin.OK;
		
		if ((cfg.getUsuarioIntegral() != null) && !cfg.getUsuarioIntegral().trim().equals("")) {
			estadoLogin = EstadoLogin.AGUARDANDO_PROMPT;
		}
        
    }
	
	void atualiza() {

		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				Buffer buf = null;
				int tam = 0;
				byte[] comandos = null;
				marcadorRecebido = false;
				
				synchronized (fila) {

					buf = fila.poll();
				
					tam = buf.getTam();
					comandos = buf.getDados();
				
					if (log != null) {
						synchronized (log) {
							log.printf("%s -FILA %d %d %s%n", Thread.currentThread().getName(), fila.size(), tam, new String(comandos, 0, tam));
							log.flush();
						}
					}
				}
				
				r.x = col;
				r.y = lin;
				r.width = 1;
				r.height = 1;
				
				String s = new String(comandos, 0, tam);
				tam = s.length();

				int linMarcadorTemp = -1;
				int colMarcadorTemp = -1;
				boolean fecharMenu = false;
				
				for (int i=0; i<tam; i++) {
					
					int c = s.charAt(i);
					char ch = s.charAt(i);
					
					if (c == MARCADOR) {
						
						if (log != null) {
							synchronized (log) {
								log.printf("marcador encontrado col=%d montar=%s menu=%s%n", col, montarMenu, menu != null);
								log.flush();
							}
						}
						marcadorRecebido = true;
						
						if (montarMenu) {
							montarMenu = false;
							menu = new Menu(dados, frente, com, pnlNavegacao);
							hide();
							menu.showAndWait();
							if ((menu != null) && menu.isEncerrar()) {
								close();
								return;
							}
							show();
							
						} else {
							if ((menu != null) && ((col == 3) || (col == 4))) {
								
								if (MenuInterno.dentroMenu(lin, col, terminal)) {
									linMarcador = lin;
									colMarcador = col;
								}
								
								fecharMenu = true;
							}
							
							if ((estadoLogin == EstadoLogin.OK) && (menu == null) && (col > 4)) {
								linMarcadorTemp = lin;
								colMarcadorTemp = col;
							}
							
						}
					}
					
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
					
								scroll();
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
							estado = ESTADO_ESC;
							break;
							
						case BELL:
							break;
					
						default:
					
							dados[lin][col] = ch;
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
					
					case ESTADO_ESC:
						seq[iseq++] = ch;
						if (Character.isAlphabetic(ch)) {
							escape.processaSeq(seq, iseq);
							// ^[[]RunDLL32.EXE shell32.dll,ShellExec_RunDLL "\tmp\enum.pdf"^[[1*^M
							if ((ch == 'R') 
									&& new String(seq, 0, iseq).equals("[]R")
									&& (i < (tam - 2))
									&& (comandos[i+1] == 'u')
									&& (comandos[i+2] == 'n')) {
								estado = ESTADO_EXEC;
								
							} else {
								iseq = 0;
								estado = ESTADO_INICIAL;	
							}
						}
						break;
						
					case ESTADO_EXEC:
						seq[iseq++] = ch;
						if (ch == '*') {
							estado = ESTADO_INICIAL;
							String cmd = new String(seq, 0, iseq);
							System.err.println(cmd);
							int p = cmd.indexOf("RunDLL ");
							if (p > 0) {
								cmd = cmd.substring(p + 6);
								p = cmd.indexOf("\u001b");
								if (p > 0) {
									cmd = cmd.substring(0, p);
									try {
										Runtime.getRuntime().exec("explorer " + cmd);
									} catch (IOException e) {
										e.printStackTrace();
									}
								}
							}
						}
						break;
						
						
					}
						
				}
				
				if (log != null) {
					Debug.gravaTela(terminal, tam);
				}
				

				if (fecharMenu) {
					if (log != null) {
						synchronized (log) {
							log.printf("fechando menu...");
							log.flush();
						}
					}
				
					menu.close();
					estadoLogin = EstadoLogin.OK;
					menu = null;
					alteraRegiao(-1, -1);
				}
				

				if (linMarcadorTemp != -1) {
					if (MenuInterno.dentroMenu(linMarcadorTemp, colMarcadorTemp, terminal)) {
						linMarcador = lin;
						colMarcador = col;
					}
					
					if (pnlNavegacao != null) {
						if (teclado.getUltimaTecla() == KeyCode.ESCAPE) {
							// verifica se tem que retirar um menu da barra de navegação
							BarraNavegacao.removeEsc(pnlNavegacao, terminal);
						}
					}
					
				}

				if (estadoLogin == EstadoLogin.AGUARDANDO_PROMPT) {
					String msg = new String(dados[12]).toLowerCase();
					
					if (msg.contains("senha:")) {
						com.envia(cfg.getUsuarioIntegral());
						if (cfg.getUsuarioIntegral().length() < 4) {
							com.envia("\n");
						}
						try {
							Thread.sleep(100);
						} catch (InterruptedException e) {
						}
						com.envia((cfg.getSenhaIntegral()));
						if (cfg.getSenhaIntegral().length() < 5) {
							com.envia("\n");
						}
						estadoLogin = EstadoLogin.AGUARDANDO_RESPOSTA;
						
					}
					return;
				}

				if (estadoLogin == EstadoLogin.AGUARDANDO_RESPOSTA) {
					String msg = new String(dados[22]).toLowerCase();
					if (msg.contains("senha invalida")) {
						erro("Erro no acesso ao sistema", "Senha do integral inválida");
						return;
					}

					if (msg.contains("nao cadastrado")) {
						erro("Erro no acesso ao sistema", "Usuário do integral não cadastrado");
						return;						
					}

					if (fila.isEmpty()) {
						verificaAviso();
						verificaMenuPrincipal();
						verificaFilial();
					}
					return;
				}
				
				if (fila.isEmpty()) {
					verificaMenuPrincipal();
					verificaFilial();
				}
				
				mostra();
				
			}
		});
		
	}

	private void verificaAviso() {
		
		String msg = new String(dados[23]);
		if (msg.contains("DIGITE A SENHA DO USUARIO PARA CONTINUAR") && !enviarSenha) {
			
			enviarSenha = true;
			
			if (log != null) {
				synchronized (log) {
					log.println("aviso encontrado");
					log.flush();
				}
			}
			
			com.envia((cfg.getSenhaIntegral()));
			if (cfg.getSenhaIntegral().length() < 5) {
				com.envia("\n");
			}
			
		}
		
	}
	
	private boolean verificaMenuPrincipal() {
		
		String msg = new String(dados[5]);
		if (msg.contains("P  r  i  n  c  i  p  a  l") && (menu == null) && !montarMenu) {
			
			if (log != null) {
				synchronized (log) {
					log.println("menu principal encontrado " + marcadorRecebido);
					log.flush();
				}
			}

			if (marcadorRecebido) {
				menu = new Menu(dados, frente, com, pnlNavegacao);
				hide();
				menu.showAndWait();
				if ((menu != null) && menu.isEncerrar()) {
					close();
					return false;
				}
				show();
			} else {
				// vai montar o menu quando receber o marcador
				// evita que o menu fique incompleto
				montarMenu = true;
			}
			return true;
		}
		return false;
		
	}

	private boolean verificaFilial() {

		String msg = new String(dados[16]);
		if (msg.contains("<99> Consolidado")) {

			if (selecao != null) {
				return false;
			}
			
			try {
				Thread.sleep(100);
			} catch (InterruptedException e1) {
				e1.printStackTrace();
			}
			
			if (log != null) {
				synchronized (log) {
					log.println("selecao de filial encontrada");
					log.flush();
				}
			}

			selecao = new SelecaoFilial(dados);
			selecao.showAndWait();
			String filial = selecao.getFilial();
			
			if (filial == null) {
				com.envia("\u001b");	
			} else {
				com.envia(filial);
				if (filial.length() < 2) {
					com.envia("\n");
				}
			}

			return true;
		}

		selecao = null;
		return false;
	}

	private void scroll() {

		for (int i=0; i<(Terminal.getLinhas() - 1); i++) {
			for (int j = terminal.getCol(); j < Terminal.getColunas(); j++) {
				dados[i][j] = dados[i+1][j];
				atributos[i][j] = atributos[i+1][j];
				frente[i][j] = frente[i+1][j];
				fundo[i][j] = fundo[i+1][j];
			}
		}
		
		int i = Terminal.getLinhas() - 1;
		for (int j = terminal.getCol(); j < Terminal.getColunas(); j++) {
			dados[i][j] = ' ';
			atributos[i][j] = terminal.getAtributo();
			frente[i][j] = terminal.getCorFrente();
			fundo[i][j] = terminal.getCorFundo();
		}
		
	}
	
	public void mostra() {
		
		if ((linMarcador != -1) && (dados[linMarcador][colMarcador] != MARCADOR)) {
			linMarcador = -1;
		}
		MenuInterno.revalida(terminal);
		
		if (estadoLogin != EstadoLogin.OK) {
			if (pnlNavegacao != null) {
				pnlNavegacao.setVisible(false);
			}
			pnlSocial.setVisible(false);
			return;
		}
		if (pnlNavegacao != null) {
			pnlNavegacao.setVisible(true);
		}
		pnlSocial.setVisible(true);

		// pinta o fundo
		for (int i=r.y; i<r.y+r.height; i++) {
			if (i >= 25) {
				break;
			}
			
			for (int j=r.x; j<r.x+r.width; j++) {
				atributos[i][j] &= ~Acs.PROCESSADO;
				int x1 = j;
				char corX1 = (atributos[i][x1] & Escape.A_REVERSE) == Escape.A_REVERSE ? frente[i][x1] : fundo[i][x1];
				int x2 = x1;
				for (; x2<r.x+r.width; x2++) {
					char corX2 = (atributos[i][x2] & Escape.A_REVERSE) == Escape.A_REVERSE ? frente[i][x2] : fundo[i][x2];
					if (corX2 != corX1) {
						x2--;
						break;
					}
				}
				if (x2 == (r.x+r.width)) {
					x2--;
				}
				contexto.setFill(converteCor(corX1));
				int x = MARGEM + x1 * larCar;
				int y = MARGEM + i * altLin;
				int lar = x2 - x1 + 1;
				contexto.fillRect(x, y, lar * larCar, altLin);				
				for (int k=j; k<=x2; k++) {
					atributos[i][k] &= ~Acs.PROCESSADO;
				}
				j = x2;
			}
		}
		
		// mostra a frente
		char ultimaCor = '?';
		for (int i=r.y; i<r.y+r.height; i++) {
			
			if (i >= 25) {
				break;
			}
			
			if (((i == 22) || (i == 23)) && cfg.isBotoesFuncao()) {
				String s = new String(dados[i]);
				if (s.contains(":") && s.contains("F")) {
					if (new BotoesFuncao().processa(s, pnlBotoes, com)) {
						contexto.setFill(converteCor(frente[i][0]));
						int x = MARGEM;
						int y = MARGEM + i * altLin;
						contexto.fillRect(x, y, COLUNAS * larCar, altLin);
						continue;
					}
				}
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
				
				String s = dados[i][j] != MARCADOR ? String.valueOf(dados[i][j]) : " ";
				contexto.strokeText(s, MARGEM + j*larCar, MARGEM + i*altLin);
			}
		}		
		
	}
	
	public void mostraHover(int y, int x1, int x2, String texto) {

		if (!cfg.isMouseMenus()) {
			return;
		}
		
		String s = new String(dados[y], x1, x2-x1+1);
		if (!s.equals(texto)) {
			return;
		}
		contexto.setFill(Color.DARKBLUE);
		int _x = MARGEM + x1 * larCar;
		int _y = MARGEM + y * altLin;
		int lar = x2 - x1 + 1;
		contexto.fillRect(_x, _y, lar * larCar, altLin);				
		
		// mostra a frente
		char ultimaCor = '?';
		for (int j=x1; j<=x2; j++) {
			char cor = (atributos[y][j] & Escape.A_REVERSE) == Escape.A_REVERSE ? fundo[y][j] : frente[y][j];
			if (cor != ultimaCor) {
				contexto.setStroke(converteCor(cor));
				ultimaCor = cor;
			}
			s = dados[y][j] != MARCADOR ? String.valueOf(dados[y][j]) : " ";
			contexto.strokeText(s, MARGEM + j*larCar, MARGEM + y*altLin);
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
	
	public void mostraCursor() {
		
		if (estadoLogin != EstadoLogin.OK) {
			return;
		}
		
		// não mostra o cursor enquando estiver recebendo dados
		if (!fila.isEmpty()) {
			return;
		}
		
		// o cursor estava em reverso e mudou de posição
		// apaga o cursor
		if ((linCursor != -1) && ((linCursor != lin) || (colCursor != col))) {
			
			final int linRestaurar = linCursor;
			final int colRestaurar = colCursor;
			
			Platform.runLater(new Runnable() {

				@Override
				public void run() {

					boolean acs = (atributos[linRestaurar][colRestaurar] & Escape.A_ACS) == Escape.A_ACS;

					if (!acs) {
						boolean reverso = (atributos[linRestaurar][colRestaurar] & Escape.A_REVERSE) == Escape.A_REVERSE;
						Color fundoCursor = converteCor(reverso ? frente[linRestaurar][colRestaurar] : fundo[linRestaurar][colRestaurar]);
						Color frenteCursor = converteCor(reverso ? fundo[linRestaurar][colRestaurar] : frente[linRestaurar][colRestaurar]);
						char ch = dados[linRestaurar][colRestaurar];
					
						contexto.setFill(fundoCursor);
						contexto.setStroke(frenteCursor);
					
						int x = MARGEM + colRestaurar * larCar;
						int y = MARGEM + linRestaurar * altLin;
					
						contexto.fillRect(x, y, larCar, altLin);
						String s = String.valueOf(ch);
						contexto.strokeText(s, x, y);
					}
				
				}
					
			});

			cursorReverso = true;
			linCursor = -1;
		}
		
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {

				boolean acs = (atributos[lin][col] & Escape.A_ACS) == Escape.A_ACS;
				if (!acs) {
					boolean reverso = (atributos[lin][col] & Escape.A_REVERSE) == Escape.A_REVERSE;
					Color fundoCursor = converteCor(reverso ? frente[lin][col] : fundo[lin][col]);
					Color frenteCursor = converteCor(reverso ? fundo[lin][col] : frente[lin][col]);
					char ch = dados[lin][col];
				
					int x = MARGEM + col * larCar;
					int y = MARGEM + lin * altLin;
				
					// fundo do cursor
					if (cursorReverso) {
						contexto.setFill(frenteCursor);
					} else {
						contexto.setFill(fundoCursor);
					}
					contexto.fillRect(x, y, larCar, altLin);

					// frente do cursor
					if (cursorReverso) {
						contexto.setStroke(fundoCursor);
					} else {
						contexto.setStroke(frenteCursor);
					}	
				
					if (ch != ' ') {
						String s = String.valueOf(ch);
						contexto.strokeText(s, x, y);
					}
					
				}

				// salva
				if (cursorReverso) {
					linCursor = lin;
					colCursor = col;
				} else {
					linCursor = -1;
				}
				
				cursorReverso = !cursorReverso;
				
			}
		});
		
	}
	
	public void reconecta(String msg) {
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				Alert alert = new Alert(AlertType.ERROR);
				alert.setTitle("Erro");
				alert.setHeaderText("Erro na comunicação com o servidor");
				alert.setContentText(msg);
				alert.showAndWait();		
				
				cfg.showAndWait();
				if (cfg.isCancelado()) {
					close();
					return;
				}
				
				Comunicacao com = new Comunicacao(terminal, cfg);
				com.setName("COMUNICACAO");
				com.start();				
				TerminalAvanco.setCom(com);
				
			}
		});
	}
	
	public void erro(String titulo, String msg) {
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				
				Alert alert = new Alert(AlertType.ERROR);
				alert.setTitle("Erro");
				alert.setHeaderText(titulo);
				alert.setContentText(msg);
				alert.showAndWait();		
				
				cfg.showAndWait();
				
				alert = new Alert(AlertType.INFORMATION);
				alert.setTitle("Aviso");
				alert.setHeaderText("O terminal será fechado. Execute novamente para carregar as novas configurações.");
				alert.showAndWait();		
				
				close();
				
			}
		});
	}
	
	public void desconecta() {
		Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
				close();
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

	public void setCom(Comunicacao com) {
		this.com = com;
		teclado.setCom(com);
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

	public FlowPane getPnlNavegacao() {
		return pnlNavegacao;
	}

	public void setPnlNavegacao(FlowPane pnlNavegacao) {
		this.pnlNavegacao = pnlNavegacao;
	}

	public static char getMARCADOR() {
		return MARCADOR;
	}

}
