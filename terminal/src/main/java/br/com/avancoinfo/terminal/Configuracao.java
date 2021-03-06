package br.com.avancoinfo.terminal;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintStream;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXPasswordField;
import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXToggleButton;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.Label;
import javafx.scene.control.Spinner;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TextField;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.stage.Stage;

public class Configuracao extends Stage {
	
	private JFXTextField edtServidor;
	private JFXTextField edtPorta;
	private JFXTextField edtUsuario;
	private JFXPasswordField edtSenha;
	private JFXTextField edtComando;
	private JFXTextField edtUsuarioIntegral;
	private JFXPasswordField edtSenhaIntegral;
	private JFXTextField edtServidorCompartilhamento;
	private JFXTextField edtPortaCompartilhamento;
	
	private Spinner<Integer> spFonte;

	private static Color black = Color.BLACK;;
	private static Color white = Color.WHITE;
	private static Color red = Color.RED;
	private static Color green = Color.GREEN;
	private static Color blue = Color.BLUE;
	private static Color magenta = Color.MAGENTA;
	private static Color cyan = Color.CYAN;
	private static Color yellow = Color.YELLOW;
	
	private ColorPicker cpBranco;
	private ColorPicker cpPreto;
	private ColorPicker cpVermelho;
	private ColorPicker cpVerde;
	private ColorPicker cpAzul;
	private ColorPicker cpAmarelo;
	private ColorPicker cpMagenta;
	private ColorPicker cpCyan;
	
	private String servidor;
	private int porta = 22;
	private String usuario;
	private String senha;
	private String comando = "TERM=ansi cobrun integral";
	private String usuarioIntegral;
	private String senhaIntegral;
	private String servidorCompartilhamento;
	private int portaCompartilhamento;	
	
	private int tamFonte = 20;

	private boolean pontoVirgula;
	private JFXToggleButton chkPontoVirgula;
	private boolean botoesFuncao;
	private JFXToggleButton chkBotoesFuncao;
	private boolean mouseMenus;
	private JFXToggleButton chkMouseMenus;
	private boolean barraNavegacao;
	private JFXToggleButton chkBarraNavegacao;
	
	private boolean cancelado;
	
