package br.com.avancoinfo.terminal;

import org.controlsfx.control.StatusBar;

import com.jfoenix.controls.JFXTextField;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class SelecaoFilial extends Stage {
	
	private String filial;

	public SelecaoFilial(char[][] dados) {
		
		String s = new String(dados[14]);
		int p = s.indexOf("Filial:");
		String[] partes = s.substring(p+7).trim().split(" ");
		String filialInicial = partes[0];
		
		BorderPane pnlSelecao = new BorderPane();
		HBox pnlFilial = new HBox(30);
		pnlFilial.setAlignment(Pos.CENTER);
		StatusBar status = new StatusBar();
		status.setText("<99> Consolidado");

		pnlSelecao.setCenter(pnlFilial);
		pnlSelecao.setBottom(status);
		
        Label lblFilial = new Label("Filial");
        
        JFXTextField edtFilial = new JFXTextField(filialInicial);
        edtFilial.setMaxWidth(30);
        edtFilial.setOnAction(new EventHandler<ActionEvent>() {
			
			@Override
			public void handle(ActionEvent event) {
				filial = edtFilial.getText().trim();
				close();
			}
		});
        edtFilial.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				if (event.getCode() == KeyCode.ESCAPE) {
					filial = null;
					close();
				}
			}
		});
        edtFilial.setOnMouseClicked(new EventHandler<MouseEvent>() {

			@Override
			public void handle(MouseEvent event) {
				if (event.getClickCount() == 2) {
					filial = edtFilial.getText().trim();
					close();					
				}				
			}
		});
        
        pnlFilial.getChildren().add(lblFilial);
        pnlFilial.getChildren().add(edtFilial);
        
		Scene scene = new Scene(pnlSelecao, 250, 100);
		setTitle("Seleção de Filial");
		setScene(scene);
		initModality(Modality.APPLICATION_MODAL);
		
    }

	public String getFilial() {
		return filial;
	}

	public void setFilial(String filial) {
		this.filial = filial;
	}

}
