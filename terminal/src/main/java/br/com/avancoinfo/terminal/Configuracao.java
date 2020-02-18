package br.com.avancoinfo.terminal;

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

	public Configuracao() {
		
        setTitle("Configuração");

        BorderPane pnlTela = new BorderPane();
        
        FlowPane pnlImagem = new FlowPane();
        pnlTela.setLeft(pnlImagem);
        
		Image imagem = new Image(getClass().getResourceAsStream("avanco.jpg"));
		ImageView ivAvanco = new ImageView(imagem);
		pnlImagem.getChildren().add(ivAvanco);
        
        TabPane pnlConfiguracao = new TabPane();
        pnlTela.setCenter(pnlConfiguracao);
        
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
        
        HBox pnlBotoes = new HBox();
        pnlBotoes.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma = new JFXButton("Confirma");
        btnConfirma.setCursor(javafx.scene.Cursor.HAND);
        
        JFXButton btnCancela = new JFXButton("Cancela");
        btnCancela.setCursor(javafx.scene.Cursor.HAND);
        btnCancela.setOnAction(event -> close());

        HBox.setMargin(btnConfirma, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela, new Insets(10, 0, 0, 10));

        pnlBotoes.getChildren().addAll(btnConfirma, btnCancela);
        
        pnlConexao.getChildren().addAll(lblServidor, edtServidor, lblPorta, edtPorta, lblUsuario, edtUsuario,
        		lblSenha, edtSenha, lblComando, edtComando, pnlBotoes);

        // aparência
		
        Label lblCor = new Label("Cor");
        GridPane.setMargin(lblCor, new Insets(10, 50, 10, 10));
        
        Label lblPadrao = new Label("Padrão");
        GridPane.setMargin(lblPadrao, new Insets(10, 50, 10, 10));
        
        Label lblAtual = new Label("Atual");
        GridPane.setMargin(lblAtual, new Insets(10, 50, 10, 10));
        
        pnlAparencia.add(lblCor, 0, 0);
        pnlAparencia.add(lblPadrao, 1, 0);
        pnlAparencia.add(lblAtual, 2, 0);

        Label lblBranco = new Label("Branco");
        GridPane.setMargin(lblBranco, new Insets(10, 50, 0, 10));
        
        TextField edtBranco = new TextField();
        edtBranco.setMaxWidth(50);
        edtBranco.setStyle("-fx-control-inner-background: white");
        GridPane.setMargin(edtBranco, new Insets(10, 50, 0, 10));
        
        ColorPicker cpBranco = new ColorPicker();
        cpBranco.setValue(Color.WHITE);
        GridPane.setMargin(cpBranco, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblBranco, 0, 1);
        pnlAparencia.add(edtBranco, 1, 1);
        pnlAparencia.add(cpBranco, 2, 1);

        Label lblPreto = new Label("Preto");
        GridPane.setMargin(lblPreto, new Insets(10, 50, 0, 10));
        
        TextField edtPreto = new TextField();
        edtPreto.setMaxWidth(50);
        GridPane.setMargin(edtPreto, new Insets(10, 50, 0, 10));
        edtPreto.setStyle("-fx-control-inner-background: black");
        
        ColorPicker cpPreto = new ColorPicker();
        cpPreto.setValue(Color.BLACK);
        GridPane.setMargin(cpPreto, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblPreto, 0, 2);
        pnlAparencia.add(edtPreto, 1, 2);
        pnlAparencia.add(cpPreto, 2, 2);

        Label lblVermelho = new Label("Vermelho");
        GridPane.setMargin(lblVermelho, new Insets(10, 50, 0, 10));
        
        TextField edtVermelho = new TextField();
        edtVermelho.setMaxWidth(50);
        GridPane.setMargin(edtVermelho, new Insets(10, 50, 0, 10));
        edtVermelho.setStyle("-fx-control-inner-background: red");
        
        ColorPicker cpVermelho = new ColorPicker();
        cpVermelho.setValue(Color.RED);
        GridPane.setMargin(cpVermelho, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblVermelho, 0, 3);
        pnlAparencia.add(edtVermelho, 1, 3);
        pnlAparencia.add(cpVermelho, 2, 3);

        Label lblAmarelo = new Label("Amarelo");
        GridPane.setMargin(lblAmarelo, new Insets(10, 50, 0, 10));
        
        TextField edtAmarelo = new TextField();
        edtAmarelo.setMaxWidth(50);
        GridPane.setMargin(edtAmarelo, new Insets(10, 50, 0, 10));
        edtAmarelo.setStyle("-fx-control-inner-background: yellow");
        
        ColorPicker cpAmarelo = new ColorPicker();
        cpAmarelo.setValue(Color.YELLOW);
        GridPane.setMargin(cpAmarelo, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblAmarelo, 0, 4);
        pnlAparencia.add(edtAmarelo, 1, 4);
        pnlAparencia.add(cpAmarelo, 2, 4);

        Label lblAzul = new Label("Azul");
        GridPane.setMargin(lblAzul, new Insets(10, 50, 0, 10));
        
        TextField edtAzul = new TextField();
        edtAzul.setMaxWidth(50);
        GridPane.setMargin(edtAzul, new Insets(10, 50, 0, 10));
        edtAzul.setStyle("-fx-control-inner-background: blue");
        
        ColorPicker cpAzul = new ColorPicker();
        cpAzul.setValue(Color.BLUE);
        GridPane.setMargin(cpAzul, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblAzul, 0, 5);
        pnlAparencia.add(edtAzul, 1, 5);
        pnlAparencia.add(cpAzul, 2, 5);

        Label lblMagenta = new Label("Magenta");
        GridPane.setMargin(lblMagenta, new Insets(10, 50, 0, 10));
        
        TextField edtMagenta = new TextField();
        edtMagenta.setMaxWidth(50);
        GridPane.setMargin(edtMagenta, new Insets(10, 50, 0, 10));
        edtMagenta.setStyle("-fx-control-inner-background: magenta");
        
        ColorPicker cpMagenta = new ColorPicker();
        cpMagenta.setValue(Color.MAGENTA);
        GridPane.setMargin(cpMagenta, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblMagenta, 0, 6);
        pnlAparencia.add(edtMagenta, 1, 6);
        pnlAparencia.add(cpMagenta, 2, 6);

        Label lblCyan = new Label("Cyan");
        GridPane.setMargin(lblCyan, new Insets(10, 50, 0, 10));
        
        TextField edtCyan = new TextField();
        edtCyan.setMaxWidth(50);
        GridPane.setMargin(edtCyan, new Insets(10, 50, 0, 10));
        edtCyan.setStyle("-fx-control-inner-background: cyan");
        
        ColorPicker cpCyan = new ColorPicker();
        cpCyan.setValue(Color.CYAN);
        GridPane.setMargin(cpCyan, new Insets(10, 50, 0, 10));
        
        pnlAparencia.add(lblCyan, 0, 7);
        pnlAparencia.add(edtCyan, 1, 7);
        pnlAparencia.add(cpCyan, 2, 7);

        Label lblFonte = new Label("Fonte");
        GridPane.setMargin(lblFonte, new Insets(10, 50, 0, 10));
        
        Spinner<Integer> spFonte = new Spinner<>(16, 24, 20, 1);
        GridPane.setMargin(spFonte, new Insets(10, 50, 0, 10));

        pnlAparencia.add(lblFonte, 0, 8);
        pnlAparencia.add(spFonte, 1, 8);

        HBox pnlBotoes2 = new HBox();
        pnlBotoes2.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma2 = new JFXButton("Confirma");
        btnConfirma2.setCursor(javafx.scene.Cursor.HAND);
        
        JFXButton btnCancela2 = new JFXButton("Cancela");
        btnCancela2.setCursor(javafx.scene.Cursor.HAND);
        btnCancela2.setOnAction(event -> close());

        HBox.setMargin(btnConfirma2, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela2, new Insets(10, 0, 0, 10));
        
        pnlBotoes2.getChildren().addAll(btnConfirma2, btnCancela2);

        pnlAparencia.add(pnlBotoes2, 2, 9);

        // avançado
        
		JFXToggleButton chkDecimal = new JFXToggleButton();
		chkDecimal.setText("Enviar ponto e vírgula como decimal");
		GridPane.setMargin(chkDecimal, new Insets(10, 50, 0, 10));
        
        pnlAvancado.getChildren().add(chkDecimal);

        HBox pnlBotoes3 = new HBox();
        pnlBotoes3.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma3 = new JFXButton("Confirma");
        btnConfirma3.setCursor(javafx.scene.Cursor.HAND);
        
        JFXButton btnCancela3 = new JFXButton("Cancela");
        btnCancela3.setCursor(javafx.scene.Cursor.HAND);
        btnCancela3.setOnAction(event -> close());

        HBox.setMargin(btnConfirma3, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela3, new Insets(10, 0, 0, 10));

        pnlBotoes3.getChildren().addAll(btnConfirma3, btnCancela3);
        pnlAvancado.getChildren().addAll(pnlBotoes3);
        
        Scene scene = new Scene(pnlTela);
        setScene(scene);
        

	}
}
