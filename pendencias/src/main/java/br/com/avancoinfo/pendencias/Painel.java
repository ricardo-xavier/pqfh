package br.com.avancoinfo.pendencias;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.sql.Connection;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.controlsfx.control.StatusBar;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXComboBox;
import com.jfoenix.controls.JFXDatePicker;
import com.jfoenix.controls.JFXRadioButton;
import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Label;
import javafx.scene.control.ToggleGroup;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableCell;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.control.cell.CheckBoxTreeTableCell;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.util.Callback;

public class Painel extends Stage {

	private JFXTreeTableView<Pendencia> treeView;
	private Connection conn;
	private ObservableList<Pendencia> pendencias;
	
	private JFXRadioButton rbProcessadasSim;
	private JFXRadioButton rbProcessadasNao;
	private JFXRadioButton rbProcessadasTodas;
	private JFXRadioButton rbCanceladasSim;
	private JFXRadioButton rbCanceladasNao;
	private JFXRadioButton rbCanceladasTodas;
	private JFXRadioButton rbInutilizadasSim;
	private JFXRadioButton rbInutilizadasNao;
	private JFXRadioButton rbInutilizadasTodas;
	private JFXRadioButton rbAutorizadasSim;
	private JFXRadioButton rbAutorizadasNao;
	private JFXRadioButton rbAutorizadasTodas;	
	private JFXDatePicker dtInicial;
	private JFXDatePicker dtFinal;
	private StatusBar statusBar;
	
	private int idx;
	private String ipAtual;

	private List<String> ips = new ArrayList<String>();			
	private List<String> cnpjs = new ArrayList<String>();			
	private List<String> nomes = new ArrayList<String>();
	
