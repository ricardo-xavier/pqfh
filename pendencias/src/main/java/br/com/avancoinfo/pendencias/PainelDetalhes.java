package br.com.avancoinfo.pendencias;

import java.io.StringReader;
import java.io.StringWriter;
import java.sql.Connection;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import com.jfoenix.controls.JFXTextField;

import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.GridPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class PainelDetalhes extends Stage {
	
	public PainelDetalhes(Connection conn, Detalhes detalhes, Date dataInclusao) {

		GridPane main = new GridPane();
        main.setPadding(new Insets(10));
        
        main.setVgap(5); 
        main.setHgap(25);
        
        Label lblChave = new Label("Chave de Acesso");
        main.add(lblChave, 0, 0);
        
        JFXTextField edtChaveAcesso = new JFXTextField();
        edtChaveAcesso.setMinWidth(400);
        edtChaveAcesso.setEditable(false);
        main.add(edtChaveAcesso, 1, 0);
        edtChaveAcesso.setText(detalhes.getChaveAcesso());
        
        Label lblSubstituta = new Label("NFCe Substituta");
        main.add(lblSubstituta, 0, 1);
        
        JFXTextField edtNfceSubstituta = new JFXTextField();
        edtNfceSubstituta.setMinWidth(400);
        edtNfceSubstituta.setEditable(false);
        main.add(edtNfceSubstituta, 1, 1);
        edtNfceSubstituta.setText(detalhes.getNfceSubstituta());

        Label lblXml = new Label("XML");
        main.add(lblXml, 0, 2);
        
        TextArea edtXml = new TextArea();
        edtXml.setPrefWidth(800);
        edtXml.setPrefRowCount(10);
        edtXml.setEditable(false);
        main.add(edtXml, 1, 2);
        edtXml.setText(prettyFormat(detalhes.getXml(), 2));

        Label lblSituacao = new Label("Situação");
        main.add(lblSituacao, 0, 3);
        
        TextArea edtSituacao = new TextArea();
        edtSituacao.setPrefWidth(800);
        edtSituacao.setPrefRowCount(10);
        edtSituacao.setEditable(false);
        main.add(edtSituacao, 1, 3);
        
        edtSituacao.setText(detalhes.getSituacao());

        DateFormat df = new SimpleDateFormat("dd/MM/yyyy HH:mm:ss");
        Label lblLogs = new Label("Logs " + df.format(dataInclusao));
        main.add(lblLogs, 0, 4);
        
        TextArea edtLogs = new TextArea();
        edtLogs.setPrefWidth(800);
        edtLogs.setPrefRowCount(10);
        edtLogs.setEditable(false);
        main.add(edtLogs, 1, 4);
        
        StringBuilder linhas = new StringBuilder();
        List<Log> logs = PendenciaDao.log(conn, dataInclusao);
        if (logs != null) {
        	for (Log log : logs) {
        		linhas.append(log.getDescricao() + "\r\n");
        	}
        }
        edtLogs.setText(linhas.toString());
        
		setTitle("Detalhes da Pendência");
		Scene scene = new Scene(main);
		scene.getStylesheets().add(PainelDetalhes.class.getResource("jfoenix-components.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		Stage janela = this;
		
		main.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				if (event.getCode() == KeyCode.ESCAPE) {
					janela.close();
				}
			}
		});
		
		
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
