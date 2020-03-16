package fx;

import java.util.ArrayList;
import java.util.List;

import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.text.Text;
import javafx.scene.layout.FlowPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class ExplorerStage  extends Stage {

	private JFXTreeTableView<FileNode> treeView;
	private ObservableList<FileNode> files;
	
	public ExplorerStage() {
		
		List<JFXTreeTableColumn<FileNode, ?>> columns = new ArrayList<JFXTreeTableColumn<FileNode,?>>();

		final JFXTreeTableColumn<FileNode, String> nameColumn = new JFXTreeTableColumn<>("Arquivo");
		columns.add(nameColumn);
		nameColumn.setPrefWidth(440);
		nameColumn.setEditable(false);
		nameColumn.setCellValueFactory((TreeTableColumn.CellDataFeatures<FileNode, String> param) -> 
			param.getValue().getValue().getName()
		);

		final JFXTreeTableColumn<FileNode, String> dateColumn = new JFXTreeTableColumn<>("Data");
		columns.add(dateColumn);
		dateColumn.setPrefWidth(200);
		dateColumn.setEditable(false);
		dateColumn.setCellValueFactory((TreeTableColumn.CellDataFeatures<FileNode, String> param) ->
			param.getValue().getValue().getDate()
		);

		final JFXTreeTableColumn<FileNode, Number> sizeColumn = new JFXTreeTableColumn<>("Tamanho");
		columns.add(sizeColumn);
		sizeColumn.setPrefWidth(100);
		sizeColumn.setEditable(false);
		sizeColumn.setCellValueFactory((TreeTableColumn.CellDataFeatures<FileNode, Number> param) -> 
			param.getValue().getValue().getSize()
		);
		
		List<FileNode> demoFiles = new ArrayList<FileNode>(); 
		demoFiles.add(new FileNode("arquivo1", "2020-01-02 21:35", 1000));
		demoFiles.add(new FileNode("arquivo2", "2020-01-02 21:49", 2000));
		files = FXCollections.observableArrayList(demoFiles);
		
		System.err.println(files.size());

		final TreeItem<FileNode> root = new RecursiveTreeItem<>(files, RecursiveTreeObject::getChildren);

		treeView = new JFXTreeTableView<>(root);
		treeView.setShowRoot(false);
		treeView.setEditable(false);
		treeView.getColumns().setAll(columns);
		
		treeView.setOnKeyPressed(new EventHandler<KeyEvent>() {

			@Override
			public void handle(KeyEvent event) {
				switch (event.getCode()) {

				case ENTER:
					select();
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
						select();
					}
				}
			}
		});
		
		BorderPane main = new BorderPane();
		main.setCenter(treeView);
		
		GridPane topPanel = new GridPane();
		topPanel.setPadding(new Insets(10));
		topPanel.setHgap(10);
		topPanel.setVgap(10);		
		main.setTop(topPanel);
		
		Label lblDir = new Label("Diretório");
		topPanel.add(lblDir, 0, 0);
		
		JFXTextField txtDir = new JFXTextField();
		txtDir.setPrefColumnCount(25);
		txtDir.setFocusTraversable(false);
		topPanel.add(txtDir, 1, 0);

		Label lblPattern = new Label("Padrão");
		topPanel.add(lblPattern, 2, 0);
		
		JFXTextField txtPattern = new JFXTextField();
		txtPattern.setFocusTraversable(false);
		txtPattern.setPrefColumnCount(20);
		topPanel.add(txtPattern, 3, 0);
		
		JFXTextField filterField = new JFXTextField();
		filterField.setPromptText("Digite para filtrar");
		topPanel.add(filterField, 3, 2);
		
		filterField.textProperty().addListener((o, oldVal, newVal) -> {
			treeView.setPredicate(userProp -> {
				final FileNode file = userProp.getValue();
				return file.getName().get().contains(newVal); 
			});
		});
		
		FlowPane msgPanel = new FlowPane();
		msgPanel.setMinHeight(45);
		main.setBottom(msgPanel);
		
		msgPanel.getStyleClass().add("pnlMensagemComentario");

		Button btnComentario = new Button();
        btnComentario.setId("btnComentario");
        btnComentario.getStyleClass().add("imagemComentario");
        Text txtMensagem = new Text("ENTER ou duplo click para selecionar. ESC para sair.");
        
        msgPanel.getChildren().add(btnComentario);
        msgPanel.getChildren().add(txtMensagem);
		
		
		setTitle("Seleção de Arquivo");
		Scene scene = new Scene(main, 750, 500);
		scene.getStylesheets().add(ExplorerStage.class.getResource("linx.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		treeView.requestFocus();
		if (files.size() > 0) {
			Platform.runLater(() ->	treeView.getSelectionModel().select(0));
		}
		
	}
	
	private void select() {

		TreeItem<FileNode> item = treeView.getSelectionModel().getSelectedItem();
		System.out.println(item.getValue().getName());
	}
	
}
