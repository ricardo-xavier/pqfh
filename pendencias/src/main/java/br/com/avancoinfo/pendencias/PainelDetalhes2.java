package br.com.avancoinfo.pendencias;

import java.io.StringReader;
import java.io.StringWriter;
import java.sql.Connection;
import java.util.Date;
import java.util.List;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.GridPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class PainelDetalhes2 extends Stage {
	
	private JFXTreeTableView<Log> treeView;
	private ObservableList<Log> logs;
	
	@SuppressWarnings("unchecked")
	public PainelDetalhes2(Connection conn, Detalhes detalhes, Date dataInclusao) {

		GridPane main = new GridPane();
        main.setPadding(new Insets(10));
        
        main.setVgap(5); 
        main.setHgap(5);
        
        Label lblChave = new Label("Chave de Acesso");
        main.add(lblChave, 0, 0);
        
        JFXTextField edtChaveAcesso = new JFXTextField();
        edtChaveAcesso.setPrefColumnCount(44);
        edtChaveAcesso.setEditable(false);
        main.add(edtChaveAcesso, 0, 1);
        edtChaveAcesso.setText(detalhes.getChaveAcesso());
        
        Label lblSubstituta = new Label("NFCe Substituta");
        main.add(lblSubstituta, 0, 2);
        
        JFXTextField edtNfceSubstituta = new JFXTextField();
        edtNfceSubstituta.setPrefColumnCount(44);
        edtNfceSubstituta.setEditable(false);
        main.add(edtNfceSubstituta, 0, 3);
        edtNfceSubstituta.setText(detalhes.getNfceSubstituta());

        Label lblXml = new Label("XML");
        main.add(lblXml, 0, 4);
        
        TextArea edtXml = new TextArea();
        edtXml.setPrefRowCount(10);
        edtXml.setEditable(false);
        main.add(edtXml, 0, 5);
        edtXml.setText(prettyFormat(detalhes.getXml(), 2));

        Label lobLogs = new Label("Logs");
        main.add(lobLogs, 0, 6);
        
        JFXTreeTableColumn<Log, String> colData = new JFXTreeTableColumn<>("Data/Hora");
        colData.setPrefWidth(150);
        colData.setEditable(false);
        colData.setCellValueFactory((TreeTableColumn.CellDataFeatures<Log, String> param) -> {
            if (colData.validateValue(param)) {
                return param.getValue().getValue().getData();
            } else {
                return colData.getComputedValue(param);
            }
        });
        
        JFXTreeTableColumn<Log, String> colDescricao = new JFXTreeTableColumn<>("Descrição");
        colDescricao.setPrefWidth(550);
        colDescricao.setEditable(false);
        colDescricao.setCellValueFactory((TreeTableColumn.CellDataFeatures<Log, String> param) -> {
            if (colDescricao.validateValue(param)) {
                return param.getValue().getValue().getDescricao();
            } else {
                return colDescricao.getComputedValue(param);
            }
        });
        
        List<Log> logsBd = PendenciaDao.log(conn, dataInclusao);
        logs = FXCollections.observableArrayList(logsBd);

        final TreeItem<Log> root = new RecursiveTreeItem<>(logs, RecursiveTreeObject::getChildren);

        treeView = new JFXTreeTableView<>(root);
        treeView.setMaxHeight(240);
        treeView.setShowRoot(false);
        treeView.setEditable(true);
        treeView.getColumns().setAll(colData, colDescricao);
        
        main.add(treeView, 0, 7);
        
		setTitle("Detalhes da Pendência");
		Scene scene = new Scene(main, 800, 600);
		scene.getStylesheets().add(PainelDetalhes2.class.getResource("jfoenix-components.css").toExternalForm());
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
