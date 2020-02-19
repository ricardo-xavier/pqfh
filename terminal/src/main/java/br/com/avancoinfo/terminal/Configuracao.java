package br.com.avancoinfo.terminal;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXPasswordField;
import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXToggleButton;

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
	
	public Configuracao() {
		
        setTitle("Configuração");

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

        pnlConfiguracao.setPrefSize(500, 500);

        // conexão
		
		String servidor = "";
		String porta = "";
		String usuario = "";
		String senha = "";
		String comando = "";

        Label lblServidor = new Label("Servidor");
        VBox.setMargin(lblServidor, new Insets(10, 10, 0, 10));
        
        JFXTextField edtServidor = new JFXTextField(servidor);
        VBox.setMargin(edtServidor, new Insets(0, 10, 4, 10));

        Label lblPorta = new Label("Porta");
        VBox.setMargin(lblPorta, new Insets(10, 10, 0, 10));
        
        JFXTextField edtPorta = new JFXTextField(porta);
        edtPorta.setMaxWidth(50);
        VBox.setMargin(edtPorta, new Insets(0, 10, 4, 10));

        Label lblUsuario = new Label("Usuário");
        VBox.setMargin(lblUsuario, new Insets(10, 10, 0, 10));
        
        JFXTextField edtUsuario = new JFXTextField(usuario);
        edtUsuario.setMaxWidth(300);
        VBox.setMargin(edtUsuario, new Insets(0, 10, 4, 10));

        Label lblSenha = new Label("Senha");
        VBox.setMargin(lblSenha, new Insets(10, 10, 0, 10));
        
        JFXPasswordField edtSenha = new JFXPasswordField();
        edtSenha.setText(senha);
        edtSenha.setMaxWidth(300);
        VBox.setMargin(edtSenha, new Insets(0, 10, 4, 10));

        Label lblComando = new Label("Comando");
        VBox.setMargin(lblComando, new Insets(10, 10, 0, 10));
        
        JFXTextField edtComando = new JFXTextField(comando);
        VBox.setMargin(edtComando, new Insets(0, 10, 4, 10));
        
        pnlConexao.getChildren().addAll(lblServidor, edtServidor, lblPorta, edtPorta, lblUsuario, edtUsuario,
        		lblSenha, edtSenha, lblComando, edtComando);

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
        edtVerde.setStyle("-fx-control-inner-background: red");
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
        
        Spinner<Integer> spFonte = new Spinner<>(16, 24, 20, 1);
        GridPane.setMargin(spFonte, new Insets(10, 30, 0, 10));

        pnlAparencia.add(lblFonte, 0, 9);
        pnlAparencia.add(spFonte, 1, 9);

        // avançado
        
		JFXToggleButton chkDecimal = new JFXToggleButton();
		chkDecimal.setText("Enviar ponto e vírgula como decimal");
		GridPane.setMargin(chkDecimal, new Insets(10, 30, 0, 10));
        
        pnlAvancado.getChildren().add(chkDecimal);

        // botões
        
        HBox pnlBotoes = new HBox();
        pnlBotoes.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma = new JFXButton("Confirma");
        btnConfirma.setCursor(javafx.scene.Cursor.HAND);
        btnConfirma.setOnAction((event) -> confirma());
        
        JFXButton btnCancela = new JFXButton("Cancela");
        btnCancela.setCursor(javafx.scene.Cursor.HAND);
        btnCancela.setOnAction(event -> close());

        HBox.setMargin(btnConfirma, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela, new Insets(10, 0, 0, 10));

        pnlBotoes.getChildren().addAll(btnConfirma, btnCancela);
        pnlDireita.setBottom(pnlBotoes);
        
        Scene scene = new Scene(pnlTela);
        setScene(scene);
        

	}

	public void carrega() {
		
		File cfg = new File("terminal.cfg");
		if (cfg.exists()) {
			try {
				BufferedReader reader = new BufferedReader(new FileReader(cfg));
				
				String linha;
				int sessao = 0;
				while ((linha = reader.readLine()) != null) {
					
					if (linha.startsWith("[CORES]")) {
						sessao = 1;
						continue;
					}
					
					switch(sessao) {
					
					case 1:
						String rgb = linha.split("=")[1];
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
					
					}
					
				}
				
				reader.close();
				
				cpBranco.setValue(getWhite());
				cpPreto.setValue(getBlack());
				cpVermelho.setValue(getRed());
				cpVerde.setValue(getGreen());
				cpAzul.setValue(getBlue());
				cpAmarelo.setValue(getYellow());
				cpMagenta.setValue(getMagenta());
				cpCyan.setValue(getCyan());
				
			} catch (IOException e) {
				e.printStackTrace();
			}
			
		}
		
	}
	
	private void confirma() {
		
		try {
			
			Configuracao.setBlack(cpPreto.getValue());
			Configuracao.setWhite(cpBranco.getValue());
			Configuracao.setRed(cpVermelho.getValue());
			Configuracao.setGreen(cpVerde.getValue());
			Configuracao.setBlue(cpAzul.getValue());
			Configuracao.setYellow(cpAmarelo.getValue());
			Configuracao.setMagenta(cpMagenta.getValue());
			Configuracao.setCyan(cpCyan.getValue());
			
			PrintStream cfg = new PrintStream("terminal.cfg");

			cfg.println("[CORES]");
			cfg.printf("PRETO=#%02x%02x%02x%n", (int) (getBlack().getRed() * 255), (int) (getBlack().getGreen() * 255), (int) (getBlack().getBlue() * 255));
			cfg.printf("BRANCO=#%02x%02x%02x%n", (int) (getWhite().getRed()* 255), (int) (getWhite().getGreen() * 255), (int) (getWhite().getBlue() * 255));
			cfg.printf("VERMELHO=#%02x%02x%02x%n", (int) (getRed().getRed() * 255), (int) (getRed().getGreen() * 255), (int) (getRed().getBlue() * 255));
			cfg.printf("VERDE=#%02x%02x%02x%n", (int) (getGreen().getRed() * 255), (int) (getGreen().getGreen() * 255), (int) (getGreen().getBlue() * 255));
			cfg.printf("AZUL=#%02x%02x%02x%n", (int) (getBlue().getRed() * 255), (int) (getBlue().getGreen() * 255), (int) (getBlue().getBlue() * 255));
			cfg.printf("AMARELO=#%02x%02x%02x%n", (int) (getYellow().getRed() * 255), (int) (getYellow().getGreen() * 255), (int) (getYellow().getBlue() * 255));
			cfg.printf("MAGENTA=#%02x%02x%02x%n", (int) (getMagenta().getRed() * 255), (int) (getMagenta().getGreen() * 255), (int) (getMagenta().getBlue() * 255));
			cfg.printf("CYAN=#%02x%02x%02x%n", (int) (getCyan().getRed() * 255), (int) (getCyan().getGreen() * 255), (int) (getCyan().getBlue() * 255));
			
			cfg.close();
			
		} catch (FileNotFoundException e) {
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
}