	public Configuracao() {
		
        setTitle("Configuração");

        setCancelado(false);
        BorderPane pnlTela = new BorderPane();
        
        FlowPane pnlImagem = new FlowPane();
        pnlTela.setLeft(pnlImagem);
        
		Image imagem = new Image(getClass().getResourceAsStream("avanco.jpg"));
		ImageView ivAvanco = new ImageView(imagem);
		pnlImagem.getChildren().add(ivAvanco);
		
		BorderPane pnlDireita = new BorderPane();
        
        TabPane pnlConfiguracao = new TabPane();
        pnlDireita.setCenter(pnlConfiguracao);
        pnlTela.setCenter(pnlDireita);
        
        VBox pnlConexao = new VBox(1);
        GridPane pnlAparencia = new GridPane();
        VBox pnlAvancado = new VBox(1);
        
        Tab tab1 = new Tab("Conexão", pnlConexao);
        Tab tab2 = new Tab("Aparência"  , pnlAparencia);
        Tab tab3 = new Tab("Avançado" , pnlAvancado);

        pnlConfiguracao.getTabs().add(tab1);
        pnlConfiguracao.getTabs().add(tab2);
        pnlConfiguracao.getTabs().add(tab3);

        pnlConfiguracao.setPrefSize(500, 600);

        // conexão
		
        Label lblServidor = new Label("Servidor");
        VBox.setMargin(lblServidor, new Insets(10, 10, 0, 10));
        
        edtServidor = new JFXTextField(servidor);
        VBox.setMargin(edtServidor, new Insets(0, 10, 4, 10));

        Label lblPorta = new Label("Porta");
        VBox.setMargin(lblPorta, new Insets(10, 10, 0, 10));
        
        edtPorta = new JFXTextField(String.valueOf(porta));
        edtPorta.setMaxWidth(50);
        VBox.setMargin(edtPorta, new Insets(0, 10, 4, 10));

        Label lblUsuario = new Label("Usuário do Linux");
        VBox.setMargin(lblUsuario, new Insets(10, 10, 0, 10));
        
        edtUsuario = new JFXTextField(usuario);
        edtUsuario.setMaxWidth(300);
        VBox.setMargin(edtUsuario, new Insets(0, 10, 4, 10));

        Label lblSenha = new Label("Senha");
        VBox.setMargin(lblSenha, new Insets(10, 10, 0, 10));
        
        edtSenha = new JFXPasswordField();
        edtSenha.setText(senha);
        edtSenha.setMaxWidth(300);
        VBox.setMargin(edtSenha, new Insets(0, 10, 4, 10));

        Label lblComando = new Label("Comando");
        VBox.setMargin(lblComando, new Insets(10, 10, 0, 10));
        
        edtComando = new JFXTextField(comando);
        VBox.setMargin(edtComando, new Insets(0, 10, 4, 10));

        Label lblUsuarioIntegral = new Label("Usuário do Integral");
        VBox.setMargin(lblUsuarioIntegral, new Insets(10, 10, 0, 10));
        
        edtUsuarioIntegral = new JFXTextField(usuarioIntegral);
        edtUsuarioIntegral.setMaxWidth(60);
        VBox.setMargin(edtUsuarioIntegral, new Insets(0, 10, 4, 10));

        Label lblSenhaIntegral = new Label("Senha");
        VBox.setMargin(lblSenhaIntegral, new Insets(10, 10, 0, 10));
        
        edtSenhaIntegral = new JFXPasswordField();
        edtSenhaIntegral.setText(senhaIntegral);
        edtSenhaIntegral.setMaxWidth(60);
        VBox.setMargin(edtSenhaIntegral, new Insets(0, 10, 4, 10));
		
        Label lblServidorCompartilhamento = new Label("Servidor Compartilhamento");
        VBox.setMargin(lblServidorCompartilhamento, new Insets(10, 10, 0, 10));
        
        edtServidorCompartilhamento = new JFXTextField(servidorCompartilhamento);
        VBox.setMargin(edtServidorCompartilhamento, new Insets(0, 10, 4, 10));

        Label lblPortaCompartilhamento = new Label("Porta Compartilhamento");
        VBox.setMargin(lblPortaCompartilhamento, new Insets(10, 10, 0, 10));
        
        edtPortaCompartilhamento = new JFXTextField(String.valueOf(portaCompartilhamento));
        edtPortaCompartilhamento.setMaxWidth(50);
        VBox.setMargin(edtPortaCompartilhamento, new Insets(0, 10, 4, 10));
        
        pnlConexao.getChildren().addAll(lblServidor, edtServidor, lblPorta, edtPorta, lblUsuario, edtUsuario,
        		lblSenha, edtSenha, lblComando, edtComando, lblUsuarioIntegral, edtUsuarioIntegral,
        		lblSenhaIntegral, edtSenhaIntegral,
        		lblServidorCompartilhamento, edtServidorCompartilhamento,
        		lblPortaCompartilhamento, edtPortaCompartilhamento);

        // aparência
		
        Label lblCor = new Label("Cor");
        GridPane.setMargin(lblCor, new Insets(10, 30, 10, 10));
        
        Label lblPadrao = new Label("Padrão");
        GridPane.setMargin(lblPadrao, new Insets(10, 30, 10, 10));
        
        Label lblAtual = new Label("Atual");
        GridPane.setMargin(lblAtual, new Insets(10, 30, 10, 10));
        
        pnlAparencia.add(lblCor, 0, 0);
        pnlAparencia.add(lblPadrao, 1, 0);
        pnlAparencia.add(lblAtual, 2, 0);

        Label lblBranco = new Label("Branco");
        GridPane.setMargin(lblBranco, new Insets(10, 30, 0, 10));        
        TextField edtBranco = new TextField();
        edtBranco.setMaxWidth(50);
        edtBranco.setStyle("-fx-control-inner-background: white");
        GridPane.setMargin(edtBranco, new Insets(10, 30, 0, 10));
        cpBranco = new ColorPicker();
        cpBranco.setValue(Color.WHITE);
        GridPane.setMargin(cpBranco, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblBranco, 0, 1);
        pnlAparencia.add(edtBranco, 1, 1);
        pnlAparencia.add(cpBranco, 2, 1);

        Label lblPreto = new Label("Preto");
        GridPane.setMargin(lblPreto, new Insets(10, 30, 0, 10));
        TextField edtPreto = new TextField();
        edtPreto.setMaxWidth(50);
        GridPane.setMargin(edtPreto, new Insets(10, 30, 0, 10));
        edtPreto.setStyle("-fx-control-inner-background: black");
        cpPreto = new ColorPicker();
        cpPreto.setValue(Color.BLACK);
        GridPane.setMargin(cpPreto, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblPreto, 0, 2);
        pnlAparencia.add(edtPreto, 1, 2);
        pnlAparencia.add(cpPreto, 2, 2);

        Label lblVermelho = new Label("Vermelho");
        GridPane.setMargin(lblVermelho, new Insets(10, 30, 0, 10));       
        TextField edtVermelho = new TextField();
        edtVermelho.setMaxWidth(50);
        GridPane.setMargin(edtVermelho, new Insets(10, 30, 0, 10));
        edtVermelho.setStyle("-fx-control-inner-background: red");
        cpVermelho = new ColorPicker();
        cpVermelho.setValue(Color.RED);
        GridPane.setMargin(cpVermelho, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblVermelho, 0, 3);
        pnlAparencia.add(edtVermelho, 1, 3);
        pnlAparencia.add(cpVermelho, 2, 3);

        Label lblVerde = new Label("Verde");
        GridPane.setMargin(lblVerde, new Insets(10, 30, 0, 10));        
        TextField edtVerde = new TextField();
        edtVerde.setMaxWidth(50);
        GridPane.setMargin(edtVerde, new Insets(10, 30, 0, 10));
        edtVerde.setStyle("-fx-control-inner-background: green");
        cpVerde = new ColorPicker();
        cpVerde.setValue(Color.GREEN);
        GridPane.setMargin(cpVerde, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblVerde, 0, 4);
        pnlAparencia.add(edtVerde, 1, 4);
        pnlAparencia.add(cpVerde, 2, 4);

        Label lblAzul = new Label("Azul");
        GridPane.setMargin(lblAzul, new Insets(10, 30, 0, 10));
        TextField edtAzul = new TextField();
        edtAzul.setMaxWidth(50);
        GridPane.setMargin(edtAzul, new Insets(10, 30, 0, 10));
        edtAzul.setStyle("-fx-control-inner-background: blue");
        cpAzul = new ColorPicker();
        cpAzul.setValue(Color.BLUE);
        GridPane.setMargin(cpAzul, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblAzul, 0, 5);
        pnlAparencia.add(edtAzul, 1, 5);
        pnlAparencia.add(cpAzul, 2, 5);
        
        Label lblAmarelo = new Label("Amarelo");
        GridPane.setMargin(lblAmarelo, new Insets(10, 30, 0, 10));
        TextField edtAmarelo = new TextField();
        edtAmarelo.setMaxWidth(50);
        GridPane.setMargin(edtAmarelo, new Insets(10, 30, 0, 10));
        edtAmarelo.setStyle("-fx-control-inner-background: yellow");
        cpAmarelo = new ColorPicker();
        cpAmarelo.setValue(Color.YELLOW);
        GridPane.setMargin(cpAmarelo, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblAmarelo, 0, 6);
        pnlAparencia.add(edtAmarelo, 1, 6);
        pnlAparencia.add(cpAmarelo, 2, 6);

        Label lblMagenta = new Label("Magenta");
        GridPane.setMargin(lblMagenta, new Insets(10, 30, 0, 10));
        TextField edtMagenta = new TextField();
        edtMagenta.setMaxWidth(50);
        GridPane.setMargin(edtMagenta, new Insets(10, 30, 0, 10));
        edtMagenta.setStyle("-fx-control-inner-background: magenta");
        cpMagenta = new ColorPicker();
        cpMagenta.setValue(Color.MAGENTA);
        GridPane.setMargin(cpMagenta, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblMagenta, 0, 7);
        pnlAparencia.add(edtMagenta, 1, 7);
        pnlAparencia.add(cpMagenta, 2, 7);

        Label lblCyan = new Label("Cyan");
        GridPane.setMargin(lblCyan, new Insets(10, 30, 0, 10));
        TextField edtCyan = new TextField();
        edtCyan.setMaxWidth(50);
        GridPane.setMargin(edtCyan, new Insets(10, 30, 0, 10));
        edtCyan.setStyle("-fx-control-inner-background: cyan");
        cpCyan = new ColorPicker();
        cpCyan.setValue(Color.CYAN);
        GridPane.setMargin(cpCyan, new Insets(10, 30, 0, 10));
        pnlAparencia.add(lblCyan, 0, 8);
        pnlAparencia.add(edtCyan, 1, 8);
        pnlAparencia.add(cpCyan, 2, 8);

        Label lblFonte = new Label("Fonte");
        GridPane.setMargin(lblFonte, new Insets(10, 30, 0, 10));
        
        spFonte = new Spinner<>(16, 22, tamFonte, 1);
        GridPane.setMargin(spFonte, new Insets(10, 30, 0, 10));

        pnlAparencia.add(lblFonte, 0, 9);
        pnlAparencia.add(spFonte, 1, 9);

        // avançado
        
		chkPontoVirgula = new JFXToggleButton();
		chkPontoVirgula.setText("Detectar ponto decimal");
		GridPane.setMargin(chkPontoVirgula, new Insets(10, 30, 0, 10));
        
        pnlAvancado.getChildren().add(chkPontoVirgula);
        
		chkBotoesFuncao = new JFXToggleButton();
		chkBotoesFuncao.setText("Criar botões para as teclas de função");
		GridPane.setMargin(chkBotoesFuncao, new Insets(10, 30, 0, 10));
        
        pnlAvancado.getChildren().add(chkBotoesFuncao);
        
		chkMouseMenus = new JFXToggleButton();
		chkMouseMenus.setText("Usar mouse nos menus");
		GridPane.setMargin(chkMouseMenus, new Insets(10, 30, 0, 10));
        
        pnlAvancado.getChildren().add(chkMouseMenus);
        
		chkBarraNavegacao = new JFXToggleButton();
		chkBarraNavegacao.setText("Barra de Navegação");
		GridPane.setMargin(chkBarraNavegacao, new Insets(10, 30, 0, 10));
        
        pnlAvancado.getChildren().add(chkBarraNavegacao);

        // botões
        
        HBox pnlBotoes = new HBox();
        pnlBotoes.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma = new JFXButton("Confirma");
        btnConfirma.setCursor(javafx.scene.Cursor.HAND);
        btnConfirma.setOnAction((event) -> confirma());
        btnConfirma.getStyleClass().add("botao");
        
        JFXButton btnCancela = new JFXButton("Cancela");
        btnCancela.setCursor(javafx.scene.Cursor.HAND);
        btnCancela.setOnAction(event -> { 
        	cancelado = true; 
        	close(); 
        });
        btnCancela.getStyleClass().add("botao");

        HBox.setMargin(btnConfirma, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela, new Insets(10, 0, 0, 10));

        pnlBotoes.getChildren().addAll(btnConfirma, btnCancela);
        pnlDireita.setBottom(pnlBotoes);
        
        Scene scene = new Scene(pnlTela);
        setScene(scene);

        // seta o foco
        
        Platform.runLater(new Runnable() {
			
			@Override
			public void run() {
		        pnlConfiguracao.getSelectionModel().select(0);
		        if ((servidor == null) || servidor.trim().equals("")) {
		        	edtServidor.requestFocus();
		        } else if ((usuario == null) || usuario.trim().equals("")) {
		        	edtUsuario.requestFocus();
		        } else if ((senha == null) || senha.trim().equals("")) {
		        	edtSenha.requestFocus();
		        } else {
		        	edtServidor.requestFocus();
		        }
			}
		});

	}

