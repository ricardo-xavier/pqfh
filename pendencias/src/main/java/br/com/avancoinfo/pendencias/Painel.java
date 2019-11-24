package br.com.avancoinfo.pendencias;

import java.sql.Connection;
import java.util.List;

import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.binding.Bindings;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.FlowPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Painel extends Stage {
	
	@SuppressWarnings("unchecked")
	public Painel(Connection conn) {

        JFXTreeTableColumn<Pendencia, String> colData = new JFXTreeTableColumn<>("Data/Hora");
        colData.setPrefWidth(200);
        colData.setEditable(false);
        colData.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colData.validateValue(param)) {
                return param.getValue().getValue().getData();
            } else {
                return colData.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colCupom = new JFXTreeTableColumn<>("Cupom");
        colCupom.setPrefWidth(100);
        colCupom.setEditable(false);
        colCupom.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colCupom.validateValue(param)) {
                return param.getValue().getValue().getCupom();
            } else {
                return colCupom.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colNota = new JFXTreeTableColumn<>("Nota");
        colNota.setPrefWidth(100);
        colNota.setEditable(false);
        colNota.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colNota.validateValue(param)) {
                return param.getValue().getValue().getNota();
            } else {
                return colNota.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colSerie = new JFXTreeTableColumn<>("Série");
        colSerie.setPrefWidth(100);
        colSerie.setEditable(false);
        colSerie.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colSerie.validateValue(param)) {
                return param.getValue().getValue().getSerie();
            } else {
                return colSerie.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colTipo = new JFXTreeTableColumn<>("Tipo pendência");
        colTipo.setPrefWidth(200);
        colTipo.setEditable(false);
        colTipo.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colTipo.validateValue(param)) {
                return param.getValue().getValue().getTipo();
            } else {
                return colTipo.getComputedValue(param);
            }
        });

        /*
        colData.setCellFactory((TreeTableColumn<Pendencia, String> param) -> new GenericEditableTreeTableCell<>(
            new TextFieldEditorBuilder()));
        colData.setOnEditCommit((CellEditEvent<Pendencia, String> t) -> t.getTreeTableView()
                                                                      .getTreeItem(t.getTreeTablePosition()
                                                                                    .getRow())
                                                                      .getValue().getData().set(t.getNewValue()));

        colCupom.setCellFactory((TreeTableColumn<Pendencia, String> param) -> new GenericEditableTreeTableCell<>(
            new TextFieldEditorBuilder()));
        colCupom.setOnEditCommit((CellEditEvent<Pendencia, String> t) -> t.getTreeTableView()
                                                                      .getTreeItem(t.getTreeTablePosition()
                                                                                    .getRow())
                                                                      .getValue().getCupom().set(t.getNewValue()));

        colNota.setCellFactory((TreeTableColumn<Pendencia, String> param) -> new GenericEditableTreeTableCell<>(
            new TextFieldEditorBuilder()));
        colNota.setOnEditCommit((CellEditEvent<Pendencia, String> t) -> t.getTreeTableView()
                                                                       .getTreeItem(t.getTreeTablePosition()
                                                                                     .getRow())
                                                                       .getValue().getNota().set(t.getNewValue()));

        colSerie.setCellFactory((TreeTableColumn<Pendencia, String> param) -> new GenericEditableTreeTableCell<>(
            new TextFieldEditorBuilder()));
        colSerie.setOnEditCommit((CellEditEvent<Pendencia, String> t) -> t.getTreeTableView()
                                                                       .getTreeItem(t.getTreeTablePosition()
                                                                                     .getRow())
                                                                       .getValue().getSerie().set(t.getNewValue()));

        colTipo.setCellFactory((TreeTableColumn<Pendencia, String> param) -> new GenericEditableTreeTableCell<>(
            new TextFieldEditorBuilder()));
        colTipo.setOnEditCommit((CellEditEvent<Pendencia, String> t) -> t.getTreeTableView()
                                                                       .getTreeItem(t.getTreeTablePosition()
                                                                                     .getRow())
                                                                       .getValue().getTipo().set(t.getNewValue()));
		*/
        
        // data
        List<Pendencia> pendenciasBd = PendenciaDao.list(conn);
        ObservableList<Pendencia> pendencias = FXCollections.observableArrayList(pendenciasBd);

        // build tree
        final TreeItem<Pendencia> root = new RecursiveTreeItem<>(pendencias, RecursiveTreeObject::getChildren);

        JFXTreeTableView<Pendencia> treeView = new JFXTreeTableView<>(root);
        treeView.setShowRoot(false);
        treeView.setEditable(true);
        treeView.getColumns().setAll(colData, colCupom, colNota, colSerie, colTipo);
        
        treeView.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent mouseEvent) {
                if (mouseEvent.getButton().equals(MouseButton.PRIMARY)) {
                    if (mouseEvent.getClickCount() == 2) {
                    	TreeItem<Pendencia> item = treeView.getSelectionModel().getSelectedItem();
                    	Pendencia pendencia = item.getValue();
                    	Detalhes detalhes = PendenciaDao.getDetalhes(conn, pendencia.getChave().getValue());
                		PainelDetalhes painel = new PainelDetalhes(detalhes);
                		painel.show();
                		painel.setIconified(true);
                		painel.setIconified(false);	            	
                    }
                }
            }
        });
        
        /*
        treeView.getSelectionModel().selectedItemProperty().addListener((obs, oldSelection, newSelection) -> {
            if (newSelection != null) {
            }
        });
        */
        
        FlowPane main = new FlowPane();
        main.setPadding(new Insets(10));
        main.getChildren().add(treeView);
        
        JFXTextField filterField = new JFXTextField();
        main.getChildren().add(filterField);

        Label size = new Label();

        filterField.textProperty().addListener((o, oldVal, newVal) -> {
            treeView.setPredicate(userProp -> {
                final Pendencia pendencia = userProp.getValue();
                return pendencia.getData().get().contains(newVal)
                    || pendencia.getCupom().get().contains(newVal)
                    || pendencia.getNota().get().contains(newVal)
                    || pendencia.getSerie().get().contains(newVal)
                    || pendencia.getTipo().get().contains(newVal);
            });
        });

        size.textProperty()
            .bind(Bindings.createStringBinding(() -> String.valueOf(treeView.getCurrentItemsCount()),
                                               treeView.currentItemsCountProperty()));
        main.getChildren().add(size);
	
		setTitle("Painel de Pendências - NFC-e");
		Scene scene = new Scene(main, 700, 500);
		scene.getStylesheets().add(Painel.class.getResource("jfoenix-components.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
	}

}
