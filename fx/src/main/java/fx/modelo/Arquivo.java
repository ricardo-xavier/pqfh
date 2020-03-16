package fx.modelo;

import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class Arquivo extends RecursiveTreeObject<Arquivo> {
	
	private StringProperty nome;
	private StringProperty data;
	private IntegerProperty tamanho;
	
	public Arquivo(String nome, String data, Integer tamanho) {
		super();
		this.nome = new SimpleStringProperty(nome);
		this.data = new SimpleStringProperty(data);
		this.tamanho = new SimpleIntegerProperty(tamanho);
	}
	
	public StringProperty nomeProperty() {
		return nome;
	}
	public StringProperty dataProperty() {
		return data;
	}
	public IntegerProperty tamanhoProperty() {
		return tamanho;
	}
	public String getNome() {
		return nome.getValue();
	}
	public void setNome(String nome) {
		this.nome.set(nome);
	}
	public String getData() {
		return data.getValue();
	}
	public void setData(String data) {
		this.data.set(data);
	}
	public Integer getTamanho() {
		return tamanho.get();
	}
	public void setTamanho(Integer tamanho) {
		this.tamanho.set(tamanho);
	}

}