	public void carrega() {
		
		File cfg = new File("terminal.cfg");
		if (cfg.exists()) {
			try {
				BufferedReader reader = new BufferedReader(new FileReader(cfg));
				
				String linha;
				int sessao = 0;
				while ((linha = reader.readLine()) != null) {

					if (linha.startsWith("[CONEXAO]")) {
						sessao = 1;
						continue;
					}
					
					if (linha.startsWith("[CORES]")) {
						sessao = 2;
						continue;
					}
					
					if (linha.startsWith("[FONTE]")) {
						sessao = 3;
						continue;
					}
					
					if (linha.startsWith("[AVANCADO]")) {
						sessao = 4;
						continue;
					}
					
					int p = linha.indexOf("=");
					if (p <= 0) {
						continue;
					}
					
					String chave = linha.substring(0, p);
					String valor = linha.substring(p+1);
					
					switch (sessao) {

					case 1:
						switch (chave) {
						case "SERVIDOR": 
							servidor = valor; 
							break;
						
						case "PORTA":
							try {
								porta = Integer.parseInt(valor);
							} catch (NumberFormatException e) {
								
							}
							break;
							
						case "USUARIO": 
							usuario = valor; 
							break;
						
						case "SENHA":
							Criptografia cript = new Criptografia();
							senha = cript.descriptografa(valor); 
							break;
							
						case "COMANDO": 
							comando = valor; 
							break;
							
						case "USUARIO_INTEGRAL": 
							usuarioIntegral = valor; 
							break;
						
						case "SENHA_INTEGRAL":
							cript = new Criptografia();
							senhaIntegral = cript.descriptografa(valor); 
							break;
							
						case "SERVIDOR_COMPARTILHAMENTO": 
							servidorCompartilhamento = valor; 
							break;
						
						case "PORTA_COMPARTILHAMENTO":
							try {
								portaCompartilhamento = Integer.parseInt(valor);
							} catch (NumberFormatException e) {
								
							}
							break;
							
						}						
						break;
					
					case 2:
						String rgb = valor;
						int r = Integer.parseInt(rgb.substring(1, 3), 16);
						int g = Integer.parseInt(rgb.substring(3, 5), 16);
						int b = Integer.parseInt(rgb.substring(5, 7), 16);
						
						switch (linha.split("=")[0]) {
						case "PRETO": setBlack(Color.rgb(r, g, b)); break;
						case "BRANCO": setWhite(Color.rgb(r, g, b)); break;
						case "VERMELHO": setRed(Color.rgb(r, g, b)); break;
						case "VERDE": setGreen(Color.rgb(r, g, b)); break;
						case "AZUL": setBlue(Color.rgb(r, g, b)); break;
						case "AMARELO": setYellow(Color.rgb(r, g, b)); break;
						case "MAGENTA": setMagenta(Color.rgb(r, g, b)); break;
						case "CYAN": setCyan(Color.rgb(r, g, b)); break;
						}
						break;

					case 3:
						switch (chave) {
						case "TAMANHO": 
							try {
								setTamFonte(Integer.parseInt(valor));
							} catch (NumberFormatException e) {
								
							}
							break;
						}

					case 4:
						switch (chave) {
						case "PONTO_VIRGULA": 
							setPontoVirgula(valor.equalsIgnoreCase("S"));
							break;
						case "BOTOES_TECLAS_FUNCAO": 
							setBotoesFuncao(valor.equalsIgnoreCase("S"));
							break;
						case "MOUSE_MENUS": 
							setMouseMenus(valor.equalsIgnoreCase("S"));
							break;
						case "BARRA_NAVEGACAO": 
							setBarraNavegacao(valor.equalsIgnoreCase("S"));
							break;
							
						}
					}
					
				}
				
				reader.close();
				
				edtServidor.setText(servidor);
				edtPorta.setText(String.valueOf(porta));
				edtUsuario.setText(usuario);
				edtSenha.setText(senha);
				edtComando.setText(comando);
				edtUsuarioIntegral.setText(usuarioIntegral);
				edtSenhaIntegral.setText(senhaIntegral);
				edtServidorCompartilhamento.setText(servidorCompartilhamento);
				edtPortaCompartilhamento.setText(String.valueOf(portaCompartilhamento));
				
				cpBranco.setValue(getWhite());
				cpPreto.setValue(getBlack());
				cpVermelho.setValue(getRed());
				cpVerde.setValue(getGreen());
				cpAzul.setValue(getBlue());
				cpAmarelo.setValue(getYellow());
				cpMagenta.setValue(getMagenta());
				cpCyan.setValue(getCyan());
				
				spFonte.getValueFactory().setValue(tamFonte);
				
				chkPontoVirgula.setSelected(pontoVirgula);
				chkBotoesFuncao.setSelected(botoesFuncao);
				chkMouseMenus.setSelected(mouseMenus);
				chkBarraNavegacao.setSelected(barraNavegacao);
				
			} catch (Exception  e) {
				e.printStackTrace();
			}
			
		}
		
	}
	
