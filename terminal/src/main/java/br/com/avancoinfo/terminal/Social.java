package br.com.avancoinfo.terminal;

import java.io.IOException;

import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;

public class Social {
	
	public static void monta(VBox pnlSocial) {
		
		FlowPane pnlSocial1;
		FlowPane pnlSocial2;
		
		pnlSocial.setMaxWidth(100);
		
		pnlSocial1 = new FlowPane();
		pnlSocial2 = new FlowPane();
		
		pnlSocial.setVisible(false);
		pnlSocial.getChildren().add(pnlSocial1);
		pnlSocial.getChildren().add(pnlSocial2);
		
		pnlSocial1.setAlignment(Pos.CENTER);
		pnlSocial2.setAlignment(Pos.CENTER);

		Image imageSite = new Image(Social.class.getResourceAsStream("a.png"));
		ImageView ivSite = new ImageView(imageSite);
		ivSite.getStyleClass().add("botao");
		pnlSocial1.getChildren().add(ivSite);
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

		Image imageFace = new Image(Social.class.getResourceAsStream("face.png"));
		ImageView ivFace = new ImageView(imageFace);
		ivFace.getStyleClass().add("botao");
		pnlSocial1.getChildren().add(ivFace);
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

		Image imageLinkedin = new Image(Social.class.getResourceAsStream("in.png"));
		ImageView ivLinkedin = new ImageView(imageLinkedin);
		ivLinkedin.getStyleClass().add("botao");
		pnlSocial1.getChildren().add(ivLinkedin);
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
		
		Image imageConhecimento = new Image(Social.class.getResourceAsStream("conhecimento.png"), 20, 20, false, false);
		ImageView ivConhecimento = new ImageView(imageConhecimento);
		ivConhecimento.getStyleClass().add("botao");
		pnlSocial2.getChildren().add(ivConhecimento);
		ivConhecimento.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://bc.avancoinfo.com.br/";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		
		Image imageNovoAvanco = new Image(Social.class.getResourceAsStream("novoavanco.png"), 20, 20, false, false);
		ImageView ivNovoAvanco = new ImageView(imageNovoAvanco);
		ivNovoAvanco.getStyleClass().add("botao");
		pnlSocial2.getChildren().add(ivNovoAvanco);
		ivNovoAvanco.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://novo.avancoinfo.net/session/login";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		
		Image imageInfoVarejo = new Image(Social.class.getResourceAsStream("infovarejo.png"), 20, 20, false, false);
		ImageView ivInfoVarejo = new ImageView(imageInfoVarejo);
		ivInfoVarejo.getStyleClass().add("botao");
		pnlSocial2.getChildren().add(ivInfoVarejo);
		ivInfoVarejo.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				String url = "https://www.infovarejo.com.br/";
				try {
					new ProcessBuilder("x-www-browser", url).start();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		
		FlowPane.setMargin(ivSite, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(ivFace, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(ivLinkedin, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(ivConhecimento, new Insets(0, 4, 2, 4));
		FlowPane.setMargin(ivNovoAvanco, new Insets(0, 4, 2, 4));
		FlowPane.setMargin(ivInfoVarejo, new Insets(0, 4, 2, 4));
		
	}


}
