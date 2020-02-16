package br.com.avancoinfo.terminal;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXPasswordField;
import com.jfoenix.controls.JFXTextField;

import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Cursor;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public class Configuracao extends Stage {

	public Configuracao() {
		
		String servidor = "";
		String porta = "";
		String usuario = "";
		String senha = "";
		String comando = "";
		
        Image image = new Image(getClass().getResourceAsStream("logo2.png"));
        getIcons().add(image);
        setTitle("Configuração");

        GridPane pnlConfiguracao = new GridPane();
        pnlConfiguracao.setPrefSize(700, 200);

        // servidor
        VBox pnlServidor = new VBox(2);

        Label lblServidor = new Label("Servidor");
        lblServidor.getStyleClass().add("lblconfig");

        JFXTextField edtServidor = new JFXTextField(servidor);
        edtServidor.setPrefColumnCount(50);

        VBox.setMargin(lblServidor, new Insets(10, 10, 1, 10));
        VBox.setMargin(edtServidor, new Insets(1, 10, 10, 10));

        pnlServidor.getChildren().addAll(lblServidor, edtServidor);

        //  porta
        VBox pnlPorta = new VBox(2);

        Label lblPorta = new Label("Porta");
        lblPorta.getStyleClass().add("lblconfig");

        JFXTextField edtPorta = new JFXTextField(porta);
        edtPorta.setPrefColumnCount(6);
        pnlPorta.getChildren().addAll(lblPorta, edtPorta);

        VBox.setMargin(lblPorta, new Insets(10, 10, 1, 10));
        VBox.setMargin(edtPorta, new Insets(1, 10, 10, 10));

        // usuario
        VBox pnlUsuario = new VBox(2);

        Label lblUsuario = new Label("Usuário");
        lblUsuario.getStyleClass().add("lblconfig");

        JFXTextField edtUsuario = new JFXTextField(usuario);
        edtUsuario.setPrefColumnCount(20);

        VBox.setMargin(lblUsuario, new Insets(10, 10, 1, 10));
        VBox.setMargin(edtUsuario, new Insets(1, 10, 10, 10));

        pnlUsuario.getChildren().addAll(lblUsuario, edtUsuario);

        // senha
        VBox pnlSenha = new VBox(2);

        Label lblSenha = new Label("Senha");
        lblSenha.getStyleClass().add("lblconfig");

        JFXPasswordField edtSenha = new JFXPasswordField();
        edtSenha.setText(senha);
        edtSenha.setPrefColumnCount(20);

        VBox.setMargin(lblSenha, new Insets(10, 10, 1, 10));
        VBox.setMargin(edtSenha, new Insets(1, 10, 10, 10));

        pnlSenha.getChildren().addAll(lblSenha, edtSenha);

        // comando
        VBox pnlComando = new VBox(2);

        Label lblComando = new Label("Comando");
        lblComando.getStyleClass().add("lblconfig");

        JFXTextField edtComando = new JFXTextField(comando);
        edtComando.setPrefColumnCount(30);

        VBox.setMargin(lblComando, new Insets(10, 10, 1, 10));
        VBox.setMargin(edtComando, new Insets(1, 10, 10, 10));
        pnlComando.getChildren().addAll(lblComando, edtComando);

        // botoes
        HBox pnlBotoes = new HBox();
        pnlBotoes.setAlignment(Pos.BASELINE_RIGHT);

        JFXButton btnConfirma = new JFXButton("Confirma");
        btnConfirma.setCursor(Cursor.HAND);
        btnConfirma.getStyleClass().add("botaoInferior");
        
        JFXButton btnCancela = new JFXButton("Cancela");
        btnCancela.setCursor(Cursor.HAND);
        btnCancela.getStyleClass().add("botaoInferior");
        btnCancela.setOnAction(event -> close());

        HBox.setMargin(btnConfirma, new Insets(10, 0, 10, 10));
        HBox.setMargin(btnCancela, new Insets(10, 0, 0, 10));

        pnlBotoes.getChildren().addAll(btnConfirma, btnCancela);

        pnlConfiguracao.add(pnlServidor, 0, 0, 4, 1);
        pnlConfiguracao.add(pnlPorta, 4, 0, 1, 1);
        pnlConfiguracao.add(pnlUsuario, 0, 1, 2, 1);
        pnlConfiguracao.add(pnlSenha, 2, 1, 2, 1);
        pnlConfiguracao.add(pnlComando, 0, 2, 2, 1);
        pnlConfiguracao.add(pnlBotoes, 0, 3, 5, 1);

        Scene scene = new Scene(pnlConfiguracao);
        setScene(scene);
        

	}
}
