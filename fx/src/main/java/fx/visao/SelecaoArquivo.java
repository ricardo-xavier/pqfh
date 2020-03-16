package fx.visao;

import java.util.ArrayList;
import java.util.List;

import com.jfoenix.controls.JFXTextField;
import com.jfoenix.controls.JFXTreeTableColumn;
import com.jfoenix.controls.JFXTreeTableView;
import com.jfoenix.controls.RecursiveTreeItem;
import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import fx.cliente.SelecaoArquivoCliente;
import fx.modelo.Arquivo;
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
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.GridPane;
import javafx.scene.text.Text;
import javafx.stage.Modality;
import javafx.stage.Stage;

/**
 * Tela para seleção de arquivo
 * 
 * @author ricardo
 *
 */
public class SelecaoArquivo extends Stage {
	
	private JFXTreeTableView<Arquivo> tabArquivos;
	private ObservableList<Arquivo> arquivos;
	private Arquivo arquivo;
	
	public SelecaoArquivo(String diretorio, String padrao) {

		carregaArquivos(diretorio, padrao);		
		defineTabela();
		
		// monta a tela

		// cria o painel principal
		BorderPane pnlPrincipal = new BorderPane();
		
		// cria o painel do cabeçalho
		// (0,0) Diretório   (1,0) diretório   (2,0) Padrão   (3,0) padrão
		//                                                    (3,2) filtro
		GridPane pnlCabec = new GridPane();
		pnlCabec.setPadding(new Insets(10));
		pnlCabec.setHgap(10);
		pnlCabec.setVgap(10);		
		pnlPrincipal.setTop(pnlCabec);
		
		Label lblDir = new Label("Diretório");
		pnlCabec.add(lblDir, 0, 0);
		
		JFXTextField edtDir = new JFXTextField();
		edtDir.setPrefColumnCount(25);
		edtDir.setFocusTraversable(false);
		pnlCabec.add(edtDir, 1, 0);
		edtDir.setText(diretorio);

		Label lblPadrao = new Label("Padrão");
		pnlCabec.add(lblPadrao, 2, 0);
		
		JFXTextField edtPadrao = new JFXTextField();
		edtPadrao.setFocusTraversable(false);
		edtPadrao.setPrefColumnCount(20);
		pnlCabec.add(edtPadrao, 3, 0);
		edtPadrao.setText(padrao);
		
		JFXTextField edtFiltro = new JFXTextField();
		edtFiltro.setPromptText("Digite para filtrar");
		pnlCabec.add(edtFiltro, 3, 2);
		
		// filtra
		edtFiltro.textProperty().addListener((o, oldVal, newVal) -> {
			tabArquivos.setPredicate(userProp -> {
				final Arquivo arquivo = userProp.getValue();
				return arquivo.getNome().contains(newVal) || arquivo.getData().contains(newVal); 
			});
		});
		
		// coloca a tabela no centro da tela 
		pnlPrincipal.setCenter(tabArquivos);
		
		// cria o painel de mensagens
		FlowPane pnlMensagem = new FlowPane();
		pnlMensagem.setMinHeight(45);
		pnlPrincipal.setBottom(pnlMensagem);
		
		pnlPrincipal.getStyleClass().add("pnlMensagemComentario");

		// imagem da mensagem
		Button btnMensagem = new Button();
		btnMensagem.setId("btnComentario");
		btnMensagem.getStyleClass().add("imagemComentario");
        
        // texto da mensagem
        Text txtMensagem = new Text("ENTER ou duplo click para selecionar. ESC para cancelar.");
        
        pnlMensagem.getChildren().add(btnMensagem);
        pnlMensagem.getChildren().add(txtMensagem);
        
        // cria a cena
		setTitle("Seleção de Arquivo");
		Scene scene = new Scene(pnlPrincipal, 900, 500);
		scene.getStylesheets().add(SelecaoArquivo.class.getResource("linx.css").toExternalForm());
		setScene(scene);
		initModality(Modality.WINDOW_MODAL);
		
		// seta o foco
		tabArquivos.requestFocus();
		if (arquivos.size() > 0) {
			Platform.runLater(() ->	tabArquivos.getSelectionModel().select(0));
		}
		
	}
	
	private void seleciona() {
		TreeItem<Arquivo> item = tabArquivos.getSelectionModel().getSelectedItem();
		System.out.println(item.getValue());
	}

	private void carregaArquivos(String diretorio, String padrao) {
		List<Arquivo> arquivosServidor = SelecaoArquivoCliente.carrega(diretorio, padrao); 
		arquivos = FXCollections.observableArrayList(arquivosServidor);
	}
	
	private void defineTabela() {
		
		List<JFXTreeTableColumn<Arquivo, ?>> colunas = new ArrayList<JFXTreeTableColumn<Arquivo,?>>();

		// define a coluna Arquivo
		final JFXTreeTableColumn<Arquivo, String> colNome = new JFXTreeTableColumn<>("Arquivo");
		colNome.setPrefWidth(440);
		colNome.setEditable(false);
		colNome.setCellValueFactory((TreeTableColumn.CellDataFeatures<Arquivo, String> param) -> 
			param.getValue().getValue().nomeProperty()
		);
		colunas.add(colNome);

		// define a coluna Data
		final JFXTreeTableColumn<Arquivo, String> colData = new JFXTreeTableColumn<>("Data");
		colData.setPrefWidth(200);
		colData.setEditable(false);
		colData.setCellValueFactory((TreeTableColumn.CellDataFeatures<Arquivo, String> param) -> 
			param.getValue().getValue().dataProperty()
		);
		colunas.add(colData);
		
		// define a coluna Tamanho
		final JFXTreeTableColumn<Arquivo, Number> colTamanho = new JFXTreeTableColumn<>("Tamanho");
		colTamanho.setPrefWidth(100);
		colTamanho.setEditable(false);
		colTamanho.setCellValueFactory((TreeTableColumn.CellDataFeatures<Arquivo, Number> param) -> 
			param.getValue().getValue().tamanhoProperty()
		);
		colunas.add(colTamanho);

		// define a tabela
		final TreeItem<Arquivo> raiz = new RecursiveTreeItem<>(arquivos, RecursiveTreeObject::getChildren);
		tabArquivos = new JFXTreeTableView<>(raiz);
		tabArquivos.setShowRoot(false);
		tabArquivos.setEditable(false);
		tabArquivos.getColumns().setAll(colunas);
		
		// seleciona arquivo com duplo click
		tabArquivos.setOnMouseClicked(new EventHandler<MouseEvent>() {
			@Override
			public void handle(MouseEvent mouseEvent) {
				if (mouseEvent.getButton().equals(MouseButton.PRIMARY)) {
					if (mouseEvent.getClickCount() == 2) {
						seleciona();
						close();
					}
				}
			}
		});
		
		tabArquivos.setOnKeyPressed(new EventHandler<KeyEvent>() {
			@Override
			public void handle(KeyEvent keyEvent) {

				// seleciona arquivo com ENTER
				if (keyEvent.getCode() == KeyCode.ENTER) {
					seleciona();
					close();
				}
				
				// retorna sem selecionar com ESC
				if (keyEvent.getCode() == KeyCode.ESCAPE) {
					arquivo = null;
					close();
				}
				
			}
		});
		
		
	}

	public Arquivo getArquivo() {
		return arquivo;
	}

	public void setArquivo(Arquivo arquivo) {
		this.arquivo = arquivo;
	}

}
