package br.com.avancoinfo.pendencias;

import java.io.IOException;
import java.sql.Connection;
import java.util.List;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.binding.Bindings;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Label;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class Painel extends Stage {
	
	private JFXTreeTableView<Pendencia> treeView;
	private Connection conn;
	private ObservableList<Pendencia> pendencias;
	
	@SuppressWarnings("unchecked")
	public Painel(Connection conn) {
		
		// tipo_pendencia, data_emissao, numero_cupom, numero_nota, serie, codigo_situacao, situacao, processada, cancelada, inutilizada, 

		this.conn = conn;
		
        JFXTreeTableColumn<Pendencia, String> colTipo = new JFXTreeTableColumn<>("Tipo");
        colTipo.setPrefWidth(50);
        colTipo.setEditable(false);
        colTipo.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colTipo.validateValue(param)) {
                return param.getValue().getValue().getTipo();
            } else {
                return colTipo.getComputedValue(param);
            }
        });
		
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
        colSerie.setPrefWidth(50);
        colSerie.setEditable(false);
        colSerie.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colSerie.validateValue(param)) {
                return param.getValue().getValue().getSerie();
            } else {
                return colSerie.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colSituacao = new JFXTreeTableColumn<>("Situação");
        colSituacao.setPrefWidth(100);
        colSituacao.setEditable(false);
        colSituacao.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colSituacao.validateValue(param)) {
                return param.getValue().getValue().getSituacao();
            } else {
                return colSituacao.getComputedValue(param);
            }
        });

        JFXTreeTableColumn<Pendencia, String> colStatus = new JFXTreeTableColumn<>("Status");
        colStatus.setPrefWidth(100);
        colStatus.setEditable(false);
        colStatus.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
            if (colStatus.validateValue(param)) {
                return param.getValue().getValue().getStatus();
            } else {
                return colStatus.getComputedValue(param);
            }
        });

        /*
        JFXTreeTableColumn<Pendencia, Boolean> colProcessada = new JFXTreeTableColumn<>("Processada");
        colProcessada.setPrefWidth(100);
        colProcessada.setEditable(false);
        colProcessada.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, Boolean> param) -> {
            if (colProcessada.validateValue(param)) {
                return param.getValue().getValue().getProcessada();
            } else {
                return colProcessada.getComputedValue(param);
            }
        });
        
        colProcessada.setCellFactory(new Callback<TreeTableColumn<Pendencia, Boolean>,TreeTableCell<Pendencia, Boolean>>() {
            @Override
            public TreeTableCell<Pendencia, Boolean> call( TreeTableColumn<Pendencia, Boolean> p ) {
                CheckBoxTreeTableCell<Pendencia, Boolean> cell = new CheckBoxTreeTableCell<Pendencia, Boolean>();
                cell.setAlignment(Pos.CENTER);
                return cell;
            }
        });
        */        

        List<Pendencia> pendenciasBd = PendenciaDao.list(conn);
        pendencias = FXCollections.observableArrayList(pendenciasBd);

        final TreeItem<Pendencia> root = new RecursiveTreeItem<>(pendencias, RecursiveTreeObject::getChildren);

        treeView = new JFXTreeTableView<>(root);
        treeView.setShowRoot(false);
        treeView.setEditable(true);
        treeView.getColumns().setAll(colTipo, colData, colCupom, colNota, colSerie, colSituacao, colStatus);
        
        treeView.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				switch (event.getCode()) {
				
				case F1:
					ajuda();
					break;
				
				case ENTER:
				case F2:
					detalhes();
					break;
					
				case F5:
					atualiza();
					break;
					
				case F6:
					executa();
					break;
					
				case ESCAPE:
					close();
					
				default:
					break;
				}
			}

		});
        
        treeView.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent mouseEvent) {
                if (mouseEvent.getButton().equals(MouseButton.PRIMARY)) {
                    if (mouseEvent.getClickCount() == 2) {
                    	detalhes();
                    }
                }
            }
        });
        
        BorderPane main = new BorderPane();
        main.setPadding(new Insets(10));
        main.setCenter(treeView);
        
        BorderPane pnlControles = new BorderPane();
        pnlControles.setPadding(new Insets(10));
        
        FlowPane pnlFiltro = new FlowPane();
        pnlFiltro.setPadding(new Insets(10));
        pnlFiltro.setMaxWidth(300);
        pnlControles.setLeft(pnlFiltro);
        
        FlowPane pnlBotoes = new FlowPane();
        pnlBotoes.setPadding(new Insets(20));
        pnlBotoes.setAlignment(Pos.CENTER_RIGHT);
        pnlControles.setRight(pnlBotoes);
        
        JFXTextField filterField = new JFXTextField();
        pnlFiltro.getChildren().add(filterField);

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
        pnlFiltro.getChildren().add(size);
        
        JFXButton btnAjuda = new JFXButton("F1-ajuda");
        pnlBotoes.getChildren().add(btnAjuda);
        btnAjuda.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				ajuda();
			}
		});        
        
        JFXButton btnDetalhes = new JFXButton("F2-detalhes");
        pnlBotoes.getChildren().add(btnDetalhes);
        btnDetalhes.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				detalhes();
			}
		});
        
        JFXButton btnRefresh = new JFXButton("F5-atualiza");
        pnlBotoes.getChildren().add(btnRefresh);
        btnRefresh.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
			}
		});
        
        
        JFXButton btnExecuta = new JFXButton("F6-executa");
        pnlBotoes.getChildren().add(btnExecuta);
        btnExecuta.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				executa();
			}
		});
        
        main.setBottom(pnlControles);
	
		setTitle("Painel de Pendências - NFC-e");
		Scene scene = new Scene(main, 750, 500);
		scene.getStylesheets().add(Painel.class.getResource("jfoenix-components.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		if (pendencias.size() > 0) {
			treeView.getSelectionModel().select(0);
		}
		
	}
	
	private void ajuda() {
		PainelAjuda painel = new PainelAjuda();
		painel.show();
	}
	
	private void detalhes() {
		
		TreeItem<Pendencia> item = treeView.getSelectionModel().getSelectedItem();
		if (item == null) {
			Alert alert = new Alert(AlertType.WARNING);
			alert.setTitle("Aviso");
			alert.setHeaderText("Nenhuma nota selecionada");
			alert.show();
			return;
		} 
		
       	Pendencia pendencia = item.getValue();
       	Detalhes detalhes = PendenciaDao.getDetalhes(conn, pendencia.getChave().getValue());
   		PainelDetalhes painel = new PainelDetalhes(detalhes);
   		painel.show();
   		painel.setIconified(true);
   		painel.setIconified(false);	            	
		
	}	
	
	private void atualiza() {
        List<Pendencia> pendenciasBd = PendenciaDao.list(conn);
        pendencias.clear();
        pendencias.addAll(pendenciasBd);
		if (pendencias.size() > 0) {
			treeView.getSelectionModel().select(0);
		}        
	}
	
	
	private void executa() {

		String comando = System.getenv("COMANDO_PENDENCIAS");
		if (comando == null) {
			Alert alert = new Alert(AlertType.WARNING);
			alert.setTitle("Aviso");
			alert.setHeaderText("Comando não definido");
			alert.show();
			return;
		} 
		
		TreeItem<Pendencia> item = treeView.getSelectionModel().getSelectedItem();
		if (item == null) {
			Alert alert = new Alert(AlertType.WARNING);
			alert.setTitle("Aviso");
			alert.setHeaderText("Nenhuma nota selecionada");
			alert.show();
			return;
		} 
		
		try {
			Runtime.getRuntime().exec(comando + " " + item.getValue().getChave().getValue());
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	

}
