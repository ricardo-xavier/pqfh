package br.com.avancoinfo.pendencias;

import java.io.StringReader;
import java.io.StringWriter;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import com.jfoenix.controls.JFXTextField;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.layout.GridPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class PainelDetalhes extends Stage {
	
	public PainelDetalhes(Detalhes detalhes) {

		GridPane main = new GridPane();
        main.setPadding(new Insets(10));
        
        main.setVgap(5); 
        main.setHgap(5);
        
        Label lblChave = new Label("Chave de Acesso");
        main.add(lblChave, 0, 0);
        
        JFXTextField edtChaveAcesso = new JFXTextField();
        edtChaveAcesso.setPrefColumnCount(44);
        main.add(edtChaveAcesso, 0, 1);
        edtChaveAcesso.setText(detalhes.getChaveAcesso());
        
        Label lblSubstituta = new Label("NFCe Substituta");
        main.add(lblSubstituta, 0, 2);
        
        JFXTextField edtNfceSubstituta = new JFXTextField();
        edtNfceSubstituta.setPrefColumnCount(44);
        main.add(edtNfceSubstituta, 0, 3);
        edtNfceSubstituta.setText(detalhes.getNfceSubstituta());

        Label lblXml = new Label("XML");
        main.add(lblXml, 0, 4);
        
        TextArea edtXml = new TextArea();
        edtXml.setPrefRowCount(10);
        main.add(edtXml, 0, 5);
        edtXml.setText(prettyFormat(detalhes.getXml(), 2));
        
		setTitle("Detalhes da Pendência");
		Scene scene = new Scene(main, 500, 500);
		scene.getStylesheets().add(PainelDetalhes.class.getResource("jfoenix-components.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
	}
	
	public static String prettyFormat(String input, int indent) {
	    try {
	    	if (input == null) {
	    		return "";
	    	}
	        Source xmlInput = new StreamSource(new StringReader("<root>" + input + "</root>"));
	        StringWriter stringWriter = new StringWriter();
	        StreamResult xmlOutput = new StreamResult(stringWriter);
	        TransformerFactory transformerFactory = TransformerFactory.newInstance();
	        transformerFactory.setAttribute("indent-number", indent);
	        Transformer transformer = transformerFactory.newTransformer(); 
	        transformer.setOutputProperty(OutputKeys.INDENT, "yes");
	        transformer.transform(xmlInput, xmlOutput);
	        return xmlOutput.getWriter().toString().replace("<root>", "").replace("</root>", "");
	    } catch (Exception e) {
	    	return "";
	    }
	}

}
