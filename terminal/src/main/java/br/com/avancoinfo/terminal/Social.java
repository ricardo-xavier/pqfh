package br.com.avancoinfo.terminal;

import java.io.IOException;

import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Tooltip;
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
		Button btSite = new Button("", ivSite);
		btSite.setTooltip(new Tooltip("Avanço Informática"));
		btSite.setPadding(new Insets(0));
		btSite.getStyleClass().add("social");
		pnlSocial1.getChildren().add(btSite);
		btSite.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		Button btFace = new Button("", ivFace);
		btFace.setTooltip(new Tooltip("Facebook"));
		btFace.setPadding(new Insets(0));		
		btFace.getStyleClass().add("social");
		pnlSocial1.getChildren().add(btFace);
		btFace.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		Button btLinkedin = new Button("", ivLinkedin);
		btLinkedin.setTooltip(new Tooltip("Linkedin"));
		btLinkedin.setPadding(new Insets(0));				
		btLinkedin.getStyleClass().add("social");
		pnlSocial1.getChildren().add(btLinkedin);
		btLinkedin.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		Button btConhecimento = new Button("", ivConhecimento);
		btConhecimento.setTooltip(new Tooltip("Base de Conhecimento"));
		btConhecimento.setPadding(new Insets(0));				
		btConhecimento.getStyleClass().add("social");
		pnlSocial2.getChildren().add(btConhecimento);
		btConhecimento.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		Button btNovoAvanco = new Button("", ivNovoAvanco);
		btNovoAvanco.setTooltip(new Tooltip("Novo Avanço"));
		btNovoAvanco.setPadding(new Insets(0));						
		btNovoAvanco.getStyleClass().add("social");
		pnlSocial2.getChildren().add(btNovoAvanco);
		btNovoAvanco.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		Button btInfoVarejo = new Button("", ivInfoVarejo);
		btInfoVarejo.setTooltip(new Tooltip("Info Varejo"));
		btInfoVarejo.setPadding(new Insets(0));								
		btInfoVarejo.getStyleClass().add("social");
		pnlSocial2.getChildren().add(btInfoVarejo);
		btInfoVarejo.setOnMouseClicked(new EventHandler<MouseEvent>() {

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
		
		FlowPane.setMargin(btSite, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(btFace, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(btLinkedin, new Insets(2, 4, 2, 4));
		FlowPane.setMargin(btConhecimento, new Insets(0, 4, 2, 4));
		FlowPane.setMargin(btNovoAvanco, new Insets(0, 4, 2, 4));
		FlowPane.setMargin(btInfoVarejo, new Insets(0, 4, 2, 4));
		
	}


}