	private void confirma() {
		
		try {
			
			servidor = edtServidor.getText();
			try {
				porta = Integer.parseInt(edtPorta.getText());
			} catch (NumberFormatException e) {
				porta = 0;
			}
			usuario = edtUsuario.getText();
			senha = edtSenha.getText();
			comando = edtComando.getText();
			usuarioIntegral = edtUsuarioIntegral.getText();
			senhaIntegral = edtSenhaIntegral.getText();
			servidorCompartilhamento = edtServidorCompartilhamento.getText();
			try {
				portaCompartilhamento = Integer.parseInt(edtPortaCompartilhamento.getText());
			} catch (NumberFormatException e) {
				portaCompartilhamento = 0;
			}
			
			tamFonte = spFonte.getValue();
			
			Configuracao.setBlack(cpPreto.getValue());
			Configuracao.setWhite(cpBranco.getValue());
			Configuracao.setRed(cpVermelho.getValue());
			Configuracao.setGreen(cpVerde.getValue());
			Configuracao.setBlue(cpAzul.getValue());
			Configuracao.setYellow(cpAmarelo.getValue());
			Configuracao.setMagenta(cpMagenta.getValue());
			Configuracao.setCyan(cpCyan.getValue());
			
			pontoVirgula = chkPontoVirgula.isSelected();
			botoesFuncao = chkBotoesFuncao.isSelected();
			mouseMenus = chkMouseMenus.isSelected();
			barraNavegacao = chkBarraNavegacao.isSelected();
			
			PrintStream cfg = new PrintStream("terminal.cfg");

			cfg.println("[CONEXAO]");
			if (servidor != null) {
				cfg.printf("SERVIDOR=%s%n", servidor);
			}
			if (porta != 0) {
				cfg.printf("PORTA=%d%n", porta);
			}
			if (usuario != null) {
				cfg.printf("USUARIO=%s%n", usuario);
			}
			if (senha != null) {
				Criptografia crip = new Criptografia();
				cfg.printf("SENHA=%s%n", crip.criptografa(senha));
			}
			if (comando != null) {
				cfg.printf("COMANDO=%s%n", comando);
			}
			if (usuarioIntegral != null) {
				cfg.printf("USUARIO_INTEGRAL=%s%n", usuarioIntegral);
			}
			if (senhaIntegral != null) {
				Criptografia crip = new Criptografia();
				cfg.printf("SENHA_INTEGRAL=%s%n", crip.criptografa(senhaIntegral));
			}
			if (servidor != null) {
				cfg.printf("SERVIDOR_COMPARTILHAMENTO=%s%n", servidorCompartilhamento);
			}
			if (portaCompartilhamento != 0) {
				cfg.printf("PORTA_COMPARTILHAMENTO=%d%n", portaCompartilhamento);
			}			
			
			cfg.println("[CORES]");
			cfg.printf("PRETO=#%02x%02x%02x%n", (int) (getBlack().getRed() * 255), (int) (getBlack().getGreen() * 255), (int) (getBlack().getBlue() * 255));
			cfg.printf("BRANCO=#%02x%02x%02x%n", (int) (getWhite().getRed()* 255), (int) (getWhite().getGreen() * 255), (int) (getWhite().getBlue() * 255));
			cfg.printf("VERMELHO=#%02x%02x%02x%n", (int) (getRed().getRed() * 255), (int) (getRed().getGreen() * 255), (int) (getRed().getBlue() * 255));
			cfg.printf("VERDE=#%02x%02x%02x%n", (int) (getGreen().getRed() * 255), (int) (getGreen().getGreen() * 255), (int) (getGreen().getBlue() * 255));
			cfg.printf("AZUL=#%02x%02x%02x%n", (int) (getBlue().getRed() * 255), (int) (getBlue().getGreen() * 255), (int) (getBlue().getBlue() * 255));
			cfg.printf("AMARELO=#%02x%02x%02x%n", (int) (getYellow().getRed() * 255), (int) (getYellow().getGreen() * 255), (int) (getYellow().getBlue() * 255));
			cfg.printf("MAGENTA=#%02x%02x%02x%n", (int) (getMagenta().getRed() * 255), (int) (getMagenta().getGreen() * 255), (int) (getMagenta().getBlue() * 255));
			cfg.printf("CYAN=#%02x%02x%02x%n", (int) (getCyan().getRed() * 255), (int) (getCyan().getGreen() * 255), (int) (getCyan().getBlue() * 255));

			cfg.println("[FONTE]");
			if (tamFonte != 0) {
				cfg.printf("TAMANHO=%d%n", tamFonte);
			}

			cfg.println("[AVANCADO]");
			cfg.printf("PONTO_VIRGULA=%c%n", pontoVirgula ? 'S' : 'N');
			cfg.printf("BOTOES_TECLAS_FUNCAO=%c%n", botoesFuncao ? 'S' : 'N');
			cfg.printf("MOUSE_MENUS=%c%n", mouseMenus ? 'S' : 'N');
			cfg.printf("BARRA_NAVEGACAO=%c%n", barraNavegacao ? 'S' : 'N');
			
			cfg.close();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		close();
		
	}

	public static Color getBlack() {
		return black;
	}

	public static void setBlack(Color black) {
		Configuracao.black = black;
	}

	public static Color getWhite() {
		return white;
	}

	public static void setWhite(Color white) {
		Configuracao.white = white;
	}

	public static Color getRed() {
		return red;
	}

	public static void setRed(Color red) {
		Configuracao.red = red;
	}

	public static Color getBlue() {
		return blue;
	}

	public static void setBlue(Color blue) {
		Configuracao.blue = blue;
	}

	public static Color getGreen() {
		return green;
	}

	public static void setGreen(Color green) {
		Configuracao.green = green;
	}

	public static Color getMagenta() {
		return magenta;
	}

	public static void setMagenta(Color magenta) {
		Configuracao.magenta = magenta;
	}

	public static Color getCyan() {
		return cyan;
	}

	public static void setCyan(Color cyan) {
		Configuracao.cyan = cyan;
	}

	public static Color getYellow() {
		return yellow;
	}

	public static void setYellow(Color yellow) {
		Configuracao.yellow = yellow;
	}

	public String getServidor() {
		return servidor;
	}

	public void setServidor(String servidor) {
		this.servidor = servidor;
	}

	public int getPorta() {
		return porta;
	}

	public void setPorta(int porta) {
		this.porta = porta;
	}

	public String getUsuario() {
		return usuario;
	}

	public void setUsuario(String usuario) {
		this.usuario = usuario;
	}

	public String getSenha() {
		return senha;
	}

	public void setSenha(String senha) {
		this.senha = senha;
	}

	public String getUsuarioIntegral() {
		return usuarioIntegral;
	}

	public void setUsuarioIntegral(String usuarioIntegral) {
		this.usuarioIntegral = usuarioIntegral;
	}

	public String getSenhaIntegral() {
		return senhaIntegral;
	}

	public void setSenhaIntegral(String senhaIntegral) {
		this.senhaIntegral = senhaIntegral;
	}

	public String getComando() {
		return comando;
	}

	public void setComando(String comando) {
		this.comando = comando;
	}

	public boolean isCancelado() {
		return cancelado;
	}

	public void setCancelado(boolean cancelado) {
		this.cancelado = cancelado;
	}

	public int getTamFonte() {
		return tamFonte;
	}

	public void setTamFonte(int tamFonte) {
		this.tamFonte = tamFonte;
	}

	public boolean isPontoVirgula() {
		return pontoVirgula;
	}

	public void setPontoVirgula(boolean pontoVirgula) {
		this.pontoVirgula = pontoVirgula;
	}

	public boolean isBotoesFuncao() {
		return botoesFuncao;
	}

	public void setBotoesFuncao(boolean botoesFuncao) {
		this.botoesFuncao = botoesFuncao;
	}

	public boolean isMouseMenus() {
		return mouseMenus;
	}

	public void setMouseMenus(boolean mouseMenus) {
		this.mouseMenus = mouseMenus;
	}

	public JFXToggleButton getChkMouseMenus() {
		return chkMouseMenus;
	}

	public void setChkMouseMenus(JFXToggleButton chkMouseMenus) {
		this.chkMouseMenus = chkMouseMenus;
	}

	public boolean isBarraNavegacao() {
		return barraNavegacao;
	}

	public void setBarraNavegacao(boolean barraNavegacao) {
		this.barraNavegacao = barraNavegacao;
	}

	public JFXToggleButton getChkBarraNavegacao() {
		return chkBarraNavegacao;
	}

	public void setChkBarraNavegacao(JFXToggleButton chkBarraNavegacao) {
		this.chkBarraNavegacao = chkBarraNavegacao;
	}

	public String getServidorCompartilhamento() {
		return servidorCompartilhamento;
	}

	public void setServidorCompartilhamento(String servidorCompartilhamento) {
		this.servidorCompartilhamento = servidorCompartilhamento;
	}

	public int getPortaCompartilhamento() {
		return portaCompartilhamento;
	}

	public void setPortaCompartilhamento(int portaCompartilhamento) {
		this.portaCompartilhamento = portaCompartilhamento;
	}
}