	public Painel(Connection conn) {
		
		ipAtual = BancoDados.getIp();
		
		try {
			BufferedReader reader = new BufferedReader(new FileReader(new File("anaipbd.cfg")));
			String linha;
			while ((linha = reader.readLine()) != null) {
				String[] partes = linha.split("\\|");
				String ip = partes[0];
				ips.add(ip);
				String cnpj = partes[1];
				cnpjs.add(cnpj);
				String nome = partes[2];
				nomes.add(nome);
			}
			reader.close();
			
		} catch (IOException e) {
		}

		this.conn = conn;

		List<JFXTreeTableColumn<Pendencia, ?>> colunas = criaColunas();

		List<Pendencia> pendenciasBd = PendenciaDao.list(conn, 
				false, false, true, 
				true, false, false, 
				true, false, false,
				true, false, false,
				null, null, null);
		pendencias = FXCollections.observableArrayList(pendenciasBd);

		final TreeItem<Pendencia> root = new RecursiveTreeItem<>(pendencias, RecursiveTreeObject::getChildren);

		treeView = new JFXTreeTableView<>(root);
		treeView.setShowRoot(false);
		treeView.setEditable(true);
		treeView.getColumns().setAll(colunas);

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

		GridPane pnlFiltro = new GridPane();
		pnlFiltro.setHgap(40);
		pnlFiltro.setVgap(10);
//		pnlFiltro.setMaxWidth(900);
		pnlControles.setLeft(pnlFiltro);

		GridPane pnlBotoes = new GridPane();
		pnlBotoes.setVgap(10);		
		pnlControles.setRight(pnlBotoes);
		
		Label lblFiltros = new Label("Filtros");
		pnlFiltro.add(lblFiltros, 0, 0);

		Label lblProcessadas = new Label("Processadas");
		pnlFiltro.add(lblProcessadas, 1, 0);
		
		ToggleGroup groupProcessadas = new ToggleGroup();

		rbProcessadasTodas = new JFXRadioButton("Todas");
		rbProcessadasTodas.setSelected(false);
		pnlFiltro.add(rbProcessadasTodas, 1, 1);
		rbProcessadasTodas.setToggleGroup(groupProcessadas);
		rbProcessadasTodas.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbProcessadasSim = new JFXRadioButton("Sim");
		rbProcessadasSim.setSelected(false);
		pnlFiltro.add(rbProcessadasSim, 1, 2);
		rbProcessadasSim.setToggleGroup(groupProcessadas);
		rbProcessadasSim.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbProcessadasNao = new JFXRadioButton("Não");
		rbProcessadasNao.setSelected(true);
		pnlFiltro.add(rbProcessadasNao, 1, 3);
		rbProcessadasNao.setToggleGroup(groupProcessadas);
		rbProcessadasNao.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		Label lblCanceladas = new Label("Canceladas");
		pnlFiltro.add(lblCanceladas, 2, 0);
		
		ToggleGroup groupCanceladas = new ToggleGroup();

		rbCanceladasTodas = new JFXRadioButton("Todas");
		rbCanceladasTodas.setSelected(true);
		pnlFiltro.add(rbCanceladasTodas, 2, 1);
		rbCanceladasTodas.setToggleGroup(groupCanceladas);
		rbCanceladasTodas.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbCanceladasSim = new JFXRadioButton("Sim");
		rbCanceladasSim.setSelected(false);
		pnlFiltro.add(rbCanceladasSim, 2, 2);
		rbCanceladasSim.setToggleGroup(groupCanceladas);
		rbCanceladasSim.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbCanceladasNao = new JFXRadioButton("Não");
		rbCanceladasNao.setSelected(false);
		pnlFiltro.add(rbCanceladasNao, 2, 3);
		rbCanceladasNao.setToggleGroup(groupCanceladas);
		rbCanceladasNao.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		Label lblInutilizadas = new Label("Inutilizadas");
		pnlFiltro.add(lblInutilizadas, 3, 0);
		
		ToggleGroup groupInutilizadas = new ToggleGroup();

		rbInutilizadasTodas = new JFXRadioButton("Todas");
		rbInutilizadasTodas.setSelected(true);
		pnlFiltro.add(rbInutilizadasTodas, 3, 1);
		rbInutilizadasTodas.setToggleGroup(groupInutilizadas);
		rbInutilizadasTodas.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbInutilizadasSim = new JFXRadioButton("Sim");
		rbInutilizadasSim.setSelected(false);
		pnlFiltro.add(rbInutilizadasSim, 3, 2);
		rbInutilizadasSim.setToggleGroup(groupInutilizadas);
		rbInutilizadasSim.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbInutilizadasNao = new JFXRadioButton("Não");
		rbInutilizadasNao.setSelected(false);
		pnlFiltro.add(rbInutilizadasNao, 3, 3);
		rbInutilizadasNao.setToggleGroup(groupInutilizadas);
		rbInutilizadasNao.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		Label lblAutorizadas = new Label("Autorizadas");
		pnlFiltro.add(lblAutorizadas, 4, 0);
		
		ToggleGroup groupAutorizadas = new ToggleGroup();

		rbAutorizadasTodas = new JFXRadioButton("Todas");
		rbAutorizadasTodas.setSelected(true);
		pnlFiltro.add(rbAutorizadasTodas, 4, 1);
		rbAutorizadasTodas.setToggleGroup(groupAutorizadas);
		rbAutorizadasTodas.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbAutorizadasSim = new JFXRadioButton("Sim");
		rbAutorizadasSim.setSelected(false);
		pnlFiltro.add(rbAutorizadasSim, 4, 2);
		rbAutorizadasSim.setToggleGroup(groupAutorizadas);
		rbAutorizadasSim.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		rbAutorizadasNao = new JFXRadioButton("Não");
		rbAutorizadasNao.setSelected(false);
		pnlFiltro.add(rbAutorizadasNao, 4, 3);
		rbAutorizadasNao.setToggleGroup(groupAutorizadas);
		rbAutorizadasNao.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
				
			}
		});
		
		Label lblEmissao = new Label("Emissão entre");
		pnlFiltro.add(lblEmissao, 5, 0);
		
		dtInicial = new JFXDatePicker();
		dtInicial.setMaxWidth(140);
		dtInicial.setValue(LocalDate.of(1900, 1, 1));
		pnlFiltro.add(dtInicial, 5, 1);
		dtInicial.valueProperty().addListener((ov, oldValue, newValue) -> {
			atualiza();
		});	
		
		dtFinal = new JFXDatePicker();
		dtFinal.setMaxWidth(140);
		Calendar hoje = Calendar.getInstance();
		dtFinal.setValue(LocalDate.of(hoje.get(Calendar.YEAR), hoje.get(Calendar.MONTH)+1, hoje.get(Calendar.DAY_OF_MONTH)));
		pnlFiltro.add(dtFinal, 5, 2);
		dtFinal.valueProperty().addListener((ov, oldValue, newValue) -> {
			atualiza();
		});	

		JFXTextField filterField = new JFXTextField();
		filterField.setPromptText("Digite para filtrar");
		pnlFiltro.add(filterField, 6, 0);
		
		filterField.textProperty().addListener((o, oldVal, newVal) -> {
			treeView.setPredicate(userProp -> {
				final Pendencia pendencia = userProp.getValue();
				return pendencia.getData().get().contains(newVal) 
						|| pendencia.getCupom().get().contains(newVal)
						|| pendencia.getNota().get().contains(newVal) 
						|| pendencia.getSerie().get().contains(newVal)
						|| pendencia.getTipo().get().contains(newVal)
						|| pendencia.getSituacao().get().contains(newVal)
						|| pendencia.getDescricao().get().contains(newVal);						
			});
		});		
		
		ObservableList<String> empresas = FXCollections.observableList(nomes);
		JFXComboBox<String> cbxEmpresas = new JFXComboBox<String>(empresas );
		cbxEmpresas.setPromptText("Selecione a empresa");
		cbxEmpresas.getSelectionModel()
        .selectedItemProperty()
        .addListener((obs, oldValue, newValue) -> {
        	int idxAnterior = idx;
        	idx = nomes.indexOf(newValue);
        	String ip = ips.get(idx);
        	if (idx != idxAnterior) {
    			statusBar.getLeftItems().clear();
    			statusBar.getLeftItems().add(new Label("CNPJ: " + cnpjs.get(idx) + "   Loja: " + nomes.get(idx)));
    			statusBar.getRightItems().clear();
    			statusBar.getRightItems().add(new Label("IP: " + ipAtual));
        	}
        	if (!ip.equals(ipAtual)) {
        		ipAtual = ip;
       			Painel context = this;
       			Task<Void> task = new Task<Void>() {

					@Override
					protected Void call() throws Exception {
						System.err.println(new Date());
						updateProgress(-1, 1);
						try {
							context.conn = BancoDados.reconecta(conn, ipAtual);
							atualiza();
						} catch (Exception e) {
							
							updateProgress(0, 0);
							Platform.runLater(new Runnable() {
								@Override
								public void run() {
									Alert alert = new Alert(AlertType.ERROR);
									alert.setTitle("Erro");
									alert.setHeaderText("Erro na conexão ao banco de dados");
									alert.setContentText(e.getMessage());
									alert.show();
								}
							});
							
							updateProgress(-1, 1);
							idx = idxAnterior;
							String ip = ips.get(idx);
							ipAtual = ip;
							try {
								context.conn = BancoDados.reconecta(conn, ipAtual);
							} catch (Exception e2) {
								e2.printStackTrace();
								updateProgress(0, 0);
								Platform.runLater(new Runnable() {
									@Override
									public void run() {
										Alert alert = new Alert(AlertType.ERROR);
										alert.setTitle("Erro");
										alert.setHeaderText("Erro na conexão ao banco de dados");
										alert.setContentText(e2.getMessage());
										alert.show();
									}
								});
								return null;
							}
							
							updateProgress(0, 0);
							Platform.runLater(new Runnable() {
								@Override
								public void run() {
					    			statusBar.getLeftItems().clear();
					    			statusBar.getLeftItems().add(new Label("CNPJ: " + cnpjs.get(idx) + "   Loja: " + nomes.get(idx)));
					    			statusBar.getRightItems().clear();
					    			statusBar.getRightItems().add(new Label("IP: " + ipAtual));
								}
							});
						}
						
						updateProgress(0, 0);
						return null;
					}
				};
				statusBar.progressProperty().bind(task.progressProperty());
				Thread thread = new Thread(task, "task-thread");
		        thread.setDaemon(true);
		        thread.start();				
        	} else if (idx != idxAnterior) {
        		atualiza();
        	}
        });
		pnlFiltro.add(cbxEmpresas, 6, 2);
		
		dtInicial = new JFXDatePicker();
		dtInicial.setMaxWidth(140);
		dtInicial.setValue(LocalDate.of(1900, 1, 1));
		pnlFiltro.add(dtInicial, 5, 1);
		dtInicial.valueProperty().addListener((ov, oldValue, newValue) -> {
			atualiza();
		});	
		
		/*

		JFXTextField filterField = new JFXTextField();
		pnlFiltro.getChildren().add(filterField);
		Label size = new Label();
		size.textProperty().bind(Bindings.createStringBinding(() -> String.valueOf(treeView.getCurrentItemsCount()),
				treeView.currentItemsCountProperty()));
		*/

		JFXButton btnAjuda = new JFXButton("F1-ajuda");
		pnlBotoes.add(btnAjuda, 0, 0);
		btnAjuda.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				ajuda();
			}
		});

		JFXButton btnDetalhes = new JFXButton("F2-detalhes");
		pnlBotoes.add(btnDetalhes, 0, 1);
		btnDetalhes.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				detalhes();
			}
		});

		JFXButton btnRefresh = new JFXButton("F5-atualiza");
		pnlBotoes.add(btnRefresh, 0, 2);		
		btnRefresh.setOnAction(new EventHandler<ActionEvent>() {

			@Override
			public void handle(ActionEvent event) {
				atualiza();
			}
		});
		
		statusBar = new StatusBar();
		statusBar.setText(null);
		statusBar.setProgress(0);
		if (idx < cnpjs.size()) {
			statusBar.getLeftItems().add(new Label("CNPJ: " + cnpjs.get(idx) + "   Loja: " + nomes.get(idx)));
			statusBar.getRightItems().add(new Label("IP: " + ipAtual));
		}
		pnlControles.setBottom(statusBar);

		main.setBottom(pnlControles);

		setTitle("Painel de Pendências - NFC-e");
		Scene scene = new Scene(main, 750, 500);
		scene.getStylesheets().add(Painel.class.getResource("jfoenix-components.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);

		treeView.requestFocus();
		if (pendencias.size() > 0) {
			treeView.getSelectionModel().select(0);
		}

	}

	private List<JFXTreeTableColumn<Pendencia, ?>> criaColunas() {

		List<JFXTreeTableColumn<Pendencia, ?>> colunas = new ArrayList<JFXTreeTableColumn<Pendencia, ?>>();

		JFXTreeTableColumn<Pendencia, String> colTipo = new JFXTreeTableColumn<>("Tipo");
		colunas.add(colTipo);
		colTipo.setPrefWidth(150);
		colTipo.setEditable(false);
		colTipo.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
			if (colTipo.validateValue(param)) {
				return param.getValue().getValue().getTipo();
			} else {
				return colTipo.getComputedValue(param);
			}
		});

		JFXTreeTableColumn<Pendencia, String> colData = new JFXTreeTableColumn<>("Data/Hora");
		colunas.add(colData);
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
		colunas.add(colCupom);
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
		colunas.add(colNota);
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
		colunas.add(colSerie);
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
		colunas.add(colSituacao);
		colSituacao.setPrefWidth(100);
		colSituacao.setEditable(false);
		colSituacao.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
			if (colSituacao.validateValue(param)) {
				return param.getValue().getValue().getSituacao();
			} else {
				return colSituacao.getComputedValue(param);
			}
		});

		JFXTreeTableColumn<Pendencia, String> colDescricao = new JFXTreeTableColumn<>("Descrição");
		colunas.add(colDescricao);
		colDescricao.setPrefWidth(200);
		colDescricao.setEditable(false);
		colDescricao.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, String> param) -> {
			if (colDescricao.validateValue(param)) {
				return param.getValue().getValue().getDescricao();
			} else {
				return colDescricao.getComputedValue(param);
			}
		});

		JFXTreeTableColumn<Pendencia, Boolean> colProcessada = new JFXTreeTableColumn<>("Processada");
		colunas.add(colProcessada);
		colProcessada.setPrefWidth(100);
		colProcessada.setEditable(false);
		colProcessada.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, Boolean> param) -> {
			if (colProcessada.validateValue(param)) {
				return param.getValue().getValue().getProcessada();
			} else {
				return colProcessada.getComputedValue(param);
			}
		});

		colProcessada
				.setCellFactory(new Callback<TreeTableColumn<Pendencia, Boolean>, TreeTableCell<Pendencia, Boolean>>() {
					@Override
					public TreeTableCell<Pendencia, Boolean> call(TreeTableColumn<Pendencia, Boolean> p) {
						CheckBoxTreeTableCell<Pendencia, Boolean> cell = new CheckBoxTreeTableCell<Pendencia, Boolean>();
						cell.setAlignment(Pos.CENTER);
						return cell;
					}
				});

		JFXTreeTableColumn<Pendencia, Boolean> colCancelada = new JFXTreeTableColumn<>("Cancelada");
		colunas.add(colCancelada);
		colCancelada.setPrefWidth(100);
		colCancelada.setEditable(false);
		colCancelada.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, Boolean> param) -> {
			if (colCancelada.validateValue(param)) {
				return param.getValue().getValue().getCancelada();
			} else {
				return colCancelada.getComputedValue(param);
			}
		});

		colCancelada
				.setCellFactory(new Callback<TreeTableColumn<Pendencia, Boolean>, TreeTableCell<Pendencia, Boolean>>() {
					@Override
					public TreeTableCell<Pendencia, Boolean> call(TreeTableColumn<Pendencia, Boolean> p) {
						CheckBoxTreeTableCell<Pendencia, Boolean> cell = new CheckBoxTreeTableCell<Pendencia, Boolean>();
						cell.setAlignment(Pos.CENTER);
						return cell;
					}
				});

		JFXTreeTableColumn<Pendencia, Boolean> colInutilizada = new JFXTreeTableColumn<>("Inutilizada");
		colunas.add(colInutilizada);
		colInutilizada.setPrefWidth(100);
		colInutilizada.setEditable(false);
		colInutilizada.setCellValueFactory((TreeTableColumn.CellDataFeatures<Pendencia, Boolean> param) -> {
			if (colInutilizada.validateValue(param)) {
				return param.getValue().getValue().getInutilizada();
			} else {
				return colInutilizada.getComputedValue(param);
			}
		});

		colInutilizada
				.setCellFactory(new Callback<TreeTableColumn<Pendencia, Boolean>, TreeTableCell<Pendencia, Boolean>>() {
					@Override
					public TreeTableCell<Pendencia, Boolean> call(TreeTableColumn<Pendencia, Boolean> p) {
						CheckBoxTreeTableCell<Pendencia, Boolean> cell = new CheckBoxTreeTableCell<Pendencia, Boolean>();
						cell.setAlignment(Pos.CENTER);
						return cell;
					}
				});

		return colunas;
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
		PainelDetalhes painel = new PainelDetalhes(conn, detalhes, pendencia.getDataInclusao());
		painel.setMaximized(true);
		painel.show();
		painel.setIconified(true);
		painel.setIconified(false);

	}

	private void atualiza() {
		List<Pendencia> pendenciasBd = PendenciaDao.list(conn, 
				rbProcessadasTodas.isSelected(), rbProcessadasSim.isSelected(), rbProcessadasNao.isSelected(),
				rbCanceladasTodas.isSelected(), rbCanceladasSim.isSelected(), rbCanceladasNao.isSelected(),
				rbInutilizadasTodas.isSelected(), rbInutilizadasSim.isSelected(), rbInutilizadasNao.isSelected(),
				rbAutorizadasTodas.isSelected(), rbAutorizadasSim.isSelected(), rbAutorizadasNao.isSelected(),
				dtInicial.getValue(), dtFinal.getValue(), idx < cnpjs.size() ? cnpjs.get(idx) : null);
		pendencias.clear();
		pendencias.addAll(pendenciasBd);
		if (pendencias.size() > 0) {
			treeView.getSelectionModel().select(0);
		}
	}

}
